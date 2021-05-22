#pragma once

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/singleton.hpp>

namespace curvesx {


    struct [[eosio::table("pairs")]] pairs_row {
        eosio::symbol_code         id;
        eosio::extended_asset      reserve0;
        eosio::extended_asset      reserve1;
        eosio::extended_asset      liquidity;
        uint64_t            amplifier;
        double              virtual_price;
        double              price0_last;
        double              price1_last;
        eosio::asset               volume0;
        eosio::asset               volume1;
        uint64_t            trades;
        eosio::time_point_sec      last_updated;

        uint64_t primary_key() const { return id.raw(); }
    };
    typedef eosio::multi_index< "pairs"_n, pairs_row> pairs_table;

    struct [[eosio::table("config")]] config_row {
        eosio::name                status = "testing"_n;
        uint8_t             trade_fee = 4;
        uint8_t             protocol_fee = 0;
        eosio::name                fee_account = "fee.sx"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;


}
