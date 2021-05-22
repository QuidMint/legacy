#include "datapreproc.hpp"

//add to the list of pairs to process
ACTION datapreproc::addpair(name newpair) {
    
    //require_auth(_self);

    if (!_shocks.exists()){
      shocktable s;
      s.shock = 1.0;
      _shocks.set(s, _self);
    }

    pairstable pairsname(name("oracleoracl2"), name("oracleoracl2").value);
    auto itr = pairsname.find(newpair.value);
    if ( itr != pairsname.end() ) { //pair must exist in the oracle
        pairtoproctb pairtoproc(_self,_self.value);
        //auto pairtoprocn = pairtoproc.get_index<name("aname")>();
        //auto it = pairtoprocn.find(newpair.value);
        auto it = pairtoproc.find(newpair.value);
        //if ( it == pairtoprocn.end() ) { //add pair if hasn't already been added
        if ( it == pairtoproc.end() ) { // add pair if hasn't already been added
            pairtoproc.emplace(_self, [&](auto& o) {
            //o.id = pairtoproc.available_primary_key();
            o.aname = newpair;
            o.base_symbol = itr->base_symbol;
            o.base_type = itr->base_type;
            o.base_contract = itr->base_contract;
            o.quote_symbol = itr->quote_symbol;
            o.quote_type = itr->quote_type;
            o.quote_contract = itr->quote_contract;
            o.quoted_precision = itr->quoted_precision;
            });
        };
   };
   if ( newpair.value == name("vigorusd").value ) { //pair must exist in the oracle
            pairtoproctb pairtoproc(_self,_self.value);
            pairtoproc.emplace(_self, [&](auto& o) {
            o.aname = newpair;
            o.base_symbol = symbol("VIGOR",4);
            o.base_type = asset_type::eosio_token;
            o.base_contract = name("vigordemo111");
            o.quote_symbol = symbol("USD",2);
            o.quote_type = asset_type::fiat;
            o.quote_contract = name("");
            o.quoted_precision = 4;
            });
   };
}

//apply a shock to the prices for testing
ACTION datapreproc::doshock(double shockvalue) {
shocktable s;
if (_shocks.exists())
  s = _shocks.get();
s.shock = shockvalue;
_shocks.set(s, _self);
}

//Clear the list of pairs to process
ACTION datapreproc::clear() {  
  
    //require_auth(_self);
   
    pairtoproctb pairtoproc(_self,_self.value);

    while (pairtoproc.begin() != pairtoproc.end()) {
        auto itr = pairtoproc.end();
        itr--;
        pairtoproc.erase(itr);
    }

    pairstable pairsname(name("oracleoracl2"), name("oracleoracl2").value);
    for ( auto it = pairsname.begin(); it != pairsname.end(); it++ ) {
      statstable store(_self, it->aname.value);

      while (store.begin() != store.end()) {
          auto itr = store.end();
          itr--;
          store.erase(itr);
      };
    };

  }

uint64_t datapreproc::get_last_price(name pair, uint64_t quoted_precision){

    uint64_t eosusd = 1;
    uint64_t eos_precision = 1;
    datapointstable dstoreosname(name("oracleoracl2"), name("eosusd").value);
    auto dstoreos = dstoreosname.get_index<name("timestamp")>();
    auto newesteos = dstoreos.begin();
    if (newesteos != dstoreos.end()){
      auto newesteos = dstoreos.end();
      newesteos--; // the last element is the latest
      pairtoproctb pairtoproc(_self,_self.value);
      //auto pairtoprocn = pairtoproc.get_index<name("aname")>();
      //auto eospair = pairtoprocn.get(name("eosusd").value);
      auto eospair = pairtoproc.get("eosusd"_n.value);
      eos_precision = eospair.quoted_precision;
      eosusd = newesteos->median;
    }
    datapointstable dstorename(name("oracleoracl2"), pair.value);
    auto dstore = dstorename.get_index<name("timestamp")>();
    auto newest = dstore.begin();
    if (newest != dstore.end()) {
      auto newest = dstore.end();
      newest--; // the last element is the latest
        if (pair==name("eosusd"))
          return (uint64_t)(pricePrecision*(newest->median/std::pow(10.0,eos_precision)));
        else
          return (uint64_t)(pricePrecision*((newest->median/std::pow(10.0,quoted_precision)) * (eosusd/std::pow(10.0,eos_precision))));
    } else
        return 0;
  }

//  get median price and store in vector as a historical time series
ACTION datapreproc::update(){
    
    getprices();
  }

