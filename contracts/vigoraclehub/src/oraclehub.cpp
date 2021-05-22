#include "oraclehub.hpp"

#include "daccustodian.hpp"
#include "delphioracle.hpp"
#include "swap.defi.hpp"
#include "curve.sx.hpp"


namespace oraclehub {

// CHECKERS

// Check if calling account is a qualified oracle, a custodian and/or an approved account
void oraclehub_contract::check_oracle(const eosio::name owner) {
  bool oracle;
    // check if owner is a DAC custodian
  eosdac::custodians_table custodians(dac_custodian, dac_custodian.value);
  oracle = (custodians.find(owner.value) != custodians.end());

  // doublecheck it's an approved account
  oracles_table oracles(_self, _self.value);
  auto itr = oracles.find(owner.value);
  if (itr != oracles.end()) {
    oracle = itr->approved;
  }

  eosio::check(oracle, "account is not an approved oracle");
}

// Ensure account cannot push data more often than every 60 seconds
void oraclehub_contract::check_last_push(const eosio::name owner) {
  oracles_table oracles(_self, _self.value);
  /*
  oracles_table oracles_pair(_self, pair.value);

  auto itr = oracles_pair.find(owner.value);
  if (itr != oracles_pair.end()) {
    eosio::time_point ctime = eosio::current_time_point();

    auto last = *itr;
    eosio::check(last.timestamp.sec_since_epoch() + one_minute - cronlag <= ctime.sec_since_epoch(), "can only call once every 60 seconds");

    oracles_pair.modify(itr, _self, [&](auto &s) {
      s.timestamp = ctime;
      s.count++;
      s.approved = true;
    });
  } else {
    oracles_pair.emplace(_self, [&](auto &s) {
      s.owner = owner;
      s.timestamp = eosio::current_time_point();
      s.count = 1;
      s.approved = true;
      s.balance = eosio::asset(0, eosio::symbol("VIG", 4));
    });
  }
 */
  auto gitr = oracles.find(owner.value);
  if (gitr != oracles.end()) {

    eosio::time_point ctime = eosio::current_time_point();

    eosio::check(gitr->timestamp.sec_since_epoch() + one_minute - cronlag <= ctime.sec_since_epoch(), "can only call once every 60 seconds");

    oracles.modify(gitr, _self, [&](auto &s) {
      s.timestamp = ctime;
      s.count++;
      s.approved = true;
    });
  } else {
    oracles.emplace(_self, [&](auto &s) {
      s.owner = owner;
      s.timestamp = eosio::current_time_point();
      s.approved = true;
      s.count = 1;
      s.balance = eosio::asset(0, eosio::symbol("VIG", 4));
    });
  }
}

// Ensure account cannot push data more often than every 60 seconds
void oraclehub_contract::check_last_pushd(const eosio::name owner) {
  
  oracles_table oracles_pair(_self, eosio::name("swap.defi").value);

  auto itr = oracles_pair.find(owner.value);
  if (itr != oracles_pair.end()) {
    eosio::time_point ctime = eosio::current_time_point();

    auto last = *itr;
    eosio::check(last.timestamp.sec_since_epoch() + one_minute - cronlag <= ctime.sec_since_epoch(), "can only call once every 60 seconds");

    oracles_pair.modify(itr, _self, [&](auto &s) {
      s.timestamp = ctime;
      s.count++;
      s.approved = true;
    });
  } else {
    oracles_pair.emplace(_self, [&](auto &s) {
      s.owner = owner;
      s.timestamp = eosio::current_time_point();
      s.count = 1;
      s.approved = true;
      s.balance = eosio::asset(0, eosio::symbol("VIG", 4));
    });
  }

}

// HELPERS

// Push oracle message on top of queue, pop old elements (older than one minute)
void oraclehub_contract::update_datapoints(const eosio::name owner, const uint64_t value, eosio::name pair, const uint64_t oraclecount) {
  datapoints_table dstore(_self, pair.value);
  eosio::time_point ctime = eosio::current_time_point();

  uint64_t size = 0;
  auto end_itr = dstore.end();
  if(dstore.begin() != end_itr ){
      end_itr--;
      size = ( end_itr->id - dstore.begin()->id ) + 1;
  }

  uint64_t median = 0;
  uint64_t primary_key;

  // Find median
  if (size > 0) {

    // Pop old points (older than one minute)
    auto latest = dstore.begin();
    while (latest != dstore.end()) {
      if (latest->timestamp.sec_since_epoch() + one_minute - cronlag <= ctime.sec_since_epoch()) {
        latest = dstore.erase(latest);
        size--;
      } else {
          latest++;
      }
    }
  }

    // overweight the median coming in from delphi by replicating it as many times as there are oracles
    uint64_t weight = 1;
    if ( owner.value == DELPHIPOLLER_CONTRACT.value )
      weight = (uint64_t)ceil(std::max(oraclecount-(uint64_t)1,(uint64_t)1)/2.0);
  
    for ( uint64_t w = 1; w <= weight; w++) {
      // Calculate new primary key by substracting one from the previous one
      if ( size == 0) {
        dstore.emplace(_self, [&](auto &s) {
          s.id = std::numeric_limits<unsigned long long>::max();
          s.owner = owner;
          s.value = value;
          s.median = value;
          s.timestamp = eosio::current_time_point();
        });
        size++;
      } else {
        auto latest = dstore.begin();
        primary_key = latest->id - 1;

      // Insert next datapoint
      auto c_itr = dstore.emplace(_self, [&](auto &s) {
        s.id = primary_key;
        s.owner = owner;
        s.value = value;
        s.timestamp = ctime;
      });
      size++;
      // Get index sorted by value
      auto value_sorted = dstore.get_index<eosio::name("value")>();
      uint64_t mid = (uint64_t)floor(size / 2.0);
      auto itr = value_sorted.begin();
      for (int i = 0; i < mid; i++) {
        itr++;
      }
      median = itr->value;
      
      // set median
      dstore.modify(c_itr, _self, [&](auto &s) { s.median = median; });
      }
    }
}

// Get last price from datapoints table (from datapreproc)
uint64_t oraclehub_contract::get_last_price(eosio::name pair, uint64_t quoted_precision) {
  uint64_t eosusd = 1;
  uint64_t eos_precision = 1;
  datapoints_table dstoreosname(_self, eosio::name("eosusd").value);
  eosio::check( dstoreosname.begin() != dstoreosname.end(), std::string("eosusd is not found in the datapoints table of the oraclehub, ") + get_self().to_string() );
  auto dstoreos = dstoreosname.get_index<eosio::name("timestamp")>();
  auto newesteos = dstoreos.begin();
  if (newesteos != dstoreos.end()) {
    auto newesteos = dstoreos.end();
    newesteos--; // the last element is the latest
    pairs_table pairs(_self, _self.value);
    auto eospair = pairs.get("eosusd"_n.value);
    eos_precision = eospair.quoted_precision;
    eosusd = newesteos->median;
  }
  datapoints_table dstorename(_self, pair.value);
  auto dstore = dstorename.get_index<eosio::name("timestamp")>();
  auto newest = dstore.begin();
  if (newest != dstore.end()) {
    auto newest = dstore.end();
    newest--; // the last element is the latest
    if (pair == eosio::name("eosusd") || pair == eosio::name("btcusd") || pair == eosio::name("ethusd") || pair == eosio::name("sxe"))
      return (uint64_t)(pricePrecision*(newest->median/std::pow(10.0,quoted_precision)));
    else if (pair == eosio::name("eosvigor") || pair == eosio::name("eosusdt"))
      return (uint64_t)(pricePrecision*( (eosusd/std::pow(10.0,eos_precision)) / (newest->median/std::pow(10.0,quoted_precision)) ));
    else
      return (uint64_t)(pricePrecision*((newest->median/std::pow(10.0,quoted_precision)) * (eosusd/std::pow(10.0,eos_precision))));
  }
  return 0;
}

// Calculate vol and correlation matrix (from datapreproc)
void oraclehub_contract::calcstats(const eosio::name pair, const uint32_t freq) {
  prices_table store(_self, pair.value);
  auto itr = store.find(freq);
  if (itr != store.end()) {
    auto& itrref = *itr;
    uint64_t vol = defaultVol;
    if ( pair == eosio::name("eosvigor") || pair == eosio::name("eosusdt") )
      vol = defaultVol/3;
    if (itrref.price.size()>5)
      vol = EWMAvolCalc(itrref.returns, itrref.returns.size(), time_series_periodicity, vol);
    store.modify( itrref, _self, [&]( auto& s ) {
      s.vol = vol;
    });
    pairs_table pairs(_self, _self.value);
    for ( auto jt = pairs.begin(); jt != pairs.end(); jt++ ) {
        if (jt->name == eosio::name("vigoreos"))
          continue;
        prices_table storej(_self, jt->name.value);
        auto jtr = storej.find(freq);
        int64_t corr = defaultCorr;
        if (jtr != storej.end() && jtr->price.size()==itrref.price.size() && itrref.price.size()>5)
          corr = corrCalc(itrref.returns, jtr->returns, itrref.returns.size());
        store.modify( itrref, _self, [&]( auto& s ) {
          if ( jt->name == eosio::name("eosvigor") || jt->name == eosio::name("eosusdt") )
            s.correlation_matrix[jt->quote_symbol] = corr;
          else
            s.correlation_matrix[jt->base_symbol] = corr;
        });
    }
  }
}

// Correlation coefficient
int64_t oraclehub_contract::corrCalc(std::deque<int64_t> X, std::deque<int64_t> Y, uint64_t n) {   
  double sum_X = 0.0, sum_Y = 0.0, sum_XY = 0.0, x = 0.0, y = 0.0;
  double squareSum_X = 0.0, squareSum_Y = 0.0; 

  for (uint64_t i = 0; i < n; i++) 
  { 
    x = X[i]/returnsPrecision;
    y = Y[i]/returnsPrecision;
    sum_X = sum_X + x; 
    sum_Y = sum_Y + y; 
    sum_XY = sum_XY + x * y; 
    squareSum_X = squareSum_X + x * x; 
    squareSum_Y = squareSum_Y + y * y; 
  }
  int64_t corr;
  if ((n * squareSum_X - sum_X * sum_X) * (n * squareSum_Y - sum_Y * sum_Y) == 0.0 )
    corr = defaultCorr;
  else
    corr = (int64_t)std::min(std::max((returnsPrecision)*(n * sum_XY - sum_X * sum_Y)
                / sqrt((n * squareSum_X - sum_X * sum_X)
                    * (n * squareSum_Y - sum_Y * sum_Y)),-1000000.0),1000000.0); 
  return corr;
}

// Volatility calculation
uint64_t oraclehub_contract::volCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq, uint64_t dflt) {
  double variance = 0.0;
  double t = returns[0]/returnsPrecision;
  for (int i = 1; i < n; i++)
  {
      t += returns[i]/returnsPrecision;
      double diff = ((i + 1) * returns[i]/returnsPrecision) - t;
      
      variance += (diff * diff) / ((i + 1) *i);

  }
  if ((variance / (n - 1)) == 0.0)
    return dflt;
  else
    return std::max(std::min((uint64_t)(volScale.at(freq)*(returnsPrecision*sqrt(variance / (n - 1)))),(uint64_t)(2.5*dflt)),(uint64_t)(100000));
}

