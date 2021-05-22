#include "oracle.hpp"

  //Check if calling account is a qualified oracle
  bool oracle::check_oracle(const name owner){
/* 
    producers_table ptable(name("eosio"), name("eosio").value);

    auto p_idx = ptable.get_index<name("prototalvote")>();

    auto p_itr = p_idx.begin();

    uint64_t count = 0;

    while (p_itr != p_idx.end()) {
      print(p_itr->owner, "\n");
      if (p_itr->owner==owner) return true;
      p_itr++;
      count++;
      if (count>standbys) break;
    }
*/
    return true;
  }

  //Ensure account cannot push data more often than every 60 seconds
  void oracle::check_last_push(const name owner, const name pair){

    statstable gstore(_self,_self.value);
    statstable store(_self, pair.value);

    auto itr = store.find(owner.value);
    if (itr != store.end()) {

      time_point ctime = current_time_point();
      auto last = store.get(owner.value);

      check(last.timestamp.sec_since_epoch() + one_minute <= ctime.sec_since_epoch() , "can only call every 60 seconds");

      store.modify( itr, _self, [&]( auto& s ) {
        s.timestamp = ctime;
        s.count++;
      });

    } else {

      store.emplace(_self, [&](auto& s) {
        s.owner = owner;
        s.timestamp = current_time_point();
        s.count = 1;
        s.balance = asset(0, symbol("EOS",4));
        s.last_claim = 0;
      });

    }

    auto gitr = gstore.find(owner.value);
    if (gitr != gstore.end()) {

      time_point ctime = current_time_point();

      gstore.modify( gitr, _self, [&]( auto& s ) {
        s.timestamp = ctime;
        s.count++;
      });

    } else {

      gstore.emplace(_self, [&](auto& s) {
        s.owner = owner;
        s.timestamp = current_time_point();
        s.count = 1;
       s.balance = asset(0, symbol("EOS",4));
       s.last_claim = 0;
      });

    }

  }

  //Push oracle message on top of queue, pop old elements (older than one minute)
  void oracle::update_datapoints(const name owner, const uint64_t value, name pair){

    datapointstable dstore(_self, pair.value);
    time_point ctime = current_time_point();

    auto size = std::distance(dstore.begin(), dstore.end());

    uint64_t median = 0;
    uint64_t primary_key ;

    //Find median
    if (size>0){

      //Calculate new primary key by substracting one from the previous one
      auto latest = dstore.begin();
      primary_key = latest->id - 1;
       
       //Pop old points (older than one minute)
       while (latest != dstore.end()){
        if (latest->timestamp.sec_since_epoch() + one_minute < ctime.sec_since_epoch())
          latest = dstore.erase(latest);
        else
          latest++;
       }

        //Insert next datapoint
        auto c_itr = dstore.emplace(_self, [&](auto& s) {
          s.id = primary_key;
          s.owner = owner;
          s.value = value;
          s.timestamp = ctime;
        });

        //Get index sorted by value
        auto value_sorted = dstore.get_index<name("value")>();
        uint64_t mid = (uint64_t)floor(std::distance(value_sorted.begin(), value_sorted.end())/2.0);
        auto itr = value_sorted.begin();
        for (int i=0; i<mid; i++){
          itr++;
        }
        
        median=itr->value;
        //set median
        dstore.modify(c_itr, _self, [&](auto& s) {
          s.median = median;
        });

      }
    else {

      //First data point starts at uint64 max
      primary_key = std::numeric_limits<unsigned long long>::max();
      median = value;

      //Push new point at the end of the queue
      dstore.emplace(_self, [&](auto& s) {
        s.id = primary_key;
        s.owner = owner;
        s.value = value;
        s.median = median;
        s.timestamp = current_time_point();
      });

    }

    globaltable gtable(_self,_self.value);

    gtable.modify(gtable.begin(), _self, [&](auto& s) {
      s.total_datapoints_count++;
    });

  }

  //Write datapoint
  
  ACTION oracle::write(const name owner, const std::vector<quote>& quotes) {
    
    require_auth(owner);
    
    int length = quotes.size();

   // print("length ", length);

    check(length>0, "must supply non-empty array of quotes");
    check(check_oracle(owner), "account is not an active producer or approved oracle");

    for (int i=0; i<length;i++){
     // print("quote ", i, " ", quotes[i].value, " ",  quotes[i].pair, "\n");
       check(quotes[i].value >= val_min && quotes[i].value <= val_max, "value outside of allowed range");
    }

    for (int i=0; i<length;i++){    
      check_last_push(owner, quotes[i].pair);
      update_datapoints(owner, quotes[i].value, quotes[i].pair);
    }

    //TODO: check if symbol exists
    
  }

  //claim rewards
 
  ACTION oracle::claim(name owner) {
    
    require_auth(owner);

    statstable gstore(_self,_self.value);

    auto itr = gstore.find(owner.value);

    check(itr != gstore.end(), "oracle not found");
    check( itr->balance.amount > 0, "no rewards to claim" );

    asset payout = itr->balance;

    //if( existing->quantity.amount == quantity.amount ) {
    //   bt.erase( *existing );
    //} else {
    gstore.modify( *itr, _self, [&]( auto& a ) {
        a.balance = asset(0, symbol("EOS",4));
    });
    //}

    //if quantity symbol == EOS -> token_contract

   // SEND_INLINE_ACTION(token_contract, transfer, {name("eostitancore"),name("active")}, {name("eostitancore"), from, quantity, std::string("")} );
      
    action act(
      permission_level{_self, name("active")},
      name("eosio.token"), name("transfer"),
      std::make_tuple(_self, owner, payout, std::string(""))
    );
    act.send();

  }

  //temp configuration
  
  ACTION oracle::configure() {
    
    require_auth(_self);

    globaltable gtable(_self,_self.value);
    pairstable pairs(_self,_self.value);

    gtable.emplace(_self, [&](auto& o) {
      o.id = 1;
      o.total_datapoints_count = 0;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("eosusd");
      o.base_symbol = symbol("EOS",4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("eosio.token");
      o.quote_symbol = symbol("USD",2);
      o.quote_type = e_asset_type::fiat;
      o.quote_contract = name("");
      o.quoted_precision = 4;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("eosusdt");
      o.base_symbol = symbol("EOS",4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("eosio.token");
      o.quote_symbol = symbol("USDT",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("tethertether");
      o.quoted_precision = 4;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("btcusd");
      o.base_symbol = symbol("PBTC",8);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("btc.ptokens");
      o.quote_symbol = symbol("USD",2);
      o.quote_type = e_asset_type::fiat;
      o.quote_contract = name("");
      o.quoted_precision = 4;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("ethusd");
      o.base_symbol = symbol("PETH",9);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("eth.ptokens");
      o.quote_symbol = symbol("USD",2);
      o.quote_type = e_asset_type::fiat;
      o.quote_contract = name("");
      o.quoted_precision = 4;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("iqeos");
      o.base_symbol = symbol("IQ",3);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("everipediaiq");
      o.quote_symbol = symbol("EOS",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("eosio.token");
      o.quoted_precision = 6;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("boxeos");
      o.base_symbol = symbol("BOX",6);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("token.defi");
      o.quote_symbol = symbol("EOS",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("eosio.token");
      o.quoted_precision = 6;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("vigeos");
      o.base_symbol = symbol("VIG",4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("vig111111111");
      o.quote_symbol = symbol("EOS",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("eosio.token");
      o.quoted_precision = 8;
    });

    pairs.emplace(_self, [&](auto& o) {
      o.active = true;
      o.aname = name("boideos");
      o.base_symbol = symbol("BOID",4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = name("boidcomtoken");
      o.quote_symbol = symbol("EOS",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("eosio.token");
      o.quoted_precision = 7;
    });

    pairs.emplace(_self, [&](auto &o) {
      o.active = true;
      o.aname = eosio::name("vigoreos");
      o.base_symbol = eosio::symbol("VIGOR", 4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = eosio::name("vigortoken11");
      o.quote_symbol = symbol("EOS",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("eosio.token");
      o.quoted_precision = 6;
    });

    pairs.emplace(_self, [&](auto &o) {
      o.active = true;
      o.aname = eosio::name("eosvigor");
      o.base_symbol = eosio::symbol("EOS", 4);
      o.base_type = e_asset_type::eosio_token;
      o.base_contract = eosio::name("eosio.token");
      o.quote_symbol = symbol("VIGOR",4);
      o.quote_type = e_asset_type::eosio_token;
      o.quote_contract = name("vigortoken11");
      o.quoted_precision = 4;
    });

  }

  //Clear all data
  ACTION oracle::clear(name pair) {
    require_auth(_self);

    globaltable gtable(_self,_self.value);
    statstable gstore(_self,_self.value);
    statstable lstore(_self, pair.value);
    datapointstable estore(_self,  pair.value);
    pairstable pairs(_self,_self.value);
    
    while (gtable.begin() != gtable.end()) {
        auto itr = gtable.end();
        itr--;
        gtable.erase(itr);
    }

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


  ACTION oracle::transfer(uint64_t sender, uint64_t receiver) {

    print("transfer notifier", "\n");

    auto transfer_data = unpack_action_data<oracle::st_transfer>();

    print("transfer ", name{transfer_data.from}, " ",  name{transfer_data.to}, " ", transfer_data.quantity, "\n");

    //if incoming transfer
    if (transfer_data.from != _self && transfer_data.to == _self){
      
      globaltable global(_self,_self.value);
      statstable gstore(_self,_self.value);

      uint64_t size = std::distance(gstore.begin(), gstore.end());

      uint64_t upperbound = std::min(size, paid);

      auto count_index = gstore.get_index<name("count")>();

      auto itr = count_index.begin();
      auto gitr = global.begin();

      uint64_t total_datapoints = 0; //gitr->total_datapoints_count;

      print("upperbound", upperbound, "\n");
      //print("itr->owner", itr->owner, "\n");

      //Move pointer to upperbound, counting total number of datapoints for oracles elligible for payout
      for (uint64_t i=1;i<=upperbound;i++){
        total_datapoints+=itr->count;
        

        if (i<upperbound ){
          itr++;
          print("increment 1", "\n");

        } 

      }

      print("total_datapoints", total_datapoints, "\n");

      uint64_t amount = transfer_data.quantity.amount;

      //Move pointer back to 0, calculating prorated contribution of oracle and allocating proportion of donation
      for (uint64_t i=upperbound;i>=1;i--){

        uint64_t datapoints = itr->count;

        double percent = ((double)datapoints / (double)total_datapoints) ;
        uint64_t uquota = (uint64_t)(percent * (double)transfer_data.quantity.amount) ;


        print("itr->owner", itr->owner, "\n");
        print("datapoints", datapoints, "\n");
        print("percent", percent, "\n");
        print("uquota", uquota, "\n");

        asset payout;

        //avoid leftover rounding by giving to top contributor
        if (i == 1){
          payout = asset(amount, symbol("EOS",4));
        }
        else {
          payout = asset(uquota, symbol("EOS",4));
        }

        amount-= uquota;
        
        print("payout", payout, "\n");

        gstore.modify(*itr, _self, [&]( auto& s ) {
          s.balance += payout;
        });
        
        if (i>1 ){
          itr--;
          print("decrement 1", "\n");

        } 
      }


    }

  }
