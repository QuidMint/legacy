#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>
#include <eosio/symbol.hpp>
#include <eosio/time.hpp>
#include <cmath>
#include <eosio/singleton.hpp>

using namespace eosio;


const uint32_t one_minute = 60; 
const uint32_t five_minute = 60 * 5;
const uint32_t fifteen_minute = 60 * 15;
const uint32_t one_hour = 60 * 60;
const uint32_t four_hour = 60 * 60 * 4; 
const uint32_t one_day = 60 * 60 * 24; 
const uint32_t cronlag = 5; //give extra time for cron jobs
const uint64_t dequesize = 30;
const double returnsPrecision = 1000000.0;
const double pricePrecision = 1000000.0;
const uint64_t defaultVol = 600000;
const int64_t defaultCorr = 1000000;
const double one_minute_scale = sqrt(252.0*24.0*(60.0/1.0));
const double five_minute_scale = sqrt(252.0*24.0*(60.0/5.0));
const double fifteen_minute_scale = sqrt(252.0*24.0*(60.0/15.0));
const double one_hour_scale = sqrt(252.0*24.0*(60.0/60.0));
const double four_hour_scale = sqrt(252.0*24.0*(60.0/(60.0*4.0)));
const double one_day_scale = sqrt(252.0*24.0*(60.0/(60.0*24.0)));


const  std::map <uint32_t, double> volScale {
         {one_minute,	    one_minute_scale},
         {five_minute,	  five_minute_scale},
         {fifteen_minute,	fifteen_minute_scale},
         {one_hour,	      one_hour_scale},
         {four_hour,	    four_hour_scale},
         {one_day,	      one_day_scale},
};


CONTRACT datapreproc : public eosio::contract {
 private:


//Types
  enum asset_type: uint16_t {
      fiat=1,
      cryptocurrency=2, 
      erc20_token=3, 
      eosio_token=4, 
      equity=5, 
      derivative=6, 
      other=7
  };

 //Holds the latest datapoints from qualified oracles
  TABLE datapoints {
    uint64_t id;
    name owner;
    uint64_t value;
    uint64_t median;
    time_point timestamp;

    uint64_t primary_key() const {return id;}
    uint64_t by_timestamp() const {return timestamp.elapsed.to_seconds();}
    uint64_t by_value() const {return value;}

  };

    typedef eosio::multi_index<name("datapoints"), datapoints,
      indexed_by<name("value"), const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>, 
      indexed_by<name("timestamp"), const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>> datapointstable;

    datapointstable _dptable;


  //Holds the list of pairs available in the oracle
  TABLE pairs {
    
    bool active = false;
    bool bounty_awarded = false;
    bool bounty_edited_by_custodians = false;

    name proposer;
    name aname;

    asset bounty_amount = asset(0, symbol("EOS",4));

    std::vector<name> approving_custodians;
    std::vector<name> approving_oracles;

    symbol base_symbol;
    uint64_t base_type;
    name base_contract;

    symbol quote_symbol;
    uint64_t quote_type;
    name quote_contract;
    
    uint64_t quoted_precision;

    uint64_t primary_key() const {return aname.value;}

  };

  typedef eosio::multi_index<name("pairs"), pairs> pairstable;
  pairstable _prstable;


  //Holds the list of pairs to process
  TABLE pairtoproc {
   // uint64_t id;
    name aname;

    symbol base_symbol;
    uint64_t base_type;
    name base_contract;

    symbol quote_symbol;
    uint64_t quote_type;
    name quote_contract;
    
    uint64_t quoted_precision;

    uint64_t primary_key() const {return aname.value;}
    //uint64_t primary_key() const {return id;}
    //uint64_t by_name() const {return aname.value;}

  };


    typedef eosio::multi_index<"pairtoproc"_n, pairtoproc> pairtoproctb;
    pairtoproctb _ptptable;


  //Holds the time series of prices, returns, volatility and correlation
  TABLE statspre {
    uint32_t freq;
    time_point timestamp;
    std::deque<uint64_t> price;
    std::deque<int64_t> returns;
    std::map <symbol, int64_t> correlation_matrix;
    std::uint64_t vol = defaultVol;

    uint32_t primary_key() const {return freq;}

  };

  typedef eosio::multi_index<name("tseries"), statspre> statstable;
  statstable _ststable;
  
  TABLE shocktable {
         double shock = 1.0;
   
      EOSLIB_SERIALIZE(shocktable, (shock))
  }; typedef eosio::multi_index<name("shock"), shocktable> shockm;
      typedef eosio::singleton<name("shock"), shocktable> shocks;
                                                            shocks _shocks;

public:

using contract::contract;

datapreproc(name receiver, name code, datastream<const char*> ds) : eosio::contract(receiver, code, ds), 
   _dptable(receiver, receiver.value), 
   _prstable(receiver, receiver.value), 
   _ptptable(receiver, receiver.value), 
   _ststable(receiver, receiver.value), 
   _shocks(receiver, receiver.value){}

//apply a shock to the prices for testing
ACTION doshock(double shockvalue);

//add to the list of pairs to process
ACTION addpair(name newpair);

//Clear the list of pairs to process
ACTION clear();

ACTION purge(const name pair, const uint32_t freq);

ACTION resetvol(const name pair, const uint32_t freq);

uint64_t get_last_price(name pair, uint64_t quoted_precision);

//  get median price and store in vector as a historical time series
ACTION update();

//  get median price and store in deque as a historical time series
void getprices();


void averageVol(name aname);

void averageCor(name aname);

//  calculate vol and correlation matrix
void calcstats(const name pair, const uint32_t freq);

// correlation coefficient
int64_t corrCalc(std::deque<int64_t> X, std::deque<int64_t> Y, uint64_t n);

uint64_t volCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq);

  
//store last price from the oracle, append to time series
void store_last_price(const name pair, const uint32_t freq, const uint64_t lastprice);

};