// Volatility calculation
uint64_t oraclehub_contract::EWMAvolCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq, uint64_t dflt) {
double rw = 0.0;
  for (int i = 0; i < n; i++)
      rw += ( 1.0 - 0.94 ) * std::pow( 0.94, (double)( n - (uint64_t)1 - (uint64_t)i ) ) * std::pow( returns[i]/returnsPrecision, 2.0 );
  if ( rw == 0.0 )
    return dflt;
  else
    return std::max(std::min((uint64_t)(volScale.at(freq)*(returnsPrecision*sqrt(rw))),(uint64_t)(2.5*dflt)),(uint64_t)(100000));
}

// Store last price from the oracle, append to time series
void oraclehub_contract::store_last_price(const eosio::name pair, const uint32_t freq, const uint64_t lastprice) {
  prices_table store(_self, pair.value);

  auto itr = store.find(freq);
  eosio::time_point ctime = eosio::current_time_point();
  if (itr != store.end()) {
    auto last = *itr;
    if (last.timestamp.sec_since_epoch() + time_series_periodicity - cronlag <= ctime.sec_since_epoch()) {
      if (size(last.price)==dequesize){ // append to time series, remove oldest
        store.modify( itr, _self, [&]( auto& s ) {
          s.timestamp = ctime;
          uint64_t prevprice = s.price.back();
          s.price.push_back(lastprice);
          s.returns.push_back((int64_t)(returnsPrecision*(((double)lastprice/(double)prevprice)-1.0)));
          s.price.pop_front();
          s.returns.pop_front();
        });
      } else { // append to time series, don't remove oldest
        store.modify( itr, _self, [&]( auto& s ) {
          s.timestamp = ctime;
          uint64_t prevprice = s.price.back();
          s.price.push_back(lastprice);
          s.returns.push_back((int64_t)(returnsPrecision*(((double)lastprice/(double)prevprice)-1.0)));
        });
      }    
    } else { // too early so just overwrite latest point
      store.modify( itr, _self, [&]( auto& s ) {
        s.price.back() = lastprice;
        if (s.price.size() > 1)
          s.returns.back() = (int64_t)(returnsPrecision*(((double)lastprice/(double)s.price[s.price.size()-2])-1.0));
      });
    };
  } else { // first data point
    store.emplace(_self, [&](auto& s) {
      s.freq=freq;
      s.timestamp = ctime;
      s.price.push_front(lastprice);
    });
  }
}

