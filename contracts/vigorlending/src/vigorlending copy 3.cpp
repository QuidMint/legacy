#include "vigorlending.hpp"

vigorlending::vigorlending(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds)
    : contract(receiver, code, ds),
      _u(receiver, receiver.value),
      _globals(receiver, receiver.value),
      _g(_globals.get_or_create(get_self())),
      _batche(receiver, receiver.value),
      _b(_batche.get_or_create(get_self())),
      _statstable(receiver, receiver.value),
      _configTable(receiver, receiver.value),
      _staked(receiver, receiver.value),
      _acctsize(receiver, receiver.value),
      _currency_stats(receiver, receiver.value),
      _log(receiver, receiver.value),
      _whitelist(receiver, receiver.value),
      _rex_balance_table(eosio::name("eosio"), eosio::name("eosio").value),
      _rex_pool_table(eosio::name("eosio"), eosio::name("eosio").value),
      _market(receiver, receiver.value),
      _bailout(receiver, receiver.value),
      _accounts(receiver, receiver.value),
      _config(_configTable.get_or_create(get_self())) {
}

vigorlending::~vigorlending() {
  
  if(!vigorlending::clearSingletons) {
    _globals.set(_g, get_self());
    _batche.set(_b, get_self());
  }
}

void vigorlending::doupdate() {
  
  eosio::require_auth(get_self());

  if(_config.getFreezeLevel() == 4)
    eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  if(_g.step == 0) eosio::check(false, "Contract is already updated. Can't run doupdate when step is 0.");

  switch(_g.step) {
    case 1:
      collectfee();
      break;
    case 2:
      distribfee();
      break;
    case 4:
      reputationrank();
      break;

    case 5: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      double bailoutCR = _config.getBailoutCR() / 100;
      double bailoutupCR = _config.getBailoutupCR() / 100;
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        update(user);
        if(user.usern != _config.getFinalReserve()) {  //identify user needing bailout
          if(_g.bailoutuser == eosio::name(0) && (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) > user.valueofcol / bailoutCR)
            _g.bailoutuser = user.usern;
          if(_g.bailoutupuser == eosio::name(0) && user.l_valueofcol > ((user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) / bailoutupCR))
            _g.bailoutupuser = user.usern;
        }
        itr++;
      }
      if(itr == _u.end()) {
        updateglobal();
        _b.usern = _u.begin()->usern;
        _g.step = 6;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 6: {
      if(_b.usern.value == _u.begin()->usern.value) {
        _b.buser = eosio::name(0);
        _b.bupuser = eosio::name(0);

        if(_g.bailoutuser != eosio::name(0)) {
          auto it = _u.find(_g.bailoutuser.value);
          if(it != _u.end()) {
            if(it->debt.amount > 0)
              _b.buser = _g.bailoutuser;
            else
              _g.bailoutuser = eosio::name(0);
          }
        }

        if(_g.bailoutupuser != eosio::name(0) && _b.buser == eosio::name(0)) {
          auto it = _u.find(_g.bailoutupuser.value);
          if(it != _u.end()) {
            if(it->l_valueofcol > 0.0)
              _b.bupuser = _g.bailoutupuser;
            else
              _g.bailoutupuser = eosio::name(0);
          }
        }

        // finalreserve bailed out by savings, under these conditions:
        // if reserve is switched on then savers (if any exist) are invoked to be insurers with pcts = user.savings/_g.savings, reserve is sent to bailout
        // first let the finalreserve try to recap itself after gaining VIG into insurance during the last distribfee
        if(_b.buser == eosio::name(0) && _b.bupuser == eosio::name(0)) {
          auto& res = _u.get(_config.getFinalReserve().value, "finalreserve not found");
          if((res.debt.amount / std::pow(10.0, res.debt.symbol.precision())) > res.valueofcol) {
            double sumpcts = 0.0;
            recap(res, res, 1.0, sumpcts, true);
            update(res);
            updateglobal();
          }
          uint64_t gsavings = _g.savings * std::pow(10.0, VIGOR_SYMBOL.precision());
          if(res.debt.amount / std::pow(10.0, res.debt.symbol.precision()) > 0)
            if(res.pcts == 1.0 && gsavings > 0)
              _b.buser = _config.getFinalReserve();
        }
      }

      if(_b.buser != eosio::name(0)) {
        eosio::action(eosio::permission_level {get_self(), eosio::name("active")}, get_self(), eosio::name("bailout"), std::make_tuple(_b.buser)).send();
      } else if(_b.bupuser != eosio::name(0)) {
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("bailoutup"), std::make_tuple(_b.bupuser)).send();
      } else {  // no bailouts
        auto itr = _u.lower_bound(_b.usern.value);
        auto stop_itr = _u.end();
        uint64_t counter = 0;
        while(itr != stop_itr && counter++ < _b.batchsize)
          itr++;
        if(itr == _u.end()) {
          _b.usern = _u.begin()->usern;
          _g.step = 7;
        } else {
          _b.usern = itr->usern;
        }
      }
      break;
    }

    case 7: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        update(user);
        itr++;
      }
      if(itr == _u.end()) {
        updateglobal();
        _b.usern = _u.begin()->usern;
        _g.step = 8;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 8: {  // final reserve recap itself
      auto it = _u.find(_config.getFinalReserve().value);
      if(it != _u.end() && (it->debt.amount / std::pow(10.0, it->debt.symbol.precision())) > it->valueofcol) {
        const auto& user = *it;
        double sumpcts = 0.0;
        recap(user, user, 1.0, sumpcts, true);
        update(user);
        updateglobal();
      } else if(it != _u.end() && it->l_valueofcol > (it->l_debt.amount / std::pow(10.0, it->l_debt.symbol.precision()))) {
        const auto& user = *it;
        double sumpcts = 0.0;
        recapup(user, user, 1.0, sumpcts, true);
        update(user);
        updateglobal();
      }

      _g.step = 9;
      break;
    }

    case 9: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.svalueofcoleavg = 0.0;
        _g.svalueofcole = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        stresscol(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 10;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 10: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.l_svalueofcoleavg = 0.0;
        _g.l_svalueofcole = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        l_stresscol(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 11;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 11: {
      stressins();

      l_stressins();

      risk();

      l_risk();

      _g.step = 12;
      break;
    }

    case 12: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.premiums = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        pricing(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 13;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 13: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.l_premiums = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        l_pricing(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 14;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 14: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        stressinsx(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 15;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 15: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.rm = 0.0;
        _g.l_rm = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        RM(user);
        l_RM(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        _g.step = 16;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 16: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      if(_b.usern.value == _u.begin()->usern.value) {
        _g.pcts = 0.0;
        _g.l_pcts = 0.0;
        _b.l_pctl = 0.0;
      }
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        pcts(user);
        l_pcts(user);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        reserve();
        _g.step = 17;
      } else {
        _b.usern = itr->usern;
      }
      break;
    }

    case 17: {
      auto itr = _u.lower_bound(_b.usern.value);
      auto stop_itr = _u.end();
      uint64_t counter = 0;
      auto market = _market.get(VIG4_SYMBOL.raw());
      double vigprice = market.marketdata.price.back() / pricePrecision;
      double vigvol = market.marketdata.vol / volPrecision;
      while(itr != stop_itr && counter++ < _b.batchsize) {
        auto& user = *itr;
        performance(user, vigvol, vigprice);
        itr++;
      }
      if(itr == _u.end()) {
        _b.usern = _u.begin()->usern;
        performanceglobal();
        uint32_t newacctsec = _config.getNewAcctSec();
        _g.availability.erase(std::remove_if(_g.availability.begin(), _g.availability.end(),
                                             [&newacctsec](const eosio::time_point& i) { return (eosio::current_time_point().sec_since_epoch() - i.sec_since_epoch()) >= newacctsec; }),
                              _g.availability.end());
        _g.totalvalue = _g.valueofcol + _g.valueofins + _g.savings * ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision) + (_g.l_totaldebt.amount / std::pow(10.0, _g.l_totaldebt.symbol.precision())) - _g.l_valueofcol - (_g.totaldebt.amount / std::pow(10.0, _g.totaldebt.symbol.precision()));
        _g.volume = 0.0;
        _g.step = 0;
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("log"), std::make_tuple(std::string("afterfull"))).send();
      } else {
        _b.usern = itr->usern;
      }
      break;
    }
  }
}

template <typename... T>
void vigorlending::schaction(const eosio::name& tag, const eosio::name& action_name, const std::tuple<T...> data, const uint32_t& delay_sec, const uint32_t& expiry_sec, const uint64_t& repeat) {
  
  croneos::queue _queue(get_self());
  _queue.schedule(
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  action_name,
                  move(data)),
    tag,
    eosio::asset(0, VIG4_SYMBOL),
    delay_sec,
    expiry_sec,
    repeat);
}

void vigorlending::reputationrank() {
  
  const auto rank = _u.get_index<"reputation"_n>();
  bool resetadata = false;
  if(_g.atimer.sec_since_epoch() < eosio::current_time_point().sec_since_epoch()){
    resetadata = true;
    _g.lprewardsp = _g.lprewardsa;
    _g.lprewardsa = 0.0;
    _g.rexproxyp = _g.rexproxya;
    _g.rexproxya = 0;
    _g.txnfeep = _g.txnfeea;
    _g.txnfeea = 0.0;
  }
  auto u = _u.get(_b.usern.value);
  auto itr = rank.lower_bound((uint64_t)(10000 * u.getReputation()));
  auto stop_itr = rank.end();
  while(itr != stop_itr && itr->usern.value != _b.usern.value)
    itr++;  // if several users have equal reputation, iterate to _b.usern starting from lower bound
  uint64_t counter = 0;
  if(_b.usern.value == rank.begin()->usern.value) {
    _b.stepdata = 0;
    _b.stepdata2 = 0;
  }

  while(itr != stop_itr && counter++ < _b.batchsize) {
    auto& user = *itr;
    _u.modify(user, get_self(), [&](auto& modified_user) {
      _b.stepdata2++;
      modified_user.setReputation_pct(((double)_b.stepdata2) / _g.ac);
      if(resetadata) {
        modified_user.txnvolume = 0.0;
        modified_user.rewardrexvot = modified_user.rewardlend;
        modified_user.rewardlend = 0.0;
      }
    });
    if(_config.getKickSeconds() > 0) {
      if(_g.ac > 5 && _b.stepdata == 0 && user.getRep_lag1().amount != -1 && (eosio::current_time_point().sec_since_epoch() > _g.kicktimer.sec_since_epoch())) {
        if(user.usern != _config.getFinalReserve() && user.usern != _config.getVigorDacFund() && _config.getExecType() == 2) {
          _b.stepdata = 1;
          // kick user with lowest reputation, periodically every KickSeconds
          eosio::action(eosio::permission_level {get_self(), "active"_n},
                        get_self(),
                        eosio::name("kick"),
                        std::make_tuple(user.usern, (uint32_t)0))
            .send();
          _g.kicktimer = eosio::time_point_sec(eosio::current_time_point().sec_since_epoch() + _config.getKickSeconds());
        }
      }
    }
    itr++;
  }
  if(itr == rank.end()) {
    if(_g.atimer.sec_since_epoch() < eosio::current_time_point().sec_since_epoch())
      _g.atimer = eosio::time_point_sec(eosio::current_time_point().sec_since_epoch() + 86400);
    _b.usern = _u.begin()->usern;
    _g.step = 5;
  } else {
    _b.usern = itr->usern;
  }
}

void vigorlending::kick(const eosio::name& usern, const uint32_t& delay_sec) {
  // /METHOD_CONTEXT;

  eosio::check(_config.getExecType() == 2, "kick user only works if mining is on, using exectype = 2");
  eosio::require_auth(get_self());
  _u.require_find(usern.value, "usern does not exist in user table");
  uint64_t step1 = 1;
  schaction(get_self(), eosio::name("returnins"), std::make_tuple(usern), delay_sec, delay_sec + (_config.getMineBuffer() * 5), 1);
  schaction(get_self(), eosio::name("predoupdate"), std::make_tuple(step1), delay_sec, delay_sec + (_config.getMineBuffer() * 6), 1);
  schaction(get_self(), eosio::name("predoupdate"), std::make_tuple(step1), delay_sec, delay_sec + (_config.getMineBuffer() * 7), 1);
  schaction(get_self(), eosio::name("returncol"), std::make_tuple(usern), delay_sec, delay_sec + (_config.getMineBuffer() * 8), 1);
  schaction(get_self(), eosio::name("dodeleteacnt"), std::make_tuple(usern), delay_sec, delay_sec + (_config.getMineBuffer() * 10), 1);

  const eosio::name& userntrace = usern;
  const uint64_t bailoutid = 0;
  std::string message = " user was scheduled to be kicked: " + usern.to_string();
  tracetable(__func__, __LINE__, userntrace, bailoutid, message);
}

// self liquidation for VIGOR debt, cost is a liquidation penalty defined by bailoutCR and reduction of credit score
// user can quickly exit a loan
// user gives debt to insuers/lenders along with an equivalent value of collateral (user retaining excess collateral)
void vigorlending::liquidate(const eosio::name& usern) {
  
  eosio::check(_config.getLiquidate() == 1, "self liquidations for VIGOR debt are disabled");
  eosio::check(_config.getExecType() == 2, "self liquidations only works if mining is on, using exectype = 2");
  eosio::check(has_auth(get_self()) || has_auth(usern), "Only Contract or user can authorize a liquidation");

  eosio::check(_g.step == 0, "liquidate: Contract is updating! Step: " + std::to_string(_g.step));

  auto userIterator = _u.require_find(usern.value, "usern doesn't exist in user table");
  const auto& user = *userIterator;
  _g.bailoutuser = usern;
  eosio::time_point_sec expiry(userIterator->lastupdate.sec_since_epoch());
  eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
  uint64_t step1 = 1;
  schaction(usern, eosio::name("predoupdate"), std::make_tuple(step1), _config.getAssetOutTime(), _config.getMineBuffer(), 1);
  const eosio::name& userntrace = usern;
  const uint64_t bailoutid = 0;
  std::string message = " user has scheduled self liquidation: " + usern.to_string();
  tracetable(__func__, __LINE__, userntrace, bailoutid, message);
}

// self liquidation for crypto debt (not VIGOR), cost is liquidation penalty defined by bailoutupCR and reduction of credit score
// user can quickly exit a loan
// user gives debt to insuers/lenders along with an equivalent value of collateral (user retaining excess collateral)
void vigorlending::liquidateup(const eosio::name& usern) {
  
  eosio::check(_config.getLiquidate() == 1, "self liquidations for crypto debt (not VIGOR) are disabled");
  eosio::check(_config.getExecType() == 2, "self liquidations only works if mining is on, using exectype = 2");
  eosio::check(has_auth(get_self()) || has_auth(usern), "Only Contract or user can authorize a liquidation");
  eosio::check(_g.step == 0, "liquidateup: Contract is updating! Step: " + std::to_string(_g.step));

  auto userIterator = _u.require_find(usern.value, "usern doesn't exist in user table");
  const auto& user = *userIterator;
  _g.bailoutupuser = usern;
  eosio::time_point_sec expiry(userIterator->lastupdate.sec_since_epoch());
  eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
  uint64_t step1 = 1;
  schaction(usern, eosio::name("predoupdate"), std::make_tuple(step1), _config.getAssetOutTime(), _config.getMineBuffer(), 1);
  const eosio::name& userntrace = usern;
  const uint64_t bailoutid = 0;
  std::string message = " user has scheduled self liquidationup: " + usern.to_string();
  tracetable(__func__, __LINE__, userntrace, bailoutid, message);
}

void vigorlending::acctstake(const eosio::name& owner) {
  

  eosio::require_auth(owner);

  eosio::check(_g.step == 0, "acctstake: Contract is updating! Step: " + std::to_string(_g.step));

  // add user to the staking table, if not exist, user pays RAM
  eosio::check(_u.find(owner.value) == _u.end(), "account already exists");

  eosio::check(_g.availability.size() >= _config.getNewAcctLim(), "Staking not required right now.");
  _staked.emplace(owner, [&](auto& st) {
    st.usern = owner;
  });
}

void vigorlending::openaccount(const eosio::name& owner) {
  
  eosio::check(is_account(owner), "this account does not exist");
  eosio::check(has_auth(owner) || has_auth(get_self()), "Needs auth of owner or contract.");
  eosio::name payer = has_auth(get_self()) ? get_self() : owner;

  eosio::check(_g.step == 0, "openaccount: Contract is updating! Step: " + std::to_string(_g.step));

  eosio::check(_g.ac < _config.getAccountsLim(), "Number of users cannot exceed maximum number of accounts, AccountsLim");

  if(owner.value == _config.getProxyPay().value || owner.value == eosio::name("lptoken.defi").value || owner.value == eosio::name("lppool.defi").value || owner.value == eosio::name("usnpool.defi").value || owner.value == eosio::name("swap.defi").value)
    eosio::check(false, "This account cannot be a user, it is an admin account.");

  if(_config.getGateKeeper() == 3) {  // Custodian
    eosdac::custodians_table custodians(_config.getDacCustodian(), _config.getDacCustodian().value);
    bool custodian = (custodians.find(owner.value) != custodians.end());
    eosio::check(custodian, "Must be DAC custodian to openaccount");
  }

  if(_config.getGateKeeper() == 2) {  // Candidate
    eosdac::candidates_table candidates(_config.getDacCustodian(), _config.getDacCustodian().value);
    auto citr = candidates.find(owner.value);
    eosio::check(citr != candidates.end() && citr->total_votes >= 1 && citr->is_active == 1, "Must be DAC candidate with at least one vote to openaccount");
  }

  if(_config.getGateKeeper() == 1) {  // Member
    regmembers regmemcheck(_config.getDacToken(), _config.getDacToken().value);
    memterms memberterms(_config.getDacToken(), _config.getDacToken().value);
    const auto& regmem = regmemcheck.get(owner.value, "Account is not registered with VIGOR DAC.");
    eosio::check((regmem.agreedtermsversion != 0), "Account has not agreed to any terms of the VIGOR constitution");
    auto latest_member_terms = (--memberterms.end());
    eosio::check(latest_member_terms->version == regmem.agreedtermsversion, "Account has not signed the latest constitution");
  }

  // add user to the user table, if not exist
  eosio::check(_u.find(owner.value) == _u.end(), "account already exists");

  auto stakeduser = _staked.find(owner.value);
  eosio::check(_g.availability.size() < _config.getNewAcctLim() || (stakeduser != _staked.end() && stakeduser->stake.amount > 0), "Daily account creation limit exceeded, stake required.");
  _u.emplace(payer, [&](auto& new_u) {
    new_u.usern = owner;
    new_u.setAnniv(eosio::current_time_point(), _config.getRepAnniv());
  });
  _g.availability.push_back(eosio::current_time_point());
  _g.ac += 1;
  _b.usern = _u.begin()->usern;
}

void vigorlending::deleteacnt(const eosio::name& owner) {
  

  eosio::require_auth(owner);

  eosio::check(_g.step == 0, "deleteacnt: Contract is updating! Step: " + std::to_string(_g.step));

  eosio::action(eosio::permission_level {get_self(), "active"_n},
                get_self(),
                eosio::name("dodeleteacnt"),
                std::make_tuple(owner))
    .send();
}

void vigorlending::dodeleteacnt(const eosio::name& owner) {
  

  eosio::require_auth(get_self());

  eosio::check(_g.step == 0, "dodeleteacnt: Contract is updating! Step: " + std::to_string(_g.step));

  auto userIter = _u.require_find(owner.value, "usern doesn't exist in user table, please use openaccount");

  auto finalreserve = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");

  const auto& user = *userIter;
  eosio::check(is_account(owner), "This account does not exist.");
  eosio::check(user.debt.amount == 0.0000, "VIGOR debt must be paid off before deleting account");
  eosio::check(user.l_debt.amount == 0.0000, "user must withdraw all VIGOR collateral before deleting account");
  // send dust to the final reserve
  auto userInsurances = getInsurances(const_cast<user_s&>(user));
  for(const auto& userInsurance : asset_container_t(*userInsurances)){
    eosio::asset dust = userInsurance - asset_swap(asset_swap(userInsurance));
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.getInsurances() -= dust;
    });
    _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
      modified_user.getInsurances() += dust;
    });
  }
  auto userLCollaterals = getLCollaterals(const_cast<user_s&>(user));
  for(const auto& userLCollateral : asset_container_t(*userLCollaterals)){
    eosio::asset dust = userLCollateral - asset_swap(asset_swap(userLCollateral));
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.getLCollaterals() -= dust;
    });
    _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
      modified_user.getLCollaterals() += dust;
    });
  }
  auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
  for(const auto& userCollateral : asset_container_t(*userCollaterals)){
    eosio::asset dust = userCollateral - asset_swap(asset_swap(userCollateral));
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.getCollaterals() -= dust;
    });
    _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
      modified_user.getCollaterals() += dust;
    });
  }
  eosio::asset amt = eosio::asset(user.vigfees, VIG10_SYMBOL);
  eosio::asset dust = amt - asset_swap(asset_swap(amt));
  if(dust.amount > 0) {
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.vigfees -= dust.amount;
      _g.vigfees -= dust.amount;
    });
    _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
      getCollaterals(modified_user) += dust;
    });
  }

  double dustd = user.savings;
  if(dustd < 0.0001 && dustd > 0.0) {
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.savings = 0.0;
    });
    auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
    _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
      modified_user.savings += dustd;
    });
  }

  eosio::check(user.savings == 0.0, "user must withdraw all VIGOR from savings before deleting account");
  eosio::check(user.vigfees == 0, "user must withdraw all VIG from vigfees before deleting account");
  eosio::check(user.getInsurances().isEmpty(), "user must withdraw all tokens from insurance/lending before deleting account");
  eosio::check(user.getLCollaterals().isEmpty(), "all borrowed tokens must be paid back before deleting account");
  eosio::check(user.getCollaterals().isEmpty(), "user must withdraw all tokens from collateral before deleting account");
  auto stakeduser = _staked.find(user.usern.value);
  eosio::check(stakeduser == _staked.end(), "user must withdraw all tokens from stakerefund before deleting account");
  auto userItera = _acctsize.find(owner.value);
  if(userItera != _acctsize.end())
    _acctsize.erase(userItera);
  if(user.usern == _g.bailoutuser)
    _g.bailoutuser = eosio::name(0);
  if(user.usern == _g.bailoutupuser)
    _g.bailoutupuser = eosio::name(0);
  const auto userIter2 = _u.find(owner.value);
  _u.erase(userIter2);
  _g.ac -= 1;
  _b.usern = _u.begin()->usern;
  const eosio::name& userntrace = user.usern;
  const uint64_t bailoutid = 0;
  std::string message = "user has been deleted " + user.usern.to_string();
  tracetable(__func__, __LINE__, userntrace, bailoutid, message);
}

// borrowers reputation
eosio::asset vigorlending::b_reputation(const user_s& user, uint64_t viga, uint64_t l_viga) {
  return eosio::asset(((1.0 - (_config.getReserveCut() + _config.getSavingsCut() * _g.savingsscale)) * (((double)user.debt.amount / (0.0001 + _g.totaldebt.amount)) * viga) + (1.0 - (_config.getReserveCut() + _config.getSavingsCut() * _g.savingsscale)) * ((user.l_valueofcol / (0.0001 + _g.l_valueofcol)) * l_viga)), _g.fee.symbol);
}

// insurers reputation
eosio::asset vigorlending::i_reputation(eosio::asset viga) {
  return viga;
}

