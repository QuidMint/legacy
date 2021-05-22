const path = require('path');
const fs = require('fs');
const eoslime = require('eoslime').init();
const config = require('../../config/config.json');
const Provider = eoslime.Provider;
// const axios = require('axios');


class TestHelper {
    
    constructor() {
        // console.log(config);

        if (!fs.existsSync(config.walletPath)){
            fs.mkdirSync(config.walletPath, { recursive: true });
        }
    }

    //  TODO: This should be sync
    async sleep(timeoutMS) {
        return new Promise(res => setTimeout(res, timeoutMS));
    }
    
    getKeyPair(accountName, level) {
        var basePath = path.join(config.walletPath, accountName + "_" + level + ".json");
        if( fs.existsSync(basePath) == false) {
            // await this.storeKeyPair(accountName, level, await eoslime.utils.generateKeys());
            throw "Key pair does not exist on local wallet.";
        }
        return JSON.parse(fs.readFileSync(basePath).toString());
    }

    storeKeyPair(accountName, level, keyPair) {
        var basePath = path.join(config.walletPath, accountName + "_" + level + ".json");
        fs.writeFileSync(basePath, JSON.stringify(keyPair));
    }

    async createAccount(name) {
        // console.log('Creating account...');
        if(name == 'eosio') {
          return true;
        }

        return await eoslime.Account.createFromName(name).then(account => {
            // console.log(JSON.stringify(account));
            // console.log('Account created successfully: ' + name);
            this.storeKeyPair(name, 'active', {"privateKey": account.privateKey, "publicKey": account.publicKey});
            return this.getAccount(name, 'active');
        }).catch(error => {
            try {
                error = JSON.parse(error);
                if(error.error.name == 'account_name_exists_exception') {
                    return true;
                }
            } catch (exc) {}

            console.log("    Account creation failed: ", error);
            console.log("        ", error.details);
            return false;
        });
    }

    async createStakedAccount(name, cpu, net, ram, payer) {
        var payerAccount = this.getAccount(payer);
        var account = await this.createAccount(name);
        await account.buyBandwidth(cpu, net, payerAccount);
        await account.buyRam(ram, payerAccount);

        return account;
    }

    getAccount(name, permission = 'active') {
        if(name == 'eosio') {
          return eoslime.Account.load('eosio', '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3', permission);
        }
        var keyPair = this.getKeyPair(name, permission);
        return eoslime.Account.load(name, keyPair.privateKey, permission);
    }

    async deployContract(name, owner) {

        var contractPath = config.user.contractsPath;
        if(await config.system.contracts.includes(name) == true) {
            var contractPath = config.system.contractsPath;  //  check here if it's a system or user contract
        }

        var wasm = path.join(contractPath, name, name + ".wasm");
        var abi = path.join(contractPath, name, name + ".abi");

        return await eoslime.Contract.deployOnAccount(wasm, abi, owner).then((contract) => {
            console.log('Contract deployed: ' + name + " to " + owner.name + " account");

            if(name == 'eosio.system') {  // Need to refresh the abi cache for this one (eosjs hack)
                eoslime.Provider.eos.fc.types.config.abiCache.abi('eosio', JSON.parse(fs.readFileSync(abi, "utf8")));
            }

            return contract;
        }).catch(error => {
            try {
                let jsonError = JSON.parse(error);
                if(jsonError.error.name == 'set_exact_code') {
                    return false;
                }
            } catch (exc) {}

            console.log('    Failed to deploy contract [' + name + ']: ', error);
            return false;
        });
    }

    async getContractFromFile(name, executor = 'eosio') {
        var contractPath = config.user.contractsPath;
        if(await config.system.contracts.includes(name) == true) {
            var contractPath = config.system.contractsPath;  //  check here if it's a system or user contract
        }

        var abi = path.join(contractPath, name, name + ".abi");
        var account = await this.getAccount(executor);

        return await eoslime.Contract.fromFile(abi, name, account);
    }

    async getContract(name, executor = 'eosio') {
        return await eoslime.Contract.at(name, this.getAccount(executor)).then(contract => {
            if(contract.executor.executiveAuthority.permission == undefined) {
                contract.executor.executiveAuthority.permission = 'active';
            }
            return contract;
        }).catch(error => {
            console.log('Failed to get contract information from the blockchain [' + name + ']: ', error);
            return null;
        });
    }


