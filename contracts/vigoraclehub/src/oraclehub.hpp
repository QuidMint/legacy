// oraclehub

#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <cmath>

#include "common.hpp"
#include "oraclehub_tables.hpp"

namespace oraclehub {

// Constants

// DAC accounts/contracts
#if defined(LOCAL_MODE)
static const eosio::name dac_custodian = eosio::name("daccustodia1");
static const eosio::name DELPHIORACLE_CONTRACT = eosio::name("oracleoracl2");
static const eosio::name SWAPDEFI_CONTRACT = eosio::name("swap.defi");
static const eosio::name CURVESX_CONTRACT = eosio::name("vigorsxcurve");
static const eosio::name DELPHIPOLLER_CONTRACT = eosio::name("delphipoller");
static const eosio::name PAYMENT_CONTRACT = eosio::name("dacholding11");
#elif defined(TEST_MODE)
static const eosio::name dac_custodian = eosio::name("daccustodia1");
static const eosio::name DELPHIORACLE_CONTRACT = eosio::name("delphioracle");
static const eosio::name SWAPDEFI_CONTRACT = eosio::name("swap.defi");
static const eosio::name CURVESX_CONTRACT = eosio::name("curve.sx");
static const eosio::name DELPHIPOLLER_CONTRACT = eosio::name("delphipoller");
static const eosio::name PAYMENT_CONTRACT = eosio::name("dacholding11");
#else
static const eosio::name dac_custodian = eosio::name("daccustodia1");
static const eosio::name DELPHIORACLE_CONTRACT = eosio::name("delphioracle");
static const eosio::name SWAPDEFI_CONTRACT = eosio::name("swap.defi");
static const eosio::name CURVESX_CONTRACT = eosio::name("curve.sx");
static const eosio::name DELPHIPOLLER_CONTRACT = eosio::name("delphipoller");
static const eosio::name PAYMENT_CONTRACT = eosio::name("dacholding11");
#endif


// Min value set to 0.01$ , max value set to 10,000$
static const uint64_t val_min = 1;
static const uint64_t val_max = 1000000;

// from datapreproc
static const uint32_t one_minute = 60; 
static const uint32_t five_minute = 60 * 5;
static const uint32_t fifteen_minute = 60 * 15;
static const uint32_t one_hour = 60 * 60;
static const uint32_t four_hour = 60 * 60 * 4; 
static const uint32_t one_day = 60 * 60 * 24; 
static const uint32_t cronlag = 5; //give extra time for cron jobs
static const uint64_t dequesize = 60;
static const double returnsPrecision = 1000000.0;
static const double pricePrecision = 1000000.0;
#if defined(LOCAL_MODE)
static const uint32_t time_series_periodicity = five_minute;
static const uint32_t stats_refresh_period = five_minute;
static const uint32_t price_refresh_period = one_minute;
#elif defined (TEST_MODE)
static const uint32_t time_series_periodicity = one_day;
static const uint32_t stats_refresh_period = one_hour;
static const uint32_t price_refresh_period = one_minute;
#else
static const uint32_t time_series_periodicity = one_day;
static const uint32_t stats_refresh_period = one_hour;
static const uint32_t price_refresh_period = one_minute;
#endif

static const double one_minute_scale = sqrt(252.0*24.0*(60.0/1.0));
static const double five_minute_scale = sqrt(252.0*24.0*(60.0/5.0));
static const double fifteen_minute_scale = sqrt(252.0*24.0*(60.0/15.0));
static const double one_hour_scale = sqrt(252.0*24.0*(60.0/60.0));
static const double four_hour_scale = sqrt(252.0*24.0*(60.0/(60.0*4.0)));
static const double one_day_scale = sqrt(252.0*24.0*(60.0/(60.0*24.0)));
static const std::map <uint32_t, double> volScale {
  {one_minute,	    one_minute_scale},
  {five_minute,	    five_minute_scale},
  {fifteen_minute,	fifteen_minute_scale},
  {one_hour,	      one_hour_scale},
  {four_hour,	      four_hour_scale},
  {one_day,	        one_day_scale},
};


// Contract

class [[eosio::contract("oraclehub")]] oraclehub_contract : public eosio::contract {
public:
  oraclehub_contract(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds) : eosio::contract(receiver, code, ds) {}

#if defined(LOCAL_MODE) || defined(TEST_MODE)

  // store shock data for test
  struct [[eosio::table("shock")]] shock_data {
    double shock = 1.0;
  };

  typedef eosio::singleton<eosio::name("shock"), shock_data> shock_singleton;

#endif

