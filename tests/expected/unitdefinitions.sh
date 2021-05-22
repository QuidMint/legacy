#!/bin/bash
#=================================================================================#
# SETUP
################ need to use constant price feeds ##################################
# quotes:[{value: parseInt(Math.round(0.0000343 * 10000000)), pair: "boideos"}]
# quotes:[{value: parseInt(Math.round((3.733) * 10000)), pair: "eosusd"}]
# quotes:[{value: parseInt(Math.round(0.000272 * 1000000)), pair: "iqeos"}]
# quotes:[{value: parseInt(Math.round(0.000318 * 100000000)), pair: "vigeos"}]
# quotes:[{value: parseInt(Math.round(((1.0/3.733)+(0.01)) * 1000000)), pair: "vigoreos"}]
################ collectfee() need to use a constant time delta:#################
# eosio::microseconds dmsec = eosio::current_time_point().time_since_epoch() - _globalsWritable.lastupdate.time_since_epoch();
# eosio::microseconds dmsec(60000000);

pkill nodeos
rm -rf ~/.local/share/eosio/nodeos/data
nodeos -e -p eosio --http-validate-host=false --delete-all-blocks --plugin eosio::producer_api_plugin --plugin eosio::chain_api_plugin --contracts-console --plugin eosio::http_plugin --plugin eosio::history_api_plugin --verbose-http-errors --max-transaction-time=500000 --http-max-response-time-ms=500 --abi-serializer-max-time-ms=500


#nodeos -e -p eosio --plugin eosio::producer_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::state_history_plugin --access-control-allow-origin='*' --contracts-console --http-validate-host=false --trace-history --chain-state-history --verbose-http-errors --filter-on='*' --disable-replay-opts >> nodeos.log 2>&1 &
#curl --request POST --url http://127.0.0.1:8888/v1/producer/get_supported_protocol_features --header 'content-type: application/x-www-form-urlencoded; charset=UTF-8'


curl -X POST http://127.0.0.1:8888/v1/producer/schedule_protocol_feature_activations -d '{"protocol_features_to_activate": ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}' | jq
CYAN='\033[1;36m'
NC='\033[0m'

# CHANGE PATH
EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio/eosio.contracts/build
cd $EOSIO_CONTRACTS_ROOT
cmake ..
make -j$( nproc )
EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio/eosio.contracts/build/contracts

OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"

#cleos wallet create --to-console
cleos wallet unlock -n default --password PW5HzuR3R2g77WZCsqaSMD91aC3WPFQzmeHkyzyEREbPTB3EmHeMC
#cleos wallet import -n default --private-key $OWNER_ACCT


#=================================================================================#

# EOSIO system-related keys
echo -e "${CYAN}-----------------------SYSTEM KEYS-----------------------${NC}"
cleos wallet import -n default --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
cleos wallet import -n default --private-key 5JgqWJYVBcRhviWZB3TU1tN9ui6bGpQgrXVtYZtTG2d3yXrDtYX
cleos wallet import -n default --private-key 5JjjgrrdwijEUU2iifKF94yKduoqfAij4SKk6X5Q3HfgHMS4Ur6
cleos wallet import -n default --private-key 5HxJN9otYmhgCKEbsii5NWhKzVj2fFXu3kzLhuS75upN5isPWNL
cleos wallet import -n default --private-key 5JNHjmgWoHiG9YuvX2qvdnmToD2UcuqavjRW5Q6uHTDtp3KG3DS
cleos wallet import -n default --private-key 5JZkaop6wjGe9YY8cbGwitSuZt8CjRmGUeNMPHuxEDpYoVAjCFZ
cleos wallet import -n default --private-key 5Hroi8WiRg3by7ap3cmnTpUoqbAbHgz3hGnGQNBYFChswPRUt26
cleos wallet import -n default --private-key 5JbMN6pH5LLRT16HBKDhtFeKZqe7BEtLBpbBk5D7xSZZqngrV8o
cleos wallet import -n default --private-key 5JUoVWoLLV3Sj7jUKmfE8Qdt7Eo7dUd4PGZ2snZ81xqgnZzGKdC
cleos wallet import -n default --private-key 5Ju1ree2memrtnq8bdbhNwuowehZwZvEujVUxDhBqmyTYRvctaF

# Create system accounts
echo -e "${CYAN}-----------------------SYSTEM ACCOUNTS-----------------------${NC}"
cleos create account eosio eosio.bpay EOS7gFoz5EB6tM2HxdV9oBjHowtFipigMVtrSZxrJV3X6Ph4jdPg3
cleos create account eosio eosio.msig EOS6QRncHGrDCPKRzPYSiWZaAw7QchdKCMLWgyjLd1s2v8tiYmb45
cleos create account eosio eosio.names EOS7ygRX6zD1sx8c55WxiQZLfoitYk2u8aHrzUxu6vfWn9a51iDJt
cleos create account eosio eosio.ram EOS5tY6zv1vXoqF36gUg5CG7GxWbajnwPtimTnq6h5iptPXwVhnLC
cleos create account eosio eosio.ramfee EOS6a7idZWj1h4PezYks61sf1RJjQJzrc8s4aUbe3YJ3xkdiXKBhF
cleos create account eosio eosio.saving EOS8ioLmKrCyy5VyZqMNdimSpPjVF2tKbT5WKhE67vbVPcsRXtj5z
cleos create account eosio eosio.stake EOS5an8bvYFHZBmiCAzAtVSiEiixbJhLY8Uy5Z7cpf3S9UoqA3bJb
#cleos create account eosio eosio.token EOS7JPVyejkbQHzE9Z4HwewNzGss11GB21NPkwTX2MQFmruYFqGXm
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.vpay EOS6szGbnziz224T1JGoUUFu2LynVG72f8D3UVAS25QgwawdH983U
cleos create account eosio eosio.rex EOS6szGbnziz224T1JGoUUFu2LynVG72f8D3UVAS25QgwawdH983U

# Bootstrap new system contracts
echo -e "${CYAN}-----------------------SYSTEM CONTRACTS-----------------------${NC}"
#eosio-cpp -contract=eosio.msig -I=$EOSIO_CONTRACTS_ROOT/eosio.msig/include -o=$EOSIO_CONTRACTS_ROOT/eosio.msig/eosio.msig.wasm -abigen $EOSIO_CONTRACTS_ROOT/eosio.msig/src/eosio.msig.cpp
#eosio-cpp -contract=eosio.system -I=$EOSIO_CONTRACTS_ROOT/eosio.system/include -I=$EOSIO_CONTRACTS_ROOT/eosio.token/include -o=$EOSIO_CONTRACTS_ROOT/eosio.system/eosio.system.wasm -abigen $EOSIO_CONTRACTS_ROOT/eosio.system/src/eosio.system.cpp
#eosio-cpp -contract=eosio.wrap -I=$EOSIO_CONTRACTS_ROOT/eosio.wrap/include -o=$EOSIO_CONTRACTS_ROOT/eosio.wrap/eosio.wrap.wasm -abigen $EOSIO_CONTRACTS_ROOT/eosio.wrap/src/eosio.wrap.cpp
#eosio-cpp -contract=eosio.token -I=$EOSIO_CONTRACTS_ROOT/eosio.token/include -o=$EOSIO_CONTRACTS_ROOT/eosio.token/eosio.token.wasm -abigen $EOSIO_CONTRACTS_ROOT/eosio.token/src/eosio.token.cpp
#eosio-cpp -contract=eosio.bios -I=$EOSIO_CONTRACTS_ROOT/eosio.bios/include -o=$EOSIO_CONTRACTS_ROOT/eosio.bios/eosio.bios.wasm -abigen $EOSIO_CONTRACTS_ROOT/eosio.bios/src/eosio.bios.cpp

cleos set contract eosio.token $EOSIO_CONTRACTS_ROOT/eosio.token/
cleos set contract eosio.msig $EOSIO_CONTRACTS_ROOT/eosio.msig
cleos push action eosio.token create '[ "eosio", "100000000000.0000 EOS" ]' -p eosio.token
cleos push action eosio.token create '[ "eosio", "100000000000.0000 SYS" ]' -p eosio.token
echo -e "      EOS TOKEN CREATED"
cleos push action eosio.token issue '[ "eosio", "10000000000.0000 EOS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "eosio", "10000000000.0000 SYS", "memo" ]' -p eosio
echo -e "      EOS TOKEN ISSUED"
cleos set contract eosio $EOSIO_CONTRACTS_ROOT/eosio.bios/
echo -e "      BIOS SET"
cleos set contract eosio $EOSIO_CONTRACTS_ROOT/eosio.system/
echo -e "      SYSTEM SET"
cleos push action eosio setpriv '["eosio.msig", 1]' -p eosio@active
cleos push action eosio init '["0", "4,EOS"]' -p eosio@active
#cleos push action eosio init '["0", "4,SYS"]' -p eosio@actives

