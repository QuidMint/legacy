// recurring actions for EOSIO smart contracts
#pragma once

#include <algorithm>

#include <libc/bits/stdint.h>

#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>


#define EOSIO_MAX_DELAY 3888000


class recurring_action {

  // the contract owner account
  eosio::name owner;

  // the deferred tx delay in seconds
  unsigned int delay_sec;

  // the action to call
  eosio::name action;

  inline uint128_t action_sender_id(eosio::name action) {
    return ((uint128_t)owner.value << 64) + action.value;
  }

  // posts a deferred tx
  void post_action(eosio::name action, unsigned int delay_sec) {
    // the deferred tx id
    uint128_t sender_id = action_sender_id(action);
    // build the tx
    eosio::transaction tx;
    tx.actions.emplace_back(
      eosio::permission_level(owner, eosio::name("active")),
      owner,
      action,
      0
    );
    tx.delay_sec = std::min(delay_sec, (unsigned int)EOSIO_MAX_DELAY);
    // send it
    tx.send(sender_id, owner, false);
  }

  // cancel a deferred tx
  void cancel_action(eosio::name action) {
    // the deferred tx id
    uint128_t sender_id = action_sender_id(action);
    // cancel it
    eosio::cancel_deferred(sender_id);
  }

  // posts a deferred tx calling our recurring action
  void next() {
    // first remove the deferred tx if any
    cancel_action(action);

    // then add new one
    post_action(action, delay_sec);
  }

protected:

  recurring_action(eosio::name owner, unsigned int delay_sec, eosio::name action) :
                   owner(owner), delay_sec(delay_sec), action(action) {
    // automatically send new deferred tx on all contract actions
    next();
  }

  // user call to another action and run a failsafe to allow the action to fail and don't break the recurring chain
  void call_action(eosio::name user_action) {
    // first remove any other deferred tx for this action
    cancel_action(user_action);
    // and call the user action
    post_action(user_action, 0);
  }

};