// loop through all users, calls collectfee() to collect the fee if user has any VIGOR borrows or risky borrows, accumulates collected fees
// then calls distribfee() to distribute the accumulated fees by looping through all users distributing to those that are identified as insurers, the amount based on pcts and l_pcts
// this is a micropayment system, the payment due is for holding debt since the last update, before any new changes to the holdings are applied
void vigorlending::collectfee() {

#ifdef LOCAL_MODE
  eosio::microseconds dmsec(600000000);
#else
  eosio::microseconds dmsec = eosio::current_time_point().time_since_epoch() - _g.lastupdate.time_since_epoch();
#endif

  double T = (double)dmsec.count() / (360.0 * 24.0 * 60.0 * 60.0 * 1000000.0);  // year fraction since last update
  auto market = _market.get(symbol_swap(_g.fee.symbol).raw());
  double vigprice = market.marketdata.price.back() / pricePrecision;
  double vigvol = market.marketdata.vol / volPrecision;
  if(_b.usern.value == _u.begin()->usern.value) {
    _b.stepdata = _g.fee.amount;
    _b.stepdata2 = _g.l_fee.amount;
    _g.fee.amount = 0;
    _g.l_fee.amount = 0;
  }

  auto itr = _u.lower_bound(_b.usern.value);
  auto stop_itr = _u.end();
  uint64_t counter = 0;

  /// collect and accumulate fees paid by all borrowers into fee and l_fee
  while(itr != stop_itr && counter++ < _b.batchsize) {
    auto& user = *itr;

    if(user.usern == _config.getFinalReserve()) {
      itr++;
      continue;
    }

    if(!user.isBorrower()) {
      _u.modify(user, get_self(), [&](auto& modified_user) {
        double days_since_anniv = (3600.0 + eosio::current_time_point().sec_since_epoch() - (modified_user.getAnniv().sec_since_epoch() - _config.getRepAnniv())) / eosio::days(1).to_seconds();
        modified_user.setRep_lag0(eosio::asset((modified_user.getVig_since_anniv().amount) / std::sqrt(days_since_anniv), _g.fee.symbol));
        if(days_since_anniv > (double)_config.getRepAnniv() / eosio::days(1).to_seconds()) {
          modified_user.reputationrotation();
          modified_user.setVig_since_anniv(eosio::asset(0, _g.fee.symbol));
          modified_user.setAnniv(eosio::current_time_point(), _config.getRepAnniv());
        }
      });
      itr++;
      continue;
    }

    double tespay = (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) * (std::pow((1.0 + user.tesprice), T) - 1.0); // borrow rate due, denominated in dollars for stable borrows
    double l_tespay = user.l_valueofcol * (std::pow((1.0 + user.l_tesprice), T) - 1.0); // borrow rate due, denominated in dollars for crypto risky borrows
    // take the borrow fee (tespay) payment from user collateral, then give it to vigordacfund collateral
    auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
    double W1 = 0.0;
    double pct = user.valueofcol > 0.0 ? std::min(tespay / user.valueofcol,1.0) : 0.0;
    if (pct>0.0) {
    auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
    for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
      eosio::asset amt = userCollateral;
      amt.amount *= pct;
      amt.amount = std::max(amt.amount, (int64_t)1);
      _u.modify(user, get_self(), [&](auto& modified_user) {
        userCollaterals -= amt; // subtract fee from user collateral and global collateral
      });
      _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
        getCollaterals(modified_user) += amt; // add fee to vigordacfund collateral and global collateral
      });
      W1 += (amt.amount) / std::pow(10.0, amt.symbol.precision()) * ((double)_market.get(symbol_swap(amt.symbol).raw()).marketdata.price.back() / pricePrecision);
      if (W1 >= tespay)
        break; 
    }
    }
    // collect the borrow fee (l_tespay) payment from user l_debt and reduce global l_debt, then give it to vigordacfund (payoff it's debt if any, deposit remainder to savings)
    eosio::asset l_tespay_vigor = eosio::asset(std::min(l_tespay, user.l_debt.amount / std::pow(10.0, VIGOR_SYMBOL.precision()))*std::pow(10.0, VIGOR_SYMBOL.precision()),VIGOR_SYMBOL);
    if (l_tespay_vigor.amount > 0) {
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.l_debt -= l_tespay_vigor;
        _g.l_totaldebt -= l_tespay_vigor;
      });
      eosio::asset l_tespay_payoff = eosio::asset(std::min(l_tespay_vigor.amount, vigordacfund->l_debt.amount), VIGOR_SYMBOL); // payoff VIGOR debt, up to the amount vigordacfund has
      eosio::asset remainder = l_tespay_vigor.amount > vigordacfund->l_debt.amount ? eosio::asset(l_tespay_vigor.amount - vigordacfund->l_debt.amount, VIGOR_SYMBOL) : eosio::asset(0, VIGOR_SYMBOL);
      if (l_tespay_payoff.amount > 0)
      _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
       modified_user.debt -= l_tespay_payoff;
       _g.totaldebt -= l_tespay_payoff;
      //clear the debt from circulating supply
      auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
      eosio::action(eosio::permission_level {get_self(), "active"_n},
                    symbolIterator->contract,
                    "retire"_n,
                    std::make_tuple(l_tespay_payoff, std::string("vigordacfund payoff VIGOR debt")))
        .send();
#endif
      });
      if (remainder.amount > 0) {
        _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
          modified_user.savings += (remainder.amount / std::pow(10.0, VIGOR_SYMBOL.precision()));
        });
        _g.savings += (remainder.amount / std::pow(10.0, VIGOR_SYMBOL.precision()));
      }
    }
    
    _g.ratesub += (tespay + l_tespay) * _config.getRateSub(); // accumulate the amount dac has been subsidizing, ie vigordacfund paying some of the borrow rate

    // calculate the full borrow rate without subsity, for in repuation scoring
    eosio::asset amta = eosio::asset(((tespay / (1.0 - _config.getRateSub())) * std::pow(10.0, VIG10_SYMBOL.precision())) / vigprice, VIG10_SYMBOL); // uses get reputation credit for the full amount, this is done to counterbalance the insurers equivalently who earn the full amount
    eosio::asset l_amta = eosio::asset(((l_tespay / (1.0 - _config.getRateSub())) * std::pow(10.0, VIG10_SYMBOL.precision())) / vigprice, VIG10_SYMBOL);

    _u.modify(user, get_self(), [&](auto& modified_user) {
        _g.fee.amount += amta.amount;
        _g.l_fee.amount += l_amta.amount;
        double days_since_anniv = (3600.0 + eosio::current_time_point().sec_since_epoch() - (modified_user.getAnniv().sec_since_epoch() - _config.getRepAnniv())) / eosio::days(1).to_seconds();
        uint64_t rep = std::pow(10.0, _g.fee.symbol.precision()) * std::sqrt(std::pow(modified_user.getVig_since_anniv().amount / std::pow(10.0, _g.fee.symbol.precision()), 2.0) + (b_reputation(user, _b.stepdata, _b.stepdata2).amount / std::pow(10.0, _g.fee.symbol.precision())));
        modified_user.setRep_lag0(eosio::asset(rep / std::sqrt(days_since_anniv), _g.fee.symbol));
        modified_user.setVig_since_anniv(eosio::asset(rep, _g.fee.symbol));
        modified_user.prem += amta.amount / std::pow(10.0, amta.symbol.precision());
        modified_user.l_prem += l_amta.amount / std::pow(10.0, l_amta.symbol.precision());
        if(days_since_anniv > (double)_config.getRepAnniv() / eosio::days(1).to_seconds()) {
          modified_user.reputationrotation();
          modified_user.setVig_since_anniv(eosio::asset(0, _g.fee.symbol));
          modified_user.setAnniv(eosio::current_time_point(), _config.getRepAnniv());
        }
    });
    itr++;
  }
  if(itr == _u.end()) {
    _b.usern = _u.begin()->usern;
    _g.step = 2;
    _g.lastupdate = eosio::current_time_point();
  } else {
    _b.usern = itr->usern;
  }
}

eosio::symbol vigorlending::symbol_swap(const eosio::symbol& s){
  if (s == VIG10_SYMBOL) return VIG4_SYMBOL;
  if (s == VIG4_SYMBOL)  return VIG10_SYMBOL;
  //if (s.precision() == 0) return eosio::symbol(s.code(),1);
  //if (s.precision() == 1) return eosio::symbol(s.code(),0);
  return s;
}

eosio::asset vigorlending::asset_swap(const eosio::asset& a){
    eosio::symbol s = symbol_swap(a.symbol);
    return s == a.symbol ? a : eosio::asset(a.amount * std::pow(10.0, s.precision()-a.symbol.precision()), s);
}

  // some whitelisted tokens accrue rewards to the holder, which is vigorlending, the rewards need to be passed on to the users.
  // this action is to be called by miners, to intiate claims, and to swap the claimed tokens for VIG. VIG received by vigorlending is used to collateralize borrowed VIGOR on account vigordacfund and pass VIGOR to the users.
  // Any tokens in vigordacfund collateral, are generally from rewards, transaction fees, and liquidation penalties are swapped for VIG
  // if this fails it is not detrimental. it is permissionless, no harm in calling it anytme.
void vigorlending::rewardswap(){

  if(_config.getFreezeLevel() == 2 || _config.getFreezeLevel() == 3 || _config.getFreezeLevel() == 4)
    eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  eosio::check(_g.step == 0, "assetin: Contract is updating! Step: " + std::to_string(_g.step));

  int count = 0;
  auto vitr = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
  auto finalreserve = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");
  const auto& user = *vitr;
  auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
  std::string memo;
  for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
    if (count>=1)
      break;
    auto symbolIterator = _whitelist.find(symbol_swap(userCollateral.symbol).raw());
    if (userCollateral.symbol == VIG10_SYMBOL)
      continue;
    else if (userCollateral.symbol == SYSTEM_TOKEN_SYMBOL) { // send these whitelisted tokens to swap.defi, it sends back VIG with memo Defibox: swap token
      memo = "swap,1,11";
      sellrex(userCollateral);
    }
    else if (userCollateral.symbol == IQ_TOKEN_SYMBOL)
      memo = "swap,1,93-11";
    else if (userCollateral.symbol == USDT_SYMBOL)
      memo = "swap,1,12-11";
    else if (userCollateral.symbol == PBTC_TOKEN_SYMBOL)
      memo = "swap,1,177-11";
    else if (userCollateral.symbol == PETH_TOKEN_SYMBOL)
      memo = "swap,1,765-11";
    else if (userCollateral.symbol == BOX_TOKEN_SYMBOL)
      memo = "swap,1,194-11";
    else if (symbolIterator->contract == eosio::name("lptoken.defi")) // LP tokens, send these to swap.defi, it sends back both swap tokens with memo Defibox: withdraw
      memo = "";
    else
      continue;
    eosio::asset asset = asset_swap(userCollateral); 
    if (asset.amount > 0 && symbolIterator->assetin){
      eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                    symbolIterator->contract,
                    eosio::name("transfer"),
                    std::make_tuple(get_self(), eosio::name("swap.defi"), asset, memo))
        .send();
      eosio::asset dust = userCollateral - asset_swap(asset_swap(userCollateral));
      _u.modify(user, get_self(), [&](auto& modified_user) {
        userCollaterals -= userCollateral; // subtract the asset transefered and the dust from user collateral, this subtracts from global collateral also.
      });
      _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
        getCollaterals(modified_user) += dust; // add dust to reserve and global collateral
      });
      count++;
    }
  }
  if (count > 0) {
  update(*vitr);
  update(*finalreserve);
  updateglobal();
  }
  // these are tokens NOT on whitelist. send them to swap.defi for VIG, these tokens are not assigned to any user, but they are in account vigorlending
  if (count == 0) {
  eosio::symbol symbol;
  eosio::name contract;
  for (int i = 0; i < 2; i++) {
    if (count>=1)
      break;
    if (i==0) {
    symbol = eosio::symbol("USN", 4);
    contract = eosio::name("danchortoken");
    memo = "swap,1,9-11";
    }
    else if (i==1) {
    symbol = eosio::symbol("NDX", 4);
    contract = eosio::name("newdexissuer");
    memo = "swap,1,1-11";
    }
    else if (i==2) {
    symbol = eosio::symbol("DAD", 6);
    contract = eosio::name("dadtoken1111");
    memo = "swap,1,588-11";
    }
    auto symbolIterator = _whitelist.find(symbol.raw());
    if(symbolIterator == _whitelist.end()) {
      accounts accounts_table(contract, get_self().value);
      const auto& itr = accounts_table.find(symbol.code().raw());
      if(itr != accounts_table.end()) {
        eosio::asset balance = itr->balance;
        if (balance.amount > 0 && balance.symbol.raw() == symbol.raw()){
          eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                        contract,
                        eosio::name("transfer"),
                        std::make_tuple(get_self(), eosio::name("swap.defi"), balance, memo))
            .send();
          count++;
        }
      }
    }
  }
  }
if(count>0) payminer(_config.getMinerFee());
}

// miners can call this to claim the rewards, it is permissionless, miners are paid a fee
// contract can take one of three values: lptoken.defi, usnpool.defi, lppool.defi
void vigorlending::rewardclaim(eosio::name contract){

  if(_config.getFreezeLevel() == 2 || _config.getFreezeLevel() == 3 || _config.getFreezeLevel() == 4)
  eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  eosio::check(_g.step == 0, "assetin: Contract is updating! Step: " + std::to_string(_g.step));

if (contract.value == eosio::name("lptoken.defi").value) {
 for(const auto& w : _whitelist) {
    if(w.contract.value == contract.value && (_g.getCollaterals().hasAsset(symbol_swap(w.sym)) || _g.getInsurances().hasAsset(symbol_swap(w.sym))))
      eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                    contract,
                    eosio::name("claimall"),
                    std::make_tuple(get_self(), w.sym.code(), 20))
        .send();
  }
} else if (contract.value == eosio::name("usnpool.defi").value) {
  eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                contract,
                eosio::name("claimall"),
                std::make_tuple(get_self()))
    .send();
} else if (contract.value == eosio::name("lppool.defi").value) {
  eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                contract,
                eosio::name("claimall"),
                std::make_tuple(get_self()))
    .send();
}
payminer(_config.getMinerFee());
}

void vigorlending::payminer(uint64_t fee){
      eosio::asset gasfee = asset_swap(eosio::asset(fee, VIG4_SYMBOL));
      eosio::asset vig = eosio::asset(0, gasfee.symbol);
      auto vitr = _u.find(_config.getVigorDacFund().value);
      if(vitr != _u.end()) {
      auto vigordacfund = *vitr;
      if (vigordacfund.getCollaterals().hasAsset(gasfee,vig) && vig.amount >= gasfee.amount) {
        eosio::name miner = get_first_receiver();
        auto mitr = _u.find(miner.value);
        if(mitr != _u.end()) {
          _u.modify(mitr, get_self(), [&](auto& modified_user) {
            modified_user.getCollaterals() += gasfee;
          });
          _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
              modified_user.getCollaterals() -= gasfee;
          });
        }
      }
      }
}

//handler for notification of transfer action on other contracts where this contract is notified by require_recipient(to)
//the purpose is to handle when user sends a token (inlcuding VIGOR) to this contract
//example, when someone sends EOS to this contract, the eosio.token transfer action notifies this contract's transferh, which updates the user account based on memo instructions
//this schedules actions that need to be mined if mine=true
void vigorlending::assetin(const eosio::name& from, const eosio::name& to, const eosio::asset& assetin, const std::string& memo) {
  

  if(from == get_self() || to != get_self() || memo == "donation") return;

  eosio::check(assetin.is_valid(), "invalid assetin");
  eosio::check(is_account(to), "to account does not exist");
  eosio::check(assetin.amount > 0, "must transfer positive assetin");
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

  eosio::name contract = get_first_receiver(); // claims received but not on whitelist so can't store in user asset containers
  if((from == _config.getProxyPay() && assetin.symbol.raw() == eosio::symbol("GEN", 8).raw() && contract.value == eosio::name("genpooltoken").value) ||
    ((from == eosio::name("swap.defi") && memo == "Defibox: withdraw" && 
    ((assetin.symbol.raw() == eosio::symbol("USN", 4).raw() && contract.value == eosio::name("danchortoken").value) ||
     (assetin.symbol.raw() == eosio::symbol("NDX", 4).raw() && contract.value == eosio::name("newdexissuer").value) ||
     (assetin.symbol.raw() == eosio::symbol("DAD", 6).raw() && contract.value == eosio::name("dadtoken1111").value)))))
    return;

  if(from == eosio::name("eosio") || from == eosio::name("eosio.bpay") ||
     from == eosio::name("eosio.msig") || from == eosio::name("eosio.names") ||
     from == eosio::name("eosio.prods") || from == eosio::name("eosio.ram") ||
     from == eosio::name("eosio.ramfee") || from == eosio::name("eosio.saving") ||
     from == eosio::name("eosio.stake") || from == eosio::name("eosio.token") ||
     from == eosio::name("eosio.unregd") || from == eosio::name("eosio.vpay") ||
     from == eosio::name("eosio.wrap") || from == eosio::name("eosio.rex")) {
    return;
  }
  if(_config.getFreezeLevel() == 2 || _config.getFreezeLevel() == 3 || _config.getFreezeLevel() == 4)
    eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  eosio::check(_g.step == 0, "assetin: Contract is updating! Step: " + std::to_string(_g.step));

  require_auth(from);
  auto symbolIterator = _whitelist.find(assetin.symbol.raw());
  eosio::check(symbolIterator != _whitelist.end(), "assetin symbol precision mismatch or unknown, not found on whitelist");
  eosio::check(symbolIterator->assetin, "assetin is disabled for this token");
  eosio::check(contract == symbolIterator->contract, "assetin token contract mismatch or unknown");
  const eosio::name& userntrace = from;
  const uint64_t bailoutid = 0;

  if(assetin.symbol == VIG4_SYMBOL) eosio::check(assetin.amount < INT64_MAX / std::pow(10.0, 6), "amount too large");

  const eosio::asset& localAssetIn = asset_swap(assetin);

// rewards handler
#if defined(LOCAL_MODE)
  bool lpclaim = memo == "test reward claims" ? true : false;
#else
  bool lpclaim = (from == eosio::name("lptoken.defi") || from == eosio::name("lppool.defi") || from == eosio::name("usnpool.defi") || from == eosio::name("swap.defi") || from == _config.getProxyPay());
#endif
  if(lpclaim) {
    if(assetin.symbol.raw() == SYSTEM_TOKEN_SYMBOL.raw() && from == _config.getProxyPay()){
      _g.rexproxy += assetin.amount; //accumulate proxy vote rewards
      const std::vector<eosio::name>& producers {};
      eosio::action(eosio::permission_level {get_self(), "active"_n},
                    eosio::name("eosio"),
                    eosio::name("voteproducer"),
                    std::make_tuple(get_self(), _config.getProxyContr(), producers))
        .send();
    }
    else if(from == eosio::name("swap.defi")){} // don't accumulate rewards, this is the rewards swapped for VIG, already accumulated
    else if(assetin.symbol.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      _g.lprewards += (assetin.amount / std::pow(10.0, SYSTEM_TOKEN_SYMBOL.precision())) * ((double)_market.get(SYSTEM_TOKEN_SYMBOL.raw()).marketdata.price.back() / pricePrecision); //accumulate LP token rewards
    else if(assetin.symbol.raw() == BOX_TOKEN_SYMBOL.raw())
      _g.lprewards += (assetin.amount / std::pow(10.0, BOX_TOKEN_SYMBOL.precision())) * ((double)_market.get(BOX_TOKEN_SYMBOL.raw()).marketdata.price.back() / pricePrecision); //accumulate LP token rewards
    else if(assetin.symbol.raw() == VIG4_SYMBOL.raw())
      _g.lprewards += (assetin.amount / std::pow(10.0, VIG4_SYMBOL.precision())) * ((double)_market.get(VIG4_SYMBOL.raw()).marketdata.price.back() / pricePrecision); //accumulate LP token rewards
    else
      return;
    auto vitr = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
    _u.modify(vitr, get_self(), [&](auto& modified_user) {
        getCollaterals(modified_user) += localAssetIn;
    });
    update(*vitr);
    updateglobal();
  }

  auto userIterator = _u.find(from.value);
  const auto& user = *userIterator;
  auto stakeduser = _staked.find(from.value);
  if(userIterator == _u.end() && memo == "stake") {
    eosio::check(stakeduser != _staked.end() && stakeduser->stake.symbol == _g.fee.symbol && stakeduser->stake.amount == 0, "User not found in staking table, or has non-zero stake balance.");
    _staked.modify(stakeduser, get_self(), [&](auto& st) {
      auto market = _market.get(symbol_swap(localAssetIn.symbol).raw());
      uint64_t reqstake = (std::pow(10.0, localAssetIn.symbol.precision()) * _config.getReqStake()) / ((double)market.marketdata.price.back() / pricePrecision);
      eosio::check(localAssetIn >= eosio::asset(reqstake, _g.fee.symbol), "Insufficient stake, required stake is " + (eosio::asset(reqstake, _g.fee.symbol)).to_string());
      st.stake = localAssetIn;
      uint64_t staketime = 2592000;
      st.unlocktime = eosio::time_point_sec(eosio::current_time_point()) + staketime;
    });
    return;
  }

  eosio::check(memo == "insurance" ||
                 memo == "collateral" ||
                 memo == "payback borrowed token" ||
                 memo == "savings" ||
                 memo == "vigfees",
               "memo must be composed of either word: 'insurance' or 'collateral' or 'payback borrowed token' or 'savings' or 'vigfees'");
  if(memo == "vigfees")
    eosio::check(assetin.symbol.raw() == VIG4_SYMBOL.raw(), "Only VIG token is acceptable for vigfees");

  if(memo == "savings")
    eosio::check(assetin.symbol.raw() == VIGOR_SYMBOL.raw(), "Only VIGOR token is acceptable for savings");

  eosio::check(userIterator != _u.end(), "Account not found in the user table");

  std::string message = std::string(user.usern.to_string()) + " tranfers assetin " + localAssetIn.to_string() + " with memo - " + memo;
  tracetable(__func__, __LINE__, userntrace, bailoutid, message);

  double user_stresscol = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.volcol)) - 1.0);
  double svalueofcole = std::max(user.debt.amount / std::pow(10.0, user.debt.symbol.precision()) - ((1.0 - user_stresscol) * user.valueofcol), 0.0);
  double user_l_stresscol = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.l_volcol) - 1.0;
  double l_svalueofcole = std::max(((1.0 + user_l_stresscol) * user.l_valueofcol) - user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()), 0.0);
  double premiums = user.tesprice * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()));
  double l_premiums = user.l_tesprice * user.l_valueofcol;
  double dRM = user.svalueofinsx == 0.0 ? 0.0 : std::max(_g.svalueofins - user.svalueofinsx, 0.0);
  double l_dRM = user.l_svalueofinsx == 0.0 ? 0.0 : std::max(_g.l_svalueofins - user.l_svalueofinsx, 0.0);
  double gvalueofins = _g.valueofins;
  double gvalueofcol = _g.valueofcol;
  double ul_pctl = l_pctl(user);
  if(_market.begin() == _market.end()) {
    for(const auto& w : _whitelist) {
      t_series stats(_config.getOracleHub(), w.feed.value);
      auto mitr = _market.find(w.sym.raw());
      if(mitr != _market.end()) {
        if(stats.begin()->price.back() != mitr->marketdata.price.back() ||
           stats.begin()->vol != mitr->marketdata.vol)
          _market.modify(mitr, get_self(), [&](auto& m) {
            m.marketdata = *stats.begin();
          });
      } else {
        _market.emplace(get_self(), [&](auto& m) {
          m.sym = w.sym;
          m.marketdata = *stats.begin();
        });
      }
    }
  }
  double totalvalbefore = _g.valueofcol + _g.valueofins + _g.savings * ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision) - _g.l_valueofcol - (_g.totaldebt.amount / std::pow(10.0, _g.totaldebt.symbol.precision()));
  for(const auto& w : _whitelist) {
    t_series stats(_config.getOracleHub(), w.feed.value);
    auto mitr = _market.find(w.sym.raw());
    if(mitr != _market.end()) {
      //if(stats.begin()->price.back() != mitr->marketdata.price.back() ||
      //   stats.begin()->vol != mitr->marketdata.vol)
        _market.modify(mitr, get_self(), [&](auto& m) {
          m.marketdata = *stats.begin();
        });
    } else {
      _market.emplace(get_self(), [&](auto& m) {
        m.sym = w.sym;
        m.marketdata = *stats.begin();
      });
    }
  }

  auto market = _market.get(symbol_swap(localAssetIn.symbol).raw());
  double valueofassetin = (localAssetIn.amount) / std::pow(10.0, localAssetIn.symbol.precision()) *
                          ((double)market.marketdata.price.back() / pricePrecision);

  if(localAssetIn.symbol == VIGOR_SYMBOL) {
    if(memo == "collateral") {
      _u.modify(userIterator, get_self(), [&](auto& modified_user) {
        modified_user.l_debt += localAssetIn;
        _g.l_totaldebt += localAssetIn;
        valueofassetin = (localAssetIn.amount / std::pow(10.0, localAssetIn.symbol.precision()));
      });
    } else if(memo == "insurance") {
      _u.modify(userIterator, get_self(), [&](auto& modified_user) {
        getInsurances(modified_user) += localAssetIn;
      });
    } else if(memo == "payback borrowed token") {
      _u.modify(userIterator, get_self(), [&](auto& modified_user) {
        eosio::check(userIterator->debt.amount >= localAssetIn.amount, "Payment too high");

        modified_user.debt -= localAssetIn;
        _g.totaldebt -= localAssetIn;
        valueofassetin = (localAssetIn.amount / std::pow(10.0, localAssetIn.symbol.precision()));

        //clear the debt from circulating supply
        auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
        eosio::action(eosio::permission_level {get_self(), "active"_n},
                      symbolIterator->contract,  // contract for vigor token
                      "retire"_n,
                      std::make_tuple(localAssetIn, memo))
          .send();
#endif
      });
    } else if(memo == "savings") {
      _u.modify(userIterator, get_self(), [&](auto& modified_user) {
        modified_user.savings += (localAssetIn.amount / std::pow(10.0, localAssetIn.symbol.precision()));
        _g.savings += (localAssetIn.amount / std::pow(10.0, localAssetIn.symbol.precision()));
      });
    }
  } else {
    _u.modify(userIterator, get_self(), [&](auto& modified_user) {
      if(memo == "insurance") {
        getInsurances(modified_user) += localAssetIn;
      } else if(memo == "collateral") {
        getCollaterals(modified_user) += localAssetIn;
      } else if(memo == "payback borrowed token") {
        paybacktok(*userIterator, localAssetIn);
      } else if(memo == "vigfees") {
        modified_user.vigfees += localAssetIn.amount;
        _g.vigfees += localAssetIn.amount;
      }
    });

    if(_config.getRexSwitch() >= 1 && localAssetIn.symbol == SYSTEM_TOKEN_SYMBOL) {
      double rexprice1 = 1.0;
      uint64_t vote_stake1 = 0;
      auto bitr = _rex_balance_table.find(get_self().value);
      if(bitr != _rex_balance_table.end() && bitr->vote_stake.amount > 0 && bitr->rex_balance.amount > 0) {
        vote_stake1 = bitr->vote_stake.amount + 1;
        rexprice1 = (double)vote_stake1 / (bitr->rex_balance.amount / std::pow(10.0, 4));
      }
      eosio::action(eosio::permission_level {get_self(), "active"_n},
                    eosio::name("eosio"),
                    eosio::name("deposit"),
                    std::make_tuple(get_self(), localAssetIn))
        .send();
      eosio::action(eosio::permission_level {get_self(), "active"_n},
                    eosio::name("eosio"),
                    eosio::name("buyrex"),
                    std::make_tuple(get_self(), localAssetIn))
        .send();
      auto rexitr = _rex_pool_table.begin();
      double rexprice2 = double(rexitr->total_lendable.amount) / (rexitr->total_rex.amount / std::pow(10.0, 4));
      _g.rexproxy += vote_stake1 * (std::max(rexprice2 / rexprice1, 1.0) - 1.0);
      auto vitr = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
      _u.modify(vitr, get_self(), [&](auto& modified_user) {
          getCollaterals(modified_user) += eosio::asset(vote_stake1 * (std::max(rexprice2 / rexprice1, 1.0) - 1.0), SYSTEM_TOKEN_SYMBOL);
      });
    }
  }
  _g.volume += valueofassetin;
  update(user);
  takefee(user, std::min(std::max(valueofassetin * _config.getTxnFee() / 10000.0, 0.0), 10.0));
  update(user);
  double vigorprice = ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision);
  double usertotal = user.valueofcol + user.valueofins + user.savings * vigorprice + (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) + user.l_valueofcol + (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()));
  eosio::check(usertotal <= _config.getInitMaxSize(), "Total value of account cannot exceed: " + std::to_string(_config.getInitMaxSize()) + " dollars.");
  switch(_config.getExecType()) {
    case 1: {  // deferred transactions
      break;
    }

    case 2: {  // mining

      // the transfer in is immediate, schedule the doupdate with zero delay.
      updateglobal();
      double totalvalafter = _g.valueofcol + _g.valueofins + _g.savings * vigorprice + (_g.l_totaldebt.amount / std::pow(10.0, _g.l_totaldebt.symbol.precision())) - _g.l_valueofcol - (_g.totaldebt.amount / std::pow(10.0, _g.totaldebt.symbol.precision()));
      bool t0 = totalvalbefore == 0.0 || totalvalafter == 0.0;                                                                     // total value before or after this txn is zero
      bool t1 = _g.pcts > 1.05 || _g.pcts < 0.95 || _g.l_pcts > 1.05 || _g.l_pcts < 0.95 || _b.l_pctl > 1.05 || _b.l_pctl < 0.95;  // large cumulative changes of insurance since last full update
      bool t2 = _g.valueofins == 0.0 || gvalueofins == 0.0 ? true : valueofassetin / gvalueofins > 0.03;                           // large individual insurance txn
      bool t3 = _g.valueofcol == 0.0 || gvalueofcol == 0.0 ? true : valueofassetin / gvalueofcol > 0.03;                           // large individual collateral txn
      bool t4 = _g.totalvalue == 0.0 ? true : totalvalafter / _g.totalvalue > 1.03 || totalvalafter / _g.totalvalue < 0.97;        // large change in total value since last full update (includes new market prices and user txn amount)
      bool t5 = _g.totalvalue == 0.0 ? true : (_g.volume + valueofassetin) / _g.totalvalue > 0.03;                                 // large change in volume since last full update
      bool t6 = (_g.lastupdate.sec_since_epoch() + 3600 < eosio::current_time_point().sec_since_epoch());                          // large amount of time has passed since last full update
      if(t0 || t1 || t2 || t3 || t4 || t5 || t6) {                                                                                 // full update
        eosio::time_point_sec expiry(userIterator->lastupdate.sec_since_epoch());
        eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
        uint64_t step = 1;
        schaction(from, eosio::name("predoupdate"), std::make_tuple(step), 0, _config.getMineBuffer(), 1);
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("log"), std::make_tuple(std::string("assetinfull"))).send();
      } else {  // partial update
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("log"), std::make_tuple(std::string("assetinpart"))).send();
        eosio::time_point_sec expiry(userIterator->lastupdate.sec_since_epoch());
        eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
        croneos::queue _queue(get_self());
        _queue.rate_limit(user.usern);
        double svalueofcoleavg = _g.svalueofcoleavg;
        _g.svalueofcole -= svalueofcole;
        stresscol(user);
        _g.svalueofcoleavg = svalueofcoleavg;
        double l_svalueofcoleavg = _g.l_svalueofcoleavg;
        _g.l_svalueofcole -= l_svalueofcole;
        l_stresscol(user);
        _g.l_svalueofcoleavg = l_svalueofcoleavg;
        stressins();
        l_stressins();
        risk();
        l_risk();
        _g.premiums -= premiums;
        pricing(user);
        _g.l_premiums -= l_premiums;
        l_pricing(user);
        _g.rm -= dRM;
        _g.l_rm -= l_dRM;
        stressinsx(user);
        RM(user);
        l_RM(user);
        _g.pcts -= user.pcts;
        _g.l_pcts -= user.l_pcts;
        _b.l_pctl -= ul_pctl;
        pcts(user);
        l_pcts(user);
        reserve();
        auto market = _market.get(VIG4_SYMBOL.raw());
        double vigprice = market.marketdata.price.back() / pricePrecision;
        double vigvol = market.marketdata.vol / volPrecision;
        performance(user, vigvol, vigprice);
        performanceglobal();
      }
      break;
    }

    case 3: {  // inline
      break;
    }
  }

  _u.modify(userIterator, get_self(), [&](auto& modified_user) {
    modified_user.lastupdate = eosio::current_time_point();
  });
}

