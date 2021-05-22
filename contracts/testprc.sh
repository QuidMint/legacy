#!/bin/bash
# Usage: ./test.sh
#=================================================================================#
# SETUP
#
# pkill nodeos keosd
#rm -rf ~/.local/share/eosio/nodeos/data
# rm -rf ~/Library/Application\ Support/eosio/nodeos/data
#nodeos -e -p eosio --http-validate-host=false --delete-all-blocks --plugin eosio::chain_api_plugin --contracts-console --plugin eosio::http_plugin --plugin eosio::history_api_plugin --verbose-http-errors --max-transaction-time=10000
# nodeos -e -p eosio --plugin eosio::producer_plugin --plugin eosio::producer_api_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::state_history_plugin --max-transaction-time=10000 --access-control-allow-origin='*' --contracts-console --http-validate-host=false --trace-history --chain-state-history --verbose-http-errors --filter-on='*' --disable-replay-opts >> nodeos.log 2>&1 &

# Wait for nodeos to be ready to accept transactions
sleep 3

CYAN='\033[1;36m'
NC='\033[0m'

# CHANGE PATH
EOSIO_CONTRACTS_ROOT="/Applications/eosio/eosio.contracts-1.8.3/build/contracts"
CONTRACT_ROOT="/Users/pedro/Desktop/vigor/vigor/contracts"

# BOOST path
BOOST_DIR="/opt/boost/1.65.1/"

OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"

#cleos wallet create --to-console
cleos wallet unlock -n default --password PW5JG4U3JJaQ1F3KLVXVsyonQTvb2N8iXjgU5EmPbPWssk4ai6g2k
#cleos wallet import -n default --private-key $OWNER_ACCT

#=================================================================================#
# BOOTSTRAP EOS CRAP

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
# Preactivate feature (needed for eosio 1.8.x)
curl -X POST http://127.0.0.1:8888/v1/producer/schedule_protocol_feature_activations -d '{"protocol_features_to_activate": ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}' | jq
sleep 3
echo -e "      PREACTIVATE FEATURE ACTIVATED"
cleos set contract eosio $EOSIO_CONTRACTS_ROOT/eosio.bios/
echo -e "      BIOS SET"
cleos set contract eosio $EOSIO_CONTRACTS_ROOT/eosio.system/
echo -e "      SYSTEM SET"
cleos push action eosio setpriv '["eosio.msig", 1]' -p eosio@active
cleos push action eosio init '["0", "4,EOS"]' -p eosio@active
#cleos push action eosio init '["0", "4,SYS"]' -p eosio@active

# Deploy eosio.wrap
echo -e "${CYAN}-----------------------EOSIO WRAP-----------------------${NC}"
cleos wallet import -n default --private-key 5J3JRDhf4JNhzzjEZAsQEgtVuqvsPPdZv4Tm6SjMRx1ZqToaray
cleos system newaccount eosio eosio.wrap EOS7LpGN1Qz5AbCJmsHzhG7sWEGd9mwhTXWmrYXqxhTknY2fvHQ1A --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 5000 --transfer
cleos push action eosio setpriv '["eosio.wrap", 1]' -p eosio@active
cleos set contract eosio.wrap $EOSIO_CONTRACTS_ROOT/eosio.wrap/

#=================================================================================#
# create the vigor1111111 account, set the contract, create VIGOR stablecoins

cleos system newaccount eosio vigor1111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set account permission vigor1111111 active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"vigor1111111","permission":"eosio.code"},"weight":1}],"waits":[]}' -p vigor1111111@active
CONTRACT_SRC=$CONTRACT_ROOT/vigor/src
CONTRACT_OUT=$CONTRACT_ROOT/vigor
CONTRACT_INCLUDE=$CONTRACT_ROOT/vigor/include
CONTRACT_INCLUDE_BOOST=$BOOST_DIR/include
CONTRACT="vigor"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"
eosio-cpp -contract=$CONTRACT -o="$CONTRACT_OUT/$CONTRACT_WASM" -I="$CONTRACT_INCLUDE" -I="$CONTRACT_INCLUDE_BOOST" -abigen "$CONTRACT_SRC/$CONTRACT_CPP"

