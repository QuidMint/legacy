#!/bin/bash
# Usage: ./test.sh
#=================================================================================#
# SETUP
#

#nodeos -d ./data --config-dir ./config --plugin eosio::chain_api_plugin --plugin eosio::producer_api_plugin -p eosio -e 2>stderr &

pkill nodeos
rm -rf ~/.local/share/eosio/nodeos/data
nodeos -e -p eosio \
--plugin eosio::chain_api_plugin \
--plugin eosio::producer_api_plugin \
--max-transaction-time=10000 
>> nodeos.log  2>stderr & tail -f nodeos.log
#--http-validate-host=false \
#--delete-all-blocks \
#--plugin eosio::chain_api_plugin \
#--contracts-console \
#--plugin eosio::http_plugin \
#--plugin eosio::producer_plugin
#--plugin eosio::history_api_plugin \
#--plugin eosio::producer_api_plugin \
#--verbose-http-errors \

#>> nodeos.log 2>&1 &  tail -f nodeos.log
#nodeos -e -p eosio --plugin eosio::producer_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::state_history_plugin --access-control-allow-origin='*' --contracts-console --http-validate-host=false --trace-history --chain-state-history --verbose-http-errors --filter-on='*' --disable-replay-opts >> nodeos.log 2>&1 &

#sleep 1s
until curl localhost:8888/v1/chain/get_info
do
  sleep 1s
done

# Sleep for 2 to allow time 4 blocks to be created so we have blocks to reference when sending transactions
sleep 2s



CYAN='\033[1;36m'
NC='\033[0m'


# CHANGE PATH
EOSIO_CONTRACTS_ROOT=/home/ik/contracts/eosio.contracts/contracts
CONTRACT_ROOT=/home/ik/contracts/vigor/contracts


#OWNER_KEY="EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx"
#OWNER_ACCT="5J3TQGkkiRQBKcg8Gg2a7Kk5a2QAQXsyGrkCnnq4krSSJSUkW12"

OWNER_KEY="EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
OWNER_ACCT="5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"

#cleos wallet create --to-console
#cleos wallet unlock -n default --password PW5HzuR3R2g77WZCsqaSMD91aC3WPFQzmeHkyzyEREbPTB3EmHeMC
#cleos wallet import -n default --private-key $OWNER_ACCT

cleos wallet create --to-console
cleos wallet unlock -n default --password PW5JxfS76qiuntgkuWHWHzokyuSmSP862BoVZbqz3ZAvPmYXAohve
cleos wallet import -n default --private-key $OWNER_ACCT
cleos wallet list

#=================================================================================#
# In another cmdline console/terminal 
curl -X POST http://127.0.0.1:8888/v1/producer/schedule_protocol_feature_activations -d '{"protocol_features_to_activate": ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}' | jq

cleos set contract eosio /home/ik/contracts/eosio.contracts/build/contracts/eosio.bios -p eosio

cleos push action eosio activate '["f0af56d2c5a48d60a4a5b5c903edfb7db3a736a94ed589d0b797df33ff9d3e1d"]' -p eosio # GET_SENDER
cleos push action eosio activate '["2652f5f96006294109b3dd0bbde63693f55324af452b799ee137a81a905eed25"]' -p eosio # FORWARD_SETCODE
cleos push action eosio activate '["8ba52fe7a3956c5cd3a656a3174b931d3bb2abb45578befc59f283ecd816a405"]' -p eosio # ONLY_BILL_FIRST_AUTHORIZER
cleos push action eosio activate '["ad9e3d8f650687709fd68f4b90b41f7d825a365b02c23a636cef88ac2ac00c43"]' -p eosio # RESTRICT_ACTION_TO_SELF
cleos push action eosio activate '["68dcaa34c0517d19666e6b33add67351d8c5f69e999ca1e37931bc410a297428"]' -p eosio # DISALLOW_EMPTY_PRODUCER_SCHEDULE
cleos push action eosio activate '["e0fb64b1085cc5538970158d05a009c24e276fb94e1a0bf6a528b48fbc4ff526"]' -p eosio # FIX_LINKAUTH_RESTRICTION
cleos push action eosio activate '["ef43112c6543b88db2283a2e077278c315ae2c84719a8b25f25cc88565fbea99"]' -p eosio # REPLACE_DEFERRED
cleos push action eosio activate '["4a90c00d55454dc5b059055ca213579c6ea856967712a56017487886a4d4cc0f"]' -p eosio # NO_DUPLICATE_DEFERRED_ID
cleos push action eosio activate '["1a99a59d87e06e09ec5b028a9cbb7749b4a5ad8819004365d02dc4379a8b7241"]' -p eosio # ONLY_LINK_TO_EXISTING_PERMISSION
cleos push action eosio activate '["4e7bf348da00a945489b2a681749eb56f5de00b900014e137ddae39f48f69d67"]' -p eosio # RAM_RESTRICTIONS

