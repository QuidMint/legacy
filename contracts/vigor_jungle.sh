#!/bin/bash
# Usage: ./test.sh
#=================================================================================#
# SETUP
# 
pkill nodeos
rm -rf ~/.local/share/eosio/nodeos/data
nodeos -e -p eosio --http-validate-host=false --delete-all-blocks --plugin eosio::chain_api_plugin --contracts-console --plugin eosio::http_plugin --plugin eosio::history_api_plugin --verbose-http-errors --max-transaction-time=10000
#nodeos -e -p eosio --plugin eosio::producer_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::state_history_plugin --access-control-allow-origin='*' --contracts-console --http-validate-host=false --trace-history --chain-state-history --verbose-http-errors --filter-on='*' --disable-replay-opts >> nodeos.log 2>&1 &


CYAN='\033[1;36m'
NC='\033[0m'

# CHANGE PATH
EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio.contracts/contracts
CONTRACT_ROOT=/home/gg/contracts/vigor/contracts


OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"

#cleos wallet create --to-console
cleos wallet unlock -n default --password PW5HzuR3R2g77WZCsqaSMD91aC3WPFQzmeHkyzyEREbPTB3EmHeMC
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
CONTRACT_ROOT=/home/gg/contracts/vigor/contracts/vigor/src
CONTRACT_OUT=/home/gg/contracts/vigor/contracts/vigor
CONTRACT_INCLUDE=/home/gg/contracts/vigor/contracts/vigor/include
CONTRACT="vigor"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"
eosio-cpp -contract=$CONTRACT -o="$CONTRACT_OUT/$CONTRACT_WASM" -I="$CONTRACT_INCLUDE" -abigen "$CONTRACT_ROOT/$CONTRACT_CPP"
cleos set contract vigor1111111 $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p vigor1111111@active
cleos push action vigor1111111 create '[ "vigor1111111", "1000000000.0000 VIGOR"]' -p vigor1111111@active
cleos push action vigor1111111 setsupply '[ "vigor1111111", "1000000000.0000 VIGOR"]' -p vigor1111111@active

#cd ~/contracts/eosio.cdt/examples/hello/src
#eosio-cpp -contract=hello -o=hello.wasm -I=../include -abigen hello.cpp
#cleos set contract vigor1111111 . hello.wasm hello.abi -p vigor1111111@active
#cleos push action vigor1111111 hi '["name","asdf"]' -p vigor1111111@active
#=================================================================================#

cleos system newaccount eosio testbrw11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio testbrw11112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio testins11111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio testins11112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

cleos push action eosio.token transfer '[ "eosio", "testbrw11111", "1000000.0000 EOS", "m" ]' -p eosio
cleos push action eosio.token transfer '[ "eosio", "testbrw11112", "1000000.0000 EOS", "m" ]' -p eosio
cleos push action eosio.token transfer '[ "eosio", "testins11111", "1000000.0000 EOS", "m" ]' -p eosio
cleos push action eosio.token transfer '[ "eosio", "testins11112", "1000000.0000 EOS", "m" ]' -p eosio

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

#=================================================================================#
# create dummy tokens
cleos system newaccount eosio dummytokensx $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set contract dummytokensx $EOSIO_CONTRACTS_ROOT/eosio.token/ -p dummytokensx@active

cleos push action dummytokensx create '[ "dummytokensx", "100000000000.000 IQ"]' -p dummytokensx@active
#cleos push action dummytokensx create '[ "dummytokensx", "100000000000.0000 UTG"]' -p dummytokensx@active
#cleos push action dummytokensx create '[ "dummytokensx", "100000000000.0000 PTI"]' -p dummytokensx@active
#cleos push action dummytokensx create '[ "dummytokensx", "100000000000.0000 OWN"]' -p dummytokensx@active

cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.000 IQ", "m"]' -p dummytokensx@active
#cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.0000 UTG", "m"]' -p dummytokensx@active
#cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.0000 PTI", "m"]' -p dummytokensx@active
#cleos push action dummytokensx issue '[ "dummytokensx", "10000000000.0000 OWN", "m"]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11111", "100000.000 IQ", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11111", "100000.0000 UTG", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11111", "100000.0000 PTI", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11111", "100000.0000 OWN", "m" ]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11112", "100000.000 IQ", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11112", "100000.0000 UTG", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11112", "100000.0000 PTI", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testbrw11112", "100000.0000 OWN", "m" ]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "testins11111", "100000.000 IQ", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11111", "100000.0000 UTG", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11111", "100000.0000 PTI", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11111", "100000.0000 OWN", "m" ]' -p dummytokensx@active

cleos push action dummytokensx transfer '[ "dummytokensx", "testins11112", "100000.000 IQ", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11112", "100000.0000 UTG", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11112", "100000.0000 PTI", "m" ]' -p dummytokensx@active
#cleos push action dummytokensx transfer '[ "dummytokensx", "testins11112", "100000.0000 OWN", "m" ]' -p dummytokensx@active

#=================================================================================#
# create the oracle contract for local testnet
cleos system newaccount eosio oracleoracl2 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos system newaccount eosio eostitanprod $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111113 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio datapreproc2 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

ORACLE_ROOT=/home/gg/contracts/vigor/contracts/oracle/src
ORACLE_OUT=/home/gg/contracts/vigor/contracts/oracle
ORACLE_INCLUDE=/home/gg/contracts/vigor/contracts/oracle/include
ORACLE="oracle"
ORACLE_WASM="$ORACLE.wasm"
ORACLE_ABI="$ORACLE.abi"
ORACLE_CPP="$ORACLE.cpp"
EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio.contracts/contracts
eosio-cpp -contract=$ORACLE -I=$ORACLE_INCLUDE -I=$EOSIO_CONTRACTS_ROOT/eosio.system/include -o="$ORACLE_OUT/$ORACLE_WASM" -abigen "$ORACLE_ROOT/$ORACLE_CPP" 
cleos set contract oracleoracl2 $ORACLE_OUT $ORACLE_WASM $ORACLE_ABI -p oracleoracl2@active
cleos push action oracleoracl2 configure '{}' -p oracleoracl2@active
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_eosusd.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_eosusd.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_eosusd.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_iqeos.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_iqeos.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_iqeos.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_vigeos.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_vigeos.js
cd /home/gg/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_vigeos.js

#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"20000","pair":"eosusd", "base": {"sym": "4,EOS", "con": "eosio.token"}}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"10000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111112","quotes": [{"value":"20000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111112@active
#cleos push action oracleoracl2 write '{"owner": "feeder111113","quotes": [{"value":"30000","pair":"eosusd"},{"value":"80000","pair":"eosbtc"}]}' -p feeder111113@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"70000","pair":"eosusd"},{"value":"70000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"60000","pair":"eosusd"},{"value":"60000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"50000","pair":"eosusd"},{"value":"50000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"40000","pair":"eosusd"},{"value":"40000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"30000","pair":"eosusd"},{"value":"30000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"20000","pair":"eosusd"},{"value":"20000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 write '{"owner": "feeder111111","quotes": [{"value":"10000","pair":"eosusd"},{"value":"10000","pair":"eosbtc"}]}' -p feeder111111@active
#cleos push action oracleoracl2 clear '{"pair":"eosusd"}' -p oracleoracl2@active
#cleos push action oracleoracl2 clear '{"pair":"eosbtc"}' -p oracleoracl2@active
cleos get table oracleoracl2 eosusd datapoints --limit -1
cleos get table oracleoracl2 iqeos datapoints --limit -1
cleos get table oracleoracl2 vigeos datapoints --limit -1
cleos get table oracleoracl2 oracleoracl2 stats
cleos get table oracleoracl2 oracleoracl2 pairs