cleos set contract vigor1111111 $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p vigor1111111@active
cleos push action vigor1111111 create '[ "vigor1111111", "1000000000.0000 VIGOR"]' -p vigor1111111@active
cleos push action vigor1111111 setsupply '[ "vigor1111111", "1000000000.0000 VIGOR"]' -p vigor1111111@active

#cd ~/contracts/eosio.cdt/examples/hello/src
#eosio-cpp -contract=hello -o=hello.wasm -I=../include -abigen hello.cpp
#cleos set contract vigor1111111 . hello.wasm hello.abi -p vigor1111111@active
#cleos push action vigor1111111 hi '["name","asdf"]' -p vigor1111111@active
#=================================================================================#

cleos --verbose system newaccount eosio testbrw11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testbrw11112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testins11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testins11112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio finalreserve $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio reinvestment $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos --verbose system newaccount eosio testbrw21111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testbrw21112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testins21111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos --verbose system newaccount eosio testins21112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos --verbose push action eosio.token transfer '[ "eosio", "testbrw11111", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testbrw11112", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testins11111", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testins11112", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "finalreserve", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "reinvestment", "1000000.0000 EOS", "m" ]' -p eosio

cleos --verbose push action eosio.token transfer '[ "eosio", "testbrw21111", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testbrw21112", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testins21111", "1000000.0000 EOS", "m" ]' -p eosio
cleos --verbose push action eosio.token transfer '[ "eosio", "testins21112", "1000000.0000 EOS", "m" ]' -p eosio

#=================================================================================#
# create the VIG token
cleos system newaccount eosio vig111111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos set contract vig111111111 $EOSIO_CONTRACTS_ROOT/eosio.token/ -p vig111111111@active
cleos push action vig111111111 create '[ "vig111111111", "100000000000.0000 VIG"]' -p vig111111111@active
cleos push action vig111111111 issue '[ "vig111111111", "10000000000.0000 VIG", "m"]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testbrw11111", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testbrw11112", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testins11111", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testins11112", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "finalreserve", "1000000.0000 VIG", "m" ]' -p vig111111111@active

cleos push action vig111111111 transfer '[ "vig111111111", "testbrw21111", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testbrw21112", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testins21111", "1000000.0000 VIG", "m" ]' -p vig111111111@active
cleos push action vig111111111 transfer '[ "vig111111111", "testins21112", "1000000.0000 VIG", "m" ]' -p vig111111111@active

#=================================================================================#
# create dummy tokens
cleos system newaccount eosio dummytokensx $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set contract dummytokensx $EOSIO_CONTRACTS_ROOT/eosio.token/ -p dummytokensx@active

cleos push action dummytokensx create '[ "dummytokensx", "100000000000.000 IQ"]' -p dummytokensx@active

cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.000 IQ", "m"]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11111", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11112", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testins11111", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testins11112", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "finalreserve", "100000.000 IQ", "m" ]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw21111", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw21112", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testins21111", "100000.000 IQ", "m" ]' -p dummytokensx@active
cleos push action dummytokensx transfer '[ "dummytokensx", "testins21112", "100000.000 IQ", "m" ]' -p dummytokensx@active

#=================================================================================#
# create the oracle contract for local testnet
cleos system newaccount eosio oracleoracle $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos system newaccount eosio eostitanprod $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111113 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio datapreprocx $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