// ACTIONS

// Approve an oracle (auth by dac account)
[[eosio::action]]
void oraclehub_contract::approve(const eosio::name owner) {
  eosio::require_auth(get_self());

  eosio::time_point ctime = eosio::current_time_point() - eosio::microseconds(60000000);

  oracles_table oracles(_self, _self.value);

  auto itr = oracles.find(owner.value);
  if (itr != oracles.end()) {
    eosio::check(!itr->approved, "oracle already approved");

    oracles.modify(itr, _self, [&](auto &s) {
      s.approved = true;
      s.timestamp = ctime;
    });
  } else {
    oracles.emplace(_self, [&](auto &s) {
      s.owner = owner;
      s.count = 0;
      s.balance = eosio::asset(0, eosio::symbol("VIG", 4));
      s.approved = true;
      s.timestamp = ctime;
    });
  }
}

// Disapprove an oracle temporarily (auth by dac account)
[[eosio::action]]
void oraclehub_contract::disapprove(const eosio::name owner) {
  eosio::require_auth(get_self());

  eosio::time_point ctime = eosio::current_time_point();

  oracles_table oracles(_self, _self.value);

  auto itr = oracles.find(owner.value);
  eosio::check(itr != oracles.end(), "oracle not found");
  eosio::check(itr->approved, "oracle not approved");

  oracles.modify(itr, _self, [&](auto &s) {
    s.approved = false;
    s.timestamp = ctime;
  });
}