//  get median price and store in deque as a historical time series
void datapreproc::getprices(){

    pairtoproctb pairtoproc(_self,_self.value);
    
    pairstable pairsname(name("oracleoracl2"), name("oracleoracl2").value);
    
    for ( auto it = pairtoproc.begin(); it != pairtoproc.end(); it++ ) {
        auto itr = pairsname.find(it->aname.value);
        if ( itr != pairsname.end() || it->aname.value==name("vigorusd").value) { //pair must exist in the oracle
        uint64_t lastprice;
        if (it->aname.value==name("vigorusd").value)
          lastprice = 1000000;
        else
          lastprice = get_last_price(it->aname, it->quoted_precision);
        eosio::print("pair to process: ", it->aname, "\n");
        store_last_price(it->aname, one_minute, lastprice);
        store_last_price(it->aname, five_minute, lastprice);
        store_last_price(it->aname, fifteen_minute, lastprice);
        store_last_price(it->aname, one_hour, lastprice);
        store_last_price(it->aname, four_hour, lastprice);
        store_last_price(it->aname, one_day, lastprice);
             
        };
    }
    
    for ( auto it = pairtoproc.begin(); it != pairtoproc.end(); it++ ) {
        auto itr = pairsname.find(it->aname.value);
        if ( itr != pairsname.end() || it->aname.value==name("vigorusd").value) { //pair must exist in the oracle
        calcstats(it->aname, one_minute);
        calcstats(it->aname, five_minute);
        calcstats(it->aname, fifteen_minute);
        calcstats(it->aname, one_hour);
        calcstats(it->aname, four_hour);
        calcstats(it->aname, one_day);
        };
    }
    for ( auto it = pairtoproc.begin(); it != pairtoproc.end(); it++ ) {
        auto itr = pairsname.find(it->aname.value);
        if ( itr != pairsname.end() || it->aname.value==name("vigorusd").value) { //pair must exist in the oracle
        uint64_t lastprice = get_last_price(it->aname, it->quoted_precision);
        averageVol(it->aname);
        averageCor(it->aname);
        };
    }
  }


void datapreproc::averageVol(name aname){

    statstable store(_self, aname.value);
    auto itr = store.find(one_minute);
    uint64_t lastprice = itr->price.back();
    uint64_t vol1 = itr->vol;
    itr = store.find(five_minute);
    uint64_t vol2 = itr->vol;
    itr = store.find(fifteen_minute);
    uint64_t vol3 = itr->vol;
    itr = store.find(one_hour);
    uint64_t vol4 = itr->vol;
    itr = store.find(four_hour);
    uint64_t vol5 = itr->vol;
    itr = store.find(one_day);
    uint64_t vol6 = itr->vol;
    uint64_t vol = (uint64_t)(0.05*(double)vol1+0.05*(double)vol2+0.05*(double)vol3+0.05*(double)vol4+0.3*(double)vol5+0.5*(double)vol6);
          time_point ctime = current_time_point();
          shocktable shockt = _shocks.get();
          double shock = shockt.shock;
          if (aname.value == name("vigorusd").value) {
            vol = defaultVol/4;
            shock = 1.0;
          }
          itr = store.find(1);
          if (itr != store.end()){
            store.modify( itr, _self, [&]( auto& s ) {
            s.vol = vol;
            s.timestamp = ctime;
            s.price[0]=(uint64_t)((((double)(lastprice/pricePrecision))*shock)*pricePrecision);
            });
          } else {
            store.emplace(_self, [&](auto& s) {
              s.freq=1;
              s.vol = vol;
              s.timestamp = ctime;
              s.price.push_front((uint64_t)((((double)(lastprice/pricePrecision))*shock)*pricePrecision));
            });
          };
  }

void datapreproc::averageCor(name aname){

    statstable store(_self, aname.value);
    const auto obj = store.get(one_minute);
    std::map <symbol, int64_t> m1 = obj.correlation_matrix;
    for (std::pair<symbol, int64_t> it : m1){
      int64_t c1 = it.second;
      auto obj = store.get(five_minute);
      std::map <symbol, int64_t> m2 = obj.correlation_matrix;
      int64_t c2 = m2[it.first];
      obj = store.get(fifteen_minute);
      std::map <symbol, int64_t> m3 = obj.correlation_matrix;
      int64_t c3 = m3[it.first];
      obj = store.get(one_hour);
      std::map <symbol, int64_t> m4 = obj.correlation_matrix;
      int64_t c4 = m4[it.first];
      obj = store.get(four_hour);
      std::map <symbol, int64_t> m5 = obj.correlation_matrix;
      int64_t c5 = m5[it.first];
      obj = store.get(one_day);
      std::map <symbol, int64_t> m6 = obj.correlation_matrix;
      int64_t c6 = m6[it.first];
      int64_t corr = (int64_t)(0.05*(double)c1+0.05*(double)c2+0.05*(double)c3+0.05*(double)c4+0.3*(double)c5+0.5*(double)c6);
      time_point ctime = current_time_point();
      if (aname.value == name("vigorusd").value)
        corr = 0.0;
      auto itr = store.find(1);
          if (itr != store.end()){
            store.modify( itr, _self, [&]( auto& s ) {
            s.correlation_matrix[it.first] = corr;
            s.timestamp = ctime;
            });
          } else {
            store.emplace(_self, [&](auto& s) {
              s.freq=1;
              s.correlation_matrix[it.first] = corr;
              s.timestamp = ctime;
            });
          };
    }
  }