ORACLE_SRC=$CONTRACT_ROOT/oracle/src
ORACLE_OUT=$CONTRACT_ROOT/oracle
ORACLE_INCLUDE=$CONTRACT_ROOT/oracle/include
ORACLE="oracle"
ORACLE_WASM="$ORACLE.wasm"
ORACLE_ABI="$ORACLE.abi"
ORACLE_CPP="$ORACLE.cpp"
#eosio-cpp -contract=$ORACLE -I=$ORACLE_INCLUDE -I=$EOSIO_CONTRACTS_ROOT/eosio.system/include -o="$ORACLE_OUT/$ORACLE_WASM" -abigen "$ORACLE_SRC/$ORACLE_CPP"
cleos set contract oracleoracle $ORACLE_OUT $ORACLE_WASM $ORACLE_ABI -p oracleoracle@active
cleos push action oracleoracle configure '{}' -p oracleoracle@active
cd $ORACLE_OUT && ORACLE=feeder111111 node updater_eosusd.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111112 node updater_eosusd.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111113 node updater_eosusd.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111111 node updater_iqeos.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111112 node updater_iqeos.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111113 node updater_iqeos.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111111 node updater_vigeos.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111112 node updater_vigeos.js >> oracles.log 2>&1 &
cd $ORACLE_OUT && ORACLE=feeder111113 node updater_vigeos.js >> oracles.log 2>&1 &

sleep 10 #let the oracles do their thing...

#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"20000","pair":"eosusd", "base": {"sym": "4,EOS", "con": "eosio.token"}}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"10000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111112","quotes": [{"value":"20000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111112@active
#cleos push action oracleoracle write '{"owner": "feeder111113","quotes": [{"value":"30000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111113@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"70000","pair":"eosusd"},{"value":"70000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"60000","pair":"eosusd"},{"value":"60000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"50000","pair":"eosusd"},{"value":"50000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"40000","pair":"eosusd"},{"value":"40000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"30000","pair":"eosusd"},{"value":"30000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"20000","pair":"eosusd"},{"value":"20000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle write '{"owner": "feeder111111","quotes": [{"value":"10000","pair":"eosusd"},{"value":"10000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracle clear '{"pair":"eosusd"}' -p oracleoracle@active
#cleos push action oracleoracle clear '{"pair":"eosbtc"}' -p oracleoracle@active

cleos get table oracleoracle eosusd datapoints --limit -1
cleos get table oracleoracle iqeos datapoints --limit -1
cleos get table oracleoracle vigeos datapoints --limit -1
cleos get table oracleoracle oracleoracle stats
cleos get table oracleoracle oracleoracle pairs

CONTRACT_SRC=$CONTRACT_ROOT/datapreproc/src
CONTRACT_OUT=$CONTRACT_ROOT/datapreproc
CONTRACT_INCLUDE=$CONTRACT_ROOT/datapreproc/include
CONTRACT="datapreproc"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"
#eosio-cpp -contract=$CONTRACT -I=$CONTRACT_INCLUDE -I=$EOSIO_CONTRACTS_ROOT/eosio.system/include -o="$CONTRACT_OUT/$CONTRACT_WASM" -abigen "$CONTRACT_SRC/$CONTRACT_CPP"
cleos set contract datapreprocx $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p datapreprocx@active
#cleos push action datapreprocx clear '{}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"eosusd"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"iqeos"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"vigeos"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"vigorusd"}' -p datapreprocx@active
#cleos push action datapreprocx update '{}' -p feeder111111@active
#cleos push action datapreprocx doshock '{"shockvalue":0.5}' -p feeder111111@active

cd $ORACLE_OUT && CONTRACT=datapreprocx OWNER=feeder111111 node dataupdate.js >> dataupdate.log 2>&1 &

cleos get table datapreprocx datapreprocx pairtoproc --limit -1
cleos get table datapreprocx eosusd tseries
cleos get table datapreprocx iqeos tseries
cleos get table datapreprocx vigeos tseries
cleos get table datapreprocx vigorusd tseries


#cleos -u http://api.eosnewyork.io:80 get code delphioracle --wasm -c delphioracle.wasm
#cleos -u http://api.eosnewyork.io:80 get code delphioracle --abi delphioracle.abi

