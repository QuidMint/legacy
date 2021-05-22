# source ./vars.sh
export EOSC_GLOBAL_INSECURE_VAULT_PASSPHRASE=devwallet
export EOSC_GLOBAL_VAULT_FILE=/Users/boid/eosc-dev-vault.json
export EOSC_GLOBAL_API_URL="https://jungle3.greymass.com"
export CONTRACTDIR=../build/contracts/vigoraclehub

eosc system setcode vigoraclehub $CONTRACTDIR/oraclehub.wasm -p vigoraclehub@active
eosc system setabi vigoraclehub $CONTRACTDIR/oraclehub.abi -p vigoraclehub@active