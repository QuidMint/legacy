#pragma once
#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/contract.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>
#include <eosio/singleton.hpp>
#include <eosio/symbol.hpp>
#include <eosio/time.hpp>

/***************************
croneos api config
***************************/

#define _ENABLE_INTERNAL_QUEUE_
#if defined(LOCAL_MODE)
#define _CRONEOS_CONTRACT_ "croncron1111"
#define _DEFAULT_EXEC_ACC_ "execexecexec"
#define _CONTRACT_NAME_ "vigorlending"  //the class name of your contract
#elif defined(TEST_MODE)
#define _CRONEOS_CONTRACT_ "croncron3333"
#define _DEFAULT_EXEC_ACC_ "execexecexec"
#define _CONTRACT_NAME_ "vigorlending"  //the class name of your contract
#else
#define _CRONEOS_CONTRACT_ "cron.eos"
#define _DEFAULT_EXEC_ACC_ "croneosexec1"
#define _CONTRACT_NAME_ "vigorlending"  //the class name of your contract
#endif

/***************************
end croneos api config
***************************/

namespace croneos {
namespace oracle {
struct source {
  std::string api_url;
  std::string json_path;  //https://www.npmjs.com/package/jsonpath
};
}  // namespace oracle

struct cronjobs {
  uint64_t id;
  eosio::name owner;
  eosio::name tag;
  eosio::name auth_bouncer;
  std::vector<eosio::action> actions;
  eosio::time_point_sec submitted;
  eosio::time_point_sec due_date;
  eosio::time_point_sec expiration;
  eosio::asset gas_fee;
  std::string description;
  uint8_t max_exec_count = 1;
  std::vector<croneos::oracle::source> oracle_srcs;

  uint64_t primary_key() const { return id; }
  uint64_t by_owner() const { return owner.value; }
  uint64_t by_due_date() const { return due_date.sec_since_epoch(); }
  uint128_t by_owner_tag() const { return (uint128_t {owner.value} << 64) | tag.value; }
};
typedef eosio::multi_index<"cronjobs"_n, cronjobs,
                           eosio::indexed_by<"byowner"_n, eosio::const_mem_fun<cronjobs, uint64_t, &cronjobs::by_owner>>,
                           eosio::indexed_by<"byduedate"_n, eosio::const_mem_fun<cronjobs, uint64_t, &cronjobs::by_due_date>>,
                           eosio::indexed_by<"byownertag"_n, eosio::const_mem_fun<cronjobs, uint128_t, &cronjobs::by_owner_tag>>>
  cronjobs_table;
//************************

struct [[eosio::table, eosio::contract("vigorlending")]] croneosqueue {
  uint64_t id;
  eosio::name tag;
  eosio::action action;
  eosio::time_point_sec due_date;
  eosio::time_point_sec expiration;
  uint64_t repeat;

  uint64_t primary_key() const { return id; }
  uint64_t by_tag() const { return tag.value; }
  uint64_t by_due_date() const { return due_date.sec_since_epoch(); }
  uint64_t by_expiration() const { return expiration.sec_since_epoch(); }
};

typedef eosio::multi_index<"croneosqueue"_n, croneosqueue,
                           eosio::indexed_by<"bytag"_n, eosio::const_mem_fun<croneosqueue, uint64_t, &croneosqueue::by_tag>>,
                           eosio::indexed_by<"byduedate"_n, eosio::const_mem_fun<croneosqueue, uint64_t, &croneosqueue::by_due_date>>,
                           eosio::indexed_by<"byexpiration"_n, eosio::const_mem_fun<croneosqueue, uint64_t, &croneosqueue::by_expiration>>>
  croneosqueue_table;
//************************

struct [[eosio::table, eosio::contract("vigorlending")]] croneosstats {
  uint64_t total_count = 0;
  uint64_t exec_count = 0;
  uint64_t cancel_count = 0;
  uint64_t expired_count = 0;
};
typedef eosio::singleton<"croneosstats"_n, croneosstats> croneosstats_table;
//************************

struct queue {
  eosio::name owner;
  croneosstats_table _croneosstats;
  croneosstats stats;

  queue(eosio::name Owner) : _croneosstats(Owner, Owner.value) {
    owner = Owner;
    stats = _croneosstats.get_or_create(owner, croneosstats());
  }

  ~queue() {
    _croneosstats.set(stats, owner);
  }