# Deploy eosio.wrap
echo -e "${CYAN}-----------------------EOSIO WRAP-----------------------${NC}"
cleos wallet import -n default --private-key 5J3JRDhf4JNhzzjEZAsQEgtVuqvsPPdZv4Tm6SjMRx1ZqToaray
cleos system newaccount eosio eosio.wrap EOS7LpGN1Qz5AbCJmsHzhG7sWEGd9mwhTXWmrYXqxhTknY2fvHQ1A --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 5000 --transfer
cleos push action eosio setpriv '["eosio.wrap", 1]' -p eosio@active
cleos set contract eosio.wrap $EOSIO_CONTRACTS_ROOT/eosio.wrap/
cleos push action eosio setparams '{"params":{"max_block_net_usage": 1048576, "target_block_net_usage_pct": 1000, "max_transaction_net_usage": 524288, "base_per_transaction_net_usage": 12, "net_usage_leeway": 500, "context_free_discount_net_usage_num": 20, "context_free_discount_net_usage_den": 100, "max_block_cpu_usage": 500000, "target_block_cpu_usage_pct": 2500, "max_transaction_cpu_usage": 400000, "min_transaction_cpu_usage": 100, "max_transaction_lifetime": 3600, "deferred_trx_expiration_window": 600, "max_transaction_delay": 3888000, "max_inline_action_size": 4096, "max_inline_action_depth": 6, "max_authority_depth": 6}}' -p eosio

#=================================================================================#
# create the vigorlending account, set the contract

cleos system newaccount eosio vigorlending $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set account permission vigorlending active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"vigorlending","permission":"eosio.code"},"weight":1}],"waits":[]}' -p vigorlending@active

#=================================================================================#
# create the vigortoken11 account, set the contract, create VIGOR stablecoins

BUILDDIR=/home/gg/contracts/vigor/build
cd $BUILDDIR
cmake ..
DESTDIR=. make install

cleos system newaccount eosio vigortoken11 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set contract vigortoken11 "$BUILDDIR/usr/local/contracts/eosio.token" eosio.token.wasm eosio.token.abi -p vigortoken11@active
#cleos set contract vigorlending "$BUILDDIR/usr/local/contracts/vigorlending" vigorlending.wasm vigorlending.abi -p vigorlending@active
cleos set code vigorlending "$BUILDDIR/usr/local/contracts/vigorlending/vigorlending.wasm" -p vigorlending@active
cleos set abi vigorlending "$BUILDDIR/usr/local/contracts/vigorlending/vigorlending.abi" -p vigorlending@active
cleos push action vigortoken11 create '[ "vigorlending", "1000000000.0000 VIGOR"]' -p vigortoken11@active

cleos system newaccount eosio swap.defi $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set contract swap.defi "$BUILDDIR/usr/local/contracts/swap.defi" swap.defi.wasm swap.defi.abi -p swap.defi@active
cleos push action swap.defi config '[]' -p swap.defi@active
cleos get table swap.defi swap.defi pairs


#cleos set code vigorlending "$BUILDDIR/usr/local/contracts/vigorlending/vigorlending.wasm" -j -d -s -x 3600 > tmp
#cleos sign tmp -k 5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12 -p

EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio/eosio.contracts/build/contracts

OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"

Public Key: EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx
Private key: 5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12

cleos --verbose system newaccount eosio account11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11113 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11114 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio finalreserve $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio vigordacfund $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio delphipoller $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set account permission delphipoller active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"delphipoller","permission":"eosio.code"},"weight":1}],"waits":[]}' -p delphipoller@active
cleos --verbose system newaccount eosio vigorrewards $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio genereospool $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer


cleos --verbose system newaccount eosio account11115 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11121 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11122 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio account11123 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos --verbose push action eosio.token transfer '[ "eosio", "account11111", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11112", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11113", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11114", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "finalreserve", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "vigordacfund", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "delphipoller", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "vigorrewards", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "genereospool", "1000000.0000 EOS", "m" ]' -p eosio

cleos --verbose push action eosio.token transfer '[ "eosio", "account11115", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11121", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11122", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "account11123", "1000000.0000 EOS", "m" ]' -p eosio

# create the VIG token
cleos system newaccount eosio vig111111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos set contract vig111111111 $EOSIO_CONTRACTS_ROOT/eosio.token/ -p vig111111111@active
cleos push action vig111111111 create '[ "vig111111111", "1000000000.0000 VIG"]' -p vig111111111@active
cleos push action vig111111111 issue '[ "vig111111111", "1000000000.0000 VIG", "m"]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "vigorlending", "1000000.0000 VIG", "donation" ]' -p vig111111111@active
cleos push action eosio.token transfer '[ "eosio", "vigorlending", "100.0000 EOS", "donation" ]' -p eosio
cleos push action vig111111111 transfer '[ "vig111111111", "account11111", "10000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11112", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11113", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11114", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "finalreserve", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "vigordacfund", "1000000.0000 VIG", "m" ]' -p vig111111111@active

cleos push action vig111111111 transfer '[ "vig111111111", "account11115", "10000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11121", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11122", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "account11123", "1000000.0000 VIG", "m" ]' -p vig111111111@active

# croneos ################################
cleos system newaccount eosio cronbot11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

#cleos system newaccount eosio croncron1111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos set account permission croncron1111 active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"croncron1111","permission":"eosio.code"},"weight":1}],"waits":[]}' -p croncron1111@active

#cleos system newaccount eosio execexecexec $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos set account permission execexecexec active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"croncron1111","permission":"active"},"weight":1}],"waits":[]}' -p execexecexec@active

#CONTRACT_ROOT=/home/gg/contracts/croneos-contracts/croneoscore/src
#CONTRACT_OUT=/home/gg/contracts/croneos-contracts/croneoscore
#CONTRACT_INCLUDE=/home/gg/contracts/croneos-contracts/croneoscore/include
#CONTRACT_INCLUDE_BOOST=/usr/include
#CONTRACT="croneoscore"
#CONTRACT_WASM="$CONTRACT.wasm"
#CONTRACT_ABI="$CONTRACT.abi"
#CONTRACT_CPP="$CONTRACT.cpp"
#eosio-cpp -contract=$CONTRACT -o="$CONTRACT_OUT/$CONTRACT_WASM" -I="$CONTRACT_INCLUDE" -abigen "$CONTRACT_ROOT/$CONTRACT_CPP"

#cleos set contract croncron1111 $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p croncron1111@active
#cleos push action croncron1111 setsettings '{"max_allowed_actions":1,"required_exec_permission":[{"actor":"execexecexec","permission":"active"}],"reward_fee_perc":75,"new_scope_fee":"1.0000 EOS","token_contract":"test"}' -p croncron1111@active
#cleos push action croncron1111 addgastoken '{"gas_token":{"quantity":"0.0001 VIG","contract":"vig111111111"}}' -p croncron1111@active
#cleos push action croncron1111 addgastoken '{"gas_token":{"quantity":"0.0001 EOS","contract":"eosio.token"}}' -p croncron1111@active
#cleos --verbose push action vig111111111 transfer '{"from":"vigorlending","to":"croncron1111","quantity":"1000.0000 VIG","memo":"deposit"}' -p vigorlending@active
#cleos --verbose push action eosio.token transfer '{"from":"vigorlending","to":"croncron1111","quantity":"100.0000 EOS","memo":"deposit"}' -p vigorlending@active

#OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
#OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"
#cleos system newaccount eosio miningpool11 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos set account permission miningpool11 miners '{"threshold":1,"keys":[],"accounts":[{"permission":{"actor":"cronbot11111","permission":"active"},"weight":1}],"waits":[]}' -p miningpool11@active

#=================================================================================#
# create dummy tokens
cleos system newaccount eosio dummytokensx $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set contract dummytokensx $EOSIO_CONTRACTS_ROOT/eosio.token/ -p dummytokensx@active

cleos push action dummytokensx create '[ "dummytokensx", "100000000000.000 IQ"]' -p dummytokensx@active

cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.000 IQ", "m"]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11111", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11112", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11113", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11114", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "finalreserve", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "vigordacfund", "100000.000 IQ", "m" ]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "account11115", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11121", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11122", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11123", "100000.000 IQ", "m" ]' -p dummytokensx@active

cleos push action dummytokensx create '[ "dummytokensx", "100000000000.0000 BOID"]' -p dummytokensx@active

cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.0000 BOID", "m"]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11111", "100000.0000 BOID", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11112", "100000.0000 BOID", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11113", "100000.0000 BOID", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "account11122", "100000.0000 BOID", "m" ]' -p dummytokensx@active

#=================================================================================#
# create the oracle contract for local testnet
cleos system newaccount eosio oracleoracl2 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos system newaccount eosio eostitanprod $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111113 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio vigoraclehub $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set account permission vigoraclehub active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"vigoraclehub","permission":"eosio.code"},"weight":1}],"waits":[]}' -p vigoraclehub@active