    async oracleUpdates(ownerAccount, feederConfig){
        let contract = await this.getContract(feederConfig.contract);

        let evalExpression = feederConfig.calculator;
        if(config.tests.fixedFeeders)
            evalExpression = feederConfig.fixed;
        else {
            let response = await fetch(feederConfig.url);
      
            if(!response.ok) {
                console.log("error:", feederConfig.url);
                console.log(response.status);
                return;
            }
            
            var jsonData = await response.json();
        }
        let result = await contract.write(ownerAccount.name, [{value: eval(evalExpression), pair: feederConfig.pair}], {from: ownerAccount}).catch(error => {
          console.log("error:", feederConfig.url);
          return null;
        });

        // if(result) {
        //     console.log("update:", {value: eval(config.calculator), pair: config.pair});
        // }

        //  Auto run this method every 'config.interval' miliseconds
        setTimeout(() => {this.oracleUpdates(ownerAccount, feederConfig);}, feederConfig.interval);
    }
    
    async scheduleFeatureActivation() {
        let data = {
            method: 'POST',
            headers: { 'Content-Type': 'application/json'},
            body: '{"protocol_features_to_activate": ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}'
        };
        await fetch('http://127.0.0.1:8888/v1/producer/schedule_protocol_feature_activations', data)
        .then(async (response) => {
            // let json = await response.json();
            // console.log("Seems ok...", json, json.error.details);
            return;
        })
        .catch((error) => {
            console.log("-->scheduleFeatureActivation failed: ", error);
            return;
        });
    }

    async getChainAccountData(user) {
        let data = { method: 'POST', body: '{"account_name": "' + user.name + '"}' };
        return await fetch('http://127.0.0.1:8888/v1/chain/get_account', data)
        .then((response) => {
            // let json = await response.json();
            // console.log("Seems ok...", json, json.error.details);
            return response.json();
        })
        .catch((error) => {
            console.log("-->scheduleFeatureActivation failed: ", error);
            return;
        });
    }

    async getTableRow(table, filter) {
        let result = null;
        try{
            result = await table.equal(filter).find();
        }catch(error){
            console.log(error);
        }
        
        return result[0];
    }

    async getBalance(user, symbol, contract) {
        let result = await user.getBalance(symbol, contract.name);
        //let result = await this.getTableRow(contract.accounts, user.name);
        result = result[0];
        if(!result)
            return undefined;
        return result.split(' ')[0];
    }

    async getUserData(user) {
        if(typeof user == "string"){
            user = this.getAccount(user);
        }
        let result = { balance: {}, vigor: {} };
        try {
            result.balance.eos = await this.getBalance(user, 'EOS', 'eosio.token');
            //result.balance.vigor = await this.getBalance(user, 'VIGOR', 'vigorlending');
            // console.log(await global.contracts.vig111111111.accounts.limit(100).find());
            // result.balance.vig = await this.getTableRow(global.contracts.vig111111111.accounts, user.name); //await this.getBalance(user, 'VIG', 'vig111111111');
            // result.balance.iq = await this.getBalance(user, 'IQ', 'dummytokensx');
            // result.balance.boid = await this.getBalance(user, 'BOID', 'dummytokensx');
        }catch(error) {
            console.log(error);
        }
        result.vigor = await this.getTableRow(global.contracts.vigorlending.user, user.name);
        if(result.vigor == undefined){
            console.log("Could not find user:", user.name);
        }
        result.vigor.lastupdate = "<removed>";
        result.eosio = await help.getChainAccountData(user);
        return result;
    }

    async getVigorGlobals() {
        let result = await global.contracts.vigorlending.globalstats.limit(1).find();
        result[0].lastupdate = "<removed>";
        return result[0];
    }

    async getVigorConfig() {
        let result = await global.contracts.vigorlending.config.limit(1).find();
        return result[0];
    }

    getLocationContext(error, skipCallers = 0) {
        skipCallers = skipCallers + 2;  //  Naturaly skip 2 caller levels
        let stackLine = error.stack.split('\n')[skipCallers].trim().split(' ');
        let location = stackLine[2].replace('(', '').replace(')', '').split(':');
        let result = { 
            caller: stackLine[1],
            file: path.basename(location[0]),
            line: location[1]
        }
        return result;
    }