// transaction fee, in basis points of the amount transacted, maximum 10 dollars, (if _config.getTxnFee() is 10 it means 10 bps or 0.0010), taken from user insurance, collateral, l_debt
void vigorlending::takefee(const user_s& user, double txnfeedollars) {

  double W1 = 0.0;
  auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
  eosio::asset asset = eosio::asset(0, VIG10_SYMBOL);
  user.getCollaterals().hasAsset(asset, asset);
  double pct = user.valueofcol > 0.0 ? std::min(txnfeedollars / (user.valueofcol - (asset.amount / std::pow(10.0, asset.symbol.precision()) * ((double)_market.get(symbol_swap(asset.symbol).raw()).marketdata.price.back() / pricePrecision))), 1.0) : 0.0;
  if (pct > 0.0) {
  auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
  for(const auto& userCollateral : asset_container_t(*userCollaterals)) { // takefee from collateral
    eosio::asset amt = userCollateral;
    if (amt.symbol.code() == VIG10_SYMBOL.code())
      continue;
    amt.amount *= pct;
    amt.amount = std::max(amt.amount, (int64_t)1);
    _u.modify(user, get_self(), [&](auto& modified_user) {
      userCollaterals -= amt; // subtract fee from user and global collateral
    });
    _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
      getCollaterals(modified_user) += amt; // add fee to vigordacfund and global collateral
    });
    W1 += (amt.amount) / std::pow(10.0, amt.symbol.precision()) * ((double)_market.get(symbol_swap(amt.symbol).raw()).marketdata.price.back() / pricePrecision);
    if (W1 >= txnfeedollars)
      break; 
  }
  }
  eosio::asset amtvigor = eosio::asset(0, VIGOR_SYMBOL);
  asset = eosio::asset(0, VIG10_SYMBOL);
  user.getInsurances().hasAsset(asset, asset);
  pct = user.valueofins > 0.0 ? std::min((txnfeedollars - W1) / (user.valueofins - (asset.amount / std::pow(10.0, asset.symbol.precision()) * ((double)_market.get(symbol_swap(asset.symbol).raw()).marketdata.price.back() / pricePrecision))), 1.0) : 0.0;
  if (pct>0.0) { // takefee from insurance
  auto userInsurances = getInsurances(const_cast<user_s&>(user));
  for(const auto& userInsurance : asset_container_t(*userInsurances)) {
    eosio::asset amt = userInsurance;
    if (amt.symbol.code() == VIG10_SYMBOL.code())
      continue;
    amt.amount *= pct;
    amt.amount = std::max(amt.amount, (int64_t)1);
    _u.modify(user, get_self(), [&](auto& modified_user) {
      userInsurances -= amt;
    });
    if (amt.symbol.code() == VIGOR_SYMBOL.code()) {
      amtvigor = amt;
    } else
      _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
        getInsurances(modified_user) += amt;
      });
    W1 += (amt.amount) / std::pow(10.0, amt.symbol.precision()) * ((double)_market.get(symbol_swap(amt.symbol).raw()).marketdata.price.back() / pricePrecision);
    if (W1 >= txnfeedollars)
      break;
  }
  }
  if (txnfeedollars - W1 > 0) { // takefee from user l_debt and reduce global l_debt, then give it to vigordacfund (payoff it's debt if any, deposit remainder to savings)
  eosio::asset l_debtfee = eosio::asset(std::min(txnfeedollars - W1, user.l_debt.amount / std::pow(10.0, VIGOR_SYMBOL.precision()))*std::pow(10.0, VIGOR_SYMBOL.precision()),VIGOR_SYMBOL);
  if (l_debtfee.amount > 0)
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.l_debt -= l_debtfee;
      _g.l_totaldebt -= l_debtfee;
      W1 += (l_debtfee.amount) / std::pow(10.0, l_debtfee.symbol.precision()) * ((double)_market.get(symbol_swap(l_debtfee.symbol).raw()).marketdata.price.back() / pricePrecision);
    });
    l_debtfee += amtvigor;
    eosio::asset l_tespay_payoff = eosio::asset(std::min(l_debtfee.amount, vigordacfund->l_debt.amount), VIGOR_SYMBOL); // payoff VIGOR debt, up to the amount vigordacfund has
    eosio::asset remainder = l_debtfee.amount > vigordacfund->l_debt.amount ? eosio::asset(l_debtfee.amount - vigordacfund->l_debt.amount, VIGOR_SYMBOL) : eosio::asset(0, VIGOR_SYMBOL);
    if (l_tespay_payoff.amount > 0)
    _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
     modified_user.debt -= l_tespay_payoff;
     _g.totaldebt -= l_tespay_payoff;
    //clear the debt from circulating supply
    auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
    eosio::action(eosio::permission_level {get_self(), "active"_n},
                  symbolIterator->contract,
                  "retire"_n,
                  std::make_tuple(l_tespay_payoff, std::string("vigordacfund payoff VIGOR debt")))
      .send();
#endif
    });
    if (remainder.amount > 0) {
      _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
      modified_user.savings += (remainder.amount / std::pow(10.0, VIGOR_SYMBOL.precision()));
      });
      _g.savings += (remainder.amount / std::pow(10.0, VIGOR_SYMBOL.precision()));
    }
  }
  _g.txnfee += _g.txnfee;
  }

//method to process details about user sending back a borrowed risky token (not VIGOR)
//this method also used when insurers need to recap a bailout involving borrowed risk tokens
//i.e. insurer recieves bailout of EOS debt (plus associated collateral) and if that user owns EOS, then the insurer can payoff the EOS debt
void vigorlending::paybacktok(const user_s& user, eosio::asset assetin) {
  
  const eosio::name& userntrace = user.usern;
  const uint64_t bailoutid = 0;
  //payback borrowed token into user l_collateral (not VIGOR)
  auto userLCollaterals = user.getLCollaterals();
  eosio::asset l_collateral;
  eosio::asset a = assetin;

  if(!userLCollaterals.hasAsset(assetin, l_collateral)) eosio::check(false, "Can't payback that asset; not found in user borrows.");
  eosio::asset dust = l_collateral - asset_swap(asset_swap(l_collateral)); // user trying to payoff dust loan of 0.0000000100 VIG10, can send minimum 0.0001 VIG4, which is "payback amount too high"
  eosio::asset plug = dust.amount == 0 ? eosio::asset(0, assetin.symbol) : asset_swap(eosio::asset(1,symbol_swap(assetin.symbol))) - dust; // plus is 1 - dust
  if (assetin - plug == l_collateral) {// full payoff, reserve gets the plug amount
    auto finalreserve = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");
    _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
      modified_user.getCollaterals() += plug;
    });
    assetin = assetin - plug;
  }
 eosio::check(l_collateral >= assetin, "Payback amount too high. " + asset_swap(a).to_string() + " > " + (asset_swap(l_collateral).amount == 0 ? eosio::asset(1,symbol_swap(assetin.symbol)) : asset_swap(l_collateral+plug)).to_string() + " owner: " + userntrace.to_string());

  _u.modify(user, get_self(), [&](auto& modified_user) {
    modified_user.getLCollaterals() -= assetin;
  });

  // subtract the located asset from the global l_collateral
  _g.getLCollaterals().move(assetin, _g.getInsurances());

  // don't add the located token to any particular insurer, just add it to global insurance
}

// before calling doupdate increment the step
void vigorlending::predoupdate(const uint64_t& step) {
  eosio::require_auth(get_self());
  eosio::check(_g.step == 0, "predoupdate: Contract is updating! Step: " + std::to_string(_g.step));

  _g.step = step;
  _b.usern = _u.begin()->usern;
}

// user directly calls assetout() action for all outbound token transfers (including VIGOR)
// memo instructions determine how the user table is updated
// this schedules actions that need to be mined if mining=true
void vigorlending::assetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo) {
  

  if(_config.getFreezeLevel() == 1 || _config.getFreezeLevel() == 3 || _config.getFreezeLevel() == 4)
    eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  eosio::check(_g.step == 0, "assetout: Contract is updating! Step: " + std::to_string(_g.step));

  eosio::require_auth(usern);

  eosio::check(usern != get_self(), "cannot transfer to self");
  eosio::check(assetout.is_valid(), "invalid assetout");
  eosio::check(is_account(usern), "to account does not exist");
  auto symbolIterator = _whitelist.find(assetout.symbol.raw());
  eosio::check(symbolIterator != _whitelist.end(), "assetout symbol precision mismatch or unknown");
  eosio::check(symbolIterator->assetout, "assetout is disabled for this token");
  eosio::check(assetout.amount > 0, "must transfer positive assetout");
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");
  eosio::check(memo == "collateral" ||
                 memo == "insurance" ||
                 memo == "borrow" ||
                 memo == "stakerefund" ||
                 memo == "savings" ||
                 memo == "vigfees",
               "memo must be composed of either word: 'insurance' | 'collateral' | 'borrow' | 'stakerefund' | 'savings' | 'vigfees'");
  if(memo == "vigfees")
    eosio::check(assetout.symbol == VIG4_SYMBOL, "Only VIG token is acceptable for vigfees");

  if(memo == "savings")
    eosio::check(assetout.symbol == VIGOR_SYMBOL, "Only VIGOR token is acceptable for savings");

  auto itr = _u.find(usern.value);
  eosio::check(itr != _u.end(), "User account does not exist please use openaccount");
  const eosio::name& userntrace = usern;
  const uint64_t bailoutid = 0;

  switch(_config.getExecType()) {
    case 1: {  // deferred transactions
      break;
    }

    case 2: {  // mining

      // preliminary check before scheduling, exec=false
      double totalval = 0.0;
      double totalvalbefore = 0.0;
      uint64_t step0 = 0;
      uint64_t step1 = 1;
      exassetout(usern, assetout, memo, false, false, step0, totalval, totalvalbefore);
      double valueofassetout = totalvalbefore - totalval;
      bool t0 = totalvalbefore == 0.0 || totalval == 0.0;                                                                          // total value before or after this txn is zero
      bool t1 = _g.pcts > 1.05 || _g.pcts < 0.95 || _g.l_pcts > 1.05 || _g.l_pcts < 0.95 || _b.l_pctl > 1.05 || _b.l_pctl < 0.95;  // large cumulative changes of insurance since last full update
      bool t2 = _g.valueofins == 0.0 ? true : valueofassetout / _g.valueofins > 0.03;                                              // large txn relative to size of insurance
      bool t3 = _g.valueofcol == 0.0 ? true : valueofassetout / _g.valueofcol > 0.03;                                              // large txn relative to size of collateral
      bool t4 = _g.totalvalue == 0.0 ? true : totalval / _g.totalvalue > 1.03 || totalval / _g.totalvalue < 0.97;                  // large change in total value since last full update
      bool t5 = _g.totalvalue == 0.0 ? true : (_g.volume + valueofassetout) / _g.totalvalue > 0.03;                                // large change in volume since last full update
      bool t6 = (_g.lastupdate.sec_since_epoch() + 3600 < eosio::current_time_point().sec_since_epoch());                          // large amount of time has passed since last full update
      if(t0 || t1 || t2 || t3 || t4 || t5 || t6) {                                                                                 // full update, totalval: total value including market price drift and all user transactions since last full update, including this user txn
        eosio::time_point_sec expiry(itr->lastupdate.sec_since_epoch());
        eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
        //schedule assetout with delay (because oracle feed is delayed). the action will be minable starting at getAssetOutTime (note, delay is not the same as rate limiting)
        schaction(usern, eosio::name("doassetout"), std::make_tuple(usern, assetout, memo, true, false, step1), _config.getAssetOutTime(), _config.getMineBuffer(), 1);
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("log"), std::make_tuple(std::string("assetoutfull"))).send();
      } else {
        eosio::action(eosio::permission_level {get_self(), "active"_n}, get_self(), eosio::name("log"), std::make_tuple(std::string("assetoutpart"))).send();
        eosio::time_point_sec expiry(itr->lastupdate.sec_since_epoch());
        eosio::check(eosio::current_time_point().sec_since_epoch() > expiry.sec_since_epoch(), "Your previous transaction needs to settle, please wait " + std::to_string((expiry.sec_since_epoch() + 1) - eosio::current_time_point().sec_since_epoch()) + " seconds.");
        croneos::queue _queue(get_self());
        _queue.rate_limit(usern);
        eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                      get_self(),
                      eosio::name("doassetout"),
                      std::make_tuple(usern, assetout, memo, true, true, step0))
          .send();
      }
      break;
    }

    case 3: {  // inline

      uint64_t step0 = 0;
      eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                    get_self(),
                    eosio::name("doassetout"),
                    std::make_tuple(usern, assetout, memo, true, false, step0))
        .send();
      break;
    }
  }
  _u.modify(itr, get_self(), [&](auto& modified_user) {
    modified_user.lastupdate = eosio::current_time_point();
  });
}

void vigorlending::tick() {
  
  eosio::check(_config.getFreezeLevel() != 4, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));
  if(_g.step != 0) {
    eosio::action(
      eosio::permission_level {get_self(), eosio::name("active")},
      get_self(),
      eosio::name("doupdate"),
      std::make_tuple())
      .send();
      payminer(_config.getTickFee());
  } else {
    croneos::queue _queue(get_self());
    bool success = _queue.exec();
    if(!success) {
      if(_g.step != 0) {
        schaction(get_self(), eosio::name("doupdate"), std::make_tuple(), 0, _config.getMineBuffer(), 1);
      } else {
        eosio::check(false, "action queue is empty or all expired");
      }
    }
  }
}

// transfer all tokens out of user insurance to specified user
void vigorlending::returnins(const eosio::name& usern) {
  

  require_auth(get_self());
  auto userIterator = _u.require_find(usern.value, errormsg_missinguser);
  const auto& user = *userIterator;

  auto userInsurances = getInsurances(const_cast<user_s&>(user));

  uint64_t step0 = 0;
  eosio::asset theAssetOut;
  for(const auto& userInsurance : asset_container_t(*userInsurances)) {
    theAssetOut = asset_swap(userInsurance);
    eosio::action(
      eosio::permission_level {get_self(), eosio::name("active")},
      get_self(),
      eosio::name("doassetout"),
      std::make_tuple(user.usern, theAssetOut, INSURANCE, true, true, step0))
      .send();
  }
  _g.bailoutuser = usern;
  _g.bailoutupuser = usern;
}

// transfer all tokens out of user collateral to specified user
void vigorlending::returncol(const eosio::name& usern) {
  

  require_auth(get_self());
  auto userIterator = _u.require_find(usern.value, "usern doesn't exist in the user table");
  const auto& user = *userIterator;
  uint64_t step0 = 0;
  auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
  eosio::asset theAssetOut;
  if((int64_t)(user.vigfees / std::pow(10.0, 6)) > 0)
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  eosio::name("doassetout"),
                  std::make_tuple(user.usern, eosio::asset(user.vigfees / std::pow(10.0, 6), VIG4_SYMBOL), VIGFEES, true, false, step0))
      .send();
  for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
    theAssetOut = asset_swap(userCollateral);
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  eosio::name("doassetout"),
                  std::make_tuple(user.usern, theAssetOut, COLLATERAL, true, false, step0))
      .send();
  }
  if(user.l_debt.amount > 0)
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  eosio::name("doassetout"),
                  std::make_tuple(user.usern, user.l_debt, COLLATERAL, true, false, step0))
      .send();
  if((int64_t)(user.savings * std::pow(10.0, VIGOR_SYMBOL.precision())) > 0)
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  eosio::name("doassetout"),
                  std::make_tuple(user.usern, eosio::asset(user.savings * std::pow(10.0, VIGOR_SYMBOL.precision()), VIGOR_SYMBOL), SAVINGS, true, false, step0))
      .send();
  auto stakeduser = _staked.find(user.usern.value);
  if(stakeduser != _staked.end() && stakeduser->stake.amount > 0) {
    _staked.modify(stakeduser, get_self(), [&](auto& modified_user) {
      modified_user.unlocktime = eosio::time_point_sec(eosio::current_time_point());
    });
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  get_self(),
                  eosio::name("doassetout"),
                  std::make_tuple(user.usern, stakeduser->stake, STAKEREFUND, true, false, step0))
      .send();
  }
}
// handles all outbound token transfers (including VIGOR)
void vigorlending::doassetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo, const bool& exec, const bool partialupdate, const uint64_t& step) {
  

  if(_config.getFreezeLevel() == 1 || _config.getFreezeLevel() == 3 || _config.getFreezeLevel() == 4)
    eosio::check(false, "contract under maintenance. freeze level:  " + std::to_string(_config.getFreezeLevel()));

  eosio::check(_g.step == 0, "doassetout: Contract is updating! Step: " + std::to_string(_g.step));

  eosio::require_auth(get_self());
  double totalvalbefore = 0.0;
  double totalval = 0.0;
  exassetout(usern, assetout, memo, exec, partialupdate, step, totalval, totalvalbefore);
}