// Revoke an oracle access definitively (auth by dac account)
[[eosio::action]]
void oraclehub_contract::revoke(const eosio::name owner) {
  eosio::require_auth(get_self());

  eosio::time_point ctime = eosio::current_time_point();

  oracles_table oracles(_self, _self.value);

  auto itr = oracles.find(owner.value);
  eosio::check(itr != oracles.end(), "oracle not found");

  oracles.erase(itr);
}

// add a new pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::addpair(const pair_data pair) {

  require_auth(get_self());

  pairs_table pairs(_self, _self.value);
  //datapoints_table dstore(_self, pair.name.value);

  auto itr = pairs.find(pair.name.value);

  eosio::check(pair.name != "system"_n, "Cannot create a pair named system");
  eosio::check(itr == pairs.end(), "A pair with this name already exists.");

  pairs.emplace(_self, [&](auto& s) {
    s.name = pair.name;
    s.base_symbol = pair.base_symbol;
    s.base_type = pair.base_type;
    s.base_contract = pair.base_contract;
    s.quote_symbol = pair.quote_symbol;
    s.quote_type = pair.quote_type;
    s.quote_contract = pair.quote_contract;
    s.quoted_precision = pair.quoted_precision;
  });
/*
  //First data point starts at uint64 max
  uint64_t primary_key = 0;
 
  for (uint16_t i=0; i < 21; i++){

    //Insert next datapoint
    auto c_itr = dstore.emplace(_self, [&](auto& s) {
      s.id = primary_key;
      s.value = 0;
      s.timestamp = eosio::time_point(eosio::microseconds(0));
    });

    primary_key++;

  }
  */

}

// add a new pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::addpaird(const uint64_t id, const token token0, const token token1, uint8_t precision_scale, eosio::name name) {

  require_auth(get_self());

  pairsd_table pairsd(_self, _self.value);
  //datapoints_table dstore(_self, pair.name.value);

  auto itr = pairsd.find(id);

  eosio::check(itr == pairsd.end(), "A pair with this id already exists.");

  pairsd.emplace(_self, [&](auto& s) {
    s.id = id;
    s.token0 = token0;
    s.token1 = token1;
    s.active = true;
    s.precision_scale = precision_scale;
    s.name = name;
  });
}

// add a new pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::addpairde(eosio::name name, const uint64_t id, const token token0, const token token1, uint8_t precision_scale, bool lp_pricing, uint8_t base_token) {

  require_auth(get_self());

  pairsde_tab pairsd(_self, _self.value);
  //datapoints_table dstore(_self, pair.name.value);

  auto itr = pairsd.find(name.value);

  eosio::check(itr == pairsd.end(), "A pair with this id already exists.");

  pairsd.emplace(_self, [&](auto& s) {
    s.name = name;
    s.id = id;
    s.token0 = token0;
    s.token1 = token1;
    s.active = true;
    s.precision_scale = precision_scale;
    s.lp_pricing = lp_pricing;
    s.base_token = base_token;
  });
}

// delete a pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::delpair(const eosio::name name, const std::string reason) {

  require_auth(get_self());
  
  pairs_table pairs(_self, _self.value);
  datapoints_table dstore(_self, name.value);
  prices_table store(_self, name.value);

  auto itr = pairs.find(name.value);

  eosio::check(itr != pairs.end(), "bounty doesn't exist");
  eosio::check(itr->active == false, "cannot delete live pair");

  // delete pair, post reason to chain
  pairs.erase(itr);

  // remove quotes as well
  while (dstore.begin() != dstore.end()) {
      auto ditr = dstore.end();
      ditr--;
      dstore.erase(ditr);
  }

  // and remove prices from prices_table
  while (store.begin() != store.end()) {
    auto itr = store.end();
    itr--;
    store.erase(itr);
  }
}


// delete a pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::delpaird(const uint64_t id, const std::string reason) {

  require_auth(get_self());
  
  pairsd_table pairsd(_self, _self.value);
  //datapoints_table dstore(_self, name.value);
  //prices_table store(_self, name.value);

  auto itr = pairsd.find(id);

  eosio::check(itr != pairsd.end(), "id doesn't exist");
  eosio::check(itr->active == false, "cannot delete live pair");

  // delete pair, post reason to chain
  pairsd.erase(itr);
}