    async genericBlockchainOperation(params, operation) {
        let result = new MetaResult();
        //  Check if there is an account or account1 in the parameters
        let account = undefined;
        if(params.hasOwnProperty('account')) {
            result.account = params.account;
        } else if(params.hasOwnProperty('account1')) {
            result.account1 = params.account1;
        }

        result.before.accounts = {}
        await Promise.all(config.user.testAccounts.map(async account => { result.before.accounts[account] = await help.getUserData(account) }));

        //  Check if there is a value parameter to use
        if(params.hasOwnProperty('value')) {
            result.setValue(params.value);
        }
        //  Get the VIGOR globals and status tables
        result.before.globals = await help.getVigorGlobals();

        //  Get the VIGOR configuration table
        result.before.config = await help.getVigorConfig();

        //  Get the location context
        if(params.hasOwnProperty('errorContext')) {
            result.context = help.getLocationContext(params.errorContext);
        }

        try {
            result.transaction = await operation();
        } catch(error) {
            // console.log(error);
            try {
                result.error = JSON.parse(error);
            } catch(error2) {
                result.error = error;
            }
            
        }
        //  Get the aftermath of all things we got before the operation
        result.after.accounts = {}
        await Promise.all(config.user.testAccounts.map(async account => { result.after.accounts[account] = await help.getUserData(account) }));

        result.after.globals = await help.getVigorGlobals();
        
        //  Get the VIGOR configuration table
        result.after.config = await help.getVigorConfig();

        return result;
    }

}

class MetaResult {
    constructor(value){
        this.before = {};
        this.after = {};
        if(value) {
            this.value = { amount: parseFloat(value.split(' ')[0]), symbol: value.split(' ')[1] };
        }
        
    }
    setValue(value) {
        this.value = { amount: parseFloat(value.split(' ')[0]), symbol: value.split(' ')[1] };
    }
    assetDiff(before, after, symbol){
        for(var index = 0; index < after.length; ++index){
            if( after[index].indexOf(symbol) >= 0 ){
                if(before == undefined || index >= before.length){
                    return parseFloat(after[index]);
                }
                return parseFloat(after[index]) - parseFloat(before[index]);
            }
        }
        return "Symbol not found";
    }
    insuranceDiff(account, symbol){
        if(!account){
            account = this.account.name;
        }
        if(!symbol){
            symbol = this.value.symbol;
        }
        return this.assetDiff(this.before.accounts[account].vigor.insurance, this.after.accounts[account].vigor.insurance, symbol);
    }
    globalInsuranceDiff(symbol){
        if(!symbol){
            symbol = this.value.symbol;
        }
        return this.assetDiff(this.before.globals.insurance, this.after.globals.insurance, symbol);
    }
    collateralDiff(account, symbol){
        if(!account){
            account = this.account.name;
        }
        if(!symbol){
            symbol = this.value.symbol;
        }
        return this.assetDiff(this.before.accounts[account].vigor.collateral, this.after.accounts[account].vigor.collateral, symbol);
    }
    globalCollateralDiff(symbol){
        if(!symbol){
            symbol = this.value.symbol;
        }
        return this.assetDiff(this.before.globals.collateral, this.after.globals.collateral, symbol);
    }
}

class ExpectedResults {
    constructor(testName) {
        this.testName = testName;
        this.stepNumber = 0;
    }

    step(stepNumber) {
        this.stepNumber = stepNumber;
    }

    getTestResult(testName, step, type, account) {
        var filePath = path.join(config.tests.resultsBasePath, testName, "step_" + step + "." + type);
        if( fs.existsSync(filePath) == false) {
            throw "Expected result does not exist on local wallet.";
        }
        var json = JSON.parse(fs.readFileSync(filePath).toString());
        
        if(type == "globals") {
            json.rows[0].lastupdate = "<removed>";
            return json.rows[0];
        }

        var user;
        json.rows.map(item => { if(item.usern == account) user = item; item.lastupdate = "<removed>" });
        if(!account) {
            return json.rows;
        }
        return user;
    }

    getAccount(account){
        return this.getTestResult(this.testName, this.stepNumber, "users", account);
    }

    getAllAccounts() {
        return this.getTestResult(this.testName, this.stepNumber, "users");
    }