// execute doassetout
void vigorlending::exassetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo, const bool& exec, const bool partialupdate, const uint64_t& step, double& totalval, double& totalvalbefore) {
  

  eosio::check(_g.step == 0, "exassetout: Contract is updating! Step: " + std::to_string(_g.step));

  auto userIterator = _u.require_find(usern.value, errormsg_missinguser);

  auto finalreserve = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");

  const auto& user = *userIterator;

  double svalueofcole, l_svalueofcole, premiums, l_premiums, dRM, l_dRM, user_l_stresscol, user_stresscol, ul_pctl = 0.0;
  if(partialupdate) {
    user_stresscol = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.volcol)) - 1.0);
    svalueofcole = std::max(user.debt.amount / std::pow(10.0, user.debt.symbol.precision()) - ((1.0 - user_stresscol) * user.valueofcol), 0.0);
    user_l_stresscol = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.l_volcol) - 1.0;
    l_svalueofcole = std::max(((1.0 + user_l_stresscol) * user.l_valueofcol) - user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()), 0.0);
    premiums = user.tesprice * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()));
    l_premiums = user.l_tesprice * user.l_valueofcol;
    dRM = user.svalueofinsx == 0.0 ? 0.0 : std::max(_g.svalueofins - user.svalueofinsx, 0.0);
    l_dRM = user.l_svalueofinsx == 0.0 ? 0.0 : std::max(_g.l_svalueofins - user.l_svalueofinsx, 0.0);
    ul_pctl = l_pctl(user);
  }

  if(assetout.symbol == VIG4_SYMBOL)
    eosio::check(assetout.amount < INT64_MAX / std::pow(10.0, VIG10_SYMBOL.precision() - VIG4_SYMBOL.precision()), "amount too large");
  const eosio::asset& localAssetOut = asset_swap(assetout);

  if(!exec)
    totalvalbefore = _g.valueofcol + _g.valueofins + _g.savings * ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision) + (_g.l_totaldebt.amount / std::pow(10.0, _g.l_totaldebt.symbol.precision())) -
                     _g.l_valueofcol - (_g.totaldebt.amount / std::pow(10.0, _g.totaldebt.symbol.precision()));

  for(const auto& w : _whitelist) {
    t_series stats(_config.getOracleHub(), w.feed.value);
    auto mitr = _market.find(w.sym.raw());
    if(mitr != _market.end()) {
      if(stats.begin()->price.back() != mitr->marketdata.price.back() ||
         stats.begin()->vol != mitr->marketdata.vol)
        _market.modify(mitr, get_self(), [&](auto& m) {
          m.marketdata = *stats.begin();
        });
    } else {
      _market.emplace(get_self(), [&](auto& m) {
        m.sym = w.sym;
        m.marketdata = *stats.begin();
      });
    }
  }

  update(user);

  if(!exec) {
    updateglobal();
    totalval = _g.valueofcol + _g.valueofins + _g.savings * ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision) + (_g.l_totaldebt.amount / std::pow(10.0, _g.l_totaldebt.symbol.precision())) - _g.l_valueofcol - (_g.totaldebt.amount / std::pow(10.0, _g.totaldebt.symbol.precision()));
  }

  const eosio::name& userntrace = usern;
  const uint64_t bailoutid = 0;
  eosio::asset vigneededa;
  double mincol = ( _config.getBailoutCR() / 100 ) + _config.getMinC();
  double mincolup = ( _config.getBailoutupCR() / 100 ) + _config.getMinC();
  double valueofasset = 0.0;
  if(memo == "borrow" && assetout.symbol == VIGOR_SYMBOL) {
    // borrow VIGOR against crypto as collateral
    eosio::asset debt = user.debt + localAssetOut;
    valueofasset = (localAssetOut.amount / std::pow(10.0, localAssetOut.symbol.precision()));

    double lifelinebuffer = 0.0;
    if(!exec && user.vigfees <= 0)
      lifelinebuffer = 0.0003;
    eosio::print("user.valueofcol ", user.valueofcol, "\n");
    eosio::print("mincol ", mincol, "\n");
    eosio::print("0.998 * (debt.amount / std::pow(10.0, debt.symbol.precision()) + lifelinebuffer) ", 0.998 * (debt.amount / std::pow(10.0, debt.symbol.precision()) + lifelinebuffer), "\n");
    eosio::check(user.valueofcol >= mincol * 0.998 * (debt.amount / std::pow(10.0, debt.symbol.precision()) + lifelinebuffer),
                 "Max allowable borrow " + eosio::asset(std::pow(10.0, debt.symbol.precision()) * std::max((user.valueofcol / (mincol)) - ((debt.amount - localAssetOut.amount) / std::pow(10.0, debt.symbol.precision())) - lifelinebuffer, 0.0), VIGOR_SYMBOL).to_string() +
                   ". Collateral Ratio limit " + std::to_string(mincol));

    bool debtCeiling = false;
    auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
    currency_statst _currency_statst(symbolIterator->contract, VIGOR_SYMBOL.code().raw());
    auto existing = _currency_statst.find(VIGOR_SYMBOL.code().raw());
    if(existing != _currency_statst.end()) {
      const auto& st = *existing;
      if(st.supply.amount + assetout.amount > _config.getDebtCeiling() * std::pow(10.0, VIGOR_SYMBOL.precision()))
        debtCeiling = true;
    }
    eosio::check(!debtCeiling, " Debt ceiling exceeded, cannot issue more VIGOR.");

    if(!exec)
      totalval -= (localAssetOut.amount / std::pow(10.0, localAssetOut.symbol.precision()));

    if(exec) {
      _g.volume += (localAssetOut.amount / std::pow(10.0, localAssetOut.symbol.precision()));
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.debt = debt;
        //modified_user.txnvolume += localAssetOut.amount / std::pow( 10.0, localAssetOut.symbol.precision() );
      });

      _g.totaldebt += localAssetOut;

      auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
      eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                    symbolIterator->contract,  // contract for vigor token
                    eosio::name("issue"),
                    std::make_tuple(get_self(), localAssetOut, localAssetOut.to_string() + std::string(" issued to ") + get_self().to_string()))
        .send();
#endif
      eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                    symbolIterator->contract,  // contract for vigor token
                    eosio::name("transfer"),
                    std::make_tuple(get_self(), usern, localAssetOut, localAssetOut.to_string() + std::string(" transferred to ") + usern.to_string()))
        .send();
      std::string message = user.usern.to_string() + " borrow VIGOR against crypto as collateral: " + localAssetOut.to_string();
      tracetable(__func__, __LINE__, userntrace, bailoutid, message);
    }
  } else {
    if(memo == "insurance") {
      //withdraw tokens from insurance pool (can also be VIGOR)
      // auto insurance = getAsset( user.insurance, localAssetOut );
      auto userInsurances = user.getInsurances();
      eosio::asset insurance;
      if(!userInsurances.hasAsset(localAssetOut, insurance))
        eosio::check(false, "Insurance asset not found in user.");

      eosio::check(insurance.amount >= localAssetOut.amount, "Insufficient insurance assets available in user.");

      valueofasset = localAssetOut.amount / std::pow(10.0, insurance.symbol.precision());
      auto market = _market.get(symbol_swap(localAssetOut.symbol).raw());

      valueofasset *= (double)market.marketdata.price.back() / pricePrecision;

      auto userIter = _acctsize.find(user.usern.value);
      if(userIter != _acctsize.end()) {
        eosio::check(user.txnvolume + valueofasset <= userIter->limit, "Total daily outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
        eosio::check(valueofasset <= userIter->limit, "Outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
      } else {
        eosio::check(user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens.");
        eosio::check(valueofasset <= _config.getDataB(), "Outbound transfers cannot exceed " + std::to_string(_config.getDataB()) + " dollars worth of tokens.");
      }

      auto globalInsurances = _g.getInsurances();
      eosio::asset globalinsurance;
      eosio::asset lendavail;
      if(!globalInsurances.hasAsset(localAssetOut, globalinsurance)) {
        lendavail = eosio::asset(0, symbol_swap(localAssetOut.symbol));
        eosio::check(false, "Withdrawal blocked, try later. Too many tokens lent out from the lending pool at this time. Available: " + lendavail.to_string());
      }
      lendavail = asset_swap(globalinsurance);
      eosio::check(globalinsurance.amount >= localAssetOut.amount, "Withdrawal blocked, try later. Too many tokens lent out from the lending pool at this time. Available: " + lendavail.to_string());

      if(!exec) totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        eosio::asset dust = insurance - asset_swap(asset_swap(insurance));
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.getInsurances() -= dust;
        });
        _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
          modified_user.getInsurances() += dust;
        });
        _u.modify(user, get_self(), [&](auto& modified_user) {
          getInsurances(modified_user) -= localAssetOut;
          modified_user.txnvolume += valueofasset;
        });
        std::string message = user.usern.to_string() + " withdraw tokens from lending/insurance pool " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    }
    if(memo == "savings") {
      //withdraw VIGOR tokens from savings
      eosio::asset savings = eosio::asset((user.savings * std::pow(10.0, VIGOR_SYMBOL.precision())), VIGOR_SYMBOL);
      eosio::check(savings.amount >= localAssetOut.amount, "Insufficient savings assets available in user.");

      auto market = _market.get(VIGOR_SYMBOL.raw());
      double localAssetOutDouble = localAssetOut.amount / std::pow(10.0, localAssetOut.symbol.precision());
      valueofasset = localAssetOutDouble * (double)market.marketdata.price.back() / pricePrecision;

      auto userIter = _acctsize.find(user.usern.value);
      if(userIter != _acctsize.end()) {
        eosio::check(user.txnvolume + valueofasset <= userIter->limit, "Total daily outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
        eosio::check(valueofasset <= userIter->limit, "Outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
      } else {
        eosio::check(user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens.");
        eosio::check(valueofasset <= _config.getDataB(), "Outbound transfers cannot exceed " + std::to_string(_config.getDataB()) + " dollars worth of tokens.");
      }

      if(!exec)
        totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        double dust = user.savings - localAssetOutDouble;
        if(savings.amount - localAssetOut.amount == 0 && dust < 0.0001 && dust > 0.0) {
          _u.modify(user, get_self(), [&](auto& modified_user) {
            modified_user.savings = 0.0;
          });
          auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
          _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
            modified_user.savings += dust;
          });
        }
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.savings = std::max(modified_user.savings - localAssetOutDouble, 0.0);
          _g.savings = std::max(_g.savings - localAssetOutDouble, 0.0);
          modified_user.txnvolume += valueofasset;
        });
        std::string message = user.usern.to_string() + " withdraw tokens from savings " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    } else if(memo == "collateral" && assetout.symbol != VIGOR_SYMBOL) {
      // withdraw tokens from collateral
      auto collaterals = user.getCollaterals();
      eosio::asset collateral;

      if(!collaterals.hasAsset(localAssetOut, collateral))
        eosio::check(false, "Collateral asset not found in user.");

      valueofasset = localAssetOut.amount / std::pow(10.0, collateral.symbol.precision());
      auto market = _market.get(symbol_swap(localAssetOut.symbol).raw());

      valueofasset *= (double)market.marketdata.price.back() / pricePrecision;
      double valueofcol = user.valueofcol - valueofasset;

      eosio::check(collateral.amount >= localAssetOut.amount, "Insufficient collateral assets available in user.");

      if(!(valueofcol >= mincol * 0.998 * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())))) {
        eosio::asset x = eosio::asset(std::pow(10.0, collateral.symbol.precision()) * (std::min((collateral.amount / std::pow(10.0, collateral.symbol.precision())) * ((double)market.marketdata.price.back() / pricePrecision), std::max((user.valueofcol - mincol * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()))), 0.0)) / ((double)market.marketdata.price.back() / pricePrecision)), collateral.symbol);
        eosio::check(false, "Max allowable collateral withdraw " + x.to_string() +
                              ". Collateral Ratio limit " + std::to_string(mincol));
      }

      auto userIter = _acctsize.find(user.usern.value);
      if(userIter != _acctsize.end()) {
        eosio::check(user.txnvolume + valueofasset <= userIter->limit, "Total daily outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
        eosio::check(valueofasset <= userIter->limit, "Outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
      } else {
        eosio::check(user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens.");
        eosio::check(valueofasset <= _config.getDataB(), "Outbound transfers cannot exceed " + std::to_string(_config.getDataB()) + " dollars worth of tokens.");
      }

      if(!exec)
        totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        eosio::asset dust = collateral - asset_swap(asset_swap(collateral));
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.getCollaterals() -= dust;
        });
        _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
          modified_user.getCollaterals() += dust;
        });
        _u.modify(user, get_self(), [&](auto& modified_user) {
          getCollaterals(modified_user) -= localAssetOut;
          modified_user.txnvolume += valueofasset;
        });
        std::string message = user.usern.to_string() + " withdraw tokens from collateral pool " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    } else if(memo == "vigfees" && assetout.symbol == VIG4_SYMBOL) {
      // withdraw tokens from vigfees
      eosio::asset vigfees = eosio::asset(user.vigfees, VIG10_SYMBOL);

      valueofasset = localAssetOut.amount / std::pow(10.0, vigfees.symbol.precision());
      auto market = _market.get(symbol_swap(localAssetOut.symbol).raw());

      valueofasset *= (double)market.marketdata.price.back() / pricePrecision;

      eosio::check(vigfees.amount >= localAssetOut.amount, "Insufficient vigfees assets available in user.");

      auto userIter = _acctsize.find(user.usern.value);
      if(userIter != _acctsize.end()) {
        eosio::check(user.txnvolume + valueofasset <= userIter->limit, "Total daily outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
        eosio::check(valueofasset <= userIter->limit, "Outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
      } else {
        eosio::check(user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens.");
        eosio::check(valueofasset <= _config.getDataB(), "Outbound transfers cannot exceed " + std::to_string(_config.getDataB()) + " dollars worth of tokens.");
      }

      if(!exec)
        totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        if((vigfees.amount - localAssetOut.amount) < 1000000 && (vigfees.amount - localAssetOut.amount) > 0) {
          eosio::asset dust = vigfees - localAssetOut;
          _u.modify(user, get_self(), [&](auto& modified_user) {
            modified_user.vigfees -= dust.amount;
            _g.vigfees -= dust.amount;
          });
          _u.modify(finalreserve, get_self(), [&](auto& modified_user) {
            getCollaterals(modified_user) += dust;
          });
        }
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.vigfees -= localAssetOut.amount;
          _g.vigfees -= localAssetOut.amount;
          modified_user.txnvolume += valueofasset;
        });
        std::string message = user.usern.to_string() + " withdraw tokens from vigfees " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    } else if(memo == "borrow" && assetout.symbol != VIGOR_SYMBOL) {
      // borrow tokens against VIGOR as collateral.

      // Example: user has 50 VIGOR in l_debt as collateral against which user requests borrow 10 EOS transfered out
      // 10 EOS gets booked into user (and global) l_collateral as a borrow, and 10 EOS subtracted from global insurance

      auto market = _market.get(symbol_swap(localAssetOut.symbol).raw());
      valueofasset = (localAssetOut.amount) / std::pow(10.0, localAssetOut.symbol.precision()) *
                               ((double)market.marketdata.price.back() / pricePrecision);

      //eosio::check( user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens." );
      double lifelinebuffer = 0.0;
      if(!exec && user.vigfees <= 0)
        lifelinebuffer = 0.0003;

      if(!(user.l_valueofcol + valueofasset + lifelinebuffer <= (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) / (mincolup * 0.998))) {
        eosio::asset x = eosio::asset(std::pow(10.0, localAssetOut.symbol.precision()) * (std::max(((user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) / mincolup - user.l_valueofcol), 0.0) / ((double)market.marketdata.price.back() / pricePrecision)), localAssetOut.symbol);
        eosio::check(false, "Max allowable crypto borrow " + x.to_string() +
                              ". Collateral Ratio limit " + std::to_string(mincolup));
      }

      // can't borrow from the finalreserve for new borrows, so record the amount to remove it from _g.insurance
      eosio::asset reserveAsset;
      eosio::asset lr = eosio::asset(0, localAssetOut.symbol);
      if(finalreserve->getInsurances().hasAsset(localAssetOut, reserveAsset))
        lr.amount = reserveAsset.amount;

      // find lends_outstanding in global l_collateral
      auto globalLCollaterals = _g.getLCollaterals();
      eosio::asset globalLCollateralAsset;
      eosio::asset lends_outstanding = eosio::asset(0, localAssetOut.symbol);
      if(globalLCollaterals.hasAsset(localAssetOut, globalLCollateralAsset))
        lends_outstanding.amount = globalLCollateralAsset.amount;

      // locate the requested asset in the global insurance for lending
      auto globalInsurances = _g.getInsurances();
      eosio::asset globalInsuranceAsset;

      if(!globalInsurances.hasAsset(localAssetOut, globalInsuranceAsset))
        eosio::check(false, "Token not available for borrowing.");

      auto symbolIterator = _whitelist.find(symbol_swap(localAssetOut.symbol).raw());

      int64_t maxBorrowAmount =
        (globalInsuranceAsset.amount - lr.amount + lends_outstanding.amount) *
          (symbolIterator->maxlends / 100.0) -
        (lends_outstanding.amount + localAssetOut.amount);

      eosio::asset remborrow = eosio::asset(std::max(localAssetOut.amount + maxBorrowAmount, (int64_t)0) * std::pow(10.0, symbol_swap(localAssetOut.symbol).precision()-localAssetOut.symbol.precision()), symbol_swap(localAssetOut.symbol));
      eosio::check(maxBorrowAmount >= 0, std::string("Can't locate enough to borrow. Remaining available to borrow: " + remborrow.to_string()));

      if(!exec)
        totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        _u.modify(user, get_self(), [&](auto& modified_user) {
          getLCollaterals(modified_user) += localAssetOut;
          //modified_user.txnvolume += valueofasset;
        });
      }

      // subtract the located asset from the global insurance
      eosio::check(globalInsuranceAsset.amount >= localAssetOut.amount, "Insufficient insurance assets available in global.");

      if(exec) {
        globalInsurances -= localAssetOut;
        std::string message = user.usern.to_string() + " borrows tokens against VIGOR " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
      // don't subtract the located asset from any particular insurer, just subtract from global insurance
    } else if(memo == "collateral" && assetout.symbol == VIGOR_SYMBOL) {
      // withdraw VIGORs from l_debt
      eosio::check(user.l_debt.amount >= localAssetOut.amount, "Insufficient collateral assets available in user.");

      eosio::asset l_debt = user.l_debt - localAssetOut;

      // if overcollateralization is C then leverage L = 1 / ( 1 - ( 1 / C ) )
      eosio::check(user.l_valueofcol * mincolup * 0.998 <= (l_debt.amount / std::pow(10.0, l_debt.symbol.precision())),
                   "Max allowable collateral withdraw " + eosio::asset(std::pow(10.0, l_debt.symbol.precision()) * std::max(((user.l_debt.amount / std::pow(10.0, l_debt.symbol.precision())) - (user.l_valueofcol * mincolup)), 0.0), VIGOR_SYMBOL).to_string() +
                     ". Collateral Ratio limit " + std::to_string(mincolup));

      valueofasset = (localAssetOut.amount / std::pow(10.0, localAssetOut.symbol.precision()));

      auto userIter = _acctsize.find(user.usern.value);
      if(userIter != _acctsize.end()) {
        eosio::check(user.txnvolume + valueofasset <= userIter->limit, "Total daily outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
        eosio::check(valueofasset <= userIter->limit, "Outbound transfers cannot exceed " + std::to_string(userIter->limit) + " dollars worth of tokens.");
      } else {
        eosio::check(user.txnvolume + valueofasset <= _config.getDataA(), "Total daily outbound transfers cannot exceed " + std::to_string(_config.getDataA()) + " dollars worth of tokens.");
        eosio::check(valueofasset <= _config.getDataB(), "Outbound transfers cannot exceed " + std::to_string(_config.getDataB()) + " dollars worth of tokens.");
      }

      if(!exec)
        totalval -= valueofasset;

      if(exec) {
        _g.volume += valueofasset;
        _u.modify(userIterator, get_self(), [&](auto& modified_user) {
          modified_user.l_debt = l_debt;
          modified_user.txnvolume += valueofasset;
        });

        _g.l_totaldebt -= localAssetOut;

        std::string message = user.usern.to_string() + " withdraw VIGOR from l_debt " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    } else if(memo == "stakerefund" && localAssetOut.symbol == _g.fee.symbol) {
      auto stakeduser = _staked.find(usern.value);
      eosio::check(stakeduser != _staked.end(), "User not found in staking table");
      eosio::check(stakeduser->stake.amount - localAssetOut.amount >= 0, "Insufficient funds");
      eosio::check(stakeduser->unlocktime <= eosio::time_point_sec(eosio::current_time_point()), "Too early, account deposit still locked");
      if(exec) {
        if(stakeduser->stake.amount - localAssetOut.amount == 0)
          _staked.erase(stakeduser);
        else
          _staked.modify(stakeduser, get_self(), [&](auto& st) {
            st.stake -= localAssetOut;
          });
        std::string message = user.usern.to_string() + " withdraw from stakerefund " + localAssetOut.to_string();
        tracetable(__func__, __LINE__, userntrace, bailoutid, message);
      }
    }

    if(_config.getRexSwitch() >= 1 && localAssetOut.symbol == SYSTEM_TOKEN_SYMBOL) {
      if(exec) sellrex(localAssetOut);
      else {
        auto rexitr = _rex_pool_table.begin();
        const int64_t lent = rexitr->total_lent.amount;
        const int64_t S0 = rexitr->total_lendable.amount;
        eosio::check((double)lent / S0 < 0.898, "insufficient available rex due to rex liqudity crunch");
        const int64_t R0 = rexitr->total_rex.amount;
        const eosio::asset rex = eosio::asset(((uint128_t)(localAssetOut.amount + 1) * R0) / S0, eosio::symbol("REX", 4));
        auto bitr = _rex_balance_table.require_find(get_self().value, " user not found in the rex balance table.");
        eosio::check(bitr->vote_stake.amount > 0, "the rex balance table shows zero vote_stake");
        eosio::check(rex.amount <= bitr->matured_rex, "insufficient available rex");
      }
    }

    if(exec) {
      const eosio::asset& lastAssetOut = asset_swap(localAssetOut);
      auto symbolIterator = _whitelist.find(lastAssetOut.symbol.raw());
      if(lastAssetOut.is_valid() && lastAssetOut.amount > 0) {
        eosio::action(eosio::permission_level {get_self(), "active"_n},
                      symbolIterator->contract,
                      "transfer"_n,
                      std::make_tuple(get_self(), usern, lastAssetOut, memo))
          .send();
      }
    }
  }

  if(exec) {
    takefee(user, std::min(std::max(valueofasset * _config.getTxnFee() / 10000.0, 0.0), 10.0));
    switch(_config.getExecType()) {
      case 1: {  //deferred transactions
        break;
      }
      case 2:  //mining

        if(partialupdate) {  // partial update
          auto market = _market.get(VIG4_SYMBOL.raw());
          double vigprice = market.marketdata.price.back() / pricePrecision;
          double vigvol = market.marketdata.vol / volPrecision;
          update(user);
          updateglobal();
          double svalueofcoleavg = _g.svalueofcoleavg;
          _g.svalueofcole -= svalueofcole;
          stresscol(user);
          _g.svalueofcoleavg = svalueofcoleavg;
          double l_svalueofcoleavg = _g.l_svalueofcoleavg;
          _g.l_svalueofcole -= l_svalueofcole;
          l_stresscol(user);
          _g.l_svalueofcoleavg = l_svalueofcoleavg;
          stressins();
          l_stressins();
          risk();
          l_risk();
          _g.premiums -= premiums;
          pricing(user);
          _g.l_premiums -= l_premiums;
          l_pricing(user);
          _g.rm -= dRM;
          _g.l_rm -= l_dRM;
          stressinsx(user);
          RM(user);
          l_RM(user);
          _g.pcts -= user.pcts;
          _g.l_pcts -= user.l_pcts;
          _b.l_pctl -= ul_pctl;
          pcts(user);
          l_pcts(user);
          reserve();
          performance(user, vigvol, vigprice);
          performanceglobal();
        } else {
          _g.step = step;
          _b.usern = _u.begin()->usern;
        }

        break;
      case 3: {  //inline
        break;
      }
    }
  }
}

void vigorlending::sellrex(eosio::asset asset){
        auto rexitr = _rex_pool_table.begin();
        const int64_t lent = rexitr->total_lent.amount;
        const int64_t S0 = rexitr->total_lendable.amount;
        const int64_t R0 = rexitr->total_rex.amount;
        const eosio::asset rex = eosio::asset(((uint128_t)(asset.amount + 1) * R0) / S0, eosio::symbol("REX", 4));
        if((double)lent / S0 < 0.898 && rex.amount > 0) {
          double rexprice1 = 1.0;
          uint64_t vote_stake1 = 0;
          auto bitr = _rex_balance_table.find(get_self().value);
          if(bitr != _rex_balance_table.end() && bitr->vote_stake.amount > 0 && bitr->rex_balance.amount > 0) {
            vote_stake1 = bitr->vote_stake.amount + 1;
            rexprice1 = (double)vote_stake1 / (bitr->rex_balance.amount / std::pow(10.0, 4));
          }
          eosio::action(eosio::permission_level {get_self(), "active"_n},
                        eosio::name("eosio"),
                        eosio::name("sellrex"),
                        std::make_tuple(get_self(), rex))
            .send();
          eosio::action(eosio::permission_level {get_self(), "active"_n},
                        eosio::name("eosio"),
                        eosio::name("withdraw"),
                        std::make_tuple(get_self(), asset))
            .send();
          rexitr = _rex_pool_table.begin();
          double rexprice2 = double(rexitr->total_lendable.amount) / (rexitr->total_rex.amount / std::pow(10.0, 4));
          _g.rexproxy += vote_stake1 * (std::max(rexprice2 / rexprice1, 1.0) - 1.0);  // accumulate REX rewards
          auto vitr = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
          _u.modify(vitr, get_self(), [&](auto& modified_user) {
              getCollaterals(modified_user) += eosio::asset(vote_stake1 * (std::max(rexprice2 / rexprice1, 1.0) - 1.0), SYSTEM_TOKEN_SYMBOL);
          });
        }
}

double vigorlending::portfolioVariance(asset_container_t& container, double valueOf) {
  
  if(valueOf <= 0.0)
    return 0.0;

  double portVariance = 0.0;
  int i = 0;
  const statspre* m[container.size()];  //market
  double w[container.size()];           // weight
  for(auto asset = container.begin(); asset != container.end(); ++asset) {
    if(i == 0) {
      m[i] = &(_market.get(symbol_swap(asset->symbol).raw()).marketdata);
      w[i] = ((*m[i]).price.back() / pricePrecision) * (asset->amount / std::pow(10.0, asset->symbol.precision())) / valueOf;
    }
    int j = i;
    double p = 0.0;
    for(auto nextAsset = asset + 1; nextAsset != container.end(); ++nextAsset) {
      j++;
      if(i == 0) {
        m[j] = &(_market.get(symbol_swap(nextAsset->symbol).raw()).marketdata);
        w[j] = ((*m[j]).price.back() / pricePrecision) * (nextAsset->amount / std::pow(10.0, nextAsset->symbol.precision())) / valueOf;
      }
      double c = (*m[i]).correlation_matrix.at(symbol_swap(nextAsset->symbol)) / corrPrecision;  // corr matrix has VIG 4 not VIG 10
      p += w[j] * c * (*m[j]).vol;
    }
    portVariance += 2.0 * w[i] * ((*m[i]).vol / volPrecision) * (p / volPrecision) + std::pow(w[i], 2) * std::pow((*m[i]).vol / volPrecision, 2);
    i++;
  }
  return portVariance;
}

double vigorlending::portfolioVariancex(asset_container_t& container, double valueOf, const user_s& user, double& user_valueofins_adj) {
  
  if(valueOf <= 0.0)
    return 0.0;

  eosio::asset userInsurance;
  eosio::asset gLCollateral;
  double portVariance = 0.0;
  int i = 0;
  const statspre* m[container.size()];  //market
  double w[container.size()];           // weight
  asset_container userInsurances = user.getInsurances();
  asset_container gLCollaterals = _g.getLCollaterals();
  for(auto asset = container.begin(); asset != container.end(); ++asset) {
    if(i == 0) {
      m[i] = &(_market.get(symbol_swap(asset->symbol).raw()).marketdata);
      if(userInsurances.hasAsset(*asset, userInsurance)) {
        user_valueofins_adj = ((*m[i]).price.back() / pricePrecision) * (userInsurance.amount / std::pow(10.0, asset->symbol.precision()));
        if(gLCollaterals.hasAsset(*asset, gLCollateral))
          user_valueofins_adj *= (1.0 - (((double)gLCollateral.amount) / (asset->amount + gLCollateral.amount)));  // reduce by lent percent. _g.valueofins exludes lent tokens (which get accounted for in _g.l_collateral), so need to remove lent tokens amount from user.valueofins
      }
      w[i] = std::max((((*m[i]).price.back() / pricePrecision) * (asset->amount / std::pow(10.0, asset->symbol.precision())) - user_valueofins_adj), 0.0) / valueOf;
    }
    int j = i;
    double p = 0.0;
    for(auto nextAsset = asset + 1; nextAsset != container.end(); ++nextAsset) {
      double ua = 0.0;
      j++;
      if(i == 0) {
        m[j] = &(_market.get(symbol_swap(nextAsset->symbol).raw()).marketdata);
        if(userInsurances.hasAsset(*nextAsset, userInsurance)) {
          ua = ((*m[j]).price.back() / pricePrecision) * (userInsurance.amount / std::pow(10.0, nextAsset->symbol.precision()));
          if(gLCollaterals.hasAsset(*nextAsset, gLCollateral))
            ua *= (1.0 - (((double)gLCollateral.amount) / (nextAsset->amount + gLCollateral.amount)));  // reduce by lent percent
        }
        w[j] = std::max((((*m[j]).price.back() / pricePrecision) * (nextAsset->amount / std::pow(10.0, nextAsset->symbol.precision())) - ua), 0.0) / valueOf;
        user_valueofins_adj += ua;
      }
      double c = (*m[i]).correlation_matrix.at(symbol_swap(nextAsset->symbol)) / corrPrecision;  // corr matrix has VIG 4 not VIG 10
      p += w[j] * c * (*m[j]).vol;
    }
    portVariance += 2.0 * w[i] * ((*m[i]).vol / volPrecision) * (p / volPrecision) + std::pow(w[i], 2) * std::pow((*m[i]).vol / volPrecision, 2);
    i++;
  }
  return portVariance;
}

//purpose is to stress test the collateral portfolio of each user, as measured by CVaR or expected shortfall
//then record for each user the amount by which the stressed collateral value would drop below the value of user debt, then aggregate for all users
//inputs required from the oracle: time series of historical returns for each token, volatility, and correlations
//example: stresscol = 0.6 would mean that 60% is the average loss expected when a rare stress event happens, rare defined as an event with probability (1-alphatest)
//example: svalueofcole = 50 means that in a stress event the user collateral value would drop below the value of debt by $50, insufficient by $50
void vigorlending::stresscol(const user_s& user) {
  
  if(user.valueofcol <= 0.0) {
    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(user.volcol != 0.0)
        modified_user.volcol = 0.0;
      //if ( user.stresscol != 0.0 )
      //    modified_user.stresscol = 0.0;
    });
  } else {
    double portVariance = portfolioVariance(*user.getCollaterals(), user.valueofcol);

    double stresscol = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(portVariance))) - 1.0);

    double svalueofcol = ((1.0 - stresscol) * user.valueofcol);                                                                                    // model suggested dollar value of the user collateral portfolio in a stress event.
    double svalueofcole = std::max(user.debt.amount / std::pow(10.0, user.debt.symbol.precision()) - ((1.0 - stresscol) * user.valueofcol), 0.0);  // model suggested dollar amount of insufficient collateral of a user loan in a stressed market.

    _g.svalueofcole += svalueofcole;  // model suggested dollar value of the sum of all insufficient collateral in a stressed market

    double stresscolavg = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(0.5), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - 0.5)) * std::sqrt(portVariance))) - 1.0);
    double svalueofcoleavg = std::max(user.debt.amount / std::pow(10.0, user.debt.symbol.precision()) - ((1.0 - stresscolavg) * user.valueofcol), 0.0);  // model suggested dollar amount of insufficient collateral of a user loan on average in stressed markets, expected loss

    _g.svalueofcoleavg += svalueofcoleavg;  // model suggested dollar value of the sum of all insufficient collateral on average in stressed markets, expected loss

    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(modified_user.volcol != std::sqrt(portVariance))
        modified_user.volcol = std::sqrt(portVariance);  // volatility of the user collateral portfolio
                                                         //  modified_user.stresscol = stresscol; // model suggested percentage loss that the user collateral portfolio would experience in a stress event.
    });
  }
}