#cleos set contract oracleoracle . delphioracle.wasm delphioracle.abi -p oracleoracle@active

#cleos push action oracleoracle setoracles '{"oracleslist":["feeder111111"]}' -p eostitanprod@active

#=================================================================================#

###### OPTIONAL FOR LOCAL TESTNET #############
# cd ~/contracts/delphioracle/scripts
# nodeosurl='http://127.0.0.1:8888' interval=15000 account="oracleoracle" defaultPrivateKey="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12" feeder="feeder111111" node updater2.js
#cleos get table oracleoracle oracleoracle eosusd --limit 1
#cleos get table oracleoracle oracleoracle oracles
#cleos get table oracleoracle oracleoracle eosusdstats

# launch two oracle feeders
#in a new shell
#=================================================================================#
# exposed actions for vigor demo starts here

cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigor1111111","quantity":"6.0000 EOS","memo":"insurance"}' -p finalreserve@active
cleos --verbose push action dummytokensx transfer '{"from":"finalreserve","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p finalreserve@active
cleos --verbose push action vig111111111 transfer '{"from":"finalreserve","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"insurance"}' -p finalreserve@active

cleos --verbose push action eosio.token transfer '{"from":"reinvestment","to":"vigor1111111","quantity":"1.0000 EOS","memo":"insurance"}' -p reinvestment@active

cleos --verbose push action eosio.token transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"15.0000 EOS","memo":"collateral"}' -p testbrw11111@active
cleos --verbose push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"collateral"}' -p testbrw11111@active
cleos --verbose push action vig111111111 transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"collateral"}' -p testbrw11111@active

cleos --verbose push action eosio.token transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"6.0000 EOS","memo":"collateral"}' -p testbrw11112@active
cleos --verbose push action dummytokensx transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.000 IQ","memo":"collateral"}' -p testbrw11112@active
cleos --verbose push action vig111111111 transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"collateral"}' -p testbrw11112@active

cleos --verbose push action eosio.token transfer '{"from":"testins11111","to":"vigor1111111","quantity":"6.0000 EOS","memo":"insurance"}' -p testins11111@active
cleos --verbose push action dummytokensx transfer '{"from":"testins11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins11111@active
cleos --verbose push action eosio.token transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"6.0000 EOS","memo":"insurance"}' -p testbrw11111@active
cleos --verbose push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testbrw11111@active
cleos --verbose push action eosio.token transfer '{"from":"testins11112","to":"vigor1111111","quantity":"12.0000 EOS","memo":"insurance"}' -p testins11112@active
cleos --verbose push action dummytokensx transfer '{"from":"testins11112","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins11112@active

cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw11111","assetout":"42.0001 VIGOR","memo":"borrow"}' -p testbrw11111@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw11112","assetout":"21.0000 VIGOR","memo":"borrow"}' -p testbrw11112@active

#cleos --verbose push action vigor1111111 assetout '{"usern":"finalreserve","assetout":"1.0000 EOS","memo":"insurance"}' -p finalreserve@active
#cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw11111","assetout":"1.0000 EOS","memo":"collateral"}' -p testbrw11111@active
#cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw11112","assetout":"1.0000 EOS","memo":"collateral"}' -p testbrw11112@active
#cleos --verbose push action vigor1111111 assetout '{"usern":"testins11111","assetout":"1.0000 EOS","memo":"insurance"}' -p testins11111@active
#cleos --verbose push action vigor1111111 assetout '{"usern":"testins11112","assetout":"1.0000 EOS","memo":"insurance"}' -p testins11112@active

cleos --verbose push action vigor1111111 transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"0.0001 VIGOR","memo":"payoff debt"}' -p testbrw11111@active
#cleos --verbose push action vigor1111111 transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"10.0000 VIGOR","memo":"payoff debt"}' -p testbrw11112@active