CONTRACT_ROOT=/home/gg/contracts/vigor/contracts/datapreproc/src
CONTRACT_OUT=/home/gg/contracts/vigor/contracts/datapreproc
CONTRACT_INCLUDE=/home/gg/contracts/vigor/contracts/datapreproc/include
CONTRACT="datapreproc"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"
EOSIO_CONTRACTS_ROOT=/home/gg/contracts/eosio.contracts/contracts
eosio-cpp -contract=$CONTRACT -I=$CONTRACT_INCLUDE -I=$EOSIO_CONTRACTS_ROOT/eosio.system/include -o="$CONTRACT_OUT/$CONTRACT_WASM" -abigen "$CONTRACT_ROOT/$CONTRACT_CPP" 
cleos set contract datapreproc1 $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p datapreproc1@active
cleos push action datapreproc1 clear '{}' -p datapreproc1@active
cleos push action datapreproc1 addpair '{"newpair":"eosusd"}' -p datapreproc1@active
cleos push action datapreproc1 addpair '{"newpair":"iqeos"}' -p datapreproc1@active
cleos push action datapreproc1 addpair '{"newpair":"vigeos"}' -p datapreproc1@active
cleos push action datapreproc1 update '{}' -p feeder111111@active
cd /home/gg/contracts/vigor/contracts/oracle && CONTRACT=datapreproc1 OWNER=feeder111111 node dataupdate.js
cleos get table datapreproc1 datapreproc1 pairtoproc --limit -1
cleos get table datapreproc1 eosusd stats
cleos get table datapreproc1 iqeos stats
cleos get table datapreproc1 vigeos stats


#cleos -u http://api.eosnewyork.io:80 get code delphioracle --wasm -c delphioracle.wasm
#cleos -u http://api.eosnewyork.io:80 get code delphioracle --abi delphioracle.abi

#cleos set contract oracleoracl2 . delphioracle.wasm delphioracle.abi -p oracleoracl2@active

#cleos push action oracleoracl2 setoracles '{"oracleslist":["feeder111111"]}' -p eostitanprod@active

#=================================================================================#

###### OPTIONAL FOR LOCAL TESTNET #############
# cd ~/contracts/delphioracle/scripts
# nodeosurl='http://127.0.0.1:8888' interval=15000 account="oracleoracl2" defaultPrivateKey="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12" feeder="feeder111111" node updater2.js
#cleos get table oracleoracl2 oracleoracl2 eosusd --limit 1
#cleos get table oracleoracl2 oracleoracl2 oracles
#cleos get table oracleoracl2 oracleoracl2 eosusdstats

# launch two oracle feeders
#in a new shell
#=================================================================================#
# exposed actions for vigor demo starts here
cleos push action eosio.token transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"6.0000 EOS","memo":"collateral"}' -p testbrw11111@active
cleos push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"collateral"}' -p testbrw11111@active
#cleos push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 UTG","memo":"collateral"}' -p testbrw11111@active
#cleos push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 PTI","memo":"collateral"}' -p testbrw11111@active
#cleos push action dummytokensx transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 OWN","memo":"collateral"}' -p testbrw11111@active
cleos push action vig111111111 transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"collateral"}' -p testbrw11111@active

cleos push action eosio.token transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"6.0000 EOS","memo":"collateral"}' -p testbrw11112@active
cleos push action dummytokensx transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.000 IQ","memo":"collateral"}' -p testbrw11112@active
#cleos push action dummytokensx transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.0000 UTG","memo":"collateral"}' -p testbrw11112@active
#cleos push action dummytokensx transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.0000 PTI","memo":"collateral"}' -p testbrw11112@active
#cleos push action dummytokensx transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.0000 OWN","memo":"collateral"}' -p testbrw11112@active
cleos push action vig111111111 transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"3000.0000 VIG","memo":"collateral"}' -p testbrw11112@active

cleos push action eosio.token transfer '{"from":"testins11111","to":"vigor1111111","quantity":"6.0000 EOS","memo":"insurance"}' -p testins11111@active
cleos push action dummytokensx transfer '{"from":"testins11111","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins11111@active
#cleos push action dummytokensx transfer '{"from":"testins11111","to":"vigor1111111","quantity":"1000.0000 UTG","memo":"insurance"}' -p testins11111@active
#cleos push action dummytokensx transfer '{"from":"testins11111","to":"vigor1111111","quantity":"1000.0000 PTI","memo":"insurance"}' -p testins11111@active
#cleos push action dummytokensx transfer '{"from":"testins11111","to":"vigor1111111","quantity":"1000.0000 OWN","memo":"insurance"}' -p testins11111@active