void vigorlending::l_stresscol(const user_s& user) {
  
  if(user.l_valueofcol <= 0.0) {
    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(user.l_volcol != 0.0)
        modified_user.l_volcol = 0.0;
      //if ( user.l_stresscol != 0.0 )
      //    modified_user.l_stresscol = 0.0;
    });
  } else {
    double l_portVariance = portfolioVariance(*user.getLCollaterals(), user.l_valueofcol);

    double l_stresscol = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(l_portVariance)) - 1.0;

    double l_svalueofcol = ((1.0 + l_stresscol) * user.l_valueofcol);                                                                                        // model suggested dollar value of the user collateral portfolio in a stress event.
    double l_svalueofcole = std::max(((1.0 + l_stresscol) * user.l_valueofcol) - user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()), 0.0);  // model suggested dollar amount of insufficient collateral of a user loan in a stressed market.

    _g.l_svalueofcole += l_svalueofcole;  // model suggested dollar value of the sum of all insufficient collateral in a stressed market

    double l_stresscolavg = std::exp(((std::exp(-1.0 * (std::pow(icdf(0.5), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - 0.5)) * std::sqrt(l_portVariance)) - 1.0;
    double l_svalueofcoleavg = std::max(((1.0 + l_stresscolavg) * user.l_valueofcol) - (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())), 0.0);  // model suggested dollar amount of insufficient collateral of a user loan on average in stressed markets, expected loss

    _g.l_svalueofcoleavg += l_svalueofcoleavg;  // model suggested dollar value of the sum of all insufficient collateral on average in stressed markets, expected loss

    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(modified_user.l_volcol != std::sqrt(l_portVariance))
        modified_user.l_volcol = std::sqrt(l_portVariance);  // volatility of the user collateral portfolio
                                                             //modified_user.l_stresscol = l_stresscol; // model suggested percentage loss that the user collateral portfolio would experience in a stress event.
    });
  }
}

//purpose is to stress test the global insurance portfolio, as measured by CVaR or expected shortfall, for downside market stress events
//inputs required from the oracle: time series of historical returns for each token, volatility, and correlations
//example: stressins = 0.6 would mean that 60% is the average loss expected when a rare stress event happens, rare defined as an event with probability (1-alphatest)
void vigorlending::stressins() {
  
  if(_g.valueofins <= 0.0)
    _g.svalueofins = 0.0;
  else {
    double portVariance = portfolioVariance(*_g.getInsurances(), _g.valueofins);
    double stressins = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(portVariance))) - 1.0);  // model suggested percentage loss that the total insurance asset portfolio would experience in a stress event.
    _g.svalueofins = (1.0 - stressins) * _g.valueofins;                                                                                                                                                                  // model suggested dollar value of the total insurance asset portfolio in a stress event.
                                                                                                                                                                                                                         //double stressinsavg = -1.0 * ( std::exp( -1.0 * ( ( ( std::exp( -1.0 * ( std::pow( icdf( 0.5 ), 2.0 ) ) / 2.0 ) / ( std::sqrt( 2.0 * M_PI ) ) ) / ( 1.0 - 0.5 ) ) * std::sqrt( portVariance ) ) ) - 1.0 ); // model suggested percentage loss that the total insurance asset portfolio would experience in a stress event.
  }
}

//purpose is to stress test the global l_insurance portfolio, as measured by CVaR or expected shortfall, for upside market stress events
//inputs required from the oracle: time series of historical returns for each token, volatility, and correlations
//example: l_stressins = 0.6 would mean that 60% is the average loss expected when a rare stress event happens, rare defined as an event with probability (1-alphatest)
void vigorlending::l_stressins() {
  
  if(_g.valueofins <= 0.0)
    _g.l_svalueofins = 0.0;
  else {
    double portVariance = portfolioVariance(*_g.getInsurances(), _g.valueofins);
    double l_stressins = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(portVariance)) - 1.0;  // model suggested percentage gain that the total insurance asset portfolio would experience in an upside stress event.
    _g.l_svalueofins = (1.0 + l_stressins) * _g.valueofins;                                                                                                                                              // model suggested dollar value of the total insurance asset portfolio in a stress event.
                                                                                                                                                                                                         //double l_stressinsavg = std::exp( ( ( std::exp( -1.0 * ( std::pow( icdf( 0.5 ), 2.0 ) ) / 2.0 ) / ( std::sqrt( 2.0 * M_PI ) ) ) / ( 1.0 - 0.5 ) ) * std::sqrt( portVariance ) ) - 1.0; // model suggested percentage gain that the total insurance asset portfolio would experience in an upside stress event.
  }
}

//pricing and stability is a function of risk bugeting, every users transaction is a credit or debit to the risk budget, and pricing adjusts to incentivise healthy risk levels
//purpose of risk() is to measure solvency (a measure of risk) of the entire system, for downside market stress events.
//given the stress testing results for both collateral and insurance, a solvency capital requirement is calculated
//i.e. solvency of 0.9 means that currently the capital available in the system is inadequate compare to the requirement.
//pricing is scaled up when solvency is below requirement to attract insurers and disincentivise borrowers
//pricing is scaled down when solvency is above requirement to attract borrowers and disincentivise insurers
//solvency, represents the current level of capital adequacy available to back the VIGOR relative to the risk requirement
void vigorlending::risk() {
  
  // market value of assets and liabilities from the perspective of insurers

  // normal markets
  double mva_n = _g.valueofins;  //market value of insurance assets in normal markets, includes the reserve which is implemented as an insurer, collateral is not an asset of the insurers
  double mvl_n = 0;              // no upfront is paid for tes, and insurers can walk away at any time, debt is not a liability of the insurers

  //stressed markets
  double mva_s = _g.svalueofins;
  double mvl_s = _g.svalueofcole;

  double own_n = mva_n - mvl_n;  // own funds normal markets
  double own_s = mva_s - mvl_s;  // own funds stressed markets
  //double own_s = std::min( mva_s - mvl_s, own_n * 0.99 ); // own funds stressed markets

  double scr = own_n - own_s;  // solvency capial requirement is the amount of insurance assets required to survive a sress event

  double solvency = scr > 0.0 ? own_n / scr : 0.0;  // solvency, represents the current level of capital adequacy available to back the VIGOR relative to the requirement

  _g.solvency = solvency;
  _g.scale = std::max(0.5 + 0.5 * std::min(_config.getSolvencyTarget() / std::max(solvency, 0.001), _config.getMaxTesScale()), _config.getMinTesScale());
}

//purpose of l_risk() is to measure solvency (a measure of risk) of the entire system, for upside market stress events.
void vigorlending::l_risk() {
  
  // market value of assets and liabilities from the perspective of insurers

  // normal markets
  double mva_n = _g.valueofins;  //market value of insurance assets in normal markets, includes the reserve which is implemented as an insurer, collateral is not an asset of the insurers
  double mvl_n = 0;              // no upfront is paid for tes, and insurers can walk away at any time, debt is not a liability of the insurers

  //stressed markets
  double mva_s = _g.l_svalueofins;
  double mvl_s = _g.l_svalueofcole;

  double own_n = mva_n - mvl_n;                           // own funds normal markets
  double own_s = std::max(mva_s - mvl_s, own_n / 100.0);  // own funds stressed markets

  double l_scr = own_n + own_s;  // solvency capial requirement is the amount of insurance assets required to survive a stress event

  double l_solvency = own_n > 0.0 ? (l_scr - own_n) / own_n : 0.0;  // solvency, represents capital adequacy to back the outstanding risky token borrows

  _g.l_solvency = l_solvency / _config.getLSolvencyTarget();
  _g.l_scale = std::max(0.2 + 0.5 * std::min(_config.getLSolvencyTarget() / std::max(l_solvency, 0.001), _config.getMaxTesScale()), _config.getMinTesScale());
}

//pricing() calculates the borrow rate for VIGOR borrows, tesprice
//the model it is an out of the money cash or nothing binary option pricer
//pricing is scaled up/down if solvency is below/above target set by custodians
//the purpose of pricing() is to provide a pricing curve accross moneyness, and scale the curve based on system risk
//this enables market price discovery as users transact, removing the need for individual quoting like an options montage
//borrowers of VIGOR pay a floating borrow rate, the rate dynamically adjusts with changes in system risk
void vigorlending::pricing(const user_s& user) {
  
  // premium payments in exchange for contingient payoff in the event that a price threshhold is breached to the downside

  double ivol = user.volcol * _config.getCalibrate() * _g.scale;  // market determined implied volaility

  double stresscol = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.volcol)) - 1.0);

  double istresscol = -1.0 * (std::exp(-1.0 * (-1.0 * std::log(1.0 + stresscol * -1.0) * _config.getCalibrate() * _g.scale)) - 1.0);

  double lp = (_config.getBailoutCR() / 100.0 - 1.0 ) * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())); // liquidation penalty

  double payoffx = std::max(std::max(0.0, 1.0 * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) - user.valueofcol * (1.0 - istresscol)) - lp * _config.getLiqCut(), 0.0); // reduce the value of the put option by the vigordacfund's share of the expected value of liquidation penalties

  double payoff = std::max(std::max(0.0, 1.0 * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) - user.valueofcol * (1.0 - istresscol)) - lp, 0.0); // reduce the value of the put option by the full expected value of the liquidation penalties

  double T = 1.0;

  double d = ((std::log(user.valueofcol / ((_config.getBailoutCR() / 100.0) * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()))))) + (-std::pow(ivol, 2) / 2.0) * T * _config.getDataC()) / (ivol * std::sqrt(T * _config.getDataC()));

  double discount = 1.0 - user.getReputation_pct() * _config.getMaxDisc();

  double tesprice = 0.0;
  double tespricex = 0.0;
  if(user.debt.amount > 0) {
    tesprice = discount * std::min(std::max(_config.getMinTesPrice(), ((payoff * std::erfc(d / std::sqrt(2.0)) / 2.0) / (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())))), _config.getMaxTesPrice());
    tespricex = discount * std::min(std::max(_config.getMinTesPrice(), ((payoffx * std::erfc(d / std::sqrt(2.0)) / 2.0) / (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())))), _config.getMaxTesPrice()); // note: expected value of insurers share of liquidation penalties = (tespricex - tesprice)
  }

  double premiums = tespricex * (user.debt.amount / std::pow(10.0, user.debt.symbol.precision()));

  _g.premiums += premiums;  // total dollar amount of premiums all borrowers would pay in one year to insure their collateral (and includes expected liquidation penalties paid)

  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(modified_user.tesprice != tesprice * (1.0 - _config.getRateSub())) // vigordacfund paying for getRateSub percentage of the borrow rate, and makes up the differnce collecting transaction fees and liquidiation penalties, 0.25 means dac is paying 25% of the rate
      modified_user.tesprice = tesprice * (1.0 - _config.getRateSub());  // annualized rate borrowers pay in periodic premiums to insure their collateral, reduced by expected liquidation penalty
  });
}

//pricing() calculates the borrow rate for risky token borrows, l_tesprice
//the model it is an out of the money cash or nothing binary option pricer
//pricing is scaled up/down if solvency is below/above target set by custodians
//the purpose of l_pricing() is to provide a pricing curve accross moneyness, and scale the curve based on system risk
//this enables market price discovery as users transact, removing the need for individual quoting like an options montage
//borrowers of risky tokens pay a floating borrow rate, the rate dynamically adjusts with changes in system risk
void vigorlending::l_pricing(const user_s& user) {
  
  // premium payments in exchange for contingient payoff in the event that a price threshhold is breached to the upside
  double l_scaleliq = 0.0;  //penalty for borrowing scarce tokens, scarce as defined by lentpct
  for(const auto& l_collateral : *user.getLCollaterals()) {
    auto market = _market.get(symbol_swap(l_collateral.symbol).raw());
    double l_valueofcol = (l_collateral.amount) / std::pow(10.0, l_collateral.symbol.precision()) *
                          ((double)market.marketdata.price.back() / pricePrecision);
    auto l_col = _whitelist.find(symbol_swap(l_collateral.symbol).raw());
    l_scaleliq += (l_valueofcol / user.l_valueofcol) * l_col->lentpct;
  }

  double l_ivol = user.l_volcol * (_config.getCalibrate() * _g.l_scale + (std::pow(l_scaleliq, 2.0) / 2.0));  // market determined implied volaility

  double l_stresscol = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * user.l_volcol) - 1.0;

  double l_istresscol = std::exp(std::log(1.0 + l_stresscol) * (_config.getCalibrate() * _g.l_scale + (std::pow(l_scaleliq, 2.0) / 2.0))) - 1.0;

  double lp = (_config.getBailoutupCR() / 100.0 - 1.0 ) * (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())); // liquidation penalty

  double payoffx = std::max(std::max(0.0, 1.0 * user.l_valueofcol * (1.0 + l_istresscol) - (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()))) - lp * _config.getLiqCut(), 0.0); // reduce the value of the put option by the vigordacfund's share of the expected value of liquidation penalties

  double payoff = std::max(std::max(0.0, 1.0 * user.l_valueofcol * (1.0 + l_istresscol) - (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()))) - lp, 0.0); // reduce the value of the put option by the full expected value of the liquidation penalties

  double T = 1.0;
  
  double d = ((std::log(user.l_valueofcol / ((user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) / (_config.getBailoutupCR() / 100.0)))) + (-std::pow(l_ivol, 2) / 2.0) * T * _config.getDataC()) / (l_ivol * std::sqrt(T * _config.getDataC()));

  double discount = 1.0 - user.getReputation_pct() * _config.getMaxDisc();

  double l_tesprice = 0.0;
  double l_tespricex = 0.0;
  if(user.l_valueofcol > 0) {
    l_tesprice = discount * std::min(std::max(_config.getMinTesPrice(),((payoff * std::erfc((-1.0 * d) / std::sqrt(2.0)) / 2.0) / user.l_valueofcol)),_config.getMaxTesPrice());
    l_tespricex = discount * std::min(std::max(_config.getMinTesPrice(),((payoffx * std::erfc((-1.0 * d) / std::sqrt(2.0)) / 2.0) / user.l_valueofcol)),_config.getMaxTesPrice()); // note: expected value of insurers share of liquidation penalties = (l_tespricex - l_tesprice)
  }

  double l_premiums = l_tespricex * user.l_valueofcol; 

  _g.l_premiums += l_premiums;  // total dollar amount of premiums all borrowers would pay in one year to insure their collateral (and includes expected liquidation penalties paid)

  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(modified_user.l_tesprice != l_tesprice * (1.0 - _config.getRateSub())) // vigordacfund paying for getRateSub percentage of the borrow rate, and makes up the differnce collecting transaction fees and liquidiation penalties, 0.25 means dac is paying 25% of the rate
      modified_user.l_tesprice = l_tesprice * (1.0 - _config.getRateSub());  // annualized rate borrowers pay in periodic premiums to insure their collateral. reduced by expected liquidation penalty
  });
}

// same as stressins, but remove the specified user
// for use in determining an insures contribution to solvency
// measure the insurance stress with/without a given insurer
void vigorlending::stressinsx(const user_s& user) {
  
  if(user.valueofins <= 0.0 || user.usern == _config.getFinalReserve()) {  // exclude the reserve because it only absorbs bailout after all insurers are wiped out, handled in reserve() method
    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(user.svalueofinsx != 0.0)
        modified_user.svalueofinsx = 0.0;
      if(user.l_svalueofinsx != 0.0)
        modified_user.l_svalueofinsx = 0.0;
    });
  } else {
    //  Calculate the global portfolioVariance without the specified user insurance
    double user_valueofins_adj = 0.0;
    double portVariancex = portfolioVariancex(*_g.getInsurances(), _g.valueofins, user, user_valueofins_adj);  // _g.valueofins is used purposefully (rather than valueofinsx) per standard methodology of contribution to risk, asset to be added/removed is assumed to be funded from/to cash
    double valueofinsx = std::max(_g.valueofins - user_valueofins_adj, 0.0);
    double stressinsx = -1.0 * (std::exp(-1.0 * (((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(portVariancex))) - 1.0);  // model suggested percentage loss that the total insurance asset portfolio (ex the specified user) would experience in a stress event.
    double l_stressinsx = std::exp(((std::exp(-1.0 * (std::pow(icdf(_config.getAlphaTest()), 2.0)) / 2.0) / (std::sqrt(2.0 * M_PI))) / (1.0 - _config.getAlphaTest())) * std::sqrt(portVariancex)) - 1.0;                  // model suggested percentage loss that the total insurance asset portfolio (ex the specified user) would experience in a stress event.
    double stressins = 1.0 - (_g.svalueofins / _g.valueofins);
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.svalueofinsx = ((1.0 - (stressinsx + (stressins - stressinsx) * 0.9)) * valueofinsx);  // compress the stressinsx closer to stressins so that svalueofinsx < svalueofins
      modified_user.l_svalueofinsx = ((1.0 + l_stressinsx) * valueofinsx);                                 // can compress this too if desired
    });
  }
}

// sum of weighted marginal contribution to risk (solvency), used for rescaling pcts so that it sums to 1
void vigorlending::RM(const user_s& user) {
  
  if(user.valueofins <= 0.0 || user.usern == _config.getFinalReserve())
    return;
  double dRM = std::max(_g.svalueofins - user.svalueofinsx, 0.0);
  _g.rm += dRM;
}