    getGlobals() {
        return this.getTestResult(this.testName, this.stepNumber, "globals");
    }
}

class TokenHelper {
    constructor(contract, vigorAccount) {
        this.contract = contract;
        this.vigorAccount = vigorAccount;
        
        if(contract.name == "vigorlending") {
            this.assetIn = async (from, toUser, amount, memo, permission = from, errorContext = new Error()) => {
                let params = {
                    account1: from,
                    value: amount,
                    errorContext: errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.assetin(from.name, toUser.name, amount, memo, { from: permission }) );
            }
            this.assetOut = async (from, amount, memo, permission = from, errorContext = new Error()) => {
                let params = {
                    account1: from,
                    value: amount,
                    errorContext: errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.assetout(from.name, amount, memo, { from: permission }) );
            }
            this.doAssetOut = async (from, amount, memo, permission = from, errorContext = new Error()) => {
                let params = {
                    account1: from,
                    value: amount,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.doassetout(from.name, amount, memo, 0, { from: permission }) );
            }
            this.assetOutInsurance = async (from, amount, permission = from) => {
                return this.assetOut(from, amount, 'insurance', permission);
            }
            this.assetOutCollateral = async (from, amount, permission = from) => {
                return this.assetOut(from, amount, 'collateral', permission);
            }
            this.assetOutBorrow = async (from, amount, permission = from) => {
                return this.assetOut(from, amount, 'borrow', permission);
            }
            this.openAccount = async (account, errorContext = new Error()) => {
                let params = {
                    account,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.openaccount(account, { from: account }) );
            }
            this.deleteAccount = async (account, errorContext = new Error()) => {
                let params = {
                    account,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.deleteacnt(account, { from: account }) );
            }
            this.configure = async (key, value, errorContext = new Error()) => {
                let params = {
                    key: key,
                    val: value,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.configure(key, value, { from: accounts.vigorlending }) );
            }
            this.doUpdate = async (errorContext = new Error()) => {
                let params = {
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.doupdate({ from: accounts.vigorlending }) );
            }
            this.log = async (message, errorContext = new Error()) => {
                let params = {
                    message,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.log(message, { from: accounts.vigorlending }) );
            }
            this.noop = async (message, errorContext = new Error()) => {
                let params = {
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.noop({ from: accounts.vigorlending }) );
            }
            this.noop2 = async (message, errorContext = new Error()) => {
                let params = {
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.noop2({ from: accounts.vigorlending }) );
            }
            this.deleteusers = async (message, errorContext = new Error()) => {
                let params = {
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.deleteusers(accounts.vigorlending.name, { from: accounts.vigorlending }) );
            }
            this.dummyusers = async (message, errorContext = new Error()) => {
                let params = {
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.dummyusers(accounts.vigorlending.name, { from: accounts.vigorlending }) );
            }
            

        } else if(contract.name == "eosio") {
            this.regProxy = async (account, proxy, isProxy, errorContext = new Error()) => {
                let params = {
                    account,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.regrpoxy(proxy.name, isProxy, [], { from: account }) );
            }
            this.voteProducer = async (account, errorContext = new Error()) => {
                let params = {
                    account,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.voteproducer(accounts.vigorlending.name, account.name, { from: account }) );
            }
        } else {
            this.transferInsurance = async (account, value, errorContext = new Error()) => {
                return this.transfer(account, accounts.vigorlending, value, 'insurance' );
            }
            this.transferCollateral = async (account, value, errorContext = new Error()) => {
                return this.transfer(account, accounts.vigorlending, value, 'collateral' );
            }
            this.transferPayBack = async (account, value, errorContext = new Error()) => {
                return this.transfer(account, accounts.vigorlending, value, 'payback borrowed token' );

            }
            this.transfer = async (from, to, value, memo = '', permission = from, errorContext = new Error()) => {
                let params = {
                    account1: from,
                    account2: to,
                    value: value,
                    errorContext
                }
                return await help.genericBlockchainOperation( params, async () => await this.contract.transfer(from.name, to.name, value, memo, { from: permission }) );
            }
        }
    }
}



module.exports = {
  "TestHelper": TestHelper,
  "TokenHelper": TokenHelper,
  "ExpectedResults": ExpectedResults,
  "Provider": Provider,
  "eoslime": eoslime,
  "config": config
}