// delete a pair (auth by dac account)
[[eosio::action]]
void oraclehub_contract::delpairde(const eosio::name name, const std::string reason) {

  require_auth(get_self());
  
  pairsde_tab pairsd(_self, _self.value);
  //datapoints_table dstore(_self, name.value);
  //prices_table store(_self, name.value);

  auto itr = pairsd.find(name.value);

  eosio::check(itr != pairsd.end(), "id doesn't exist");
  eosio::check(itr->active == false, "cannot delete live pair");

  // delete pair, post reason to chain
  pairsd.erase(itr);
}

// switch a pair active or inactive (auth by dac account)
[[eosio::action]]
void oraclehub_contract::switchpair(const eosio::name name, const bool active) {
  require_auth(get_self());

  pairs_table pairs(_self, _self.value);

  auto itr = pairs.find(name.value);

  eosio::check(itr != pairs.end(), "Pair not found");

  pairs.modify(itr, _self, [&](auto& s) {
    s.active = active;
  });
}

// switch a pair active or inactive (auth by dac account)
[[eosio::action]]
void oraclehub_contract::switchpaird(const uint64_t id, const bool active) {
  require_auth(get_self());

  pairsd_table pairsd(_self, _self.value);

  auto itr = pairsd.find(id);

  eosio::check(itr != pairsd.end(), "Pair not found");

  pairsd.modify(itr, _self, [&](auto& s) {
    s.active = active;
  });
}

// switch a pair active or inactive (auth by dac account)
[[eosio::action]]
void oraclehub_contract::swpairde(const eosio::name name, const bool active) {
  require_auth(get_self());

  pairsde_tab pairsd(_self, _self.value);

  auto itr = pairsd.find(name.value);

  eosio::check(itr != pairsd.end(), "Pair not found");

  pairsd.modify(itr, _self, [&](auto& s) {
    s.active = active;
  });
}


template <class T>
bool oraclehub_contract::is_unique(std::vector<T> X) {
  typename std::vector<T>::iterator i,j;
  for(i=X.begin();i!=X.end();++i) {
    for(j=i+1;j!=X.end();++j) {
      if((*i).pair.value == (*j).pair.value) return 0;
    }
  }
  return 1;
}

// Push quotes
[[eosio::action]]
void oraclehub_contract::push(const eosio::name owner, std::vector<quote_data> &quotes) {
  eosio::require_auth(owner);

  check_oracle(owner);

  int length_owner = quotes.size();
  eosio::check(length_owner > 0, "must supply non-empty array of quotes");

  eosio::check( is_unique(quotes), "duplicate pairs not allowed");

  pairs_table pairs(_self, _self.value);
  auto qitr = quotes.begin();
  while (qitr != quotes.end()) {
    auto itr = pairs.find(qitr->pair.value);
    if (itr==pairs.end() || itr->active == false || qitr->value < val_min || qitr->value / std::pow(10, itr->quoted_precision) > val_max) {
        quotes.erase(qitr);
    } else {
      ++qitr;
    }
  }

  oracles_table oracles(_self, _self.value);
  uint64_t oraclecount = 0;
  for (auto o : oracles) 
    oraclecount++;

  // query delphioracle if delphipoller is whitelisted and it has been at least one minute since last update
  auto itro = oracles.find(DELPHIPOLLER_CONTRACT.value);
  if ( itro != oracles.end() && (itro->timestamp.sec_since_epoch() + one_minute - cronlag <= eosio::current_time_point().sec_since_epoch()) ) {
    delphioracle::pairs_table delphi_pairs(DELPHIORACLE_CONTRACT, DELPHIORACLE_CONTRACT.value);
    std::vector<oraclehub::quote_data> quotesd;
    // go through all oraclehub pairs
    for (auto pair : pairs) {
      // check if pair exists in delphi and is active
      auto itr = delphi_pairs.find(pair.name.value);
      if (itr != delphi_pairs.end() && itr->active) {
        // get latest data point
        delphioracle::datapoints_table delphi_data(DELPHIORACLE_CONTRACT, pair.name.value);
        auto data = delphi_data.get_index<eosio::name("timestamp")>();
        if (data.begin() != data.end()) {
          auto newest = data.end();
          newest--; // the last element is the newest
          // push median to quotes vector, scaling precision if needed
          int64_t precision_scaling = std::pow(10, pair.quoted_precision) / std::pow(10, itr->quoted_precision);
          if ( newest->median >= val_min && newest->median / std::pow(10, itr->quoted_precision) <= val_max )
            quotesd.push_back({
              newest->median * precision_scaling,
              pair.name
            });
        }
      }
    }
    int length = quotesd.size();
    check_last_push(DELPHIPOLLER_CONTRACT);
    for (int i = 0; i < length; i++) { //delphi quotes added to datapoints table
      update_datapoints(DELPHIPOLLER_CONTRACT, quotesd[i].value, quotesd[i].pair, oraclecount);
    }
    check_last_push(owner);
    for (int i = 0; i < length_owner; i++) { //owner quotes added to datapoints table
      update_datapoints(owner, quotes[i].value, quotes[i].pair, oraclecount);
    }
    // update time series of prices and statistics
    for ( int i = 0; i < length; i++ ) {
      prices_table store(_self, quotesd[i].pair.value);
      auto itr = store.begin();
      eosio::time_point timestamp = itr->timestamp;
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + price_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        updateprices( quotesd[i].pair ); // update prices when the current price (first row) is out of date by price_refresh_period
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + stats_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        calcstats(quotesd[i].pair, 1); // update stats when the current price (first row) is out of date by stats_refresh_period
    }
    for ( int i = 0; i < length_owner; i++ ) {
      prices_table store(_self, quotes[i].pair.value);
      auto itr = store.begin();
      eosio::time_point timestamp = itr->timestamp;
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + price_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        updateprices( quotes[i].pair ); // update prices when the current price (first row) is out of date by price_refresh_period
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + stats_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        calcstats(quotes[i].pair, 1); // update stats when the current price (first row) is out of date by stats_refresh_period
    }

  } else { //delphipoller not whitelisted or less than one minute has passed since last query into delphioracle

    check_last_push(owner);
    for (int i = 0; i < length_owner; i++) {
      update_datapoints(owner, quotes[i].value, quotes[i].pair, oraclecount);
    }
    // update time series of prices and statistics
    for ( int i = 0; i < length_owner; i++ ) {
      prices_table store(_self, quotes[i].pair.value);
      auto itr = store.begin();
      eosio::time_point timestamp = itr->timestamp;
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + price_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        updateprices( quotes[i].pair ); // update prices when the current price (first row) is out of date by price_refresh_period
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + stats_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        calcstats( quotes[i].pair, 1 ); // update stats when the current price (first row) is out of date by stats_refresh_period
    }
  }
}
 
