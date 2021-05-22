#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <string>

using namespace eosio;
using namespace std;

class [[eosio::contract("curve.sx")]] curveSx : public contract {
public:
    using contract::contract;

    struct [[eosio::table("pairs")]] pairs_row {
        symbol_code         id;
        extended_asset      reserve0;
        extended_asset      reserve1;
        extended_asset      liquidity;
        uint64_t            amplifier;
        double              virtual_price;
        double              price0_last;
        double              price1_last;
        asset               volume0;
        asset               volume1;
        uint64_t            trades;
        time_point_sec      last_updated;

        uint64_t primary_key() const { return id.raw(); }
    };
    typedef eosio::multi_index< "pairs"_n, pairs_row> pairs_table;
    pairs_table _p;

    struct [[eosio::table("config")]] config_row {
        name                status = "testing"_n;
        uint8_t             trade_fee = 4;
        uint8_t             protocol_fee = 0;
        name                fee_account = "fee.sx"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    ACTION config();
    
    [[eosio::action]]
        
    void addpair(symbol_code id, extended_asset reserve0, extended_asset reserve1, extended_asset liquidity, uint64_t amplifier, double virtual_price, double price0_last, double price1_last, asset volume0, asset volume1, uint64_t trades, time_point_sec last_updated);
    
    curveSx( eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds );
    ~curveSx();
};