BUILDDIR=/home/gg/contracts/vigor/build
cleos set contract oracleoracl2 "$BUILDDIR/usr/local/contracts/oracle" oracle.wasm oracle.abi -p oracleoracl2@active
cleos push action oracleoracl2 configure '{}' -p oracleoracl2@active
#cleos push action oracleoracl2 clear '{"pair":"eosusd"}' -p oracleoracl2@active
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_eosusd_const.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_iqeos_const.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_vigeos_const.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_boideos_const.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_eosvigor_const.js

#cleos get table oracleoracl2 oracleoracl2 pairs
#cleos get table oracleoracl2 eosvigor datapoints
#cleos get table oracleoracl2 vigoreos datapoints
#cleos get table oracleoracl2 eosusd datapoints
#cleos get table oracleoracl2 boideos datapoints
#cleos get table oracleoracl2 vigeos datapoints

BUILDDIR=/home/gg/contracts/vigor/build
cleos set contract vigoraclehub "$BUILDDIR/usr/local/contracts/vigoraclehub" oraclehub.wasm oraclehub.abi -p vigoraclehub@active
#cleos push action datapreproc2 clear '{}' -p datapreproc2@active
#cleos push action vigoraclehub configure '{}' -p vigoraclehub@active
#cleos push action vigoraclehub addpair '{"pair":{"name":"vigoreos","active":"1","base_symbol":"4,VIGOR","base_type":"4","base_contract":"vigortoken11","quote_symbol":"4,EOS","quote_type":"4","quote_contract":"eosio.token","quoted_precision":"6"}}' -p vigorlending@active

cleos push action vigoraclehub addpair '{"pair":{"name":"vigeos","active":"1","base_symbol":"4,VIG","base_type":"4","base_contract":"vig111111111","quote_symbol":"4,EOS","quote_type":"4","quote_contract":"eosio.token","quoted_precision":"8"}}' -p vigoraclehub@active
cleos push action vigoraclehub addpair '{"pair":{"name":"eosusd","active":"1","base_symbol":"4,EOS","base_type":4,"base_contract":"eosio.token","quote_symbol":"2,USD","quote_type":1,"quote_contract":"","quoted_precision":"4"}}' -p vigoraclehub@active
cleos push action vigoraclehub addpair '{"pair":{"name":"iqeos","active":"1","base_symbol":"3,IQ","base_type":"4","base_contract":"everipediaiq","quote_symbol":"4,EOS","quote_type":"4","quote_contract":"eosio.token","quoted_precision":"6"}}' -p vigoraclehub@active
cleos push action vigoraclehub addpair '{"pair":{"name":"boideos","active":"1","base_symbol":"4,BOID","base_type":"4","base_contract":"boidcomtoken","quote_symbol":"4,EOS","quote_type":"4","quote_contract":"eosio.token","quoted_precision":"7"}}' -p vigoraclehub@active
cleos push action vigoraclehub addpair '{"pair":{"name":"eosvigor","active":"1","base_symbol":"4,EOS","base_type":"4","base_contract":"eosio.token","quote_symbol":"4,VIGOR","quote_type":"4","quote_contract":"vigortoken11","quoted_precision":"4"}}' -p vigoraclehub@active

cleos push action vigoraclehub addpaird '{"id":11,"token0":{"contract":"vig111111111","symbol":"4,VIG"},"token1":{"contract":"eosio.token","symbol":"4,EOS"},"precision_scale":8,"name":"vigeos"}' -p vigoraclehub@active
cleos push action vigoraclehub addpaird '{"id":76,"token0":{"contract":"eosio.token","symbol":"4,EOS"},"token1":{"contract":"vigortoken11","symbol":"4,VIGOR"},"precision_scale":4,"name":"eosvigor"}' -p vigoraclehub@active
cleos push action vigoraclehub addpaird '{"id":12,"token0":{"contract":"eosio.token","symbol":"4,EOS"},"token1":{"contract":"tethertether","symbol":"4,USDT"},"precision_scale":4,"name":"eosusd"}' -p vigoraclehub@active

#cleos push action vigoraclehub switchpaird '{"id":11,"active":false}' -p vigoraclehub@active
#cleos push action vigoraclehub delpaird '{"id":11,"reason":"none"}' -p vigoraclehub@active
#cleos push action vigoraclehub switchpaird '{"id":76,"active":false}' -p vigoraclehub@active
#cleos push action vigoraclehub delpaird '{"id":76,"reason":"none"}' -p vigoraclehub@active
#cleos push action vigoraclehub switchpaird '{"id":12,"active":false}' -p vigoraclehub@active
#cleos push action vigoraclehub delpaird '{"id":12,"reason":"none"}' -p vigoraclehub@active

#cleos push action vigoraclehub switchpair '{"name":"vigoreos","active":0}' -p vigorlending@active
#cleos push action vigoraclehub delpair '{"name":"vigoreos","reason":"none"}' -p vigorlending@active

#cleos push action vigoraclehub purge '{}' -p vigoraclehub@active
#cleos get table vigoraclehub vigoraclehub pairs --limit -1
#cleos get table vigoraclehub vigoraclehub pairsd --limit 300
cleos push action vigoraclehub approve '{"oracle":"delphipoller"}' -p vigoraclehub@active
#cleos get table vigoraclehub vigoraclehub oracles --limit -1
#cleos push action vigoraclehub approve '{"oracle":"feeder111111"}' -p vigorlending@active
#cleos get table datapreproc2 eosusd tseries

#cleos push action datapreproc2 addpair '{"newpair":"eosusd"}' -p datapreproc2@active
#cleos push action datapreproc2 addpair '{"newpair":"iqeos"}' -p datapreproc2@active
#cleos push action datapreproc2 addpair '{"newpair":"vigeos"}' -p datapreproc2@active
#cleos push action datapreproc2 addpair '{"newpair":"boideos"}' -p datapreproc2@active
#cleos push action datapreproc2 addpair '{"newpair":"vigorusd"}' -p datapreproc2@active
#cleos --verbose push action datapreproc2 update '{}' -p feeder111111@active
#cleos push action datapreproc2 doshock '{"shockvalue":0.5}' -p feeder111111@active
# cd /home/gg/contracts/vigor/contracts/oracle && CONTRACT=datapreproc2 OWNER=feeder111111 node dataupdate.js
#cleos get table vigoraclehub vigoraclehub pairtoproc --limit -1
#cleos get table datapreproc2 eosusd tseries
#cleos get table datapreproc2 iqeos tseries
#cleos get table datapreproc2 vigeos tseries
#cleos get table datapreproc2 boideos tseries
#cleos get table datapreproc2 vigorusd tseries


#BUILDDIR=/home/gg/contracts/vigor/build
#cleos set contract delphipoller "$BUILDDIR/usr/local/contracts/delphipoller" delphipoller.wasm delphipoller.abi -p delphipoller@active
#cleos --verbose push action delphipoller update '{}' -p delphipoller@active
#cleos get table vigoraclehub eosvigor tseries
#cleos get table vigoraclehub vigoreos tseries
#cleos get table vigoraclehub eosusd tseries
#cleos get table vigoraclehub vigeos tseries
#cleos get table vigoraclehub boideos tseries
#cleos get table vigoraclehub iqeos tseries

cleos push action vigoraclehub approve '{"oracle":"feeder111111"}' -p vigoraclehub@active
cleos push action vigoraclehub approve '{"oracle":"feeder111112"}' -p vigoraclehub@active
cleos push action vigoraclehub approve '{"oracle":"feeder111113"}' -p vigoraclehub@active


cleos set code account11111 "$BUILDDIR/usr/local/contracts/account11111/account11111.wasm" -p account11111@active
cleos set abi account11111 "$BUILDDIR/usr/local/contracts/account11111/account11111.abi" -p account11111@active
cleos set account permission account11111 active --add-code

#cleos get table vigoraclehub eosvigor datapoints
#cleos get table vigoraclehub vigoreos datapoints
#cleos get table vigoraclehub eosusd datapoints
#cleos get table vigoraclehub vigeos datapoints
#cleos get table vigoraclehub boideos datapoints
#cleos get table vigoraclehub iqeos datapoints
#cleos --verbose push action vigoraclehub push '{"owner": "feeder111111","quotes": [{"value":"40431","pair":"eosusd"},{"value":"30000","pair":"vigeos"}]}' -p feeder111111@active
cleos --verbose push action vigoraclehub push '{"owner": "feeder111111","quotes": [{"value":"25540","pair":"eosusd"},{"value":"31900","pair":"vigeos"},{"value":"344","pair":"boideos"},{"value":"273","pair":"iqeos"},{"value":"27788","pair":"eosvigor"}]}' -p feeder111111@active
cleos --verbose push action vigoraclehub push '{"owner": "feeder111112","quotes": [{"value":"25540","pair":"eosusd"},{"value":"32000","pair":"vigeos"},{"value":"345","pair":"boideos"},{"value":"274","pair":"iqeos"},{"value":"27788","pair":"eosvigor"}]}' -p feeder111112@active
cleos --verbose push action vigoraclehub push '{"owner": "feeder111113","quotes": [{"value":"25540","pair":"eosusd"},{"value":"32100","pair":"vigeos"},{"value":"346","pair":"boideos"},{"value":"275","pair":"iqeos"},{"value":"27788","pair":"eosvigor"}]}' -p feeder111113@active


