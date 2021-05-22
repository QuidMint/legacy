#include "microauction.hpp"

void microauction::init(settings setting){
  require_auth(_self);
  settings_t settings_table(_self, _self.value);
  settings_table.set(setting, _self);

  payments_t payments_table(_self, _self.value);
  while (payments_table.begin() != payments_table.end()) {
  auto itr = payments_table.end();
  itr--;
  payments_table.erase(itr);
  }

  cycles_t cycles_table(_self, _self.value);
  while (cycles_table.begin() != cycles_table.end()) {
  auto itr = cycles_table.end();
  itr--;
  cycles_table.erase(itr);
  }
  
}

void microauction::setlimit(asset max_per_cycle){
  require_auth(_self);
  limit_t limit_table(_self, _self.value);
  limit new_limit;
  new_limit.max_per_cycle = max_per_cycle;
  limit_table.set(new_limit, _self);
}


// aggregate payments for a user
void microauction::claim(name payer){
  send_tokens_for_user(payer, true);
}


void microauction::send_tokens_for_user(name payer, bool do_asserts) {
  payments_t payments_table(_self, _self.value);
  settings_t settings_table(_self, _self.value);
  auto current_settings = settings_table.get();
  int64_t current_cycle = getCurrentCycle(current_settings);
  cycles_t cycles_table(_self, _self.value);

  double total_payouts = 0;
  uint16_t count = current_settings.payout_cycles_per_user;
  eosio::check(count > 0, "payout_cycles_per_user cannot be zero");
  bool found = false;
  vector<uint64_t> processed_cycles;

  auto payidx = payments_table.get_index<"ordbyuser"_n>();
  auto payitr = payidx.lower_bound(ordbyuser(0, payer));
  while( count-- > 0 && payitr != payidx.end() && payitr->account == payer && payitr->cycle_number < current_cycle && payitr->vested_date.sec_since_epoch() < current_time_point().sec_since_epoch()) {
    auto cycle_entry = cycles_table.find(payitr->cycle_number);
    eosio::check(cycle_entry != cycles_table.end(), "Cannot find cycle by number");
    found = true;
    processed_cycles.emplace_back(payitr->cycle_number);

    // our_payin * total_payout / total_payins
    double payout =
      (double)(payitr->quantity.amount * current_settings.quota_per_cycle.quantity.amount) /
      cycle_entry->total_payins.amount;
    total_payouts += payout;
    payitr = payidx.erase(payitr);
  }

  if( do_asserts ) {
    eosio::check(found, "There is nothing to claim for this account");
  }

  extended_asset payout;
  payout.contract = current_settings.quota_per_cycle.contract;
  payout.quantity.amount = total_payouts;
  payout.quantity.symbol = current_settings.quota_per_cycle.quantity.symbol;

  if( payout.quantity.amount > 0 ) {
    transferToken(payer, payout);
  }

  action {
    permission_level{_self, "active"_n},
    _self,
    "receipt"_n,
    receipt_abi {.payer=payer, .cycles=processed_cycles, .payout=payout}
  }.send();
}



// Send up to this many transfers. Anyone can trigger this action.
void microauction::sendtokens(uint16_t count) {
  payments_t payments_table(_self, _self.value);
  settings_t settings_table(_self, _self.value);
  auto current_settings = settings_table.get();
  uint64_t current_cycle = getCurrentCycle(current_settings);
  cycles_t cycles_table(_self, _self.value);

  auto payidx = payments_table.get_index<"ordbycycle"_n>();
  auto payitr = payidx.begin();
  while( payitr->account != _self && count-- > 0 && payitr != payidx.end() && payitr->cycle_number < current_cycle ) {
    send_tokens_for_user(payitr->account, false);
    payitr = payidx.begin();
  }
}




void microauction::receipt(name payer, vector<uint64_t> cycles, extended_asset payout) {
  require_auth(_self);
  require_recipient(payer);
}