eosio::name oraclehub_contract::getname(int n)
{
    char str[50];
    int i = 0;
    while (n > 0) {
        int rem = n % 26;
        if (rem == 0) {
            str[i++] = 'z';
            n = (n / 26) - 1;
        }
        else
        {
            str[i++] = std::tolower((rem - 1) + 'A');
            n = n / 26;
        }
    }
    str[i] = 'x';
    str[i+1] = 'o';
    str[i+2] = 'b';
    str[i+3] = '\0';
    std::reverse(str, str + strlen(str));
    return eosio::name(std::string_view(str));
}

// Push quotes
[[eosio::action]]
void oraclehub_contract::pushd(const eosio::name owner) {
  eosio::require_auth(owner);

  check_oracle(owner);

  int length_owner = 0;
  std::vector<oraclehub::quote_data> quotes;

  oracles_table oracles(_self, _self.value);
  uint64_t oraclecount = 0;
  for (auto o : oracles) 
    oraclecount++;

    check_last_pushd(owner);

    pairsde_tab pairsd(_self, _self.value);

    swapdefi::pairs swapdefi_pairs(SWAPDEFI_CONTRACT, SWAPDEFI_CONTRACT.value);
    auto p = pairsd.begin();
    while (p != pairsd.end()) {
          auto itr = swapdefi_pairs.find(p->id);
          if (itr != swapdefi_pairs.end() && p->active && p->name.value != eosio::name("sxe").value) {
            double price;
            if (!p->lp_pricing){ // not LP token
                if (p->base_token == 0)
                  price = itr->price0_last;
                else
                  price = itr->price1_last;
                if ( price * std::pow(10, p->precision_scale) >= val_min && price <= val_max ) {
                    quotes.push_back({
                      (uint64_t)(price * std::pow(10, p->precision_scale)),
                      p->name
                    });
                    length_owner++;
                }
              } else {// LP token
                price = 0.0;
                eosio::name n = getname(p->id);
                if (p->base_token == 0 && itr->token1.symbol.raw() == eosio::symbol("EOS", 4).raw())
                  price = (std::pow(10, p->precision_scale) * ( ( 2.0 * itr->reserve1.amount / std::pow(10.0, itr->reserve1.symbol.precision()) ) / itr->liquidity_token )); // price in EOS
                else if (p->base_token == 1 && itr->token0.symbol.raw() == eosio::symbol("EOS", 4).raw())
                  price = (std::pow(10, p->precision_scale) * ( ( 2.0 * itr->reserve0.amount / std::pow(10.0, itr->reserve0.symbol.precision()) ) / itr->liquidity_token )); // price in EOS
                else if (p->base_token == 0 && itr->token1.symbol.raw() == eosio::symbol("USDT", 4).raw()) {
                  auto sit = swapdefi_pairs.find(12);
                  price = (std::pow(10, p->precision_scale) * (( ( 2.0 * itr->reserve1.amount / std::pow(10.0, itr->reserve1.symbol.precision()) ) / itr->liquidity_token ) / sit->price0_last)); // price in EOS
                }
                else if (p->base_token == 1 && itr->token0.symbol.raw() == eosio::symbol("USDT", 4).raw()) {
                  auto sit = swapdefi_pairs.find(12);
                  price = (std::pow(10, p->precision_scale) * (( ( 2.0 * itr->reserve0.amount / std::pow(10.0, itr->reserve0.symbol.precision()) ) / itr->liquidity_token ) / sit->price0_last)); // price in EOS
                }
                if ( (uint64_t)price >= val_min && (uint64_t)price <= val_max ) {
                  quotes.push_back({
                    (uint64_t)price,
                    n
                  });
                  length_owner++;
                }
              }
          }
          p++;
    }
    
    curvesx::pairs_table curvesx_pairs(CURVESX_CONTRACT, CURVESX_CONTRACT.value);
    // curvesx has price USDT/VIGOR, need to query EOS/USDT from swap.defi so we have EOS/VIGOR
    auto sit = swapdefi_pairs.find(12);
    auto itr = curvesx_pairs.find(eosio::symbol("SXE",4).code().raw());
    p = pairsd.find(eosio::name("eosvigor").value);
    if (itr != curvesx_pairs.end() && p != pairsd.end() && p->active && sit != swapdefi_pairs.end()) {
    double price = sit->price0_last/itr->price1_last;
    if ( price * std::pow(10, p->precision_scale) >= val_min && price <= val_max ) {
        quotes.push_back({
          (uint64_t)(price * std::pow(10, p->precision_scale)),
          eosio::name("eosvigor") // this an additional quote source for eosvigor, it also comes from delphioracle
        });
        length_owner++;
    }

    p = pairsd.find(eosio::name("sxe").value);
    price = itr->virtual_price; // price in USDT
    if ( price * std::pow(10, p->precision_scale) >= val_min && price <= val_max ) {
       quotes.push_back({
          (uint64_t)(price * std::pow(10, p->precision_scale)),
          eosio::name("sxe")
        });
        length_owner++;
    }
    }
    
    for (int i = 0; i < length_owner; i++) {
      update_datapoints(owner, quotes[i].value, quotes[i].pair, oraclecount);
    }
    
    // update time series of prices and statistics
    bool dostats = false;
    for ( int i = 0; i < length_owner; i++ ) {
      prices_table store(_self, quotes[i].pair.value);
      auto itr = store.begin();
      eosio::time_point timestamp = itr->timestamp;
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + price_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        updateprices( quotes[i].pair ); // update prices when the current price (first row) is out of date by price_refresh_period
      if ( itr == store.end() || ( itr != store.end() && timestamp.sec_since_epoch() + stats_refresh_period - cronlag <= eosio::current_time_point().sec_since_epoch() ) )
        calcstats(quotes[i].pair, 1); // update stats when the current price (first row) is out of date by stats_refresh_period
    }
}

