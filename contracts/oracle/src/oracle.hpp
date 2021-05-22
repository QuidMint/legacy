/*
  oracle
  Author: Guillaume "Gnome" Babin-Tremblay - EOS Titan
  
  Website: https://eostitan.com
  Email: guillaume@eostitan.com
  Github: https://github.com/eostitan/delphioracle/
  
  Published under MIT License
TODO
OK - add multi instrument support
OK - open it to BPs rank 50 or better
OK - replace the average with median
OK - do the automated fee distribution based on contribution
*/

//#include <eosio.system/eosio.system.hpp>
#include <eosio/symbol.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <math.h>
#include <eosio/system.hpp>

using namespace eosio;


//Controlling account to be phased out 
//static const name titan_account = name("delphioracle");

//Min value set to 0.01$ , max value set to 10,000$
static const uint64_t val_min = 1;
static const uint64_t val_max = 10000000000;

const uint32_t one_minute = 55; //give extra time for cron jobs

static const uint64_t standbys = 50; //allowed standby producers rank cutoff
static const uint64_t paid = 21; //maximum number of oracles getting paid from donations

CONTRACT oracle : public eosio::contract {
 public:
  oracle(name receiver, name code, datastream<const char*> ds) : eosio::contract(receiver, code, ds) {}

  //Types
  enum e_asset_type: uint16_t {
      fiat=1,
      cryptocurrency=2,
      erc20_token=3,
      eosio_token=4,
      equity=5,
      derivative=6,
      other=7
  };
  
  typedef uint16_t asset_type;

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
    EOSLIB_SERIALIZE( datapoints, (id)(owner)(value)(median)(timestamp))
  };

  //Global config
  TABLE global {
    uint64_t id;
    uint64_t total_datapoints_count;
    
    uint64_t primary_key() const {return id;}

  };

  //Holds the count and time of last writes for qualified oracles
  TABLE stats {
    name owner; 
    time_point timestamp;
    uint64_t count;
    uint64_t last_claim;
    asset balance;

    uint64_t primary_key() const {return owner.value;}
    uint64_t by_count() const {return count;}

  };

  //Holds the list of pairs
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
    asset_type base_type;
    name base_contract;

    symbol quote_symbol;
    asset_type quote_type;
    name quote_contract;
    
    uint64_t quoted_precision;

    uint64_t primary_key() const {return aname.value;}

  };

  //Quote
  struct quote {
    uint64_t value;
    name pair;

  };

/*   struct blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }
      uint64_t             max_ram_size = 64ll*1024 * 1024 * 1024;
      uint64_t             total_ram_bytes_reserved = 0;
      int64_t              total_ram_stake = 0;
      block_timestamp      last_producer_schedule_update;
      time_point           last_pervote_bucket_fill;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
      uint32_t             total_unpaid_blocks = 0; /// all blocks which have been produced but not paid
      int64_t              total_activated_stake = 0;
      time_point           thresh_activated_stake_time;
      uint16_t             last_producer_schedule_size = 0;
      double               total_producer_vote_weight = 0; /// the sum of all producer votes
      block_timestamp      last_name_close;
      uint64_t primary_key()const { return 1;      }  
   };
*/

  TABLE producer_info {
    name                  owner;
    double                total_votes = 0;
    //eosio::public_key     producer_key; /// a packed public key object
    bool                  is_active = true;
    std::string           url;
    uint32_t              unpaid_blocks = 0;
    time_point            last_claim_time;
    uint16_t              location = 0;

    uint64_t primary_key()const { return owner.value;                             }
    double   by_votes()const    { return is_active ? -total_votes : total_votes;  }
    bool     active()const      { return is_active;                               }
    //void     deactivate()       { producer_key = public_key(); is_active = false; }

  };

  struct st_transfer {
      name  from;
      name  to;
      asset         quantity;
      std::string   memo;
  };

  //Multi index types definition
  typedef eosio::multi_index<name("global"), global> globaltable;

  typedef eosio::multi_index<name("stats"), stats,
      indexed_by<name("count"), const_mem_fun<stats, uint64_t, &stats::by_count>>> statstable;

  typedef eosio::multi_index<name("pairs"), pairs> pairstable;

  typedef eosio::multi_index<name("datapoints"), datapoints,
      indexed_by<name("value"), const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>, 
      indexed_by<name("timestamp"), const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>> datapointstable;

typedef eosio::multi_index<name("producers"), producer_info,
      indexed_by<name("prototalvote"), const_mem_fun<producer_info, double, &producer_info::by_votes>>> producers_table;

  //Check if calling account is a qualified oracle
  bool check_oracle(const name owner); 

  //Ensure account cannot push data more often than every 60 seconds
  void check_last_push(const name owner, const name pair); 

  //Push oracle message on top of queue, pop old elements (older than one minute)
  void update_datapoints(const name owner, const uint64_t value, name pair); 

  //Write datapoint
  
  ACTION write(const name owner, const std::vector<quote>& quotes); 

  //claim rewards
 
  ACTION claim(name owner); 

  //temp configuration
  
  ACTION configure(); 

  //Clear all data
  ACTION clear(name pair); 


  ACTION transfer(uint64_t sender, uint64_t receiver);

};



extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        if(code==receiver)
        {
            switch(action)
            {
                EOSIO_DISPATCH_HELPER(oracle, (write)(clear)(claim)(configure)(transfer))
            }
        }
        else if(code=="eosio.token"_n.value && action=="transfer"_n.value) {
            execute_action( name(receiver), name(code), &oracle::transfer);
        }
    }
}
