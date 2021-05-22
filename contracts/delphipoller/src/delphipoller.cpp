#include "delphipoller.hpp"


namespace delphipoller {

// poll action
[[eosio::action]]
void delphipoller_contract::update() {
  /*
  oraclehub::pairs_table hub_pairs(VIGORACLEHUB_CONTRACT, VIGORACLEHUB_CONTRACT.value);
  delphioracle::pairs_table delphi_pairs(DELPHIORACLE_CONTRACT, DELPHIORACLE_CONTRACT.value);

  std::vector<oraclehub::quote_data> quotes;

  // go through all oraclehub pairs
  for (auto pair : hub_pairs) {
    // check if pair exists in delphi
    auto itr = delphi_pairs.find(pair.name.value);
    if (itr != delphi_pairs.end()) {
      // get latest data point
      delphioracle::datapoints_table delphi_data(DELPHIORACLE_CONTRACT, pair.name.value);
      auto data = delphi_data.get_index<eosio::name("timestamp")>();
      if (data.begin() != data.end()) {
        auto newest = data.end();
        newest--; // the last element is the newest

        // push median to quotes vector, scaling precision if needed
        int64_t precision_scaling = std::pow(10, pair.quoted_precision) / std::pow(10, itr->quoted_precision);
        quotes.push_back({
          newest->median * precision_scaling,
          pair.name
        });
      }
    }
  }
  
  // check if we got any quotes
  if (quotes.size() > 0) {
    // Push a deferred transaction to oraclehub
    // note: this could be an inline action, but if some check failed it would break this recurring action chain, so better be safe

     eosio::action( eosio::permission_level {_self, "active"_n },
        VIGORACLEHUB_CONTRACT,
        eosio::name("push"), 
        std::make_tuple(_self, quotes)
    ).send();
    */
/*
    eosio::transaction t{};
    t.actions.emplace_back(
      eosio::permission_level(_self, eosio::name("active")),
      VIGORACLEHUB_CONTRACT,
      eosio::name("push"),
      std::make_tuple(_self, quotes)
    );
    t.delay_sec = 0;
    t.send(uint128_t(_self.value) << 64 | eosio::time_point_sec(eosio::current_time_point()).sec_since_epoch(), _self, false);
    */
 // }
}

} // namespace delphipoller