#cleos --verbose push action vigor1111111 doupdate '{}' -p vigor1111111@active
#sleep 5
#cleos --verbose push action vigor1111111 doupdate '' -p vigor1111111@active
#sleep 5
#cleos --verbose get table vigor1111111 vigor1111111 user

# get all the user data
cleos --verbose get table vigor1111111 vigor1111111 user
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestbrw11111 -Utestbrw11111
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestbrw11112 -Utestbrw11112
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestins11111 -Utestins11111
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestins11112 -Utestins11112
cleos --verbose get table vigor1111111 vigor1111111 user -Lfinalreserve -Ufinalreserve
cleos --verbose get table vigor1111111 vigor1111111 globals

cleos --verbose push action datapreprocx doshock '{"shockvalue":0.1}' -p feeder111111@active
cleos --verbose push action eosio.token transfer '{"from":"finalreserve","to":"vigor1111111","quantity":"100.0000 EOS","memo":"insurance"}' -p finalreserve@active

# cleos --verbose get table vigor1111111 VIGOR stat
# cleos --verbose get table eosio.token vigor1111111 accounts
# cleos --verbose get table eosio.token testbrw11111 accounts
# cleos --verbose get table vigor1111111 testbrw11111 accounts
# cleos --verbose get currency balance vigor1111111 testbrw11111
# cleos --verbose get currency balance eosio.token vigor1111111

#cleos --verbose -u https://api.kylin.alohaeos.com get table delphioracle delphioracle eosusd
#cleos --verbose -u https://api.kylin.alohaeos.com get table eostitantest eostitantest eosusd
#cleos --verbose -u https://api.kylin.alohaeos.com get table eostitantest eosusd datapoints
#cleos --verbose -u https://api.kylin.alohaeos.com get table eostitantest eosbtc datapoints
#cleos --verbose -u https://api.kylin.alohaeos.com get table eostitantest eostitantest stats
#cleos --verbose -u https://api.kylin.alohaeos.com get table eostitantest eostitantest pairs
#cleos --verbose -u https://api.kylin.alohaeos.com get table eosio eosio producers

#=================================================================================#
#lender.... borrow EOS native tokens against stablecoin collateral



cleos --verbose get table vigor1111111 testbrw11111 accounts
cleos --verbose get table vigor1111111 testbrw21111 accounts
cleos --verbose get table vigor1111111 testbrw21112 accounts
cleos --verbose get table vigor1111111 vigor1111111 accounts

cleos --verbose push action vigor1111111 transfer '{"from":"testbrw11111","to":"testbrw21111","quantity":"37.0000 VIGOR","memo":""}' -p testbrw11111@active
cleos --verbose push action vigor1111111 transfer '{"from":"testbrw11111","to":"testbrw21112","quantity":"5.0000 VIGOR","memo":""}' -p testbrw11111@active

#deposit and withdraw stablecoin collateral
cleos --verbose push action vigor1111111 transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"37.0000 VIGOR","memo":"collateral"}' -p testbrw21111@active
cleos --verbose push action vigor1111111 transfer '{"from":"testbrw21112","to":"vigor1111111","quantity":"5.0000 VIGOR","memo":"collateral"}' -p testbrw21112@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21111","assetout":"1.0000 VIGOR","memo":"collateral"}' -p testbrw21111@active

#deposit and withdraw to the insurance pool
cleos --verbose push action eosio.token transfer '{"from":"testins21111","to":"vigor1111111","quantity":"2.0000 EOS","memo":"insurance"}' -p testins21111@active
cleos --verbose push action dummytokensx transfer '{"from":"testins21111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins21111@active
cleos --verbose push action eosio.token transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"1.0000 EOS","memo":"insurance"}' -p testbrw21111@active
cleos --verbose push action dummytokensx transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testbrw21111@active
cleos --verbose push action eosio.token transfer '{"from":"testins21112","to":"vigor1111111","quantity":"12.0000 EOS","memo":"insurance"}' -p testins21112@active
cleos --verbose push action dummytokensx transfer '{"from":"testins21112","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins21112@active
cleos --verbose push action vigor1111111 transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"1.0000 VIGOR","memo":"insurance"}' -p testbrw21111@active