// sum of weighted marginal contribution to risk (l_solvency), used for rescaling l_pcts so that it sums to 1
void vigorlending::l_RM(const user_s& user) {
  
  if(user.valueofins <= 0.0 || user.usern == _config.getFinalReserve())
    return;
  double l_dRM = std::max(_g.l_svalueofins - user.l_svalueofinsx, 0.0);
  _g.l_rm += l_dRM;
}

// percent contribution to l_solvency, l_pcts
// a measure that shows the change in the l_solvency by including the given insurer
// insurers earn VIG fees, and accept the share of bailouts for upside market stress events, comensurate with l_pcts
//
// percent contribution to liquidity, l_pctl
// a measure that accounts for the risk of liquidity crunch that a lender faces when depositing a scarce token into the lending pool.
// tokens in high demand for borrowing may reach maxlends limit, the insurers can withdraw (1-maxlends) amount of token but beyond that lenders must wait in line to withdraw, waiting for either borrowed tokens to be repaid or new lenders depositing
// insurers earn VIG fees comensurate with l_pctl, and accept the risks of liquidity crunch
void vigorlending::l_pcts(const user_s& user) {
  

  if(user.usern == _config.getFinalReserve())  // exclude the reserve because it only absorbs bailout after all insurers are wiped out, handled in reserve() method
    return;

  if(user.valueofins <= 0.0 || _g.l_rm <= 0.0) {  // insurer bears no bailout risk nor shall be compensated, so l_pcts is set to zero.
    if(user.l_pcts != 0.0)
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.l_pcts = 0.0;
      });
  } else {
    double l_dRM = std::max(_g.l_svalueofins - user.l_svalueofinsx, 0.0);
    double l_pcts = std::max(l_dRM / _g.l_rm, 0.0);
    _g.l_pcts += l_pcts;
    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(modified_user.l_pcts != l_pcts)
        modified_user.l_pcts = l_pcts;  // percent contribution to l_solvency (weighted marginal contribution to risk (l_solvency) rescaled by sum of that
    });
  }

  if(user.valueofins <= 0.0 || _b.l_rmliq <= 0.0) {  // insurer bears no bailout risk nor shall be compensated, so l_pctl is set to zero.
  } else {
    double l_dRMliq = 0.0;
    for(const auto& userIns : *user.getInsurances()) {
      auto ins = _whitelist.find(symbol_swap(userIns.symbol).raw());
      if(ins->lendable.amount > 0) {
        l_dRMliq += ((double)userIns.amount / ins->lendable.amount) * ins->lendablepct * ins->lentpct;
      }
    }
    double l_pctl = std::max(l_dRMliq / _b.l_rmliq, 0.0);
    _b.l_pctl += l_pctl;  // percent contribution to liquidity (weighted marginal contribution to risk (liquidity) rescaled by sum of that
  }
}

double vigorlending::l_pctl(const user_s& user) {  // percent contribution to liquidity

  double l_pctl = 0.0;
  if(user.usern == _config.getFinalReserve()) {  // exclude the reserve because it only absorbs bailout after all insurers are wiped out, handled in reserve() method
    if(user.l_pcts == 1.0)
      l_pctl = 1.0;
    return l_pctl;
  }

  if(user.valueofins <= 0.0 || _b.l_rmliq <= 0.0) {  // insurer bears no liquidity risk nor shall be compensated, so l_pctl is set to zero.
  } else {
    double l_dRMliq;
    for(const auto& userIns : *user.getInsurances()) {
      auto ins = _whitelist.find(symbol_swap(userIns.symbol).raw());
      if(ins->lendable.amount > 0)
        l_dRMliq += ((double)userIns.amount / ins->lendable.amount) * ins->lendablepct * ins->lentpct;
    }
    l_pctl = std::max(l_dRMliq / _b.l_rmliq, 0.0);
  }
  return l_pctl;
}

// percent contribution to solvency
// a measure that shows the change in the solvency by including the given insurer
// insurers earn VIG fees, and accept the share of bailouts for downside market stress events, comensurate with pcts
void vigorlending::pcts(const user_s& user) {  // percent contribution to solvency
  

  if(user.usern == _config.getFinalReserve())  // exclude the reserve because it only absorbs bailout after all insurers are wiped out, handled in reserve() method
    return;

  if(user.valueofins <= 0.0 || _g.rm <= 0.0) {  //insurer bears no bailout risk nor shall be compensated, so pcts is set to zero.
    if(user.pcts != 0.0)
      _u.modify(user, get_self(), [&](auto& modified_user) {
        if(modified_user.pcts != 0.0)
          modified_user.pcts = 0.0;  // percent contribution to solvency (weighted marginal contribution to risk (solvency) rescaled by sum of that
      });
  } else {
    double dRM = std::max(_g.svalueofins - user.svalueofinsx, 0.0);
    double pcts = std::max(dRM / _g.rm, 0.0);
    _g.pcts += pcts;
    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(modified_user.pcts != pcts)
        modified_user.pcts = pcts;  // percent contribution to solvency (weighted marginal contribution to risk (solvency) rescaled by sum of that
    });
  }
}

// loop through all users to pay the aggregated VIG payments collected in collectfee(), amount paid out according to each insurer's pcts and l_pcts
// user having tokens in insurance/lending (excluding VIG): user is paid VIG rewards into user vigfees
// user having VIG in insurance/lending: user is paid VIGOR rewards into savings (vigordacfund receives VIG fees and takes a VIGOR loan against that to pay the user)
// user having VIGOR in savings: user is paid VIG into vigfees, from vigordacfund as an expense
void vigorlending::distribfee() {
  
  auto market = _market.get(symbol_swap(_g.fee.symbol).raw());
  double vigprice = market.marketdata.price.back() / pricePrecision;
  double vigvol = market.marketdata.vol / volPrecision;
  eosio::asset viga = eosio::asset(0, _g.fee.symbol);
  eosio::asset vigsavings = eosio::asset(0, _g.fee.symbol);
  eosio::asset vigorpayissue = eosio::asset(0, VIGOR_SYMBOL);
  eosio::asset pctamtatotal = eosio::asset(0, viga.symbol);
  eosio::name bailoutuser = _g.bailoutuser;
  eosio::name bailoutupuser = _g.bailoutupuser;
  eosio::name finalreserve = _config.getFinalReserve();
  eosio::name vigordacfund = _config.getVigorDacFund();
  // rewards handling
  double rewardrexvotamt = (_g.rexproxy / std::pow(10.0, SYSTEM_TOKEN_SYMBOL.precision())) * ((double)_market.get(SYSTEM_TOKEN_SYMBOL.raw()).marketdata.price.back() / pricePrecision); // dollars worth of EOS accumulations
  uint64_t lptokendefiamt = _g.lprewards; // dollar amount of rewards to deposits of lptoken.defi
  double lptokensxamt = (_g.txnfee * _config.getLpsxCut()); // transaction fee cut in dollars for deposits of lptoken.sx

  if(_b.usern.value == _u.begin()->usern.value) {
    _b.lastflag = false;
    const auto& lastInsurer = *std::find_if(_u.rbegin(), _u.rend(), [&bailoutuser, &bailoutupuser, &finalreserve, &vigordacfund](const auto& user) {//  Find the last eligible user to receive rewards, so we can give dust
      return user.isEligible(bailoutuser, bailoutupuser, finalreserve, vigordacfund);
    });
    _b.lastinsurer = lastInsurer.usern;
    _b.stepdata = (_g.fee.amount + _g.l_fee.amount + (uint64_t)(std::pow(10.0, _g.fee.symbol.precision()) * (rewardrexvotamt/vigprice + lptokendefiamt/vigprice + lptokensxamt/vigprice)));
  }
  auto lastinsureritr = _u.find(_b.lastinsurer.value);
  const auto& lastInsurer = *lastinsureritr;
  double savings_pct;
  auto itr = _u.lower_bound(_b.usern.value);
  auto stop_itr = _u.end();
  uint64_t counter = 0;

  while(itr != stop_itr && counter++ < _b.batchsize) {
    if(_b.lastflag || (_g.fee.amount + _g.l_fee.amount + (uint64_t)(std::pow(10.0, _g.fee.symbol.precision()) * (rewardrexvotamt/vigprice + lptokendefiamt/vigprice + lptokensxamt/vigprice)) <= 0)) {
      itr++;
      continue;
    }

    auto& user = *itr;
    if(!user.isEligible(bailoutuser, bailoutupuser, finalreserve, vigordacfund)) {
      itr++;
      continue;
    }

  double valueofins = 0.0;
  double valueofcol = 0.0;
  double l_valueofcol = 0.0;
  double rewardrexvot = 0.0;
  double lptokendefi = 0.0;
  double lptokensx = 0.0;

  for(const auto& insurance : *user.getInsurances()) {
    auto market = _market.get(symbol_swap(insurance.symbol).raw());
    valueofins += (insurance.amount) / std::pow(10.0, insurance.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
    auto symbolIterator = _whitelist.find(symbol_swap(insurance.symbol).raw());
    if (symbolIterator->contract.value == eosio::name("lptoken.defi").value)
      lptokendefi += valueofins;
    if (symbolIterator->sym.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      rewardrexvot += valueofins;
    if (symbolIterator->contract.value == eosio::name("lptoken.sx").value)
      lptokensx += valueofins;
  }

  for(const auto& collateral : *user.getCollaterals()) {
    auto market = _market.get(symbol_swap(collateral.symbol).raw());
    valueofcol += (collateral.amount) / std::pow(10.0, collateral.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
    auto symbolIterator = _whitelist.find(symbol_swap(collateral.symbol).raw());
    if (symbolIterator->contract.value == eosio::name("lptoken.defi").value)
      lptokendefi += valueofcol;
    if (symbolIterator->sym.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      rewardrexvot += valueofcol;
    if (symbolIterator->contract.value == eosio::name("lptoken.sx").value)
      lptokensx += valueofcol;
  }
  rewardrexvot = _g.totalrexvot == 0.0 ? 0.0 : rewardrexvot / _g.totalrexvot;
  lptokendefi = _g.totrewlend == 0.0 ? 0.0 : lptokendefi / _g.totrewlend;
  lptokensx = _g.totrewlendsx == 0.0 ? 0.0 : lptokensx / _g.totrewlendsx;

    savings_pct = 0.0;
    if(_g.savings > 0.0)
      savings_pct = (_config.getSavingsCut() * _g.savingsscale) * user.savings / _g.savings;
    else if(user.usern == _config.getFinalReserve())
      savings_pct = (_config.getSavingsCut() * _g.savingsscale);

    viga.amount = _g.fee.amount * ((user.pcts / _g.pcts) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale) + savings_pct) +
                  (_g.l_fee.amount / 2.0) * ((user.l_pcts / _g.l_pcts) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale) + savings_pct) +
                  (_g.l_fee.amount / 2.0) * ((l_pctl(user) / _b.l_pctl) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale) + savings_pct);

    uint64_t rewardlend = std::pow(10.0, _g.fee.symbol.precision()) * (rewardrexvot * rewardrexvotamt/vigprice + lptokendefi * lptokendefiamt/vigprice + lptokensx * lptokensxamt/vigprice);

    viga.amount += rewardlend;

    if(user.usern == _config.getVigorDacFund())
      viga.amount += _g.fee.amount * _config.getVigorDacCut() + _g.l_fee.amount * _config.getVigorDacCut();

    if(user.usern == _config.getFinalReserve())
      viga.amount += _g.fee.amount * _config.getReserveCut() + _g.l_fee.amount * _config.getReserveCut();


    //pay all rewards in VIGOR. The VIG fees collected serve as collateral to borrow VIGOR for paying lenders. Calculate how much VIGOR to print
    eosio::asset vigorpay = eosio::asset(0, VIGOR_SYMBOL);
    eosio::asset pctamta = eosio::asset(0, viga.symbol);
    eosio::asset ins = eosio::asset(0, _g.fee.symbol);
    if(viga.amount > 0 && user.usern.value != _config.getFinalReserve().value) {
      double pct = 1.0;                                                                       // percentage of user insurance to receive VIGOR payment (backed by VIG) rather than VIG payment
      double oc = (_config.getBailoutCR() / 100) + 0.07;                                      // +2.0 * vigvol / sqrt(12); // overcollateralize the VIGOR borrow by this amount, enough collateral to survive a one monthly standard deviation move of VIG price
      double pctamt = pct * viga.amount / std::pow(10.0, viga.symbol.precision());            // this insurers share of the VIG premiums to be used as collateral, for borrowing VIGOR
      pctamta.amount = pctamt * std::pow(10.0, viga.symbol.precision());                      // cast as a uint64_t
      if(pctamta.amount > 0) {                                                                // amount may be too small, may become zero when cast
        vigorpay.amount = pctamt / oc * vigprice * std::pow(10.0, VIGOR_SYMBOL.precision());  // amount of VIGOR that can be borrowed considering the amount of VIG collateral and overcollateralization needed
        pctamta.amount = std::min(viga.amount, pctamta.amount);                               // dust
        viga -= pctamta;                                                                      // user gives this amount to vigordacfund
        pctamtatotal += pctamta;                                                              // note if vigorpay.amount rounds to zero, then the user gives the dust amount of VIG pctamta to vigordacfund for nothing in return.
        vigorpayissue += vigorpay;
      }
    }

    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(user.usern == _config.getFinalReserve()) {
        getInsurances(modified_user) += viga;  // reserve receives the fee into insurance and also adds to global insurance
        auto dacitr = _u.find(_config.getVigorDacFund().value);
        auto& dac = *dacitr;
        _u.modify(dac, get_self(), [&](auto& dac) {
          getCollaterals(modified_user) -= viga;
        });
      } else {
        if(vigorpay.amount > 0) {
          modified_user.savings += vigorpay.amount / std::pow(10.0, vigorpay.symbol.precision());
          _g.savings += vigorpay.amount / std::pow(10.0, vigorpay.symbol.precision());
          modified_user.rewardlend2.utc_seconds += vigorpay.amount; // store amount of VIGOR paid, accumulates
        }
      }
      modified_user.rewardlend += rewardlend*vigprice;
      double days_since_anniv = (3600.0 + eosio::current_time_point().sec_since_epoch() - (modified_user.getAnniv().sec_since_epoch() - _config.getRepAnniv())) / eosio::days(1).to_seconds();
      uint64_t rep = std::pow(10.0, _g.fee.symbol.precision()) * std::sqrt(std::pow(modified_user.getVig_since_anniv().amount / std::pow(10.0, _g.fee.symbol.precision()), 2.0) + (i_reputation(viga - eosio::asset(rewardlend, viga.symbol) + pctamta).amount / std::pow(10.0, _g.fee.symbol.precision())));
      modified_user.setRep_lag0(eosio::asset(rep / std::sqrt(days_since_anniv), _g.fee.symbol));
      modified_user.setVig_since_anniv(eosio::asset(rep, _g.fee.symbol));
      if(days_since_anniv > (double)_config.getRepAnniv() / eosio::days(1).to_seconds()) {
        modified_user.reputationrotation();
        modified_user.setVig_since_anniv(eosio::asset(0, _g.fee.symbol));
        modified_user.setAnniv(eosio::current_time_point(), _config.getRepAnniv());
      }
    });
    if(user == lastInsurer) {
      _b.lastflag = true;
      itr++;
      continue;
    }

    _b.stepdata -= (viga.amount + pctamta.amount);
    itr++;
  }
  if(vigorpayissue.amount > 0) {  // don't issue if dust amount
    auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  symbolIterator->contract,  // contract for vigor token
                  eosio::name("issue"),
                  std::make_tuple(get_self(), vigorpayissue, std::string("VIGOR issued to ") + get_self().to_string()))
      .send();
#endif
  }

  auto dacitr = _u.find(_config.getVigorDacFund().value);
  auto& dac = *dacitr;
  _u.modify(dac, get_self(), [&](auto& dac) {
    dac.debt += vigorpayissue;            // this may be zero, which means that pctamta is a small dust amount
  });
  _g.totaldebt += vigorpayissue;  // this may be zero, which means that pctamta is a small dust amount
  if(itr == _u.end()) {
    _g.rexproxya += _g.rexproxy; // accumulate until atimer resets to zero
    _g.rexproxy = 0;
    _g.lprewardsa += _g.lprewards; // accumulate until atimer resets to zero
    _g.lprewards = 0.0;
    _g.txnfeea += _g.txnfee; // accumulate until atimer resets to zero
    _g.txnfee = 0.0;
    _b.usern = _u.begin()->usern;
    const auto rank = _u.get_index<"reputation"_n>();
    _b.usern = rank.begin()->usern;
    _g.step = 4;
  } else {
    _b.usern = itr->usern;
  }
}

//query the oracle to read prices and update the dollar value of the user insurance and collateral and l_collateral.
void vigorlending::update(const user_s& user) {
  
  double valueofins = 0.0;
  double valueofcol = 0.0;
  double l_valueofcol = 0.0;

  for(const auto& insurance : *user.getInsurances()) {
    auto market = _market.get(symbol_swap(insurance.symbol).raw());
    valueofins += (insurance.amount) / std::pow(10.0, insurance.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
  }

  for(const auto& collateral : *user.getCollaterals()) {
    auto market = _market.get(symbol_swap(collateral.symbol).raw());
    valueofcol += (collateral.amount) / std::pow(10.0, collateral.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
  }

  for(const auto& l_collateral : *user.getLCollaterals()) {
    auto market = _market.get(symbol_swap(l_collateral.symbol).raw());
    l_valueofcol += (l_collateral.amount) / std::pow(10.0, l_collateral.symbol.precision()) *
                    ((double)market.marketdata.price.back() / pricePrecision);
  }

  _u.modify(user, get_self(), [&](auto& modified_user) {  // Update value of collateral
    if(modified_user.valueofins != valueofins)
      modified_user.valueofins = valueofins;
    if(modified_user.valueofcol != valueofcol)
      modified_user.valueofcol = valueofcol;
    if(modified_user.l_valueofcol != l_valueofcol)
      modified_user.l_valueofcol = l_valueofcol;
  });
}

//query the oracle to read prices and update the dollar value of the global insurance and collateral and l_collateral.
void vigorlending::updateglobal() {
  
  double valueofins = 0.0;
  double valueofcol = 0.0;
  double l_valueofcol = 0.0;
  double totrewlend = 0.0;
  double totalrexvot = 0.0;
  double totrewlendsx = 0.0;

  for(auto& insurance : *_g.getInsurances()) {
    auto market = _market.get(symbol_swap(insurance.symbol).raw());
    valueofins += (insurance.amount) / std::pow(10.0, insurance.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
    auto symbolIterator = _whitelist.find(symbol_swap(insurance.symbol).raw());
    if (symbolIterator->contract.value == eosio::name("lptoken.defi").value)
      totrewlend += valueofins;
    if (symbolIterator->sym.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      totalrexvot += valueofins;
    if (symbolIterator->contract.value == eosio::name("lptoken.sx").value)
      totrewlendsx += valueofins;
  }

  for(auto& collateral : *_g.getCollaterals()) {
    auto market = _market.get(symbol_swap(collateral.symbol).raw());
    valueofcol += (collateral.amount) / std::pow(10.0, collateral.symbol.precision()) *
                  ((double)market.marketdata.price.back() / pricePrecision);
    auto symbolIterator = _whitelist.find(symbol_swap(collateral.symbol).raw());
    if (symbolIterator->contract.value == eosio::name("lptoken.defi").value)
      totrewlend += valueofcol;
    if (symbolIterator->sym.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      totalrexvot += valueofcol;
    if (symbolIterator->contract.value == eosio::name("lptoken.sx").value)
      totrewlendsx += valueofcol;
  }

  for(auto& l_collateral : *_g.getLCollaterals()) {
    auto market = _market.get(symbol_swap(l_collateral.symbol).raw());
    l_valueofcol += (l_collateral.amount) / std::pow(10.0, l_collateral.symbol.precision()) *
                    ((double)market.marketdata.price.back() / pricePrecision);
    auto symbolIterator = _whitelist.find(symbol_swap(l_collateral.symbol).raw());
    if (symbolIterator->contract.value == eosio::name("lptoken.defi").value)
      totrewlend += l_valueofcol;
    if (symbolIterator->sym.raw() == SYSTEM_TOKEN_SYMBOL.raw())
      totalrexvot += l_valueofcol;
    if (symbolIterator->contract.value == eosio::name("lptoken.sx").value)
      totrewlendsx += l_valueofcol;
  }

  auto market = _market.get(VIGOR_SYMBOL.raw());
  if(_g.savingsscale != std::max(std::min(std::pow(1.0 / ((double)market.marketdata.price.back() / pricePrecision), 4.0), 1.3), 0.7))
    _g.savingsscale = std::max(std::min(std::pow(1.0 / ((double)market.marketdata.price.back() / pricePrecision), 4.0), 1.3), 0.7);

  if(_g.valueofins != valueofins)
    _g.valueofins = valueofins;
  if(_g.valueofcol != valueofcol)
    _g.valueofcol = valueofcol;
  if(_g.l_valueofcol != l_valueofcol)
    _g.l_valueofcol = l_valueofcol;

    _g.totrewlend = totrewlend;
    _g.totalrexvot = totalrexvot;
    _g.totrewlend = totrewlendsx;

  auto& res = _u.get(_config.getFinalReserve().value, "finalreserve not found");
  auto itr = _whitelist.begin();
  _b.l_rmliq = 0;
  while(itr != _whitelist.end()) {
    eosio::asset a = eosio::asset(0, symbol_swap(itr->sym));
    eosio::asset i = a;
    eosio::asset r = a;
    eosio::asset l_col = a;
    eosio::asset lendable = a;
    _g.getInsurances().hasAsset(a, i);
    res.getInsurances().hasAsset(a, r);
    _g.getLCollaterals().hasAsset(a, l_col);
    lendable = i - r + l_col;
    auto market = _market.get(itr->sym.raw());
    double valueoflendable = (lendable.amount) / std::pow(10.0, lendable.symbol.precision()) *
                             ((double)market.marketdata.price.back() / pricePrecision);
    double lendablepct = (_g.valueofins - res.valueofins + _g.l_valueofcol) > 0 ? valueoflendable / (_g.valueofins - res.valueofins + _g.l_valueofcol) : 0.0;
    double lentpct = (lendable.amount * (itr->maxlends / 100.0)) > 0 ? std::min(l_col.amount / (lendable.amount * (itr->maxlends / 100.0)), 1.0) : 0.0;
    _whitelist.modify(itr, get_self(), [&](auto& a) {
      if(a.lendable.amount != lendable.amount)
        a.lendable = lendable;
      if(a.lendablepct != lendablepct)
        a.lendablepct = lendablepct;
      if(a.lentpct != lentpct)
        a.lentpct = lentpct;
    });
    _b.l_rmliq += lendablepct * lentpct;
    itr++;
  }
}

// calculate earnrate for each user, VIG earned by insurers (paid by VIGOR borrowers and risky token borrowers) as a percentage of the value of their insurance portfolio
void vigorlending::performance(const user_s& user, const double& vigvol, const double& vigprice) {
  
  // all fees and rewards collected are swapped for VIG which collateralizes new VIGOR debt, to pay lender VIGOR. the overcollateralization amount is a haircut
  double haircut = (user.usern.value != _config.getFinalReserve().value && user.usern.value != _config.getVigorDacFund().value) ? (1.0 / ((_config.getBailoutCR() / 100) + 0.075)) : 1.0;
  double rewards = 365.0 * user.rewardrexvot; // dollars worth of rewards dsitribed in total (REX/vote and LP token rewards) for one period (atimer peiod, hardcoded at 86400 sec which is 1 day), then scaled to annual
  
  double cut = (user.pcts / _g.pcts) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale);
  double l_cut = (user.l_pcts / _g.l_pcts) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale);
  double l_cutliq = (l_pctl(user) / _b.l_pctl) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale);

  if(user.usern == _config.getFinalReserve()) {
    cut = 0;
    l_cut = 0;
  }

  if(user.usern == _config.getVigorDacFund()) {
    cut += _config.getVigorDacCut();
    l_cut += _config.getVigorDacCut();
  }

  double earnrate = 0.0;

  if((user.valueofcol + user.valueofins) != 0.0)
    earnrate = (haircut * rewards) / (user.valueofcol + user.valueofins);  // annualized rate of return on user portfolio of insurance and collateral crypto assets, from rewards like rex/proxy and LP token rewards

  if(user.valueofins != 0.0)
    earnrate += (haircut * (cut * _g.premiums + l_cut * (_g.l_premiums / 2.0) + l_cutliq * (_g.l_premiums / 2.0))) / user.valueofins;  // annualized rate of return on user portfolio of insurance crypto assets, insuring for downside and upside price jumps
  
  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(modified_user.earnrate != earnrate)
      modified_user.earnrate = earnrate;
  });
}