cleos push action eosio.token transfer '{"from":"testins11112","to":"vigor1111111","quantity":"6.0000 EOS","memo":"insurance"}' -p testins11112@active
cleos push action dummytokensx transfer '{"from":"testins11112","to":"vigor1111111","quantity":"3000.000 IQ","memo":"insurance"}' -p testins11112@active
#cleos push action dummytokensx transfer '{"from":"testins11112","to":"vigor1111111","quantity":"1000.0000 UTG","memo":"insurance"}' -p testins11112@active
#cleos push action dummytokensx transfer '{"from":"testins11112","to":"vigor1111111","quantity":"1000.0000 PTI","memo":"insurance"}' -p testins11112@active
#cleos push action dummytokensx transfer '{"from":"testins11112","to":"vigor1111111","quantity":"1000.0000 OWN","memo":"insurance"}' -p testins11112@active

cleos push action vigor1111111 assetout '{"usern":"testbrw11111","assetout":"5.6000 VIGOR","memo":"borrow"}' -p testbrw11111@active
cleos push action vigor1111111 assetout '{"usern":"testbrw11112","assetout":"24.6000 VIGOR","memo":"borrow"}' -p testbrw11112@active

cleos push action vigor1111111 assetout '{"usern":"testbrw11111","assetout":"1.0000 EOS","memo":"collateral"}' -p testbrw11111@active
cleos push action vigor1111111 assetout '{"usern":"testbrw11112","assetout":"1.0000 EOS","memo":"collateral"}' -p testbrw11112@active
cleos push action vigor1111111 assetout '{"usern":"testins11111","assetout":"1.0000 EOS","memo":"insurance"}' -p testins11111@active
cleos push action vigor1111111 assetout '{"usern":"testins11112","assetout":"1.0000 EOS","memo":"insurance"}' -p testins11112@active

cleos push action vigor1111111 transfer '{"from":"testbrw11111","to":"vigor1111111","quantity":"10.0000 VIGOR","memo":"payoff debt"}' -p testbrw11111@active
cleos push action vigor1111111 transfer '{"from":"testbrw11112","to":"vigor1111111","quantity":"10.0000 VIGOR","memo":"payoff debt"}' -p testbrw11112@active

#cleos push action vigor1111111 doupdate '{}' -p vigor1111111@active
#sleep 5
#cleos push action vigor1111111 doupdate '' -p vigor1111111@active
#sleep 5
#cleos get table vigor1111111 vigor1111111 user

# get all the user data
cleos get table vigor1111111 vigor1111111 user
cleos get table vigor1111111 vigor1111111 user -Ltestborrow31 -Utestborrow31
cleos get table vigor1111111 vigor1111111 user -Ltestborrow32 -Utestborrow32
cleos get table vigor1111111 vigor1111111 user -Ltestinsure31 -Utestinsure31
cleos get table vigor1111111 vigor1111111 user -Ltestinsure32 -Utestinsure32
cleos get table vigor1111111 vigor1111111 globals
# cleos get table vigor1111111 VIGOR stat
# cleos get table eosio.token vigor1111111 accounts
# cleos get table eosio.token testbrw11111 accounts
# cleos get table vigor1111111 testbrw11111 accounts
# cleos get currency balance vigor1111111 testbrw11111
# cleos get currency balance eosio.token vigor1111111

cleos -u https://api.kylin.alohaeos.com get table delphioracle delphioracle eosusd
cleos -u https://api.kylin.alohaeos.com get table eostitantest eostitantest eosusd
cleos -u https://api.kylin.alohaeos.com get table eostitantest eosusd datapoints
cleos -u https://api.kylin.alohaeos.com get table eostitantest eosbtc datapoints
cleos -u https://api.kylin.alohaeos.com get table eostitantest eostitantest stats
cleos -u https://api.kylin.alohaeos.com get table eostitantest eostitantest pairs
cleos -u https://api.kylin.alohaeos.com get table eosio eosio producers