cleos --verbose push action vigoraclehub pushd '{"owner": "feeder111111"}' -p feeder111111@active
cleos --verbose push action vigoraclehub pushd '{"owner": "feeder111112"}' -p feeder111112@active
cleos --verbose push action vigoraclehub pushd '{"owner": "feeder111113"}' -p feeder111113@active

#cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigoraclehub","quantity":"100.0000 VIG","memo":""}' -p account11111@active
cleos get table vigoraclehub vigoraclehub oracles


cleos push action vigorlending whitelist '{"sym":"4,EOS","contract":"eosio.token","feed":"eosusd","assetin":true,"assetout":true,"maxlends":"50"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIGOR","contract":"vigortoken11","feed":"eosvigor","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIG","contract":"vig111111111","feed":"vigeos","assetin":true,"assetout":true,"maxlends":"50"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"3,IQ","contract":"dummytokensx","feed":"iqeos","assetin":true,"assetout":true,"maxlends":"50"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,BOID","contract":"dummytokensx","feed":"boideos","assetin":true,"assetout":true,"maxlends":"50"}' -p vigorlending@active

#eosio::check(from.value == eosio::name("stuffedshirt").value,"contract under maintenance");

#cleos push action vigorlending unwhitelist '{"sym":"4,EOS"}' -p vigorlending@active
#cleos push action vigorlending unwhitelist '{"sym":"4,VIGOR"}' -p vigorlending@active
#cleos push action vigorlending unwhitelist '{"sym":"4,VIG"}' -p vigorlending@active
#cleos push action vigorlending unwhitelist '{"sym":"3,IQ"}' -p vigorlending@active
#cleos push action vigorlending unwhitelist '{"sym":"4,BOID"}' -p vigorlending@active

#cleos --verbose push action vigorlending configure '{"key":"newacctlim","value":"0"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"exectype","value":"2"}' -p vigorlending@active
#cleos --verbose push action vigorlending freezelevel '{"value":"0"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"assetouttime","value":"12"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"assetintime","value":"12"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"minebuffer","value":"30"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"gatekeeper","value":"0"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"dataa","value":"100000"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"freezelevel","value":"0"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"dataa","value":"2000000"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"datab","value":"1000000"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"maxlends","value":"0.2"}' -p vigorlending@active  # not used
cleos --verbose push action vigorlending configure '{"key":"kickseconds","value":"1200000"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"newacctsec","value":"86400"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"batchsize","value":"5"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"rexswitch","value":"1"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"bailoutcr","value":"110"}' -p vigorlending@active
cleos --verbose push action vigorlending configure '{"key":"bailoutupcr","value":"110"}' -p vigorlending@active
#leos --verbose push action vigorlending configure '{"key":"repanniv","value":"120"}' -p vigorlending@active


#cleos --verbose push action vigorlending configure '{"key":"dataa","value":"10000"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"datab","value":"1000"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"accountslim","value":"100"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"liquidate","value":"1"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"liquidateup","value":"1"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"initmaxsize","value":"1000000"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"kickseconds","value":"5184000"}' -p vigorlending@active
#cleos --verbose push action vigorlending configure '{"key":"maxlends","value":"0.35"}' -p vigorlending@active


######################################################################
########################## unit test starts here #####################
######################################################################

# exposed actions for vigor demo starts here
cleos --verbose push action vigorlending openaccount '{"owner":"finalreserve"}' -p finalreserve@active
cleos --verbose push action vigorlending openaccount '{"owner":"vigordacfund"}' -p vigordacfund@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11111"}' -p account11111@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11112"}' -p account11112@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11113"}' -p account11113@active

#cleos --verbose push action vigorlending acctstake '{"owner":"account11114"}' -p account11114@active
#cleos --verbose push action vig111111111 transfer '{"from":"account11114","to":"vigorlending","quantity":"85000.0000 VIG","memo":"stake"}' -p account11114@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11114"}' -p account11114@active
cleos --verbose push action vigortoken11 open '{"owner":"account11114","symbol":"4,VIGOR","ram_payer":"account11114"}' -p account11114@active

#cleos --verbose push action vigorlending assetout '{"usern":"account11114","assetout":"85000.0000 VIG","memo":"stakerefund"}' -p account11114@active
cleos get table vigorlending vigorlending stake

cleos --verbose push action vigorlending openaccount '{"owner":"account11115"}' -p account11115@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11121"}' -p account11121@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11122"}' -p account11122@active
cleos --verbose push action vigorlending openaccount '{"owner":"account11123"}' -p account11123@active

cleos --verbose push action eosio regproxy  '{"proxy":"vigorrewards","isproxy":1}' -p vigorrewards@active
cleos --verbose push action eosio voteproducer  '{"voter":"vigorlending","proxy":"vigorrewards","producers":[]}' -p vigorlending@active

cleos --verbose push action eosio voteproducer  '{"voter":"account11111","proxy":"vigorrewards","producers":[]}' -p account11111@active
cleos --verbose push action eosio deposit '{"owner":"account11111","amount":"50000.0000 EOS"}' -p account11111@active
cleos --verbose push action eosio buyrex  '{"from":"account11111","amount":"50000.0000 EOS"}' -p account11111@active
cleos --verbose push action eosio sellrex  '{"from":"account11111","rex":"10000.0000 REX"}' -p account11111@active
cleos --verbose push action eosio withdraw '{"owner":"account11111","amount":"1.0000 EOS"}' -p account11111@active

cleos --verbose push action eosio deposit '{"owner":"account11112","amount":"1.0000 EOS"}' -p account11112@active
cleos --verbose push action eosio rentcpu '{"from":"account11112","receiver":"account11112","loan_payment":"1.0000 EOS","loan_fund":"0.0000 EOS"}' -p account11112@active
cleos --verbose push action eosio deposit '{"owner":"account11113","amount":"1000.0000 EOS"}' -p account11113@active
cleos --verbose push action eosio rentcpu '{"from":"account11113","receiver":"account11112","loan_payment":"1000.0000 EOS","loan_fund":"0.0000 EOS"}' -p account11113@active

#cd /home/gg/contracts/croneos-miner_local && node my_croneos_miner.js

#cleos --verbose push action eosio.token transfer '[ "vigorlending", "croncron1111", "1.2000 EOS", "deposit gas" ]' -p vigorlending@active
#cleos --verbose push action croncron1111 cancel '{"owner":"vigorlending","id":0, "scope":""}' -p vigorlending@active


#cleos --verbose push action croncron1111 cancel '{"owner":"vigorlending","id":13}' -p vigorlending@active

#cleos --verbose get table croncron1111 croncron1111 cronjobs -l 1000
#cleos --verbose push action croncron1111 exec '{"executer":"vigorlending", "id":5, "scope":"", "oracle_response":""}' -p vigorlending@active
#cleos --verbose push action croncron1111 cancel '{"owner":"vigorlending","id":232,"scope":""}' -p vigorlending@active
#cleos --verbose push action croncron1111 clear '{}' -p croncron1111@active
cleos --verbose push action vigorlending doupdate '{}' -p vigorlending@active
cleos --verbose push action vigorlending tick '{}' -p vigorlending@active
cleos get table croncron1111 vigorlending deposits
cleos get table vigorlending vigorlending croneosqueue -l 1000
cleos get table vigorlending vigorlending croneosstats -l 1000
cleos get table vigorlending vigorlending bailout -l 1000
cleos get table vigorlending vigorlending market -l 1000
cleos get table vigorlending vigorlending whitelist -l 1000
cleos get table vigorlending vigorlending acctsize -l -1

#1	0		100.0001 EOS	994178.6188 REX	9943	[ { "key": "2020-06-01T00:00:00", "value": "9941776245" } ]

cleos get account eosio.rex
cleos get account account11111
cleos get table eosio eosio rexbal
cleos get table eosio eosio rexpool
cleos get table eosio eosio rexfund

cleos --verbose get table vigorlending vigorlending user -Laccount11111 -Uaccount11111 
cleos --verbose get table vigorlending vigorlending user -Laccount11112 -Uaccount11112
cleos --verbose get table vigorlending vigorlending user -Laccount11113 -Uaccount11113
cleos --verbose get table vigorlending vigorlending user -Laccount11114 -Uaccount11114
cleos --verbose get table vigorlending vigorlending user -Laccount11115 -Uaccount11115
cleos --verbose get table vigorlending vigorlending user -Laccount11121 -Uaccount11121
cleos --verbose get table vigorlending vigorlending user -Laccount11122 -Uaccount11122
cleos --verbose get table vigorlending vigorlending user -Laccount11123 -Uaccount11123
cleos --verbose get table vigorlending vigorlending user -Lfinalreserve -Ufinalreserve
cleos --verbose get table vigorlending vigorlending globalstats
cleos --verbose get table vigorlending vigorlending user -Lvigordacfund -Uvigordacfund
cleos --verbose get table vigorlending vigorlending config
cleos --verbose get table vigorlending vigorlending log -l 3000
cleos --verbose get table vigorlending vigorlending bailout -l 3000
cleos --verbose get table vigorlending vigorlending batchse -l 3000
cleos --verbose get table vigorlending vigorlending stat -l -1
cleos --verbose get table vigorlending vigorlending cor -l -1
cleos --verbose get table vigorlending vigorlending whitelist -l 3000
cleos --verbose get table vigorlending vigorlending log --index 5 --key-type i64 -l -1
cleos --verbose get table vigorlending vigorlending user --index 2 --key-type i64
cleos --verbose push action vigorlending kick '{"usern":"account11115","delay_sec":5}' -p vigorlending@active
cleos --verbose push action vigorlending deleteacnt '{"usern":"account11111"}' -p vigorlending@active

cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"49999.7078 VIG","memo":"collateral"}' -p account11111@active
cleos --verbose push action vigorlending setacctsize '{"usern":"account11111","limit":"10000"}' -p vigorlending@active


cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"6.0000 EOS","memo":"insurance"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"3000.000 IQ","memo":"insurance"}' -p account11111@active
cleos --verbose push action vigorlending liquidate '{"usern":"account11114"}' -p account11114@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"0.9292 EOS","memo":"insurance"}' -p account11115@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"1820.834 IQ","memo":"insurance"}' -p account11115@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"1074.0917 VIG","memo":"insurance"}' -p account11115@active
cleos --verbose push action vigorlending liquidateup '{"usern":"account11115"}' -p account11115@active

cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"915.601 IQ","memo":"insurance"}' -p account11111@active

for i in {0..32} 
do
  cleos --verbose push action vigorlending tick '{}' -p vigorlending@active -f
done

cd /home/gg/contracts/tick-worker && node tick-worker.js

VIGOR_PATH=/home/gg/contracts/vigor
TEST_RESULTS_PATH=$VIGOR_PATH/tests/expected

TESTS_BORROW_HOME="$TEST_RESULTS_PATH/borrow"

mkdir -p $TESTS_BORROW_HOME

STEP=0
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

STEP=1
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"5.0000 EOS","memo":"insurance"}' -p finalreserve@active
cleos --verbose push action dummytokensx transfer '{"from":"finalreserve","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p finalreserve@active
cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"50000.0000 VIG","memo":"insurance"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"5.0000 VIG","memo":"insurance"}' -p account11111@active


cleos --verbose push action eosio.token transfer '{"from":"account11111","to":"vigorlending","quantity":"16.0000 EOS","memo":"collateral"}' -p account11111@active
cleos --verbose push action eosio.token transfer '{"from":"account11111","to":"vigorlending","quantity":"16.0001 EOS","memo":"collateral"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"16.0000 EOS","memo":"collateral"}' -p account11111@active
cleos --verbose push action dummytokensx transfer '{"from":"account11111","to":"vigorlending","quantity":"3000.000 IQ","memo":"collateral"}' -p account11111@active
cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"50000.0000 VIG","memo":"collateral"}' -p account11111@active
cleos --verbose push action eosio.token transfer '{"from":"account11111","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"6.0000 EOS","memo":"insurance"}' -p account11111@active
cleos --verbose push action dummytokensx transfer '{"from":"account11111","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11111@active
cleos --verbose push action eosio.token transfer '{"from":"account11112","to":"vigorlending","quantity":"15.0000 EOS","memo":"collateral"}' -p account11112@active
cleos --verbose push action dummytokensx transfer '{"from":"account11112","to":"vigorlending","quantity":"3000.000 IQ","memo":"collateral"}' -p account11112@active
cleos --verbose push action vig111111111 transfer '{"from":"account11112","to":"vigorlending","quantity":"50000.0000 VIG","memo":"collateral"}' -p account11112@active
cleos --verbose push action eosio.token transfer '{"from":"account11113","to":"vigorlending","quantity":"6.0000 EOS","memo":"insurance"}' -p account11113@active
cleos --verbose push action dummytokensx transfer '{"from":"account11113","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11113@active
cleos --verbose push action eosio.token transfer '{"from":"account11114","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p account11114@active
#cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"50000.0000 VIG","memo":"insurance"}' -p account11111@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"0.0001 EOS","memo":"collateral"}' -p account11111@active
#cleos --verbose push action eosio.token transfer '{"from":"account11111","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p account11111@active
#cleos --verbose push action eosio.token transfer '{"from":"account11112","to":"vigorlending","quantity":"0.0001 EOS","memo":"collateral"}' -p account11112@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11112","assetout":"0.0001 EOS","memo":"collateral"}' -p account11112@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11112","assetout":"0.0001 EOS","memo":"collateral"}' -p account11112@active
#cleos --verbose push action eosio.token transfer '{"from":"genereospool","to":"vigorlending","quantity":"0.1000 EOS","memo":"collateral"}' -p genereospool@active

#cleos --verbose push action dummytokensx transfer '{"from":"account11112","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11112@active
#cleos --verbose push action vig111111111 transfer '{"from":"account11112","to":"vigorlending","quantity":"50000.0000 VIG","memo":"insurance"}' -p account11112@active
#cleos --verbose push action eosio.token transfer '{"from":"account11112","to":"vigorlending","quantity":"5.0000 EOS","memo":"insurance"}' -p account11112@active
#cleos --verbose push action vig111111111 transfer '{"from":"finalreserve","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p finalreserve@active
#cleos --verbose push action vigorlending assetout '{"usern":"finalreserve","assetout":"1.0000 VIGOR","memo":"borrow"}' -p finalreserve@active
#borrow VIGOR stablecoin
#cleos --verbose push action vig111111111 transfer '{"from":"finalreserve","to":"vigorlending","quantity":"10000.0000 VIG","memo":"vigfees"}' -p finalreserve@active
#cleos --verbose push action vigortoken11 transfer '{"from":"vigordacfund","to":"vigorlending","quantity":"1.0000 VIGOR","memo":"savings"}' -p vigordacfund@active
#cleos --verbose push action vigortoken11 transfer '{"from":"account11111","to":"vigordacfund","quantity":"1.0000 VIGOR","memo":""}' -p account11111@active
STEP=2
#cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"74.0000 VIGOR","memo":"borrow"}' -p account11111@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#cleos --verbose push action eosio.token transfer '{"from":"vigordacfund","to":"vigorlending","quantity":"16.0000 EOS","memo":"collateral"}' -p vigordacfund@active
#cleos --verbose push action vig111111111 transfer '{"from":"vigordacfund","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p vigordacfund@active
#cleos --verbose push action vigorlending assetout '{"usern":"vigordacfund","assetout":"35.0000 VIGOR","memo":"borrow"}' -p vigordacfund@active
#cleos --verbose push action eosio.token transfer '{"from":"vigordacfund","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p vigordacfund@active

#borrow VIGOR stablecoin
STEP=3
#cleos --verbose push action vig111111111 transfer '{"from":"account11112","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p account11112@active
cleos --verbose push action vigorlending assetout '{"usern":"account11112","assetout":"73.0000 VIGOR","memo":"borrow"}' -p account11112@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#run out of VIG, it automaically borrows some for you
#cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0001 EOS","memo":"insurance"}' -p finalreserve@active
#cleos --verbose push action vigortoken11 transfer '{"from":"account11112","to":"vigorlending","quantity":"40.0000 VIGOR","memo":"payback borrowed token"}' -p account11112@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11112","assetout":"49998.0000 VIG","memo":"collateral"}' -p account11112@active
#cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"10000.0000 VIG","memo":"insurance"}' -p account11111@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11112","assetout":"0.0714 VIG","memo":"collateral"}' -p account11112@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"207.3431 VIG","memo":"vigfees"}' -p account11111@active
#cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"75.0000 VIG","memo":"vigfees"}' -p account11115@active

#payback VIGOR stablecoin
STEP=4
cleos --verbose push action vigortoken11 transfer '{"from":"account11111","to":"vigorlending","quantity":"1.0000 VIGOR","memo":"payback borrowed token"}' -p account11111@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#trade VIGOR stablecoin externally, deposit to savings
STEP=5
cleos --verbose push action vigortoken11 transfer '{"from":"account11111","to":"account11115","quantity":"70.0000 VIGOR","memo":""}' -p account11111@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11112","to":"account11121","quantity":"72.0000 VIGOR","memo":""}' -p account11112@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11111","to":"vigorlending","quantity":"1.0000 VIGOR","memo":"savings"}' -p account11111@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11112","to":"vigorlending","quantity":"1.0000 VIGOR","memo":"savings"}' -p account11112@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#cleos --verbose push action vigortoken11 transfer '{"from":"account11115","to":"vigorlending","quantity":"40.0000 VIGOR","memo":"collateral"}' -p account11115@active
#cleos --verbose push action vigortoken11 transfer '{"from":"account11121","to":"vigorlending","quantity":"40.0000 VIGOR","memo":"collateral"}' -p account11121@active
#cleos --verbose push action vigortoken11 transfer '{"from":"account11115","to":"vigorlending","quantity":"35.0000 VIGOR","memo":"insurance"}' -p account11115@active
#cleos --verbose push action vigortoken11 transfer '{"from":"account11121","to":"vigorlending","quantity":"40.0000 VIGOR","memo":"insurance"}' -p account11121@active
#cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0001 EOS","memo":"collateral"}' -p finalreserve@active