//global earnrate: annualized rate of return on the global insurance portfolio. all premiums (paid by VIGOR borrowers and risky token borrowers) as a percentage of the global insurance portfolio
//savingsrate: annualized rate of return on VIGOR deposited into l_debt, savings account, risk free earings
void vigorlending::performanceglobal() {
  
  //_g.raroc = 0.0;
  //double scr = _g.valueofins - ( _g.svalueofins - _g.svalueofcole );

  //if( scr != 0.0 )
  //    _g.raroc = ( ( _g.premiums + _g.l_premiums ) - _g.svalueofcoleavg - _g.l_svalueofcoleavg ) / scr; // RAROC risk adjusted return on capital. expected return on capital employed. (Revenues - Expected Loss) / SCR

  // all fees and rewards collected are swapped for VIG which collateralizes new VIGOR debt, to pay lender VIGOR. the overcollateralization amount is a haircut
  double haircut = (1.0 / ((_config.getBailoutCR() / 100) + 0.075));
  double rexproxy = (_g.rexproxyp / std::pow(10.0, SYSTEM_TOKEN_SYMBOL.precision())) * ((double)_market.get(SYSTEM_TOKEN_SYMBOL.raw()).marketdata.price.back() / pricePrecision);
  double rewards = 365.0 * (rexproxy + _g.lprewardsp + (_g.txnfeep * _config.getLpsxCut())); // dollars worth of rewards dsitribed to this user (REX/vote and and LP token rewards) for one period (atimer peiod), then scaled to annual
  
  _g.earnrate = 0.0;
  _g.earnrate = 0.0;

  if((_g.valueofcol + _g.valueofins + _g.l_valueofcol) > 0.0)
    _g.earnrater = std::min((haircut * rewards) / (_g.valueofcol + _g.valueofins + _g.l_valueofcol), 100.0);  // annualized rate of return on total portfolio of insurance and collateral crypto assets, for rewards like rex/proxy and LP token rewards

  if((_g.valueofins + _g.l_valueofcol) > 0.0)
    _g.earnrate = _g.earnrater + std::min((haircut * ((_g.premiums + _g.l_premiums) * (1.0 - _config.getReserveCut() - _config.getVigorDacCut() - _config.getSavingsCut() * _g.savingsscale))) / (_g.valueofins + _g.l_valueofcol), 100.0);  // annualized rate of return on total portfolio of insurance crypto assets, insuring for downside and upside price jumps

  _g.savingsrate = 0.0;

  if(_g.savings > 0.0)
    _g.savingsrate = std::min((haircut * (_config.getSavingsCut() * _g.savingsscale * (_g.premiums + _g.l_premiums))) / _g.savings, 100.0);  // annualized rate of return on VIGOR deposited into l_debt, savings account, risk free earings
}

//switch the reserve on/off, enabling it to receive all bailouts, if all insurers are wiped out
void vigorlending::reserve() {
  
  auto useritr = _u.find(_config.getFinalReserve().value);
  //eosio::check( useritr != _u.end(), "finalreserve not exist" );
  auto& user = *useritr;

  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(_g.pcts <= 0.0) {
      modified_user.pcts = 1.0;  // trigger reserve to accept bailouts
      _g.pcts = 1.0;
    } else {
      modified_user.pcts = 0.0;  // reserve does not participate in bailouts unless insurers are wiped out.
    }
    if(_g.l_pcts <= 0.0) {
      modified_user.l_pcts = 1.0;  // trigger reserve to accept bailouts
      _g.l_pcts = 1.0;
      _b.l_pctl = 1.0;
    } else {
      modified_user.l_pcts = 0.0;  // reserve does not participate in bailouts unless insurers are wiped out.
    }
    if(_b.l_pctl <= 0.0)
      _b.l_pctl = 1.0;
  });
}

// if a VIGOR borrower's debt becomes undercollateralized, move their debt and associated collateral to the insurers comensurate with pcts
// insurers receiving the bailout, will automatically recapitalized the debt
// borrowers retain excess collateral
void vigorlending::bailout(const eosio::name& usern) {
  

  require_auth(get_self());

  auto userIterator = _u.require_find(usern.value, "usern doesn't exist in the user table");
  const auto& user = *userIterator;

  bool logmax = false;
  auto end_itr = _bailout.end();
  if(_bailout.begin() != end_itr) {
    end_itr--;
    uint64_t count = (end_itr->id - _bailout.begin()->id) + 1;
    if(count >= _config.getLogCount())
      logmax = true;
  }

  // Before bailout, set aside user excess collateral, to be given back to user after the bailout
  if(_b.usern.value == _u.begin()->usern.value) {
    auto& res = _u.get(_config.getFinalReserve().value, "finalreserve not found");
    //if reserve is switched on then savers (if any exist) are invoked to be insurers with pcts = user.savings/_g.savings
    uint64_t gsavings = _g.savings * std::pow(10.0, VIGOR_SYMBOL.precision());
    _b.stepdata2 = 0;
    if(usern.value == res.usern.value && res.pcts == 1.0 && gsavings > 0) _b.stepdata2 = 1;
    _b.stepdata = gsavings;
    _b.sumpcts = 0.0;
    _b.lastflag = false;
    _b.excesspct = (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) / user.valueofcol;
    asset_container_t assetcontainer;
    _b.assetcont = assetcontainer;
    std::string message;

    if(_g.bailoutid == INT64_MAX)
      _g.bailoutid = 1;
    else
      _g.bailoutid += 1;

    // user sent to bailout, amount if any excess above bailoutCR is retained by user, amount if any between bailoutCR and 100% is a liquidation penalty given to lenders after vigordacfund takes 50% cut.
    // if finalreserve is sent to bailout (bailout out by savers), amount if any between bailoutCR and 100% is a liquidation penalty given to savers after vigordacfund takes 50% cut.
    _b.excesspct *= _config.getBailoutCR() / 100;
    double liqvalueofcol = user.valueofcol;
    if(_b.excesspct < 1.0) {
      auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
      for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
        eosio::asset amt(userCollateral.amount, userCollateral.symbol);
        amt.amount *= (1.0 - _b.excesspct);
        _b.assetcont.push_back(amt);
        _u.modify(user, get_self(), [&](auto& modified_user) {
          userCollaterals -= amt; // subtract excess above bailoutCR from user and global collateral
        });
      }
      if (!_b.assetcont.empty())
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.valueofcol *= _b.excesspct;
          liqvalueofcol = modified_user.valueofcol;
        });
    }
    double vigordaccut = (user.debt.amount / std::pow(10.0, user.debt.symbol.precision())) / user.valueofcol;
    if(vigordaccut < 1.0) {
      _g.liqcut += user.valueofcol * ( 1.0 - vigordaccut ) * _config.getLiqCut();
      auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
      auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
      for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
        eosio::asset amt(userCollateral.amount, userCollateral.symbol);
        amt.amount *= ( 1.0 - vigordaccut ) * _config.getLiqCut(); // dac cut of the liquidation pentalty is 50%
        _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
          getCollaterals(modified_user) += amt; // add dac cut of liquidation penalty to vigordacfund collateral and global collateral
        });
        _u.modify(user, get_self(), [&](auto& modified_user) {
          userCollaterals -= amt; // subtract dac cut of liquidation penalty from user and global collateral
        });
      }
      liqvalueofcol *= ( vigordaccut + ( ( 1.0 - vigordaccut ) * _config.getLiqCut() ) );
    }
    if (!_b.assetcont.empty()) {
      message.append(std::string("Borrower sent to bailout : " + user.usern.to_string() + " liquidated debt: " + user.debt.to_string() + " liquidated collateral: (dollar value) " + std::to_string(liqvalueofcol) + " " + user.getCollaterals().to_string() + ". user retains excess collateral: (dollar value) " + std::to_string(user.valueofcol / _b.excesspct - user.valueofcol) + " "));
      for(auto col : _b.assetcont)
        message.append(std::string(col.to_string() + " "));
    } else {
      message.append(std::string("Borrower sent to bailout : " + user.usern.to_string() + " liquidated debt: " + user.debt.to_string() + " liquidated collateral: (dollar value) " + std::to_string(liqvalueofcol) + " " + user.getCollaterals().to_string()));
    }
    if(logmax) _bailout.erase(_bailout.begin());
    _bailout.emplace(get_self(), [&](auto& b) {
      b.id = _bailout.available_primary_key();
      b.timestamp = eosio::current_time_point();
      b.usern = user.usern;
      b.bailoutid = _g.bailoutid;
      b.type = eosio::name("liquidate");
      b.pcts = user.pcts / _g.pcts;
      b.debt.push_back(user.debt);
      b.collateral = user.getCollaterals().getContainer();
    });
    const eosio::name& userntrace = user.usern;
    const uint64_t bailoutid = _g.bailoutid;
    tracetable(__func__, __LINE__, userntrace, bailoutid, message);
    //  Find the last insurer, to take the dust
    if(_b.stepdata2 == 1) {
      const auto& lastInsurer = *std::find_if(_u.rbegin(), _u.rend(), [&](const auto& item) { return (uint64_t)(item.savings * std::pow(10.0, VIGOR_SYMBOL.precision())) > 0 && item != user; });
      _b.lastinsurer = lastInsurer.usern;
    } else {
      if(user.usern.value != _config.getFinalReserve().value && user.pcts / _g.pcts == 1.0) {
        _b.lastinsurer = user.usern;
        _b.lastflag = true;
      } else {
        const auto& lastInsurer = *std::find_if(_u.rbegin(), _u.rend(), [&](const auto& item) {
          return item.pcts > 0 && item != user;
        });
        _b.lastinsurer = lastInsurer.usern;
      }
    }
  }
  auto lastinsureritr = _u.find(_b.lastinsurer.value);
  const auto& lastInsurer = *lastinsureritr;

  //  The particular user being bailout out, is to be processed last (if the user is also an insurer, would participate in self bailout)
  //  doing so makes the math simple ratios
  auto itr = _u.lower_bound(_b.usern.value);
  auto stop_itr = _u.end();
  uint64_t counter = 0;

  while(itr != stop_itr && counter++ < _b.batchsize) {
    const auto& insurerUser = *itr;
    double pcts = insurerUser.pcts / _g.pcts;
    if(_b.stepdata2 == 1) {
      uint64_t usavings = (uint64_t)(insurerUser.savings * std::pow(10.0, VIGOR_SYMBOL.precision()));
      pcts = ((double)usavings) / _b.stepdata;
    }
    if(_b.lastflag || user.debt.amount == 0 || pcts <= 0 || insurerUser == user) {  //burn the wick
      itr++;
      continue;
    }

    if(logmax) _bailout.erase(_bailout.begin());

    recap(user, insurerUser, pcts, _b.sumpcts, (insurerUser == lastInsurer) && user.pcts <= 0);

    if(insurerUser == lastInsurer)
      _b.lastflag = true;
    itr++;
  }

  if(itr == _u.end()) {
    double pcts = user.pcts / _g.pcts;
    if(_b.stepdata2 == 1) {
      uint64_t usavings = (uint64_t)(user.savings * std::pow(10.0, VIGOR_SYMBOL.precision()));
      pcts = ((double)usavings) / _b.stepdata;
    }

    if(pcts > 0) {
      if(logmax)
        _bailout.erase(_bailout.begin());
      recap(user, user, pcts, _b.sumpcts, true);
    }

    // After bailout, release excess collateral back to user
    if(!_b.assetcont.empty())  {
      auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
      for(const auto& userExcessCollateral : _b.assetcont) {
        _u.modify(user, get_self(), [&](auto& modified_user) {
          userCollaterals += userExcessCollateral; // add back excess above bailoutCR to user and global collateral
        });
      }
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.valueofcol = user.valueofcol / _b.excesspct - user.valueofcol;
      });
    }
    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.resetReputation();
    });
    if(user.usern == _g.bailoutuser)
      _g.bailoutuser = eosio::name(0);

    _b.usern = _u.begin()->usern;
    _g.step = 7;

    switch(_config.getExecType()) {
      case 2: {  // mining
        uint64_t step1 = 1;
        schaction(get_self(), eosio::name("predoupdate"), std::make_tuple(step1), 0, _config.getMineBuffer(), 1);
        break;
      }
    }
  } else {
    _b.usern = itr->usern;
  }
}

void vigorlending::recap(const user_s& user, const user_s& insurerUser, double insurerpcts, double& sumpcts, bool lastInsurer) {
  
  // all insurers participate to recap bad debt, each insurer taking ownership of a fraction of the imparied collateral and debt; insurer participation is based on their percent contribution to solvency
  double pcts = insurerpcts * (1.0 / (1.0 - sumpcts));
  sumpcts += insurerpcts;
  eosio::asset debtshare = user.debt;  // insurer share of failed loan debt, in VIGOR
  debtshare.amount *= pcts;
  debtshare.amount = std::min(std::max(debtshare.amount, (int64_t)1), user.debt.amount);
  double userValueofcol = 0.0;
  asset_container_t collateral_t, debt_t, recap1_t, recap2_t, blockedins_t, blockeddebt_t;

  // assign ownership of the impaired collateral and debt to the insurers
  // subtract impaired collateral from user collateral and global collateral
  auto userCollaterals = getCollaterals(const_cast<user_s&>(user));
  for(const auto& userCollateral : asset_container_t(*userCollaterals)) {
    eosio::asset amt = userCollateral;
    amt.amount *= pcts;
    amt.amount = std::max(amt.amount, (int64_t)1);

    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(lastInsurer)
        amt = userCollateral;
      userCollaterals -= amt; // subtract impaired collateral from user and global collateral (yes this subtracts from global too)
    });
    // add impaired collateral to insurers and global insurance
    _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
      getInsurances(modified_user) += amt;
    });
    collateral_t.push_back(amt);
    auto market = _market.get(symbol_swap(amt.symbol).raw());
    userValueofcol += amt.amount / std::pow(10.0, amt.symbol.precision()) * ((double)market.marketdata.price.back() / pricePrecision);
  }

  // subtract debt from users debt
  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(lastInsurer)
      debtshare = user.debt;
    modified_user.debt -= debtshare;
  });
  // add debt to insurers debt
  _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
    modified_user.debt += debtshare;
  });
  debt_t.push_back(debtshare);

  if(debtshare.amount == 0) {
    if(user.usern.value != _config.getFinalReserve().value)
      _bailout.emplace(get_self(), [&](auto& b) {
        b.id = _bailout.available_primary_key();
        b.timestamp = eosio::current_time_point();
        b.usern = insurerUser.usern;
        b.bailoutid = _g.bailoutid;
        b.type = eosio::name("recap");
        b.pcts = insurerpcts;
        b.debt = debt_t;
        b.collateral = collateral_t;
      });
    return;
  }

  // recap method 1: if insurer has VIGOR in their insurance, then use it to cancel insurers share of the bad debt
  double valueofins = insurerUser.valueofins + userValueofcol;  //adjusted value of insurance to include the newly acquired collateral
  eosio::symbol vigorSymbol = VIGOR_SYMBOL;
  auto insurerInsurances = insurerUser.getInsurances();
  eosio::name btype = eosio::name("recap");

  if(!(_b.stepdata2 == 1)) {
    if(insurerInsurances.hasAsset(vigorSymbol)) {
      _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
        eosio::asset amt = insurerInsurances.getAsset(vigorSymbol);
        amt.amount = std::min(amt.amount, debtshare.amount);
        double vigorprice = ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision);
        valueofins -= vigorprice * (amt.amount / std::pow(10.0, amt.symbol.precision()));  //adjusted value of insurance to include cancelling some debt
        debtshare -= amt;                                                                  // remaining debtshare after cancelling some debt
        recap1_t.push_back(amt);
         
        insurerInsurances -= amt; // subtract VIGOR from insurers insurance
        _g.getInsurances() -= amt; // subtract VIGOR from global insurance
        modified_user.debt -= amt; // subtract VIGOR from insurers debt
        _g.totaldebt -= amt; // subtract VIGOR from global debt

        //clear the debt from circulating supply
        auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
        eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                      symbolIterator->contract,  // contract for vigor token
                      eosio::name("retire"),
                      std::make_tuple(amt, std::string("Retire " + amt.to_string() + " from " + insurerUser.usern.to_string() + " insurance to cancel bailout debt ")))
          .send();
#endif
      });
    }
  } else {
    uint64_t usavings = insurerUser.savings * std::pow(10.0, VIGOR_SYMBOL.precision());
    if(usavings > 0) {
      _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
        eosio::asset amt = eosio::asset(usavings, vigorSymbol);
        amt.amount = std::min(amt.amount, debtshare.amount);
        debtshare -= amt;  // remaining debtshare after cancelling some debt
        recap1_t.push_back(amt);

        // subtract VIGOR from savers savings, and global savings, and insurers debt
        modified_user.savings -= amt.amount / std::pow(10.0, amt.symbol.precision());
        modified_user.debt -= amt;

        _g.savings -= amt.amount / std::pow(10.0, amt.symbol.precision());
        // subtract VIGOR from global debt
        _g.totaldebt -= amt;

        //clear the debt from circulating supply
        auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
#ifdef TEST_MODE
#else
        eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                      symbolIterator->contract,  // contract for vigor token
                      eosio::name("retire"),
                      std::make_tuple(amt, std::string("Retire " + amt.to_string() + " from " + insurerUser.usern.to_string() + " saver to cancel bailout debt ")))
          .send();
#endif
      });
    }
    btype = eosio::name("recapsaver");
  }

  if(debtshare.amount == 0 || valueofins <= 0.0) {  //no debt left to be recapped, or no insurance assets available for recap
    if(_b.stepdata2 == 1 || user.usern.value != _config.getFinalReserve().value)
      _bailout.emplace(get_self(), [&](auto& b) {
        b.id = _bailout.available_primary_key();
        b.timestamp = eosio::current_time_point();
        b.usern = insurerUser.usern;
        b.bailoutid = _g.bailoutid;
        b.type = btype;
        b.pcts = insurerpcts;
        b.debt = debt_t;
        b.collateral = collateral_t;
        b.recap1 = recap1_t;
      });
    return;
  }

  // recap method 2: insurers automatically convert some of their insurance assets into collateral to recapitalize the bad debt
  // recapReq: required amount of insurance assets to be converted to collateral to recap the failed loan; overcollateralize to cover a 2 standard deviations monthly event
  double sp = std::sqrt(portfolioVariance(*insurerUser.getInsurances(), valueofins) / 12.0);                                           // volatility of the particular insurers insurance portfolio, monthly
  double recapReq = std::min((debtshare.amount / std::pow(10.0, debtshare.symbol.precision())) * ((_config.getBailoutCR() / 100) + 2.0 * sp) / valueofins, 1.0);  // recapReq as a percentage of the insurers insurance assets, size to cover a 2 sigma monthly move

  // subtract the recap amount from the insurers insurance, and global insurance. And add it to insurers collateral and global collateral
  for(const auto& insurance : asset_container_t(*insurerInsurances)) {
    eosio::asset amt = insurance;
    amt.amount *= recapReq;
    amt.amount = std::max(amt.amount, (int64_t)1);
    eosio::asset blocked = eosio::asset(0, amt.symbol); // if token amount breaches maxlends limit, then insurers can't withdraw nor convert to collateral for recap

    auto globalInsurances = _g.getInsurances();
    eosio::asset globalInsurance = eosio::asset(0, amt.symbol);
    globalInsurances.hasAsset(insurance, globalInsurance);
     if(globalInsurance.amount >= amt.amount) {  // recap was not blocked due to maxlends
       //subtract the full recap amount from the insurers insurance, and global insurance. And add it to insurers collateral and global collateral.
       _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
         getInsurances(modified_user) -= amt;
         getCollaterals(modified_user) += amt;
       });
       recap2_t.push_back(amt);
     } else {  // some or all of the recap was blocked due to maxlends subtract partial recap amount from the insurers insurance, and global insurance
       // recap partially as much as is available
       _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
         getInsurances(modified_user) -= globalInsurance;
         getCollaterals(modified_user) += globalInsurance;
       });
       blocked = amt - globalInsurance;
       if (globalInsurance.amount>0)
        recap2_t.push_back(globalInsurance);
     }

    if(blocked.amount > 0 && insurerUser.usern != _config.getFinalReserve()) {
      // send the blocked amount from the insurers insurance to the reserve insurance, and an equivalent dollar worth of debt from the insurer debt to the reserve debt (up to the max amount of debt the insurer has)
      auto market = _market.get(symbol_swap(blocked.symbol).raw());
      double debtpct = std::min((((blocked.amount) / std::pow(10.0, blocked.symbol.precision())) * ((double)market.marketdata.price.back() / pricePrecision)) / (insurerUser.debt.amount / std::pow(10.0, insurerUser.debt.symbol.precision())), 1.0);     // blocked amt as a pct of insurer debt
      double blockedpct = std::min((insurerUser.debt.amount / std::pow(10.0, insurerUser.debt.symbol.precision())) / (((blocked.amount) / std::pow(10.0, blocked.symbol.precision())) * ((double)market.marketdata.price.back() / pricePrecision)), 1.0);  // insurer debt as a pct of the blocked amt
      blocked.amount *= blockedpct;
      blocked.amount = std::max(blocked.amount, (int64_t)1);
      blockedins_t.push_back(blocked);
      blockeddebt_t.push_back(eosio::asset(insurerUser.debt.amount * debtpct, insurerUser.debt.symbol));

      _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
        modified_user.getInsurances() -= blocked;
      });
      auto res = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");
      _u.modify(res, get_self(), [&](auto& modified_user) {
        modified_user.getInsurances() += blocked;
      });
      uint64_t debtamt = std::max((uint64_t)(insurerUser.debt.amount * debtpct), (uint64_t)1);
        if ( insurerUser.debt.amount > 0 ) {
        _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
          modified_user.debt.amount -= debtamt;
        });
        _u.modify(res, get_self(), [&](auto& modified_user) {
          modified_user.debt.amount += debtamt;
        });
      }
      if(insurerUser.debt.amount == 0)
        break;
    }
  }
  if(_b.stepdata2 == 1 || user.usern.value != _config.getFinalReserve().value)
    _bailout.emplace(get_self(), [&](auto& b) {
      b.id = _bailout.available_primary_key();
      b.timestamp = eosio::current_time_point();
      b.usern = insurerUser.usern;
      b.bailoutid = _g.bailoutid;
      b.type = btype;
      b.pcts = insurerpcts;
      b.debt = debt_t;
      b.collateral = collateral_t;
      b.recap1 = recap1_t;
      b.recap2 = recap2_t;
      b.blockedins = blockedins_t;
      b.blockeddebt = blockeddebt_t;
    });
}

