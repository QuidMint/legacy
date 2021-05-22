var helper = require('./tests_helper.js');
var config = helper.config;

beforeAll(async function() {
    
    if(config.setup.eosio) {

        var result = null;

        console.log('==================[system initialization]==================');
        await help.scheduleFeatureActivation();

        console.log('  Creating system accounts');
        await Promise.all(config.system.accounts.map(account => help.createAccount(account)));


        // console.log('------------------[get accounts]------------------');
        let eosioAccount = await help.getAccount('eosio');
        let eosioTokenAccount = await help.getAccount('eosio.token');
        let eosioMsigAccount = await help.getAccount('eosio.msig');

        console.log('  Deploying eosio.msig constract');
        await help.deployContract('eosio.msig', eosioMsigAccount);
        console.log('  Deploying eosio.token constract');
        await help.deployContract('eosio.token', eosioTokenAccount);

        // console.log('------------------[get contracts]------------------');
        let msigContract = await help.getContract('eosio.msig', 'eosio.msig');
        let eosioTokenContract = await help.getContract('eosio.token', 'eosio.token');

        console.log('  Creating tokens');
        await eosioTokenContract.create('eosio', '100000000000.0000 EOS', { from: eosioTokenAccount });
        await eosioTokenContract.create('eosio', '100000000000.0000 SYS', { from: eosioTokenAccount });
        await eosioTokenContract.issue('eosio', '100000000000.0000 EOS', 'memo', { from: eosioAccount });
        await eosioTokenContract.issue('eosio', '100000000000.0000 SYS', 'memo', { from: eosioAccount });

        console.log('  Deploying system contract');
        let eosioContract = await help.deployContract('eosio.system', eosioAccount); //  We need tokens created and issued before deploying eosio.system
        global.contracts['eosio'] = eosioContract;

        //  Activate features
        if(eosioContract.activate) {
            console.log('  Activate features');
            await eosioContract.activate('f0af56d2c5a48d60a4a5b5c903edfb7db3a736a94ed589d0b797df33ff9d3e1d'); //  GET_SENDER
            await eosioContract.activate('2652f5f96006294109b3dd0bbde63693f55324af452b799ee137a81a905eed25'); //  FORWARD_SETCODE
            await eosioContract.activate('8ba52fe7a3956c5cd3a656a3174b931d3bb2abb45578befc59f283ecd816a405'); //  ONLY_BILL_FIRST_AUTHORIZER
            await eosioContract.activate('ad9e3d8f650687709fd68f4b90b41f7d825a365b02c23a636cef88ac2ac00c43'); //  RESTRICT_ACTION_TO_SELF
            await eosioContract.activate('68dcaa34c0517d19666e6b33add67351d8c5f69e999ca1e37931bc410a297428'); //  DISALLOW_EMPTY_PRODUCER_SCHEDULE
            await eosioContract.activate('e0fb64b1085cc5538970158d05a009c24e276fb94e1a0bf6a528b48fbc4ff526'); //  FIX_LINKAUTH_RESTRICTION
            await eosioContract.activate('ef43112c6543b88db2283a2e077278c315ae2c84719a8b25f25cc88565fbea99'); //  REPLACE_DEFERRED
            await eosioContract.activate('4a90c00d55454dc5b059055ca213579c6ea856967712a56017487886a4d4cc0f'); //  NO_DUPLICATE_DEFERRED_ID
            await eosioContract.activate('1a99a59d87e06e09ec5b028a9cbb7749b4a5ad8819004365d02dc4379a8b7241'); //  ONLY_LINK_TO_EXISTING_PERMISSION
            await eosioContract.activate('4e7bf348da00a945489b2a681749eb56f5de00b900014e137ddae39f48f69d67'); //  RAM_RESTRICTIONS
        }
        console.log('  Configure eosio.msig account');
        await eosioContract.setpriv('eosio.msig', 1);
        await eosioContract.init(0, '4,EOS'); // never forget that we need the eosio.rex account created before this init

        console.log('  Configure eosio.wrap account');
        let wrapAccount = await help.createStakedAccount('eosio.wrap', '50000.0000 EOS', '10000.0000 EOS', 50000000, 'eosio');
        await eosioContract.setpriv('eosio.wrap', 1);
        await help.sleep(3000);

        let wrapContract = await help.deployContract('eosio.system', wrapAccount); //  Needs +2MB (2641766 bytes) to make the deploy

        console.log("Initializing system - end");
    }

    
    
    console.log("Caching system accounts and contracts");
    cacheAccount('eosio');
    cacheAccount('eosio.token');
    cacheAccount('eosio.wrap');
    cacheAccount('eosio.msig');
    
    cacheContract('eosio.msig');
    cacheContract('eosio.token');
    cacheContract('eosio.wrap');
    
    expect(accounts['eosio']).not.toBeNull();
    expect(accounts['eosio.token']).not.toBeNull();
    expect(accounts['eosio.wrap']).not.toBeNull();
    expect(accounts['eosio.msig']).not.toBeNull();

    expect(contracts['eosio.token']).not.toBeNull();
    expect(contracts['eosio.wrap']).not.toBeNull();
    expect(contracts['eosio.msig']).not.toBeNull();
    // console.log("Caching system accounts and contracts - end");

    
    
    
    
    console.log('==================[VIGOR]==================');
    
    // console.log('Initializing VIGOR - begin');
    
    if(config.setup.vigor.contractAccounts) {
        console.log("Creating contract accounts");
        await Promise.all(config.user.contractAccounts.map(account => help.createStakedAccount(account, '500000.0000 EOS', '100000.0000 EOS', 1000000000, 'eosio')));
    }
    
    if(config.setup.vigor.feederAccounts) {
        console.log("Creating feeder accounts");
        await Promise.all(config.user.feederAccounts.map(account => help.createStakedAccount(account, '500000.0000 EOS', '100000.0000 EOS', 1000000000, 'eosio')));
    }
    
    let vigorTokenAccount = await help.getAccount('vigortoken11');
    let vigorLendingAccount = await help.getAccount('vigorlending');
    let vigTokenAccount = await help.getAccount('vig111111111');
    let dummyTokenAccount = await help.getAccount('dummytokensx');
    let oracleAccount = await help.getAccount('oracleoracl2');
    let oracleHubAccount = await help.getAccount('vigoraclehub');
    let delphiPollerAccount = await help.getAccount('delphipoller');

    if(config.setup.vigor.vigorLendingContract != "skip") {
        console.log('------------------[vigor lending]------------------');
        console.log('  deploying contract');
        vigorLendingAccount.addPermission('eosio.code');
        let vigorLendingContract = await help.deployContract('vigorlending', vigorLendingAccount); //  Needs +2MB (2444196 bytes) to make the deploy
        console.log('.done');
    }
    let vigorLendingContract = await help.getContract(vigorLendingAccount.name);
    
    if(config.setup.vigor.vigorTokenContract != "skip") {
        console.log('------------------[vigor token]------------------');
        console.log('  deploying contract');
        let vigorTokenContract = await help.deployContract('eosio.token', vigorTokenAccount); //  Needs +2MB (2444196 bytes) to make the deploy
        if(config.setup.vigor.vigorTokenContract == "create"){
            console.log('  initializing contract');
            result = await vigorTokenContract.create(vigorLendingAccount.name, '1000000000.0000 VIGOR', { from: vigorTokenAccount });
        }
        console.log('.done');
    }
    let vigorTokenContract = await help.getContract(vigorTokenAccount.name);

    if(config.setup.vigor.tokenContracts != "skip") {
        console.log('------------------[configure vig token]------------------');
        let vigTokenContract = await help.deployContract('eosio.token', vigTokenAccount);
        if(config.setup.vigor.tokenContracts == "create") {
            await vigTokenContract.create(vigTokenAccount.name, '1000000000.0000 VIG', {from: vigTokenAccount});
            await vigTokenContract.issue(vigTokenAccount.name, '1000000000.0000 VIG', 'm', {from: vigTokenAccount});
        }


        console.log('------------------[configure IQ token]------------------');
        let dummyTokenContract = await help.deployContract('eosio.token', dummyTokenAccount);
        if(config.setup.vigor.tokenContracts == "create") {
            await dummyTokenContract.create(dummyTokenAccount.name, '10000000000.000 IQ', {from: dummyTokenAccount});
            await dummyTokenContract.issue(dummyTokenAccount.name, '10000000000.000 IQ', 'm', {from: dummyTokenAccount});

            await dummyTokenContract.create(dummyTokenAccount.name, '100000000000.0000 BOID', {from: dummyTokenAccount});
            await dummyTokenContract.issue(dummyTokenAccount.name, '100000000000.0000 BOID', 'm', {from: dummyTokenAccount});
        }
        console.log('.done');
    }
    let vigTokenContract = await help.getContract(vigTokenAccount.name);
    let dummyTokenContract = await help.getContract(dummyTokenAccount.name);
    
    if(config.setup.vigor.testAccounts) {
        console.log("Creating test accounts");
        await Promise.all(config.user.testAccounts.map(account => help.createStakedAccount(account, '50.0000 EOS', '10.0000 EOS', 50000, 'eosio')));

        console.log('  making transfer to test accounts');
        const users = config.user.testAccounts;
        const contractAccounts = config.user.contractAccounts;

        console.log('  Transfering EOS');
        await Promise.all(users.map(async account => { accounts['eosio'].send(await help.getAccount(account), '1000000.0000', 'EOS', 'memo') }));
        await Promise.all(contractAccounts.map(async account => { accounts['eosio'].send(await help.getAccount(account), '1000000.0000', 'EOS', 'memo') }));
        await Promise.all(users.map(async account => {
            try {
                await contracts.eosio.delegatebw(account, account, '1000.0000 EOS', '1000.0000 EOS', 0, {from: await help.getAccount(account)});
            } catch(error) {
                console.log(error);
            }
        }));
        await Promise.all(contractAccounts.map(async account => { 
            try {
                await contracts.eosio.delegatebw(account, account, '1000.0000 EOS', '1000.0000 EOS', 0, {from: await help.getAccount(account)})
            } catch(error) {
                console.log(error);
            }
        }));


        console.log('  Transfering VIG');
        await Promise.all(users.map(async account => await vigTokenContract.transfer(vigTokenAccount.name, account, '100000.0000 VIG', 'memo', { from: vigTokenAccount })));
        console.log('  Transfering IQ');
        await Promise.all(users.map(async account => await dummyTokenContract.transfer(dummyTokenAccount.name, account, '100000.000 IQ', 'memo', { from: dummyTokenAccount })));
        console.log('  Transfering BOID');
        await Promise.all(users.map(async account => await dummyTokenContract.transfer(dummyTokenAccount.name, account, '100000.0000 BOID', 'memo', { from: dummyTokenAccount })));
        console.log('  Opening accounts in vigor');
        await Promise.all(users.map(async account => { vigorLendingContract.openaccount(account, {from: await help.getAccount(account)})}));
    }

    if(config.setup.vigor.oracleContract != "skip") {
        console.log('------------------[deploying oracle contract]------------------');
        let oracleContract = await help.deployContract('oracle', oracleAccount);
        if(oracleContract) {
            await oracleContract.configure('{}', { from: oracleAccount });
        }
    }
    let oracleContract = await help.getContract(oracleAccount.name);    
    
    if(config.setup.vigor.oracleHubContract != "skip") {
        console.log('------------------[deploying oraclehub contract]------------------');
        let oracleHubContract = await help.deployContract('vigoraclehub', oracleHubAccount);
        if(oracleHubContract) {
            console.log('  Configuring vigoraclehub contract');
            await oracleHubContract.configure('{}', { from: oracleHubAccount });
            console.log('  Approving delphipoller user');
            await oracleHubContract.approve(delphiPollerAccount.name, { from: vigorLendingAccount });
        }
        console.log('.done');
    }

    if(config.setup.vigor.delphiPollerContract != "skip") {
        console.log('------------------[deploying oraclehub contract]------------------');
        delphiPollerAccount.addPermission('eosio.code');
        try{
            let delphiPollerContract = await help.deployContract('delphipoller', delphiPollerAccount);
            if(delphiPollerContract) {
                console.log('  Updating...');
                await delphiPollerContract.update({ from: delphiPollerAccount });
            }
        } catch(error) {
            console.log(error);
        }
        console.log('.done');
    }


//     let oracleContract = await help.getContract(oracleHubAccount.name);
//     if(config.setup.vigor.oracleContract != "skip") {
//         console.log('------------------[deploying oracle contract]------------------');
//         let oracleContract = await help.deployContract('oracle', oracleHubAccount);
//         if(oracleContract) {
//             await oracleContract.configure('{}', { from: oracleHubAccount });
//         }
//     }
//     let oracleContract = await help.getContract(oracleHubAccount.name);

// //     console.log(await oracleContract.datapoints.limit(10).find());

//     if(config.setup.vigor.dataPreProcContract != 'skip') {
//         console.log('------------------[deploying datapreprocx contract]------------------');
//         let datapreprocContract = await help.deployContract('datapreproc', datapreprocAccount);
//         if(config.setup.vigor.dataPreProcContract == 'create') {
//             await datapreprocContract.addpair('eosusd', { from: datapreprocAccount });
//             await datapreprocContract.addpair('iqeos', { from: datapreprocAccount });
//             await datapreprocContract.addpair('vigeos', { from: datapreprocAccount });
//             await datapreprocContract.addpair('vigorusd', { from: datapreprocAccount });
//         }
//         console.log('.done');
//     }
//     let datapreprocContract = await help.getContract(datapreprocAccount.name);
    

    
    
    
    
    console.log('Caching VIGOR accounts');
    await Promise.all(config.user.contracts.map(account => cacheContract(account)));
    await Promise.all(config.user.testAccounts.map(account => cacheAccount(account)));
    await Promise.all(config.user.contractAccounts.map(account => cacheAccount(account)));
    await Promise.all(config.user.feederAccounts.map(account => cacheAccount(account)));
    // cacheContract('vigortoken11');
    
    // console.log('Caching vigor accounts - end');
    try{
        await contracts.eosio.regproxy("testproxy111", 1, {from: accounts.testproxy111});
        await contracts.eosio.delegatebw('vigorlending', 'vigorlending', '100.0000 EOS', '100.0000 EOS', 0, {from: accounts.vigorlending});
        await contracts.eosio.voteproducer('vigorlending', 'testproxy111', [], {from: accounts.vigorlending});
    } catch(error) {
        console.log(error);
    }
    
    console.log("Stating feeders");
    config.data.map(config => {
        console.log('starting data feeder: ', config.pair);
        help.oracleUpdates(global.accounts['feeder111111'], config);
        help.oracleUpdates(global.accounts['feeder111112'], config);
        help.oracleUpdates(global.accounts['feeder111113'], config);
    });
    
    if(config.setup.oracles.waitForFeeders) {
        await help.sleep(10000);  //  sleep here for a while...let the feeders do their work
    }


    // console.log("Waiting for feeders");
    // console.log("Updating datapreproc");
    // var aaa = await global.contracts['datapreproc2'].update({from: datapreprocAccount});
    // await contracts.vigoraclehub.update({from: accounts.vigoraclehub});
    await contracts.delphipoller.update({from: accounts.vigoraclehub});
    
    console.log('==================[system initialization finished]==================');
}, 600000);