#deposit and withdraw collateral and insurance (including stablecoin)
STEP=6
cleos --verbose push action eosio.token transfer '{"from":"account11115","to":"vigorlending","quantity":"1.0000 EOS","memo":"collateral"}' -p account11115@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11115","to":"vigorlending","quantity":"70.0000 VIGOR","memo":"collateral"}' -p account11115@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11121","to":"vigorlending","quantity":"72.0000 VIGOR","memo":"collateral"}' -p account11121@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"1.0000 VIGOR","memo":"collateral"}' -p account11115@active
cleos --verbose push action vig111111111 transfer '{"from":"account11115","to":"vigorlending","quantity":"1000.0000 VIG","memo":"collateral"}' -p account11115@active
cleos --verbose push action vig111111111 transfer '{"from":"account11121","to":"vigorlending","quantity":"1000.0000 VIG","memo":"collateral"}' -p account11121@active
cleos --verbose push action eosio.token transfer '{"from":"account11122","to":"vigorlending","quantity":"2.0000 EOS","memo":"insurance"}' -p account11122@active
cleos --verbose push action dummytokensx transfer '{"from":"account11122","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11122@active
cleos --verbose push action eosio.token transfer '{"from":"account11115","to":"vigorlending","quantity":"1.0000 EOS","memo":"insurance"}' -p account11115@active
cleos --verbose push action dummytokensx transfer '{"from":"account11115","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11115@active
cleos --verbose push action eosio.token transfer '{"from":"account11123","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p account11123@active
cleos --verbose push action dummytokensx transfer '{"from":"account11123","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11123@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11115","to":"vigorlending","quantity":"1.0000 VIGOR","memo":"insurance"}' -p account11115@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

# borrow cryptos against stablecoin collateral
STEP=7
#cleos --verbose push action vig111111111 transfer '{"from":"account11115","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p account11115@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"14.0000 EOS","memo":"borrow"}' -p account11115@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

# borrow cryptos against stablecoin collateral
STEP=8
cleos push action vigorlending whitelist '{"sym":"3,IQ","contract":"dummytokensx","feed":"iqeos","assetin":true,"assetout":true,"maxlends":"90"}' -p vigorlending@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"6753.000 IQ","memo":"borrow"}' -p account11115@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

# borrow cryptos against stablecoin collateral
STEP=9
cleos push action vigorlending whitelist '{"sym":"4,EOS","contract":"eosio.token","feed":"eosusd","assetin":true,"assetout":true,"maxlends":"95"}' -p vigorlending@active
#cleos --verbose push action vig111111111 transfer '{"from":"account11121","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p account11121@active
cleos --verbose push action vigorlending assetout '{"usern":"account11121","assetout":"16.0000 EOS","memo":"borrow"}' -p account11121@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#payback cryptos
STEP=10
cleos --verbose push action eosio.token transfer '{"from":"account11115","to":"vigorlending","quantity":"2.0000 EOS","memo":"payback borrowed token"}' -p account11115@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#payback cryptos
STEP=11
cleos --verbose push action eosio.token transfer '{"from":"account11121","to":"vigorlending","quantity":"16.0000 EOS","memo":"payback borrowed token"}' -p account11121@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#down market shock and bailout
STEP=12
cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"50000.0000 VIG","memo":"insurance"}' -p account11111@active
cleos --verbose push action vig111111111 transfer '{"from":"account11115","to":"vigorlending","quantity":"10.0000 VIG","memo":"vigfees"}' -p account11115@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"1000.0000 VIG","memo":"borrow"}' -p account11115@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0001 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose push action vigoraclehub doshock '{"shockvalue":0.1}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"eosusd"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"boideos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"iqeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigoreos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatestatsa '{}' -p vigoraclehub@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0001 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#recovery from shock
STEP=13
cleos --verbose push action vigoraclehub doshock '{"shockvalue":1.0}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"eosusd"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"boideos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"iqeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigoreos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatestatsa '{}' -p vigoraclehub@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0002 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#up market shock and bailoutup 
STEP=14
cleos --verbose push action eosio.token transfer '{"from":"account11113","to":"vigorlending","quantity":"1.0000 EOS","memo":"insurance"}' -p account11113@active
cleos --verbose push action vig111111111 transfer '{"from":"account11113","to":"vigorlending","quantity":"10000.0000 VIG","memo":"insurance"}' -p account11113@active
cleos --verbose push action eosio.token transfer '{"from":"account11123","to":"vigorlending","quantity":"1.0000 EOS","memo":"insurance"}' -p account11123@active
cleos --verbose push action vig111111111 transfer '{"from":"account11115","to":"vigorlending","quantity":"100.0000 VIG","memo":"vigfees"}' -p account11115@active
cleos push action vigorlending whitelist '{"sym":"4,VIG","contract":"vig111111111","feed":"vigeos","assetin":true,"assetout":true,"maxlends":"90"}' -p vigorlending@active
cleos --verbose push action vigorlending assetout '{"usern":"account11115","assetout":"5000.0000 VIG","memo":"borrow"}' -p account11115@active
cleos --verbose push action vigoraclehub doshock '{"shockvalue":1.9}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"eosusd"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"boideos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"iqeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigoreos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatestatsa '{}' -p vigoraclehub@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0001 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#recovery from shock
STEP=15
cleos --verbose push action vigoraclehub doshock '{"shockvalue":1.0}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"eosusd"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"boideos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"iqeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigoreos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatestatsa '{}' -p vigoraclehub@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0003 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

#insurers return
STEP=16
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"0.0004 EOS","memo":"collateral"}' -p finalreserve@active
cleos --verbose push action eosio.token transfer '{"from":"account11122","to":"vigorlending","quantity":"2.0000 EOS","memo":"insurance"}' -p account11122@active
cleos --verbose push action dummytokensx transfer '{"from":"account11122","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11122@active
cleos --verbose push action eosio.token transfer '{"from":"account11115","to":"vigorlending","quantity":"1.0000 EOS","memo":"insurance"}' -p account11115@active
cleos --verbose push action dummytokensx transfer '{"from":"account11115","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11115@active
cleos --verbose push action eosio.token transfer '{"from":"account11123","to":"vigorlending","quantity":"12.0000 EOS","memo":"insurance"}' -p account11123@active
cleos --verbose push action dummytokensx transfer '{"from":"account11123","to":"vigorlending","quantity":"3000.000 IQ","memo":"insurance"}' -p account11123@active
cleos --verbose push action eosio.token transfer '{"from":"account11123","to":"vigorlending","quantity":"12.0001 EOS","memo":"insurance"}' -p account11123@active
cleos --verbose push action dummytokensx transfer '{"from":"account11123","to":"vigorlending","quantity":"3000.001 IQ","memo":"insurance"}' -p account11123@active
cleos --verbose get table vigorlending vigorlending user >> $TESTS_BORROW_HOME/step_$STEP.users
cleos --verbose get table vigorlending vigorlending globalstats >> $TESTS_BORROW_HOME/step_$STEP.globals

