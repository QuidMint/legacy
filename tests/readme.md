# How to run the tests:

### On a first console, start nodeos. This will  grab the output so that you can follow it easily

    condole1> cd <path to unit tests directory>
    console1> ./nodeos_start --clear

### On a second console, initialize and run the tests

    console2> cd <path to build directory>
    console2> cmake -DEOSIO_CONTRACTS_PATH="<path to eosio contracts>" ..
    condole2> cd <path to unit tests directory>
    console2> npm install

The setup is done. Now just run the tests

    console2> jasmine