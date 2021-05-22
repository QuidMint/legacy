// Vigor Oracle HUB poller for DelphiOracle

#pragma once

#include <eosio/eosio.hpp>
#include <cmath>

#include "../../vigoraclehub/src/oraclehub_tables.hpp"
#include "delphioracle.hpp"
#include "recurring_action.hpp"


namespace delphipoller {

// Configurations
#define VIGORACLEHUB_CONTRACT eosio::name("vigoraclehub")
#define DELPHIORACLE_CONTRACT eosio::name("delphioracle")
#define RECURRING_DELAYSEC    60
#define RECURRING_ACTION      eosio::name("update")


class [[eosio::contract("delphipoller")]] delphipoller_contract : public eosio::contract {
public:
  delphipoller_contract(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds) :
    eosio::contract(receiver, code, ds)
  {}

  // poll action
  [[eosio::action]]
  void update();
};

} // namespace delphipoller