//  calculate vol and correlation matrix
void datapreproc::calcstats(const name pair, const uint32_t freq){
    
          statstable store(_self, pair.value);
          auto itr = store.find(freq);
          if (itr != store.end()) {
            const auto& itrref = *itr;
            auto last = store.get(freq);
            uint64_t vol = defaultVol;
                if (last.price.size()>5)
                  vol = volCalc(last.returns, last.returns.size(), freq);
                store.modify( itrref, _self, [&]( auto& s ) {
                  s.vol = vol;
                });
                
            pairstable pairsname(name("oracleoracl2"), name("oracleoracl2").value);    
            pairtoproctb pairtoproc(_self,_self.value);    
            for ( auto jt = pairtoproc.begin(); jt != pairtoproc.end(); jt++ ) {
                auto jto = pairsname.find(jt->aname.value);
                if ( jto != pairsname.end() ) { //pair must exist in the oracle

                  statstable storej(_self, jt->aname.value);
                  auto jtr = storej.find(freq);
                  if (jtr != storej.end()) {
                    auto lastj = storej.get(freq);
                    int64_t corr = defaultCorr;
                    if (lastj.price.size()==last.price.size() && last.price.size()>5)
                      corr = corrCalc(last.returns, lastj.returns, last.returns.size());
                    store.modify( itrref, _self, [&]( auto& s ) {
                      s.correlation_matrix[jt->base_symbol] = corr;
                    });
                      
                  }
                }
            }
          }
  }

// correlation coefficient
int64_t datapreproc::corrCalc(std::deque<int64_t> X, std::deque<int64_t> Y, uint64_t n) { 
  
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
      corr = (int64_t)(returnsPrecision)*(n * sum_XY - sum_X * sum_Y)
                  / sqrt((n * squareSum_X - sum_X * sum_X)
                      * (n * squareSum_Y - sum_Y * sum_Y)); 
    return corr;
} 

uint64_t datapreproc::volCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq) {

     double variance = 0.0;
     double t = returns[0]/returnsPrecision;
     for (int i = 1; i < n; i++)
     {
          t += returns[i]/returnsPrecision;
          double diff = ((i + 1) * returns[i]/returnsPrecision) - t;
         
          variance += (diff * diff) / ((i + 1) *i);

     }
     if ((variance / (n - 1)) == 0.0)
        return defaultVol;
     else
        return (uint64_t)(volScale.at(freq)*(returnsPrecision*sqrt(variance / (n - 1))));
}

void datapreproc::purge(const name pair, const uint32_t freq){
 eosio::print("pair ",pair,"\n");
 eosio::print("freq ",freq,"\n");
    statstable store(_self, pair.value);
    auto itr = store.find(freq);
    store.modify( itr, _self, [&]( auto& s ) {
      s.returns.clear();
      s.price.clear();
   //for (int i = 0; i < s.returns.size(); ++i){

     //    eosio::print("s.returns[i] ",s.returns[i],"\n");
      // if (s.returns[i]<-250000){
       //  s.returns[i] = 0;
        // eosio::print("s.returns[i] ",s.returns[i],"\n");
       //}
//}
    });

}

void datapreproc::resetvol(const name pair, const uint32_t freq){
  
 eosio::print("pair ",pair,"\n");
 eosio::print("freq ",freq,"\n");
    statstable store(_self, pair.value);
    auto itr = store.find(freq);
    store.modify( itr, _self, [&]( auto& s ) {
         s.vol = defaultVol;
    });
}
   //store last price from the oracle, append to time series
void datapreproc::store_last_price(const name pair, const uint32_t freq, const uint64_t lastprice){

    statstable store(_self, pair.value);

    auto itr = store.find(freq);
    time_point ctime = current_time_point();
    if (itr != store.end()) {
      auto last = store.get(freq);
      if (last.timestamp.sec_since_epoch() + freq - cronlag <= ctime.sec_since_epoch()){

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

extern "C" {
  void apply(uint64_t receiver, uint64_t code, uint64_t action) {
      if(code==receiver)
      {
          switch(action)
          {
              EOSIO_DISPATCH_HELPER(datapreproc, (update)(addpair)(clear)(doshock)(purge)(resetvol))
          }
      }
     // else if(code=="eosio.token"_n.value && action=="transfer"_n.value) {
     //     execute_action( name(receiver), name(code), &datapreproc::transfer);
    //  }
  }
}