cleos --verbose push action vigor1111111 assetout '{"usern":"testins21111","assetout":"1.0000 EOS","memo":"insurance"}' -p testins21111@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testins21112","assetout":"1.0000 EOS","memo":"insurance"}' -p testins21112@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21111","assetout":"0.0001 VIGOR","memo":"insurance"}' -p testbrw21111@active

# borrow cryptos against stablecoin collateral
# locates crypto in the insurance pool, locating accross one insurer at a time until fully located
# transfers the borrowed crypto out of the contract to the borrower external account
# books the borrowed asset such as 0.5 EOS into the borrowers l_collateral vector
# moves VIGOR to the reinvestment user as insurance to start earning VIG based on pcts, the earbed VIG is paid to (and bailout recapped by) all insurance pool participants according to l_pcts
# record entry into lender l_lrtoken, l_lrpayment, l_lrname vectors: amount, asset, strike, borrower name. ex: 0.5000 EOS @ 2.0000 VIGOR borrowerA

cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21111","assetout":"2.0000 EOS","memo":"borrow"}' -p testbrw21111@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21111","assetout":"6.0000 EOS","memo":"borrow"}' -p testbrw21111@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21111","assetout":"100.000 IQ","memo":"borrow"}' -p testbrw21111@active
cleos --verbose push action vigor1111111 assetout '{"usern":"testbrw21112","assetout":"1.0000 EOS","memo":"borrow"}' -p testbrw21112@active

#payback cryptos
# when borrowerA paysback, then loop through lenders to look for and payoff the outstanding records
cleos --verbose push action eosio.token transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"3.0000 EOS","memo":"payback borrowed token"}' -p testbrw21111@active
cleos --verbose push action eosio.token transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"3.0000 EOS","memo":"payback borrowed token"}' -p testbrw21111@active
cleos --verbose push action eosio.token transfer '{"from":"testbrw21111","to":"vigor1111111","quantity":"1.5000 EOS","memo":"payback borrowed token"}' -p testbrw21111@active
cleos --verbose push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"payback borrowed token"}' -p testbrw11111@active
cleos --verbose push action vig111111111 transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"payback borrowed token"}' -p testbrw11111@active


# if insurer wants to leave then locate crypto in the insurance pool, proportionally from each lender and move lending receipts records to other lenders, finalreserve is last in line


#reserve should not pay VIG for loan insurance because nobody backs the reserve except itself, reserve needs to have a self bailout function which basically re-ups the reserve collateral



# get all the user data
cleos --verbose get table vigor1111111 vigor1111111 user
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestbrw21111 -Utestbrw21111
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestbrw21112 -Utestbrw21112
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestins21111 -Utestins21111
cleos --verbose get table vigor1111111 vigor1111111 user -Ltestins21112 -Utestins21112
cleos --verbose get table vigor1111111 vigor1111111 user -Lfinalreserve -Ufinalreserve
cleos --verbose get table vigor1111111 vigor1111111 user -Lreinvestment -Ureinvestment
cleos --verbose get table vigor1111111 vigor1111111 globals

# Shutdown
pkill '^node$' # stop the oracles

exit 0


VIG airdrop 5% to eosDAC community. VIGOR thanks eosDAC !

VIG. a token. a utility for community supported financial dapps

VIGOR. a decentralized finance project enabled by eosDAC

VIGOR. stablecoin. a borrow & earn community. go long, go short, hedge. bull, bear, banker.

don't get it? visit vigor.ai

vote for eosdacserve
