
beforeAll(function() {
    console.log("init_helper beforeAll");
    global.helper = require('./tests_helper')
    global.help = new helper.TestHelper();
    global.config = helper.config;
    global.eoslime = require('eoslime').init();
    global.accounts = {};
    global.contracts = {};
    global.tables = {};
    global.cacheAccount = function(name) {
        global.accounts[name] = help.getAccount(name);
    }
    global.cacheContract = async function(name) {
        global.contracts[name] = await help.getContract(name);
    }
    // global.cacheTable = function(contract, tableName) {
    //     if(global.tables[contract.name] == null) {
    //         global.tables[contract.name] = {};
    //     }
        
    //     global.tables[contract.name] = contract[tableName];
    // }
    // global.getTable(contract, tableName) {
    //     return global.tables[contract.name][tableName];
    // }
});