sed -i 's/.*lastupdate.*/      "lastupdate": "<removed>",/' $TESTS_BORROW_HOME/*





# testing for dust recapup and recap
cleos push action vigorlending whitelist '{"sym":"4,EOS","contract":"eosio.token","feed":"eosusd","assetin":true,"assetout":true,"maxlends":"100"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIGOR","contract":"vigortoken11","feed":"eosvigor","assetin":true,"assetout":true,"maxlends":"100"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIG","contract":"vig111111111","feed":"vigeos","assetin":true,"assetout":true,"maxlends":"100"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"3,IQ","contract":"dummytokensx","feed":"iqeos","assetin":true,"assetout":true,"maxlends":"100"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,BOID","contract":"dummytokensx","feed":"boideos","assetin":true,"assetout":true,"maxlends":"100"}' -p vigorlending@active

cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"5.0000 EOS","memo":"insurance"}' -p finalreserve@active

cleos --verbose push action eosio.token transfer '{"from":"account11111","to":"vigorlending","quantity":"32.0000 EOS","memo":"collateral"}' -p account11111@active
cleos --verbose push action vig111111111 transfer '{"from":"account11111","to":"vigorlending","quantity":"200.0000 VIG","memo":"vigfees"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"0.0005 VIGOR","memo":"borrow"}' -p account11111@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11111","to":"vigorlending","quantity":"0.0005 VIGOR","memo":"collateral"}' -p account11111@active

cleos --verbose push action dummytokensx transfer '{"from":"account11112","to":"vigorlending","quantity":"0.0001 BOID","memo":"insurance"}' -p account11112@active
cleos --verbose push action dummytokensx transfer '{"from":"account11112","to":"vigorlending","quantity":"0.001 IQ","memo":"insurance"}' -p account11112@active

cleos --verbose push action eosio.token transfer '{"from":"account11113","to":"vigorlending","quantity":"500.0000 EOS","memo":"collateral"}' -p account11113@active
cleos --verbose push action eosio.token transfer '{"from":"account11113","to":"vigorlending","quantity":"0.0005 EOS","memo":"insurance"}' -p account11113@active
cleos --verbose push action vig111111111 transfer '{"from":"account11113","to":"vigorlending","quantity":"1000.0000 VIG","memo":"insurance"}' -p account11113@active
cleos --verbose push action vig111111111 transfer '{"from":"account11113","to":"vigorlending","quantity":"1000.0000 VIG","memo":"vigfees"}' -p account11113@active
cleos --verbose push action vigorlending assetout '{"usern":"account11113","assetout":"1000.0000 VIGOR","memo":"borrow"}' -p account11113@active
cleos --verbose push action vigortoken11 transfer '{"from":"account11113","to":"vigorlending","quantity":"1000.0000 VIGOR","memo":"collateral"}' -p account11113@active
cleos --verbose push action vigorlending assetout '{"usern":"account11113","assetout":"0.0001 BOID","memo":"borrow"}' -p account11113@active
cleos --verbose push action vigorlending assetout '{"usern":"account11113","assetout":"0.001 IQ","memo":"borrow"}' -p account11113@active


cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"0.0001 EOS","memo":"borrow"}' -p account11111@active
cleos --verbose push action vigorlending assetout '{"usern":"account11111","assetout":"0.0001 VIG","memo":"borrow"}' -p account11111@active
cleos push action vigorlending whitelist '{"sym":"4,EOS","contract":"eosio.token","feed":"eosusd","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIGOR","contract":"vigortoken11","feed":"eosvigor","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,VIG","contract":"vig111111111","feed":"vigeos","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"3,IQ","contract":"dummytokensx","feed":"iqeos","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active
cleos push action vigorlending whitelist '{"sym":"4,BOID","contract":"dummytokensx","feed":"boideos","assetin":true,"assetout":true,"maxlends":"0"}' -p vigorlending@active

cleos --verbose push action vigoraclehub doshock '{"shockvalue":100.0}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"eosusd"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"boideos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"iqeos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatepricea '{"pair":"vigoreos"}' -p vigoraclehub@active
cleos --verbose push action vigoraclehub updatestatsa '{}' -p vigoraclehub@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigorlending","quantity":"1.0001 EOS","memo":"collateral"}' -p finalreserve@active
# testing for dust recapup and recap



OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"
cleos --verbose system newaccount eosio dacauth11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio daccustodia1 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio dacholding11 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio dacmultisig1 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio dactoken1111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cd ~/DAC/daccontracts/dactoken1111
eosio-cpp -o ./output/mainnet/eosdactokens/eosdactokens.wasm eosdactokens.cpp -I . -abigen -abigen_output output/mainnet/eosdactokens/eosdactokens.abi
cleos --verbose set contract dactoken1111 ~/DAC/daccontracts/dactoken1111/output/mainnet/eosdactokens -p dactoken1111
cd ~/DAC/daccontracts/daccustodia1
eosio-cpp -contract daccustodian -o ./output/mainnet/daccustodian/daccustodian.wasm -I . -abigen ./daccustodian.cpp
cleos --verbose set contract daccustodia1 ~/DAC/daccontracts/daccustodia1/output/mainnet/daccustodian -p daccustodia1
cd ~/DAC/daccontracts/dacmultisig1
eosio-cpp -contract dacmultisig1 -o ./output/mainnet/dacmultisigs/dacmultisigs.wasm -I . -abigen ./dacmultisigs.cpp
cleos --verbose set contract dacmultisig1 ~/DAC/daccontracts/dacmultisig1/output/mainnet/dacmultisigs -p dacmultisig1

cd ~/DAC/constitution
echo '["https://raw.githubusercontent.com/eosusd/constitution/7b6a65bc8982a367698e25c829102da63ce4c15d/constitution.md", "7f4863d19cbe0bac3b32c790a4dcc54c"]' > terms.json
cleos --verbose push action dactoken1111 newmemterms ~/DAC/constitution/terms.json -p dactoken1111
echo '["https://raw.githubusercontent.com/vigorstablecoin/constitution/master/constitution.md", "09ea07f0052e6f2a47f7ff5c086116c1"]' > terms.json
cleos --verbose push action dactoken1111 newmemterms ~/DAC/constitution/terms.json -p dactoken1111

echo "[daccustodia1]" > ~/DAC/token_config.json
cleos --verbose push action dactoken1111 updateconfig ~/DAC/token_config.json -p dactoken1111

cleos --verbose push action dactoken1111 create '["dactoken1111", "1000000000.0000 VIG", 0]' -p dactoken1111
cleos --verbose push action dactoken1111 issue '["dactoken1111", "1000000000.0000 VIG", "Issue"]' -p dactoken1111

echo '[["20000.0000 VIG", 5, 3, 1, "dacauth11111", "dacholding11", "", 0, 0, 0, 2, 2, 2, 60, "10000.0000 VIG"]]' > ~/DAC/dac_config.json

cleos --verbose push action daccustodia1 updateconfig ~/DAC/dac_config.json -p daccustodia1
#cleos --verbose push action daccustodia1 updateconfig ~/DAC/dac_config.json -p dacauth11111@active


cd ~/DAC
echo '{
    "threshold" : 1,
    "keys" : [],
    "accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],
    "waits": []
}' > resign.json

echo '{
    "threshold": 1,
    "keys": [],
    "accounts": [
        {"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1},
        {"permission":{"actor":"daccustodia1", "permission":"eosio.code"}, "weight":1}
    ],
    "waits": []
}' > daccustodian_transfer.json

echo '{
    "threshold": 1,
    "keys": [{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx", "weight":1}],
    "accounts": [
        {"permission":{"actor":"daccustodia1", "permission":"eosio.code"}, "weight":1}
    ],
    "waits": []
}' > dacauthority_owner.json

echo '{
    "threshold": 1,
    "keys": [{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx", "weight":1}],
    "accounts": [
        {"permission":{"actor":"dacauth11111", "permission":"high"}, "weight":1}
    ],
    "waits": []
}' > dacauthority_active.json


cd ~/DAC
# These have to be set now because they are required in daccustodian_transfer.json
# These permissions are set in new period to the custodians with each configured threshold
cleos --verbose set account permission dacauth11111 high EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx active -p dacauth11111@owner
cleos --verbose set account permission dacauth11111 med EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx high -p dacauth11111@owner
cleos --verbose set account permission dacauth11111 low EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx med -p dacauth11111@owner
cleos --verbose set account permission dacauth11111 one EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx low -p dacauth11111@owner

# resign dactokens account to dacauthority@active
cleos --verbose set account permission dactoken1111 active ./resign.json owner -p dactoken1111@owner
cleos --verbose set account permission dactoken1111 owner ./resign.json '' -p dactoken1111@owner

# resign dacmultisigs account to dacauthority@active
cleos --verbose set account permission dacmultisig1 active ./resign.json owner -p dacmultisig1@owner
cleos --verbose set account permission dacmultisig1 owner ./resign.json '' -p dacmultisig1@owner

# resign dacowner account to dacauthority@active, must allow timelocked transfers
# from daccustodian@eosio.code
# daccustodian_transfer.json allows the contract to make transfers with a time delay, or
# dacauthority@med without a time delay.  dacowner must have permission in xfer to transfer tokens
cleos --verbose set account permission dacholding11 xfer ./daccustodian_transfer.json active -p dacholding11@owner
cleos --verbose set action permission dacholding11 eosio.token transfer xfer -p dacholding11@owner
cleos --verbose set action permission dacholding11 vig111111111 transfer xfer -p dacholding11@owner
# Resign eosdacthedac
cleos --verbose set account permission dacholding11 active ./resign.json owner -p dacholding11@owner
cleos --verbose set account permission dacholding11 owner ./resign.json '' -p dacholding11@owner

# Create xfer permission and give it permission to transfer TESTDAC tokens
cleos --verbose set account permission daccustodia1 xfer ./daccustodian_transfer.json active -p daccustodia1@owner
#cleos --verbose set action permission daccustodia1 dactoken1111 transfer xfer -p daccustodia1@owner
cleos --verbose set action permission daccustodia1 vig111111111 transfer xfer -p daccustodia1@active

# Resign daccustodia1
cleos --verbose set account permission daccustodia1 active ./resign.json owner -p daccustodia1@owner
cleos --verbose set account permission daccustodia1 owner ./resign.json '' -p daccustodia1@owner

# Allow high to call any action on daccustodia1
cleos --verbose set action permission dacauth11111 daccustodia1 '' high -p dacauth11111@owner
# These 2 actions require a medium permission
cleos --verbose set action permission dacauth11111 daccustodia1 firecust med -p dacauth11111@owner
cleos --verbose set action permission dacauth11111 daccustodia1 firecand med -p dacauth11111@owner
# Allow one to call the multisig actions
cleos --verbose set action permission dacauth11111 dacmultisig1 '' one -p dacauth11111@owner
# set dacauthority@owner to point to daccustodian@eosio.code
cleos --verbose set account permission dacauth11111 active ./dacauthority_active.json owner -p dacauth11111@owner
# Only run this once you are done making any code changes:
cleos --verbose set account permission dacauth11111 owner ./dacauthority_owner.json '' -p dacauth11111@owner

#cleos --verbose transfer -c dactoken1111 dactoken1111 account11111 "0.0001 VIG" "" -p dactoken1111
#cleos --verbose transfer -c dactoken1111 dactoken1111 account11112 "0.0001 VIG" "" -p dactoken1111
#cleos --verbose transfer -c dactoken1111 dactoken1111 account11113 "0.0001 VIG" "" -p dactoken1111
#cleos --verbose transfer -c dactoken1111 dactoken1111 account11115 "19000.0000 VIG" "" -p dactoken1111
#cleos push action vig111111111 transfer '[ "dacholding11", "account11115", "1.0000 VIG", "m" ]' -p dacholding11@xfer
cleos push action vig111111111 transfer '[ "vig111111111", "dacholding11", "100000.0000 VIG", "m" ]' -p vig111111111@active

cleos --verbose push action dactoken1111 memberreg '["account11111", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11111
cleos --verbose transfer -c vig111111111 account11111 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11111
cleos --verbose push action daccustodia1 nominatecand '["account11111", "9000.0000 VIG"]' -p account11111
cleos --verbose push action daccustodia1 votecust '["account11111",["account11111","account11112", "account11115","account11121"]]' -p account11111

cleos --verbose push action dactoken1111 memberreg '["account11112", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11112
cleos --verbose transfer -c vig111111111 account11112 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11112
cleos --verbose push action daccustodia1 nominatecand '["account11112", "10000.0000 VIG"]' -p account11112
cleos --verbose push action daccustodia1 votecust '["account11112",["account11112"]]' -p account11112
#cleos --verbose push action daccustodia1 resigncust '["account11111"]' -p account11111

cleos --verbose push action dactoken1111 memberreg '["account11113", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11113
cleos --verbose transfer -c vig111111111 account11113 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11113
cleos --verbose push action daccustodia1 nominatecand '["account11113", "10000.0000 VIG"]' -p account11113
cleos --verbose push action daccustodia1 votecust '["account11113",["account11113","account11115"]]' -p account11113

cleos --verbose push action dactoken1111 memberreg '["account11114", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11114
cleos --verbose transfer -c vig111111111 account11114 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11114
cleos --verbose push action daccustodia1 nominatecand '["account11114", "10000.0000 VIG"]' -p account11114
cleos --verbose push action daccustodia1 votecust '["account11114",["account11114"]]' -p account11114




cleos --verbose push action dactoken1111 memberreg '["account11115", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11115
cleos --verbose transfer -c vig111111111 account11115 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11115
cleos --verbose push action daccustodia1 nominatecand '["account11115", "10000.0000 VIG"]' -p account11115
cleos --verbose push action daccustodia1 votecust '["account11115",["account11115"]]' -p account11115

cleos --verbose push action dactoken1111 memberreg '["account11121", "09ea07f0052e6f2a47f7ff5c086116c1"]' -p account11121
cleos --verbose transfer -c vig111111111 account11121 daccustodia1 "20000.0000 VIG" "daccustodia1" -p account11121
cleos --verbose push action daccustodia1 nominatecand '["account11121", "10000.0000 VIG"]' -p account11121
cleos --verbose push action daccustodia1 votecust '["account11121",["account11121"]]' -p account11121

cleos --verbose push action daccustodia1 unstake '["account11115"]' -p account11113
cleos --verbose push action daccustodia1 withdrawcand '["account11115"]' -p account11115


cleos --verbose push action daccustodia1 firecust '["account11121"]' -p dacauth11111@active
cleos --verbose push action daccustodia1 firecand '["account11113", true]' -p dacauth11111@active
cleos --verbose push action daccustodia1 resigncust '["account11115"]' -p account11115@active
cleos --verbose push action daccustodia1 claimpay '[2]' -p account11113@active
cleos --verbose push action daccustodia1 updatereqpay '["account11111", "9000.0000 VIG"]' -p account11111

cleos --verbose push action daccustodia1 newperiod '{"message":"New Period"}' -p account11111

cleos get account daccustodia1
cleos --verbose get table daccustodia1 daccustodia1 config
cleos --verbose get table daccustodia1 daccustodia1 state
cleos --verbose get table daccustodia1 daccustodia1 candidates
cleos --verbose get table daccustodia1 daccustodia1 custodians
cleos --verbose get table daccustodia1 daccustodia1 pendingpay
cleos --verbose get table daccustodia1 daccustodia1 pendingstake
cleos --verbose get table daccustodia1 daccustodia1 votes
cleos get account dacauth11111
cleos get table vig111111111 account11115 accounts 

cd ~/DAC
echo '{
    "threshold": 1,
    "keys": [],
    "accounts": [
        {"permission":{"actor":"daccustodia1", "permission":"eosio.code"}, "weight":1}
    ],
    "waits": []
}' > dacauthority_owner.json

echo '{
    "threshold": 1,
    "keys": [],
    "accounts": [
        {"permission":{"actor":"dacauth11111", "permission":"high"}, "weight":1}
    ],
    "waits": []
}' > dacauthority_active.json

# set dacauthority@owner to point to daccustodian@eosio.code
cleos --verbose set account permission dacauth11111 active ./dacauthority_active.json owner -p dacauth11111@owner
# Only run this once you are done making any code changes:
cleos --verbose set account permission dacauth11111 owner ./dacauthority_owner.json '' -p dacauth11111@owner
cleos get account dacauth11111




cleos push action eosio updateauth '{"account": "vigorlending", "permission": "beta", "parent": "active", "auth": {"threshold": 1,"keys": [{"key":"EOS6VjJiLK95R7BBRdZVbHikM6DgNf33Azbkcb4UhD6NfGgMceM2z", "weight":1}],"accounts": [],"waits": []}}' -p vigorlending@active
cleos push action eosio linkauth '{"account":"vigorlending", "code":"eosio", "type":"setabi","requirement":"beta"}' -p vigorlending@active
cleos push action eosio linkauth '{"account":"vigorlending", "code":"eosio", "type":"setcode","requirement":"beta"}' -p vigorlending@active
cleos push action eosio linkauth '{"account":"vigorlending", "code":"vigorlending", "type":"","requirement":"beta"}' -p vigorlending@active
cleos push action eosio updateauth '{"account": "vigorlending", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1},{"permission":{"actor":"vigorlending", "permission":"eosio.code"}, "weight":1}],"waits": []}}' -p vigorlending@active
cleos push action eosio updateauth '{"account": "vigorlending", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigorlending@owner

cleos push action eosio updateauth '{"account": "vigoraclehub", "permission": "beta", "parent": "active", "auth": {"threshold": 1,"keys": [{"key":"EOS6VjJiLK95R7BBRdZVbHikM6DgNf33Azbkcb4UhD6NfGgMceM2z", "weight":1}],"accounts": [],"waits": []}}' -p vigoraclehub@active
cleos push action eosio linkauth '{"account":"vigoraclehub", "code":"eosio", "type":"setabi","requirement":"beta"}' -p vigoraclehub@active
cleos push action eosio linkauth '{"account":"vigoraclehub", "code":"eosio", "type":"setcode","requirement":"beta"}' -p vigoraclehub@active
cleos push action eosio linkauth '{"account":"vigoraclehub", "code":"vigoraclehub", "type":"","requirement":"beta"}' -p vigoraclehub@active
cleos push action eosio updateauth '{"account": "vigoraclehub", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1},{"permission":{"actor":"vigoraclehub", "permission":"eosio.code"}, "weight":1}],"waits": []}}' -p vigoraclehub@active
cleos push action eosio updateauth '{"account": "vigoraclehub", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigoraclehub@owner

cleos push action eosio updateauth '{"account": "vigortoken11", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigortoken11@active
cleos push action eosio updateauth '{"account": "vigortoken11", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigortoken11@owner

cleos push action eosio updateauth '{"account": "delphipoller", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p delphipoller@active
cleos push action eosio updateauth '{"account": "delphipoller", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p delphipoller@owner

cleos push action eosio updateauth '{"account": "vigordacfund", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigordacfund@active
cleos push action eosio updateauth '{"account": "vigordacfund", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p vigordacfund@owner

cleos push action eosio updateauth '{"account": "finalreserve", "permission": "active", "parent": "owner", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p finalreserve@active
cleos push action eosio updateauth '{"account": "finalreserve", "permission": "owner", "parent": "", "auth": {"threshold": 1,"keys": [],"accounts": [{"permission":{"actor":"dacauth11111", "permission":"active"}, "weight":1}],"waits": []}}' -p finalreserve@owner