cleos set contract eosio /home/ik/contracts/eosio.contracts/build/contracts/eosio.system/

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
cleos create account eosio eosio.token EOS7JPVyejkbQHzE9Z4HwewNzGss11GB21NPkwTX2MQFmruYFqGXm
cleos create account eosio eosio.vpay EOS6szGbnziz224T1JGoUUFu2LynVG72f8D3UVAS25QgwawdH983U
cleos create account eosio eosio.rex EOS6szGbnziz224T1JGoUUFu2LynVG72f8D3UVAS25QgwawdH983U

# Bootstrap new system contracts
echo -e "${CYAN}-----------------------SYSTEM CONTRACTS-----------------------${NC}"
#eosio-cpp -contract=eosio.msig -I=/home/ik/contracts/eosio.contracts/contracts/eosio.msig/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.msig/eosio.msig.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.msig/src/eosio.msig.cpp
#eosio-cpp -contract=eosio.system -I=/home/ik/contracts/eosio.contracts/contracts/eosio.system/include -I=/home/ik/contracts/eosio.contracts/contracts/eosio.token/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.system/eosio.system.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.system/src/eosio.system.cpp
#eosio-cpp -contract=eosio.wrap -I=/home/ik/contracts/eosio.contracts/contracts/eosio.wrap/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.wrap/eosio.wrap.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.wrap/src/eosio.wrap.cpp
#eosio-cpp -contract=eosio.token -I=/home/ik/contracts/eosio.contracts/contracts/eosio.token/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.token/eosio.token.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.token/src/eosio.token.cpp
#eosio-cpp -contract=eosio.bios -I=/home/ik/contracts/eosio.contracts/contracts/eosio.bios/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.bios/eosio.bios.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.bios/src/eosio.bios.cpp

#eosio-cpp -contract=eosio.msig -I=/home/ik/contracts/eosio.contracts/contracts/eosio.msig/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.msig/eosio.msig.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.msig/src/eosio.msig.cpp
#eosio-cpp -contract=eosio.system -I=/home/ik/contracts/eosio.contracts/contracts/eosio.system/include -I=/home/ik/contracts/eosio.contracts/contracts/eosio.token/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.system/eosio.system.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.system/src/eosio.system.cpp
#eosio-cpp -contract=eosio.wrap -I=/home/ik/contracts/eosio.contracts/contracts/eosio.wrap/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.wrap/eosio.wrap.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.wrap/src/eosio.wrap.cpp
#eosio-cpp -contract=eosio.token -I=/home/ik/contracts/eosio.contracts/contracts/eosio.token/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.token/eosio.token.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.token/src/eosio.token.cpp
#eosio-cpp -contract=eosio.bios -I=/home/ik/contracts/eosio.contracts/contracts/eosio.bios/include -o=/home/ik/contracts/eosio.contracts/contracts/eosio.bios/eosio.bios.wasm -abigen /home/ik/contracts/eosio.contracts/contracts/eosio.bios/src/eosio.bios.cpp



#cleos set contract eosio.msig /home/ik/contracts/eosio.contracts/contracts/eosio.msig
cleos set contract eosio.msig /home/ik/contracts/eosio.contracts/build/contracts/eosio.msig
cleos set contract eosio.token /home/ik/contracts/eosio.contracts/build/contracts/eosio.token
cleos push action eosio.token create '[ "eosio", "100000000000.0000 EOS" ]' -p eosio.token
cleos push action eosio.token create '[ "eosio", "100000000000.0000 SYS" ]' -p eosio.token
echo -e "      EOS TOKEN CREATED"
cleos push action eosio.token issue '[ "eosio", "10000000000.0000 EOS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "eosio", "10000000000.0000 SYS", "memo" ]' -p eosio
echo -e "      EOS TOKEN ISSUED"
#cleos set contract eosio /home/ik/contracts/eosio.contracts/build/contracts/eosio.bios -p eosio
echo -e "      BIOS SET"
#cleos set contract eosio /home/ik/contracts/eosio.contracts/build/contracts/eosio.system/
echo -e "      SYSTEM SET"
cleos push action eosio setpriv '["eosio.msig", 1]' -p eosio@active
cleos push action eosio init '["0", "4,EOS"]' -p eosio@active
#cleos push action eosio init '["0", "4,SYS"]' -p eosio@active

# Deploy eosio.wrap
echo -e "${CYAN}-----------------------EOSIO WRAP-----------------------${NC}"
cleos wallet import -n default --private-key 5J3JRDhf4JNhzzjEZAsQEgtVuqvsPPdZv4Tm6SjMRx1ZqToaray
cleos system newaccount eosio eosio.wrap EOS7LpGN1Qz5AbCJmsHzhG7sWEGd9mwhTXWmrYXqxhTknY2fvHQ1A --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 5000 --transfer
cleos push action eosio setpriv '["eosio.wrap", 1]' -p eosio@active
cleos set contract eosio.wrap /home/ik/contracts/eosio.contracts/build/contracts/eosio.wrap/


#=================================================================================#
# create the vigor1111111 account, set the contract, create VIGOR stablecoins