[[eosio::action]]
void oraclehub_contract::claim(eosio::name owner) {
  eosio::require_auth(owner);

  oracles_table gstore(_self, _self.value);

  auto itr = gstore.find(owner.value);

  eosio::check(itr != gstore.end(), "oracle not found");
  eosio::check(itr->balance.amount > 0, "no rewards to claim");

  eosio::asset payout = itr->balance;

  // if( existing->quantity.amount == quantity.amount ) {
  //   bt.erase( *existing );
  //} else {
  gstore.modify(*itr, _self, [&](auto &a) {
    a.balance = eosio::asset(0, eosio::symbol("VIG", 4));
  });
  //}

  // if quantity symbol == EOS -> token_contract

  // SEND_INLINE_ACTION(token_contract, transfer,
  // {name("eostitancore"),name("active")}, {name("eostitancore"), from,
  // quantity, std::string("")} );

  eosio::action act(eosio::permission_level{_self, eosio::name("active")},
                    eosio::name("vig111111111"), eosio::name("transfer"),
                    std::make_tuple(_self, owner, payout, std::string("")));
  act.send();
}

// TODO: Clear all data - move to test mode only?
[[eosio::action]]
void oraclehub_contract::clear(eosio::name pair) {
  eosio::require_auth(_self);

  // globaltable gtable(_self,_self.value);
  oracles_table gstore(_self, _self.value);
  oracles_table lstore(_self, pair.value);
  datapoints_table estore(_self, pair.value);
  pairs_table pairs(_self, _self.value);

  // while (gtable.begin() != gtable.end()) {
  //     auto itr = gtable.end();
  //     itr--;
  //     gtable.erase(itr);
  // }

  while (gstore.begin() != gstore.end()) {
    auto itr = gstore.end();
    itr--;
    gstore.erase(itr);
  }

  while (lstore.begin() != lstore.end()) {
    auto itr = lstore.end();
    itr--;
    lstore.erase(itr);
  }

  while (estore.begin() != estore.end()) {
    auto itr = estore.end();
    itr--;
    estore.erase(itr);
  }

  while (pairs.begin() != pairs.end()) {
    auto itr = pairs.end();
    itr--;
    pairs.erase(itr);
  }
}

#if defined(LOCAL_MODE) || defined(TEST_MODE)