  //schedules an action in the queue
  void schedule(eosio::action action, eosio::name tag, eosio::asset gas_fee, uint32_t delay_sec, uint32_t expiration_sec) {
    schedule(action, tag, gas_fee, delay_sec, expiration_sec, 1);
  }

  void schedule(eosio::action action, eosio::name tag, eosio::asset gas_fee, uint32_t delay_sec, uint32_t expiration_sec, uint64_t repeat) {
    eosio::time_point_sec due_date = eosio::time_point_sec(now.sec_since_epoch() + delay_sec);
    eosio::time_point_sec expiration = eosio::time_point_sec(now.sec_since_epoch() + delay_sec + expiration_sec);
    croneos::croneosqueue_table _croneosqueue(owner, owner.value);

    uint64_t start_id = stats.total_count;

    clean_expired(_croneosqueue, 100);

    //rate limit to prevent a given user from scheduling more than one job (owner is allowed to have multiple)
    if(tag != owner)
      ratelimit(_croneosqueue, tag);

    _croneosqueue.emplace(owner, [&](auto& n) {
      n.id = stats.total_count;
      n.tag = tag;
      n.action = action;
      n.due_date = due_date;
      n.expiration = expiration;
      n.repeat = repeat;
    });
    stats.total_count += repeat;
  }

  //exec 1 action from the queue
  bool exec() {
    croneosqueue_table _croneosqueue(owner, owner.value);
    auto by_due_date = _croneosqueue.get_index<"byduedate"_n>();
    if(by_due_date.begin()->expiration < now) {
      clean_expired(_croneosqueue, 50);
    }
    auto itr_first = by_due_date.begin();
    if(itr_first != by_due_date.end()) {
      if(itr_first->due_date <= now) {
        itr_first->action.send();
        if(itr_first->repeat == 1) by_due_date.erase(itr_first);
        else
          by_due_date.modify(itr_first, eosio::name(_CONTRACT_NAME_), [&](auto& cq) {
            cq.repeat--;
          });
        stats.exec_count++;
        return true;
      } else {
        eosio::check(false, "queued action not ready yet, wait " + std::to_string(itr_first->due_date.sec_since_epoch() - now.sec_since_epoch()) + " sec");
        return false;
      }
    } else {
      return false;
    }
  }

  bool clean_expired(int32_t batch_size) {
    croneosqueue_table _croneosqueue(owner, owner.value);
    return clean_expired(_croneosqueue, batch_size);
  }

  void rate_limit(eosio::name usern) {
    croneos::croneosqueue_table _croneosqueue(owner, owner.value);
    clean_expired(_croneosqueue, 25);
    ratelimit(_croneosqueue, usern);
  }

  void cancel_by_tag(eosio::name id, uint8_t size) {
    croneosqueue_table _croneosqueue(owner, owner.value);
    auto itr = _croneosqueue.find(id.value);
    if(itr != _croneosqueue.end()) {
      eosio::check(itr->repeat - size >= 0, "Can't cancel requested number of jobs: " + std::to_string(size) + " from croneosqueue, job id: " + std::to_string(id.value) + ", there are only this many repeats: " + std::to_string(itr->repeat));
      stats.cancel_count += size;
      if(itr->repeat - size > 0)
        _croneosqueue.modify(itr, "vigorlending"_n, [&](auto& cq) {
          cq.repeat -= size;
        });
      else
        _croneosqueue.erase(itr);
    }
  }

  void resetStats() {
    stats = croneosstats();
  };

 private:
  eosio::time_point_sec now = eosio::time_point_sec(eosio::current_time_point());

  bool clean_expired(croneosqueue_table& idx, uint32_t batch_size) {
    auto by_expiration = idx.get_index<"byexpiration"_n>();
    auto stop_itr = by_expiration.upper_bound(now.sec_since_epoch());
    uint32_t counter = 0;
    auto itr = by_expiration.begin();
    while(itr != stop_itr && counter++ < batch_size) {
      stats.expired_count += itr->repeat;
      itr = by_expiration.erase(itr);
    }
    return counter > 0 ? true : false;
  }

  void ratelimit(croneosqueue_table& _croneosqueue, eosio::name usern) {
    auto by_tag = _croneosqueue.get_index<"bytag"_n>();
    auto itu = by_tag.find(usern.value);
    eosio::check(itu == by_tag.end(), "Your previous transaction is being mined, please wait " + std::to_string(itu->expiration.sec_since_epoch() - eosio::current_time_point().sec_since_epoch()) + " seconds.");
  }
};

}  // namespace croneos