cleos system newaccount eosio vigor1111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos set account permission vigor1111111 active '{"threshold":1,"keys":[{"key":"EOS6TnW2MQbZwXHWDHAYQazmdc3Sc1KGv4M9TSgsKZJSo43Uxs2Bx","weight":1}],"accounts":[{"permission":{"actor":"vigor1111111","permission":"eosio.code"},"weight":1}],"waits":[]}' -p vigor1111111@active
CONTRACT_ROOT=/home/ik/contracts/vigor/contracts/vigor/src
CONTRACT_OUT=/home/ik/contracts/vigor/contracts/vigor
CONTRACT_INCLUDE=/home/ik/contracts/vigor/contracts/vigor/include
CONTRACT_INCLUDE_BOOST=/home/ik/contracts
CONTRACT="vigor"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"
#eosio-cpp -contract=$CONTRACT -o="$CONTRACT_OUT/$CONTRACT_WASM" -I="$CONTRACT_INCLUDE" -I="$CONTRACT_INCLUDE_BOOST" -abigen "$CONTRACT_ROOT/$CONTRACT_CPP"
#eosio-cpp -contract=vigor -o="/home/ik/contracts/vigor/contracts/vigor/vigor.wasm" -I="/home/ik/contracts/vigor/contracts/vigor/include" -I="/home/ik/contracts" -abigen "/home/ik/contracts/vigor/contracts/vigor/src/vigor.cpp"
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

cleos set contract vig111111111 /home/ik/contracts/eosio.contracts/contracts/eosio.token/ -p vig111111111@active
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
cleos set contract dummytokensx /home/ik/contracts/eosio.contracts/contracts/eosio.token/ -p dummytokensx@active

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

#=================================================================================#
# create the oracle contract for local testnet
cleos system newaccount eosio oracleoracle $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
#cleos system newaccount eosio eostitanprod $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111111 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111112 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio feeder111113 $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer
cleos system newaccount eosio datapreprocx $OWNER_KEY --stake-cpu "50 EOS" --stake-net "10 EOS" --buy-ram-kbytes 50000 --transfer

ORACLE_ROOT=/home/ik/contracts/vigor/contracts/oracle/src
ORACLE_OUT=/home/ik/contracts/vigor/contracts/oracle
ORACLE_INCLUDE=/home/ik/contracts/vigor/contracts/oracle/include
ORACLE="oracle"
ORACLE_WASM="$ORACLE.wasm"
ORACLE_ABI="$ORACLE.abi"
ORACLE_CPP="$ORACLE.cpp"/home/ik/contracts/eosio.contracts/contracts=/home/ik/contracts/eosio.contracts/contracts
#eosio-cpp -contract=$ORACLE -I=$ORACLE_INCLUDE -I=/home/ik/contracts/eosio.contracts/contracts/eosio.system/include -o="$ORACLE_OUT/$ORACLE_WASM" -abigen "$ORACLE_ROOT/$ORACLE_CPP" 
cleos set contract oracleoracle $ORACLE_OUT $ORACLE_WASM $ORACLE_ABI -p oracleoracle@active
cleos push action oracleoracle configure '{}' -p oracleoracle@active
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_eosusd.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_eosusd.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_eosusd.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_iqeos.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_iqeos.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_iqeos.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111111 node updater_vigeos.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111112 node updater_vigeos.js
cd /home/ik/contracts/vigor/contracts/oracle && ORACLE=feeder111113 node updater_vigeos.js


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

CONTRACT_ROOT=/home/ik/contracts/vigor/contracts/datapreproc/src
CONTRACT_OUT=/home/ik/contracts/vigor/contracts/datapreproc
CONTRACT_INCLUDE=/home/ik/contracts/vigor/contracts/datapreproc/include
CONTRACT="datapreproc"
CONTRACT_WASM="$CONTRACT.wasm"
CONTRACT_ABI="$CONTRACT.abi"
CONTRACT_CPP="$CONTRACT.cpp"/home/ik/contracts/eosio.contracts/contracts=/home/ik/contracts/eosio.contracts/contracts
#eosio-cpp -contract=$CONTRACT -I=$CONTRACT_INCLUDE -I=/home/ik/contracts/eosio.contracts/contracts/eosio.system/include -o="$CONTRACT_OUT/$CONTRACT_WASM" -abigen "$CONTRACT_ROOT/$CONTRACT_CPP" 
cleos set contract datapreprocx $CONTRACT_OUT $CONTRACT_WASM $CONTRACT_ABI -p datapreprocx@active
#cleos push action datapreprocx clear '{}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"eosusd"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"iqeos"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"vigeos"}' -p datapreprocx@active
cleos push action datapreprocx addpair '{"newpair":"vigorusd"}' -p datapreprocx@active
#cleos push action datapreprocx update '{}' -p feeder111111@active
#cleos push action datapreprocx doshock '{"shockvalue":0.5}' -p feeder111111@active
cd /home/ik/contracts/vigor/contracts/oracle && CONTRACT=datapreprocx OWNER=feeder111111 node dataupdate.js
cleos get table datapreprocx datapreprocx pairtoproc --limit -1
cleos get table datapreprocx eosusd tseries
cleos get table datapreprocx iqeos tseries
cleos get table datapreprocx vigeos tseries
cleos get table datapreprocx vigorusd tseries