  // Transfer
  struct transfer_data {
    eosio::name from;
    eosio::name to;
    eosio::asset quantity;
    std::string memo;
  };

  // CHECKERS

  // Check if calling account is a qualified oracle
  void check_oracle(const eosio::name owner);

  // Ensure account cannot push data more often than every 60 seconds
  void check_last_push(const eosio::name owner);

  // Ensure account cannot push data more often than every 60 seconds
  void check_last_pushd(const eosio::name owner);

  // HELPERS

  // Push oracle message on top of queue, pop old elements (older than one
  // minute)
  void update_datapoints(const eosio::name owner, const uint64_t value,
                         eosio::name pair, const uint64_t oraclecount);

  // Get last price from datapoints table (from datapreproc)
  uint64_t get_last_price(eosio::name pair, uint64_t quoted_precision);

  // Calculate vol and correlation matrix (from datapreproc)
  void calcstats(const eosio::name pair, const uint32_t freq);

  // Correlation coefficient (from datapreproc)
  int64_t corrCalc(std::deque<int64_t> X, std::deque<int64_t> Y, uint64_t n);

  // Volatility calculation (from datapreproc)
  uint64_t volCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq, uint64_t dflt);
  
   // EWMA Volatility calculation (from datapreproc)
  uint64_t EWMAvolCalc(std::deque<int64_t> returns, uint64_t n, const uint32_t freq, uint64_t dflt);

  // Store last price from the oracle, append to time series (from datapreproc)
  void store_last_price(const eosio::name pair, const uint32_t freq, const uint64_t lastprice);

  eosio::name getname(int n);

  // ACTIONS

  // Approve an oracle (auth by dac account)
  [[eosio::action]]
  void approve(const eosio::name oracle);

  // Disapprove an oracle temporarily (auth by dac account)
  [[eosio::action]]
  void disapprove(const eosio::name oracle);

  // Revoke an oracle access definitively (auth by dac account)
  [[eosio::action]]
  void revoke(const eosio::name oracle);

  // Add a new pair (auth by dac account)
  [[eosio::action]]
  void addpair(const pair_data pair);

  // Delete a pair (auth by dac account)
  [[eosio::action]]
  void delpair(const eosio::name name, const std::string reason);

  // Switch a pair active or inactive (auth by dac account)
  [[eosio::action]]
  void switchpair(const eosio::name name, const bool active);

    // Add a new pair (auth by dac account)
  [[eosio::action]]
  void addpaird(const uint64_t id, const token token0, const token token1, uint8_t precision_scale, eosio::name name);

  // Delete a pair (auth by dac account)
  [[eosio::action]]
  void delpaird(const uint64_t id, const std::string reason);

  // Switch a pair active or inactive (auth by dac account)
  [[eosio::action]]
  void switchpaird(const uint64_t id, const bool active);

    // Add a new pair (auth by dac account)
  [[eosio::action]]
  void addpairde(eosio::name name, const uint64_t id, const token token0, const token token1, uint8_t precision_scale, bool lp_pricing, uint8_t base_token);

  // Delete a pair (auth by dac account)
  [[eosio::action]]
  void delpairde(const eosio::name name, const std::string reason);

  // Switch a pair active or inactive (auth by dac account)
  [[eosio::action]]
  void swpairde(const eosio::name name, const bool active);

  // Push quotes
  [[eosio::action]]
  void push(const eosio::name owner, std::vector<quote_data> &quotes);

  // 
  [[eosio::action]]
  void pushd(const eosio::name owner);

  // Claim rewards
  [[eosio::action]]
  void claim(eosio::name owner);

  // TODO: Clear all data - move to test mode only?
  [[eosio::action]]
  void clear(eosio::name pair);

#if defined(LOCAL_MODE) || defined(TEST_MODE)

  // Apply a shock to the prices for testing
  [[eosio::action]]
  void doshock(double shockvalue);

  // TODO: reset volatility - still needed?
  [[eosio::action]]
  void resetvol(const eosio::name pair, const uint32_t freq);

  [[eosio::action]]
  void updatepricea();

  [[eosio::action]]
  void updatestatsa();

#endif

  // TODO: purge price information - still needed?
  [[eosio::action]]
  void purge();

  // Get median price and store in deque as a historical time series (from datapreproc)
  void updateprices(eosio::name pair);

  template <class T>
  bool is_unique(std::vector<T> X);
  
  // NOTIFICATION HANDLERS

  [[eosio::on_notify("vig111111111::transfer")]]
  void transfer(uint64_t sender, uint64_t receiver);

};

} // namespace oraclehub