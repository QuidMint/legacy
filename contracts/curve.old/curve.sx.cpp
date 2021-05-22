#include "curve.sx.hpp"

curveSx::curveSx( eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds )
  : contract( receiver, code, ds )
  , _p( receiver, receiver.value ) {
}

curveSx::~curveSx() {
}

void curveSx::config() {
}

[[eosio::action]]
void curveSx::addpair(symbol_code id, extended_asset reserve0, extended_asset reserve1, extended_asset liquidity, uint64_t amplifier, double virtual_price, double price0_last, double price1_last, asset volume0, asset volume1, uint64_t trades, time_point_sec last_updated) {

  require_auth(get_self());

  pairs_table pairs(_self, _self.value);
  //datapoints_table dstore(_self, pair.name.value);

  auto itr = pairs.find(id.raw());

  if (itr == pairs.end()) {
  pairs.emplace(_self, [&](auto& a) {
                   a.id = id;   
                   a.reserve0 = reserve0;
                   a.reserve1 = reserve1;
                   a.liquidity = liquidity;
                   a.amplifier = amplifier;
                   a.virtual_price = virtual_price;
                   a.price0_last = price0_last;
                   a.price1_last = price1_last;
                   a.volume0 = volume0;
                   a.volume1 = volume1;
                   a.trades = trades;
  });
  } else {
  pairs.modify(itr, _self, [&](auto& a) {
                   a.id = id;   
                   a.reserve0 = reserve0;
                   a.reserve1 = reserve1;
                   a.liquidity = liquidity;
                   a.amplifier = amplifier;
                   a.virtual_price = virtual_price;
                   a.price0_last = price0_last;
                   a.price1_last = price1_last;
                   a.volume0 = volume0;
                   a.volume1 = volume1;
                   a.trades = trades;
  });
  }
}
/*

token token0 = {eosio::name("vig111111111"), eosio::symbol("VIG", 4)};
token token1 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
        
        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=11,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(52705836340, eosio::symbol("VIG", 4)),
         a.reserve1=eosio::asset(35937721, eosio::symbol("EOS", 4)),
         a.liquidity_token=19043041;
         a.price0_last = 0.0043335890659;
         a.price1_last = 1466.5881662334;
         a.price0_cumulative_last = 7294;
         a.price1_cumulative_last = 100;
        });


token0 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
token1 = {eosio::name("vigortoken11"), eosio::symbol("VIGOR", 4)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=76,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(23377060, eosio::symbol("EOS", 4)),
         a.reserve1=eosio::asset(119386804, eosio::symbol("VIGOR", 4)),
         a.liquidity_token=48008628;
         a.price0_last = 5.10700678357329707;
         a.price1_last = 0.19580941290630413;
         a.price0_cumulative_last = 75789851;
         a.price1_cumulative_last = 6463414;
        });

token0 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
token1 = {eosio::name("tethertether"), eosio::symbol("USDT", 4)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=12,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(10837150660, eosio::symbol("EOS", 4)),
         a.reserve1=eosio::asset(52107879487, eosio::symbol("USDT", 4)),
         a.liquidity_token=7556034061;
         a.price0_last = 4.80826382522580786;
         a.price1_last = 0.20797527680441652;
         a.price0_cumulative_last = 67287525;
         a.price1_cumulative_last = 6666427;
        });

token0 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
token1 = {eosio::name("danchortoken"), eosio::symbol("USN", 4)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=9,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(18681762184, eosio::symbol("EOS", 4)),
         a.reserve1=eosio::asset(89577003945, eosio::symbol("USN", 4)),
         a.liquidity_token=63686699020;
         a.price0_last = 4.79489049602174333;
         a.price1_last = 0.20855533631679110;
         a.price0_cumulative_last = 71396660;
         a.price1_cumulative_last = 6777524;
        });

token0 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
token1 = {eosio::name("token.defi"), eosio::symbol("BOX", 6)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=194,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(12799633408, eosio::symbol("EOS", 4)),
         a.reserve1=eosio::asset(369403587617, eosio::symbol("BOX", 6)),
         a.liquidity_token=52001370091;
         a.price0_last = 0.28860481846778219;
         a.price1_last = 3.46494561424529035;
         a.price0_cumulative_last = 13750012;
         a.price1_cumulative_last = 49478317;
        });

token0 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};
token1 = {eosio::name("btc.ptokens"), eosio::symbol("PBTC", 8)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=177,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(2749490298, eosio::symbol("EOS", 4)),
         a.reserve1=eosio::asset(2223095166, eosio::symbol("PBTC", 8)),
         a.liquidity_token=2305837548;
         a.price0_last = 0.00008085481034856;
         a.price1_last = 12367.84794484142366855;
         a.price0_cumulative_last = 48;
         a.price1_cumulative_last = 143352907016;
        });

token0 = {eosio::name("newdexissuer"), eosio::symbol("NDX", 4)};
token1 = {eosio::name("eosio.token"), eosio::symbol("EOS", 4)};

        _p.emplace( get_self(), [&]( auto & a ) {
         a.id=1,
         a.token0=token0,
         a.token1=token1,
         a.reserve0=eosio::asset(3229061268666, eosio::symbol("NDX", 4)),
         a.reserve1=eosio::asset(890865148, eosio::symbol("EOS", 4)),
         a.liquidity_token=550693705;
         a.price0_last = 0.00027588982489887;
         a.price1_last = 3624.63530638197062217;
         a.price0_cumulative_last = 167;
         a.price1_cumulative_last = 94109847100;
        });
        */

        

