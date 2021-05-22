// oraclehub

#pragma once


#include <eosio/eosio.hpp>
#include <eosio/symbol.hpp>
#include <eosio/asset.hpp>


namespace oraclehub {

// Defaults
static const uint64_t defaultVol = 600000;
static const int64_t defaultCorr = 1000000;

// Types
enum e_asset_type : uint16_t {
  fiat = 1,
  cryptocurrency = 2,
  erc20_token = 3,
  eosio_token = 4,
  equity = 5,
  derivative = 6,
  other = 7
};

typedef uint16_t asset_type;

// Quote
struct quote_data {
  uint64_t value;
  eosio::name pair;
};


// Status of qualified oracles
struct [[eosio::table("oracles"), eosio::contract("oraclehub")]] oracle_data {
  eosio::name owner;
  bool approved;
  eosio::time_point timestamp;
  uint64_t count;
  eosio::asset balance;

  uint64_t primary_key() const { return owner.value; }
  uint64_t by_count() const { return count; }
};

typedef eosio::multi_index<
    eosio::name("oracles"), oracle_data,
    eosio::indexed_by<
        eosio::name("count"),
        eosio::const_mem_fun<oracle_data, uint64_t, &oracle_data::by_count>>>
    oracles_table;

// Accepted pairs
struct [[eosio::table("pairs"), eosio::contract("oraclehub")]] pair_data {
  eosio::name name;

  bool active = true;

  eosio::symbol base_symbol;
  asset_type base_type;
  eosio::name base_contract;

  eosio::symbol quote_symbol;
  asset_type quote_type;
  eosio::name quote_contract;

  uint64_t quoted_precision;

  uint64_t primary_key() const { return name.value; }
};

typedef eosio::multi_index<eosio::name("pairs"), pair_data> pairs_table;

    struct token {
        eosio::name                 contract;
        eosio::symbol               symbol;
   };


// Accepted pairs, depricated, superceded by pairsde_tab
struct [[eosio::table("pairsd"), eosio::contract("oraclehub")]] paird_data {
        uint64_t                    id;
        token                       token0;
        token                       token1;
        bool active = true;
        uint8_t                     precision_scale;
        eosio::name                 name;

  uint64_t primary_key() const { return id; }
};

typedef eosio::multi_index<eosio::name("pairsd"), paird_data> pairsd_table;

// Accepted pairs
struct [[eosio::table("pairsde"), eosio::contract("oraclehub")]] pairde_data {
        eosio::name                 name;
        uint64_t                    id;
        token                       token0;
        token                       token1;
        bool                        active = true;
        uint8_t                     precision_scale;
        bool                        lp_pricing = false;
        uint8_t                     base_token = 1;

  uint64_t primary_key() const { return name.value; }
};

typedef eosio::multi_index<eosio::name("pairsde"), pairde_data> pairsde_tab;

// Holds the latest datapoints from qualified oracles
struct [[eosio::table("datapoints"), eosio::contract("oraclehub")]] datapoints
{
    uint64_t id;
    eosio::name owner;
    uint64_t value;
    uint64_t median;
    eosio::time_point timestamp;

    uint64_t primary_key() const { return id; }
    uint64_t by_timestamp() const { return timestamp.elapsed.to_seconds(); }
    uint64_t by_value() const { return value; }
};

typedef eosio::multi_index<eosio::name("datapoints"), datapoints,
                           eosio::indexed_by<eosio::name("value"), eosio::const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
                           eosio::indexed_by<eosio::name("timestamp"), eosio::const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>>
    datapoints_table;

// Holds the time series of prices, returns, volatility and correlation
struct [[eosio::table("tseries"), eosio::contract("oraclehub")]] statspre {
  uint32_t freq;
  eosio::time_point timestamp;
  std::deque<uint64_t> price;
  std::deque<int64_t> returns;
  std::map <eosio::symbol, int64_t> correlation_matrix;
  std::uint64_t vol = defaultVol;

  uint32_t primary_key() const {return freq;}
};

typedef eosio::multi_index<eosio::name("tseries"), statspre> prices_table;

} // namespace oraclehub