// if a Crypto borrower's debt becomes undercollateralized, move their debt and associated collateral to the insurers comensurate with l_pcts
// insurers receiving the bailout, will automatically recapitalized the debt
// borrowers retain excess collateral
void vigorlending::bailoutup(const eosio::name& usern) {
  

  require_auth(get_self());

  auto userIterator = _u.find(usern.value);
  const auto& user = *userIterator;

  bool logmax = false;
  auto end_itr = _bailout.end();
  if(_bailout.begin() != end_itr) {
    end_itr--;
    uint64_t count = (end_itr->id - _bailout.begin()->id) + 1;
    if(count >= _config.getLogCount())
      logmax = true;
  }
  // Before bailout, set aside user excess collateral, to be given back to user after the bailout
  if(_b.usern.value == _u.begin()->usern.value) {
    _b.sumpcts = 0.0;
    _b.lastflag = false;
    _b.excesspct = (user.l_valueofcol) / (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()));
    _b.excesspct *= _config.getBailoutupCR() / 100;
    std::string message;
    if(_g.bailoutid == INT64_MAX)
      _g.bailoutid = 1;
    else
      _g.bailoutid += 1;
    _b.asset = eosio::asset(0, user.l_debt.symbol);
    if(_b.excesspct < 1.0) {
      _b.asset = user.l_debt;
      _b.asset.amount *= (1.0 - _b.excesspct);
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.l_debt -= _b.asset;
      });
    }
    if (user.l_debt.amount > 0) {
      double vigordaccut = (user.l_valueofcol) / (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision()));
      if (vigordaccut < 1.0){
        _g.liqcut += (user.l_debt.amount / std::pow(10.0, user.l_debt.symbol.precision())) * (1.0 - vigordaccut) * _config.getLiqCut();
        eosio::asset amt = user.l_debt;
        amt.amount *= (1.0 - vigordaccut) * _config.getLiqCut();
        _u.modify(user, get_self(), [&](auto& modified_user) {
          modified_user.l_debt -= amt;
        });
        auto vigordacfund = _u.require_find(_config.getVigorDacFund().value, "VigorDacFund not found");
        _u.modify(vigordacfund, get_self(), [&](auto& modified_user) {
          modified_user.l_debt += amt;
        });
      }
    }
    if (_b.asset.amount > 0) {
      message.append(std::string("Borrower sent to bailoutup : " + user.usern.to_string() + " liquidated crypto debt: (dollar value) " + std::to_string(user.l_valueofcol) + " " + user.getLCollaterals().to_string() + " liquidated collateral: " + user.l_debt.to_string() + ". user retains excess collateral: " + _b.asset.to_string() + " "));
    } else {
      message.append(std::string("Borrower sent to bailoutup : " + user.usern.to_string() + " liquidated crypto debt: (dollar value) " + std::to_string(user.l_valueofcol) + " " + user.getLCollaterals().to_string() + " liquidated collateral: " + user.l_debt.to_string() + " "));
    }
    if(logmax)
      _bailout.erase(_bailout.begin());
    _bailout.emplace(get_self(), [&](auto& b) {
      b.id = _bailout.available_primary_key();
      b.timestamp = eosio::current_time_point();
      b.usern = user.usern;
      b.bailoutid = _g.bailoutid;
      b.type = eosio::name("liquidateup");
      b.pcts = user.l_pcts / _g.l_pcts;
      b.debt = user.getLCollaterals().getContainer();
      b.collateral.push_back(user.l_debt);
    });
    const eosio::name& userntrace = user.usern;
    const uint64_t bailoutid = _g.bailoutid;
    tracetable(__func__, __LINE__, userntrace, bailoutid, message);
    //  Find the last insurer, to take the dust
    if(user.usern.value != _config.getFinalReserve().value && user.l_pcts == 1.0) {
      _b.lastinsurer = user.usern;
      _b.lastflag = true;
    } else {
      const auto& lastInsurer = *std::find_if(_u.rbegin(), _u.rend(), [&](const auto& item) {
        return item.l_pcts > 0 && item != user;
      });
      _b.lastinsurer = lastInsurer.usern;
    }
  }
  auto lastinsureritr = _u.find(_b.lastinsurer.value);
  const auto& lastInsurer = *lastinsureritr;

  //  The particular user being bailout out, is to be processed last (if the user is also an insurer, would participate in self bailout)
  //  doing so makes the math simple ratios
  auto itr = _u.lower_bound(_b.usern.value);
  auto stop_itr = _u.end();
  uint64_t counter = 0;

  while(itr != stop_itr && counter++ < _b.batchsize) {
    const auto& insurerUser = *itr;
    if(_b.lastflag || user.l_valueofcol == 0 || itr->l_pcts <= 0 || insurerUser == user) {  //burn the wick
      itr++;
      continue;
    }

    if(logmax)
      _bailout.erase(_bailout.begin());

    recapup(user, insurerUser, insurerUser.l_pcts / _g.l_pcts, _b.sumpcts, (insurerUser == lastInsurer) && user.l_pcts <= 0);

    if(insurerUser == lastInsurer)
      _b.lastflag = true;
    itr++;
  }

  if(itr == _u.end()) {
    if(user.l_pcts > 0) {
      if(logmax)
        _bailout.erase(_bailout.begin());
      recapup(user, user, user.l_pcts / _g.l_pcts, _b.sumpcts, true);
    }

    // After bailout, release excess collateral back to user
    if(_b.asset.amount > 0)
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.l_debt += _b.asset;
      });

    _u.modify(user, get_self(), [&](auto& modified_user) {
      modified_user.resetReputation();
    });
    if(user.usern == _g.bailoutupuser)
      _g.bailoutupuser = eosio::name(0);

    _b.usern = _u.begin()->usern;
    _g.step = 7;
    
    switch(_config.getExecType()) {
      case 2: {  // mining
        uint64_t step1 = 1;
        schaction(get_self(), eosio::name("predoupdate"), std::make_tuple(step1), 0, _config.getMineBuffer(), 1);
        break;
      }
    }
  } else {
    _b.usern = itr->usern;
  }
}

void vigorlending::recapup(const user_s& user, const user_s& insurerUser, double insurerlpcts, double& sumpcts, bool lastInsurer) {
      
  double valueofins = insurerUser.valueofins;
  const eosio::name& userntrace = insurerUser.usern;
  const uint64_t bailoutid = _g.bailoutid;
  double l_pcts = insurerlpcts * (1.0 / (1.0 - sumpcts));
  sumpcts += insurerlpcts;
  eosio::asset l_debtshare = user.l_debt;  // insurer share of the the l_debt
  l_debtshare.amount *= l_pcts;
  l_debtshare.amount = std::min(std::max(l_debtshare.amount, (int64_t)1), user.l_debt.amount);
  double l_valueofcol = insurerUser.l_valueofcol;
  double W1 = 0.0;
  auto userLCollaterals = getLCollaterals(const_cast<user_s&>(user));
  asset_container_t debt_t, collateral_t, recap1_t, recap2_t, recap3_t, blockedins_t, blockeddebt_t;
  asset_container blockeddebt(blockeddebt_t);
  // assign ownership of the l_collateral and l_debt to the insurers
  eosio::asset total;
  for(const auto& userLCollateral : asset_container_t(*userLCollaterals)) {
    eosio::asset amt = userLCollateral;
    amt.amount *= l_pcts;
    amt.amount = std::max(amt.amount, (int64_t)1);

    _u.modify(user, get_self(), [&](auto& modified_user) {
      if(lastInsurer)
        amt = userLCollateral;
      userLCollaterals -= amt; // subtract borrows from user l_collateral and global l_collateral
    });
    _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
      getLCollaterals(modified_user) += amt; // add borrows to insurers l_collateral and global l_collateral
    });
    debt_t.push_back(amt);
    // recap method 1: if the insurer has the borrowed token type in their insurance then use it to paybacktok
    auto market = _market.get(symbol_swap(amt.symbol).raw());
    W1 += (amt.amount) / std::pow(10.0, amt.symbol.precision()) * ((double)market.marketdata.price.back() / pricePrecision); 
    eosio::asset userInsurance;
    if (insurerUser.getInsurances().hasAsset(amt, userInsurance)){
        eosio::asset amti = userInsurance;
        amti.amount = std::min(amti.amount, amt.amount);
        recap1_t.push_back(amti);
        valueofins -= (amti.amount) / std::pow(10.0, userInsurance.symbol.precision()) * ((double)market.marketdata.price.back() / pricePrecision);  //adjusted value of insurance to include cancelling some of the borrow 
        W1 -= (amti.amount) / std::pow(10.0, amti.symbol.precision()) * ((double)market.marketdata.price.back() / pricePrecision);          //adjusted value of the share of l_collateral to include cancelling some of the borrow
        paybacktok(insurerUser, amti);

        // subtract borrow from insurers insurance and global insurance
        _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
          getInsurances(modified_user) -= amti;
        });
    }
  }

  // subtract l_debt from users l_debt
  _u.modify(user, get_self(), [&](auto& modified_user) {
    if(lastInsurer)
      l_debtshare = user.l_debt;  // the amount allocated to last insurer should bring the l_debt to zero otherwise it is dust leftover
    modified_user.l_debt -= l_debtshare;
  });
  // add l_debt to insurers l_debt
  _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
    modified_user.l_debt += l_debtshare;
  });
  collateral_t.push_back(l_debtshare);

  if(W1 <= 0) {
    if(user.usern.value != _config.getFinalReserve().value)
      _bailout.emplace(get_self(), [&](auto& b) {
        b.id = _bailout.available_primary_key();
        b.timestamp = eosio::current_time_point();
        b.usern = insurerUser.usern;
        b.bailoutid = _g.bailoutid;
        b.type = eosio::name("recapup");
        b.pcts = insurerlpcts;
        b.debt = debt_t;
        b.collateral = collateral_t;
        b.recap1 = recap1_t;
      });
    return;
  }
  // recap method 2: If insurer has VIGOR in insurance then use it as l_debt to collateralize insurers share of adjusted l_collateral W1
  // move VIGOR from insurance into l_debt
  double sp = user.l_volcol / std::sqrt(12.0);       
  double vigorneeded = std::max((W1 * ((_config.getBailoutupCR() / 100) + 2.0 * sp) - (l_debtshare.amount / std::pow(10.0, l_debtshare.symbol.precision()))), 0.0);  // need this many VIGOR to recap the borrowed risky tokens, enough to cover a 2 monthly standard deviation upward move       
  eosio::asset vigorneededa = eosio::asset(std::max((uint64_t)(vigorneeded * std::pow(10.0, VIGOR_SYMBOL.precision())),(uint64_t)1), VIGOR_SYMBOL);
  eosio::asset userInsurance = eosio::asset(0, vigorneededa.symbol);
   if (insurerUser.getInsurances().hasAsset(vigorneededa, userInsurance)){
      eosio::asset amt = userInsurance;
      amt.amount = std::min(amt.amount, vigorneededa.amount);
      double vigorprice = ((double)_market.get(VIGOR_SYMBOL.raw()).marketdata.price.back() / pricePrecision);
      valueofins -= vigorprice * (amt.amount / std::pow(10.0, amt.symbol.precision()));  //adjusted value of insurance to include moving some VIGOR into l_debt
      vigorneededa -= amt;                                                               // adjusted value, need this many VIGOR to recap the borrowed risky tokens
      recap2_t.push_back(amt);

      _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
        getInsurances(modified_user) -= amt; // subtract VIGOR from insurers insurance, global insurance
        modified_user.l_debt += amt; // add to insurers l_debt
      });
      _g.l_totaldebt += amt; // add VIGOR to global l_debt
   }
  
  if(vigorneededa.amount <= 0 || W1 <= 0 || valueofins <= 0.0) {  // check if recap is still needed
    if(user.usern.value != _config.getFinalReserve().value)
      _bailout.emplace(get_self(), [&](auto& b) {
        b.id = _bailout.available_primary_key();
        b.timestamp = eosio::current_time_point();
        b.usern = insurerUser.usern;
        b.bailoutid = _g.bailoutid;
        b.type = eosio::name("recapup");
        b.pcts = insurerlpcts;
        b.debt = debt_t;
        b.collateral = collateral_t;
        b.recap1 = recap1_t;
        b.recap2 = recap2_t;
      });
    return;
  }
  // recap method 3: borrow VIGOR against insurance (move insurance to collateral) to recap the borrowed tokens
  double spi = std::sqrt(portfolioVariance(*insurerUser.getInsurances(), valueofins) / 12.0);  // volatility of the insurer insurance portfolio, monthly
  // recapReq: amount of collateral needed to recap the risky debt (enough to cover a 2 monthly std upward move),
  // against which the insurer can borrow VIGOR to recap the borrowed risky tokens, as a pct of insurers insurance
  double recapReq = std::min((((_config.getBailoutCR() / 100) + 2.0 * spi) * (vigorneededa.amount / std::pow(10.0, vigorneededa.symbol.precision()))) / valueofins, 1.0); // recap needed as a pct of how much is available
  double vigorborrow = std::min( (vigorneededa.amount / std::pow(10.0, vigorneededa.symbol.precision())), valueofins / ((_config.getBailoutCR() / 100) + 2.0 * spi) );
  eosio::asset vigorborrowa = eosio::asset((uint64_t)(vigorborrow * std::pow(10.0, VIGOR_SYMBOL.precision())), VIGOR_SYMBOL); // allowed zero if dust insurer, proceed to move dust insurance to collateral so they cease being a dust insurer
  auto symbolIterator = _whitelist.find(VIGOR_SYMBOL.raw());
  currency_statst _currency_statst(symbolIterator->contract, VIGOR_SYMBOL.code().raw());
  auto existing = _currency_statst.find(VIGOR_SYMBOL.code().raw());
  if(existing != _currency_statst.end()) {
    const auto& st = *existing;
    if(st.supply.amount + vigorborrowa.amount > _config.getDebtCeiling() * std::pow(10.0, VIGOR_SYMBOL.precision()))
      return;
  }
  // subtract the recap amount from the insurers insurance, and global insurance. And add it to insurers collateral and global collateral
  for(const auto& insurance : asset_container_t(*insurerUser.getInsurances())) {
    eosio::asset amt = insurance;
    amt.amount *= recapReq;
    amt.amount = std::max(amt.amount, (int64_t)1);
    eosio::asset blocked = eosio::asset(0, amt.symbol);  // if token amount breaches maxlends limit, then insurers can't withdraw nor convert to collateral for recap

    auto globalInsurances = _g.getInsurances();
    eosio::asset globalInsurance = eosio::asset(0, amt.symbol);
    globalInsurances.hasAsset(insurance, globalInsurance);
     if(globalInsurance.amount >= amt.amount) {  // recap was not blocked due to maxlends
       //subtract the full recap amount from the insurers insurance, and global insurance. And add it to insurers collateral and global collateral
       _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
         getInsurances(modified_user) -= amt;
         getCollaterals(modified_user) += amt;
       });
       recap3_t.push_back(amt);
     } else {  // some or all of the recap was blocked due to maxlends
       // recap partially as much as is available
       _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
         getInsurances(modified_user) -= globalInsurance;
         getCollaterals(modified_user) += globalInsurance;
       });
       //some or all of the recap was blocked due to maxlends. subtract partial recap amount from the insurers insurance, and global insurance. Add it to insurers collateral and global collateral
       blocked = amt - globalInsurance;
       if (globalInsurance.amount > 0)
        recap3_t.push_back(globalInsurance);
     }

    if(blocked.amount > 0) {
      //send the blocked amount from the insurers insurance to the reserve insurance, and an equivalent dollar worth of l_collateral from the insurer l_collateral to the reserve l_collateral (up to the max amount of l_collateral the insurer has)
      auto market = _market.get(symbol_swap(amt.symbol).raw());
      double blockedval = (((blocked.amount) / std::pow(10.0, blocked.symbol.precision())) * ((double)market.marketdata.price.back() / pricePrecision));
      double l_collateralpct = std::min(blockedval / (W1 + l_valueofcol), 1.0);  // blocked amt as a pct of insurer l_collateral
      double blockedpct = std::min((W1 + l_valueofcol) / blockedval, 1.0);       // insurer l_collateral as a pct of the blocked amt
      blocked.amount *= blockedpct;
      blocked.amount = std::max(blocked.amount, (int64_t)1);
      blockedins_t.push_back(blocked);
      vigorborrowa.amount -= blockedval * blockedpct * ((_config.getBailoutupCR() / 100) + 2.0 * sp) * std::pow(10.0, vigorborrowa.symbol.precision());
      W1 *= (1.0 - l_collateralpct);
      l_valueofcol *= (1.0 - l_collateralpct);

      // send the blocked amount from the insurers insurance to the reserve insurance
      _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
        modified_user.getInsurances() -= blocked;
      });
      auto res = _u.require_find(_config.getFinalReserve().value, "finalreserve not found");
      _u.modify(res, get_self(), [&](auto& modified_user) {
        modified_user.getInsurances() += blocked;
      });

      // subtract an equivalent dollar worth of l_collateral from the insurer l_collateral, and add to the reserve l_collateral
      // (up to the max amount of l_collateral the insurer has)
      for(const auto& insurerLCollateral : asset_container_t(*insurerUser.getLCollaterals())) {
        eosio::asset l_colequiv = insurerLCollateral;
        l_colequiv.amount *= l_collateralpct;
        l_colequiv.amount = std::max(l_colequiv.amount, (int64_t)1);
        _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
          modified_user.getLCollaterals() -= l_colequiv;
        });
        _u.modify(res, get_self(), [&](auto& modified_user) {
          modified_user.getLCollaterals() += l_colequiv;
        });
        blockeddebt += l_colequiv;
      }
    }
    if(insurerUser.getLCollaterals().isEmpty() || insurerUser.getInsurances().isEmpty() || vigorborrowa.amount <= 0)
      break;
  }

  if(vigorborrowa.amount > 0) {
    //borrow VIGOR debt against the recapReq in the insurers collateral and move it to l_debt to recap the insurers l_collateral
    _u.modify(_u.iterator_to(insurerUser), get_self(), [&](auto& modified_user) {
      modified_user.debt += vigorborrowa;
      modified_user.l_debt += vigorborrowa;
    });
    _g.totaldebt += vigorborrowa;
    _g.l_totaldebt += vigorborrowa;
#ifdef TEST_MODE
#else
    eosio::action(eosio::permission_level {get_self(), eosio::name("active")},
                  symbolIterator->contract,  // contract for vigor token
                  eosio::name("issue"),
                  std::make_tuple(get_self(), vigorborrowa, std::string("VIGOR issued to ") + get_self().to_string()))
      .send();
#endif
  }

  if(user.usern.value != _config.getFinalReserve().value)
    _bailout.emplace(get_self(), [&](auto& b) {
      b.id = _bailout.available_primary_key();
      b.timestamp = eosio::current_time_point();
      b.usern = insurerUser.usern;
      b.bailoutid = _g.bailoutid;
      b.type = eosio::name("recapup");
      b.pcts = insurerlpcts;
      b.debt = debt_t;
      b.collateral = collateral_t;
      b.recap1 = recap1_t;
      b.recap2 = recap2_t;
      b.recap3 = recap3_t;
      if (vigorborrowa.amount>0)
       b.recap3b.push_back(vigorborrowa);
      b.blockedins = blockedins_t;
      b.blockeddebt = blockeddebt.getContainer();
    });
}

double vigorlending::approx(double t) {
  // 
  // Abramowitz and Stegun formula 26.2.23.
  // The absolute value of the error should be less than 4.5 e-4.
  double c[] = {2.515517, 0.802853, 0.010328};
  double d[] = {1.432788, 0.189269, 0.001308};
  return t - ((c[2] * t + c[1]) * t + c[0]) /
               (((d[2] * t + d[1]) * t + d[0]) * t + 1.0);
}

double vigorlending::icdf(double p) {
  

  // See article above for explanation of this section.
  if(p < 0.5) {
    // F^-1(p) = - G^-1(p)
    return -approx(std::sqrt(-2.0 * std::log(p)));
  }

  // F^-1(p) = G^-1(1-p)
  return approx(std::sqrt(-2.0 * std::log(1 - p)));
}

void vigorlending::setacctsize(const eosio::name& usern, uint64_t& limit) {
  
  eosio::require_auth(get_self());

  auto userIter = _acctsize.find(usern.value);
  if(userIter != _acctsize.end())
    _acctsize.modify(userIter, get_self(), [&](auto& a) {
      a.limit = limit;
    });
  else
    _acctsize.emplace(get_self(), [&](auto& a) {
      a.limit = limit;
      a.usern = usern;
    });
}

// whitelist a token for use as collateral, insurance, borrowing, (or update the contract and feed for an already existing token)
// feed name corresponds the feed name in the oracle hub.
void vigorlending::whitelist(eosio::symbol sym, eosio::name contract, eosio::name feed, bool assetin, bool assetout, uint8_t maxlends) {
  
  eosio::require_auth(get_self());
  eosio::check(sym.is_valid(), "invalid symbol");
  eosio::check(is_account(contract), "contract account does not exist");
  t_series stats(_config.getOracleHub(), feed.value);
  eosio::check(stats.begin() != stats.end(), std::string("feed does not exist in the oracle, ") + _config.getOracleHub().to_string());
  auto itr = _whitelist.find(sym.raw());
  if(itr != _whitelist.end())
    _whitelist.modify(itr, get_self(), [&](auto& a) {
      a.contract = contract;
      a.feed = feed;
      a.assetin = assetin;
      a.assetout = assetout;
      a.maxlends = maxlends;
    });
  else
    _whitelist.emplace(get_self(), [&](auto& a) {
      a.sym = sym;
      a.contract = contract;
      a.feed = feed;
      a.assetin = assetin;
      a.assetout = assetout;
      a.maxlends = maxlends;
    });
}

// unwhitelist a token which is currently in use as collateral, insurance, borrowing
void vigorlending::unwhitelist(eosio::symbol sym) {
  
  eosio::require_auth(get_self());
  eosio::check(sym.is_valid(), "invalid symbol");
  auto itr = _whitelist.find(sym.raw());
  if(itr != _whitelist.end())
    _whitelist.erase(itr);
  auto mitr = _market.find(sym.raw());
  if(mitr != _market.end())
    _market.erase(mitr);
}

// writes messages into the log table
void vigorlending::tracetable(const char* fname, int lineno, const eosio::name& usern, const uint64_t& bailoutid, const std::string& message) {
  if(usern.value == eosio::name("beforefull").value || usern.value == eosio::name("afterfull").value) return;

  auto end_itr = _log.end();
  if(_log.begin() != end_itr) {
    end_itr--;
    uint64_t count = (end_itr->id - _log.begin()->id) + 1;
    if(count >= _config.getLogCount())
      _log.erase(_log.begin());
  }
  _log.emplace(get_self(), [&](auto& a) {
    a.id = _log.available_primary_key();
    a.timestamp = eosio::current_time_point();
    a.message = message;
    a.usern = usern;
    a.bailoutid = bailoutid;
    a.function = eosio::name(std::string(fname));
  });
}
// security feature to lock down specific actions in the contract.
// this wraps configure action, allowing for freeze to be called by a configurable number of custodians, by adding an action permission to dacauth11111@low
void vigorlending::freezelevel(std::string value) {
  
  eosio::require_auth(get_self());
  uint16_t val = static_cast<unsigned short>(std::stoul(value.c_str(), NULL, 0));
  eosio::check(val == 1 || val == 2 || val == 3 || val == 4, "choose: 1 freeze assetout, 2 freeze assetin, 3 freeze assetout & assetin, 4 freeze assetout & assetin & doupdate.");
  configure(eosio::name("freezelevel"), value);
}

void vigorlending::configure(eosio::name key, std::string value) {
  
  eosio::require_auth(get_self());

  if(key.value == eosio::name("batchsize").value) {
    eosio::check(std::stoull(value) >= 1 && std::stoull(value) < INT64_MAX, "batch size must be greater than or equal to one and less than int64_max.");
    _b.batchsize = std::stoull(value);
    return;
  }

  _config.set_value(key, value);
  _configTable.set(_config, get_self());

  if(key.value == eosio::name("repanniv").value) {
    for(const auto& user : _u) {
      _u.modify(user, get_self(), [&](auto& modified_user) {
        modified_user.setVig_since_anniv(eosio::asset(0, _g.fee.symbol));
        modified_user.setAnniv(eosio::current_time_point(), _config.getRepAnniv());
      });
    }
  }

  if(key.value == eosio::name("kickseconds").value)
    _g.kicktimer = eosio::time_point_sec(eosio::current_time_point().sec_since_epoch() + _config.getKickSeconds());
}

void vigorlending::cleanbailout(const uint32_t batchSize) {
  eosio::require_auth(get_self());
  uint32_t counter = 0;
  auto itr = _bailout.begin();
  while(itr != _bailout.end() && counter++ < batchSize) {
    itr = _bailout.erase(itr);
  }
}

void vigorlending::clearconfig() {
  eosio::require_auth(get_self());
  vigorlending::clearSingletons = true;
  _configTable.remove();
}

void vigorlending::setconfig(const config& config) {
  eosio::require_auth(get_self());
  vigorlending::clearSingletons = true;
  _configTable.set(config, get_self());
}

void vigorlending::clearglobal() {
  eosio::require_auth(get_self());
  vigorlending::clearSingletons = true;
  _globals.remove();
}

void vigorlending::setglobal(const globalstats& globalstats) {
  eosio::require_auth(get_self());
  vigorlending::clearSingletons = true;
  _globals.set(globalstats, get_self());
}


#if defined(DEBUG)

void vigorlending::dbgclrqueue(const uint32_t numrows) {
  eosio::require_auth(get_self());
  cleanTable<croneos::croneosqueue_table>(get_self(), get_self().value, numrows);
}

void vigorlending::dbgresetall(const uint32_t numrows) {
  eosio::require_auth(get_self());
  cleanTable<user_t>(get_self(), get_self().value, numrows);
  cleanTable<whitelist_t>(get_self(), get_self().value, numrows);
  cleanTable<market_t>(get_self(), get_self().value, numrows);
  cleanTable<staked>(get_self(), get_self().value, numrows);
  cleanTable<acctsize_t>(get_self(), get_self().value, numrows);
  cleanTable<log_t>(get_self(), get_self().value, numrows);
  cleanTable<croneos::croneosqueue_table>(get_self(), get_self().value, numrows);
  cleanTable<bailout_t>(get_self(), get_self().value, numrows);
  vigorlending::clearSingletons = true;

  _batche.set(batchse(), get_self());
  _configTable.set(config(), get_self());
  _globals.set(globalstats(), get_self());

  croneos::queue _queue(get_self());
  _queue.resetStats();
}

void vigorlending::dbggivevig(const uint64_t quantity, const eosio::name usrn, const std::string memo) {
  // eosio::require_auth(get_self());
  vigorlending::assetin(usrn, get_self(), eosio::asset(quantity, VIG10_SYMBOL), memo);
}
#endif