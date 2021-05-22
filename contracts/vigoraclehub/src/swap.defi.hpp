#pragma once

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>

namespace swapdefi {
    struct token {
        eosio::name                 contract;
        eosio::symbol               symbol;
   };
    struct [[eosio::table("pairs")]] pairs_row {
        uint64_t                    id;
        token                       token0;
        token                       token1;
        eosio::asset                reserve0;
        eosio::asset                reserve1;
        uint64_t                    liquidity_token;
        double                     price0_last;
        double                     price1_last;
        uint64_t                      price0_cumulative_last;
        uint64_t                      price1_cumulative_last;
        eosio::time_point_sec              block_time_last;
        uint64_t primary_key() const { return id; }
    };

    typedef eosio::multi_index< "pairs"_n, pairs_row > pairs;

}