// Apply a shock to the prices for testing
[[eosio::action]]
void oraclehub_contract::doshock(double shockvalue) {
  eosio::require_auth(_self);
  shock_data s;
  shock_singleton shocks(get_self(), get_self().value);
  if (shocks.exists())
    s = shocks.get();
  s.shock = shockvalue;
  shocks.set(s, _self);
}

// TODO: reset volatility - still needed?
[[eosio::action]]
void oraclehub_contract::resetvol(const eosio::name pair, const uint32_t freq) {  
  eosio::require_auth(_self);
  eosio::print("pair ",pair,"\n");
  eosio::print("freq ",freq,"\n");

  prices_table store(_self, pair.value);
  auto itr = store.find(freq);
  store.modify( itr, _self, [&]( auto& s ) {
    s.vol = defaultVol;
  });
}

[[eosio::action]]
void oraclehub_contract::updatepricea() {
  eosio::require_auth(_self);
  pairs_table pairs(_self,_self.value);
  for ( auto it = pairs.begin(); it != pairs.end(); it++ ) {
    updateprices(it->name);
  }
}

[[eosio::action]]
void oraclehub_contract::updatestatsa() {
  eosio::require_auth(_self);
    pairs_table pairs(_self,_self.value);
  for ( auto it = pairs.begin(); it != pairs.end(); it++ ) {
    calcstats(it->name,1);
  }
}

#endif

[[eosio::action]]
void oraclehub_contract::purge(){
  eosio::require_auth(_self);
  pairs_table pairs(_self,_self.value);
  for ( auto it = pairs.begin(); it != pairs.end(); it++ ) {
    prices_table store(_self, it->name.value);
    auto latest = store.begin();
    while (latest != store.end())
        latest = store.erase(latest);
  }
}

// Get median price and store in deque as a historical time series (from datapreproc)
void oraclehub_contract::updateprices(eosio::name pair) {
    //eosio::require_auth(_self);
    pairs_table pairs(_self,_self.value);
    auto it = pairs.find(pair.value);
    if (it != pairs.end()) {
    uint64_t lastprice = get_last_price(pair, it->quoted_precision);
    #if defined(LOCAL_MODE) || defined(TEST_MODE)
      double shock = 1.0;
      shock_data s;
      shock_singleton shocks(get_self(), get_self().value);
      if (shocks.exists()) {
        s = shocks.get();
        shock = s.shock;
      }
      lastprice *= shock;
    #endif
    store_last_price(pair, 1, lastprice);
    }
    
 }

// NOTIFICATION HANDLERS

[[eosio::on_notify("vig111111111::transfer")]]
void oraclehub_contract::transfer(uint64_t sender, uint64_t receiver) {


  auto transfer_data = eosio::unpack_action_data<oraclehub_contract::transfer_data>();

  // if incoming transfer
  if (transfer_data.from != _self && transfer_data.to == _self) {

    eosio::check( transfer_data.from.value == PAYMENT_CONTRACT.value, std::string("Incomming transfers required to be from: ") + PAYMENT_CONTRACT.to_string() );

    // globaltable global(_self, _self.value);
    oracles_table gstore(_self, _self.value);

    uint64_t size = 0;
    for (auto o : gstore)
      size++;

    //auto count_index = gstore.get_index<eosio::name("count")>();

    //auto itr = count_index.begin();
     auto itr = gstore.begin();

    uint64_t total_datapoints = 0; // gitr->total_datapoints_count;

    // Move pointer to size, counting total number of datapoints for
    // oracles elligible for payout
    for (uint64_t i = 1; i <= size; i++) {
      if ( itr->owner.value != DELPHIPOLLER_CONTRACT.value )
        total_datapoints += itr->count;
      
      if (i < size) {
        itr++;
      }
    }

    uint64_t amount = transfer_data.quantity.amount;

    // Move pointer back to 0, calculating prorated contribution of oracle and
    // allocating proportion of donation
    if ( total_datapoints > 0 )
    for (uint64_t i = size; i >= 1; i--) {
      uint64_t datapoints = itr->count;

      double percent = ((double)datapoints / (double)total_datapoints);
      uint64_t uquota =
          (uint64_t)(percent * (double)transfer_data.quantity.amount);

      eosio::asset payout;

      // avoid leftover rounding by giving to top contributor
      if (i == 1) {
        payout = eosio::asset(amount, eosio::symbol("VIG", 4));
      } else {
        payout = eosio::asset(uquota, eosio::symbol("VIG", 4));
      }

      if ( itr->owner.value != DELPHIPOLLER_CONTRACT.value ) {
        amount -= uquota;
        gstore.modify(itr, _self, [&](auto &s) { 
          s.balance += payout;
          s.count = 0;
          });
      } else
      {
        gstore.modify(itr, _self, [&](auto &s) { 
          s.balance = eosio::asset(0, eosio::symbol("VIG", 4));
          s.count = 0;
          });
      }
      

      if (i > 1) {
        itr--;
      }
    }
  }
}

} // namespace oraclehub