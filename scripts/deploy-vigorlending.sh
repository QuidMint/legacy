export EOSC_GLOBAL_INSECURE_VAULT_PASSPHRASE=devwallet
export EOSC_GLOBAL_VAULT_FILE=/Users/boid/eosc-dev-vault.json
export EOSC_GLOBAL_API_URL="https://jungle3.greymass.com"
export CONTRACTDIR=../build/contracts/vigorlending

eosc system setcode vigorlending $CONTRACTDIR/vigorlending.wasm -p vigorlending@active
eosc system setabi vigorlending $CONTRACTDIR/vigorlending.abi -p vigorlending@active