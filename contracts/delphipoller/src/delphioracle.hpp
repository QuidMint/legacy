// data structures from delphioracle
// to be used by oraclehub delphipoller

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>


namespace delphioracle {

//Types

typedef uint16_t asset_type;

// Holds the latest datapoints from qualified oracles
struct [[eosio::table("datapoints"), eosio::contract("delphioracle")]] datapoints_data
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

typedef eosio::multi_index<eosio::name("datapoints"), datapoints_data,
                           eosio::indexed_by<eosio::name("value"), eosio::const_mem_fun<datapoints_data, uint64_t, &datapoints_data::by_value>>,
                           eosio::indexed_by<eosio::name("timestamp"), eosio::const_mem_fun<datapoints_data, uint64_t, &datapoints_data::by_timestamp>>>
  datapoints_table;


// Holds the list of pairs
struct [[eosio::table("pairs"), eosio::contract("delphioracle")]] pairs_data {
  bool active = false;
  bool bounty_awarded = false;
  bool bounty_edited_by_custodians = false;

  eosio::name proposer;
  eosio::name name;

  eosio::asset bounty_amount = eosio::asset(0, eosio::symbol("EOS", 4));

  std::vector<eosio::name> approving_custodians;
  std::vector<eosio::name> approving_oracles;

  eosio::symbol base_symbol;
  asset_type base_type;
  eosio::name base_contract;

  eosio::symbol quote_symbol;
  asset_type quote_type;
  eosio::name quote_contract;

  uint64_t quoted_precision;

  uint64_t primary_key() const {return name.value;}
};

typedef eosio::multi_index<eosio::name("pairs"), pairs_data> pairs_table;

} // namespace delphioracle
