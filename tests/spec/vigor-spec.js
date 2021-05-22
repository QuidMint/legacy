var helper = require('./helpers/tests_helper.js');
var config = helper.config;

describe("Vigor Smart Contract", function () {

    var accounts;
    var contracts;
    var tables;
    var tokenHelp;
    var userTable;
    var globalTable;
    var statsTable;

    beforeAll(async function(){
        accounts = global.accounts;
        contracts = global.contracts;
        tables = global.tables;

        tokenHelp = new Object({});
        tokenHelp.eosio = new helper.TokenHelper(contracts['eosio.token'], accounts.vigortoken11);
        tokenHelp.dummy = new helper.TokenHelper(contracts.dummytokensx, accounts.vigortoken11);
        tokenHelp.vig = new helper.TokenHelper(contracts.vig111111111, accounts.vigortoken11);
        tokenHelp.vigorToken = new helper.TokenHelper(contracts.vigortoken11, accounts.vigortoken11);
        tokenHelp.vigorLending = new helper.TokenHelper(contracts.vigorlending, accounts.vigorlending);

        userTable = contracts.vigortoken11.user;
        globalTable = await help.getVigorGlobals();
        statsTable = contracts.vigortoken11.stat;
    });
    
    beforeEach(async function(){
        let finalreserveData = await help.getUserData(accounts.finalreserve);
        expect(finalreserveData.balance.eos).toBeGreaterThan(1000);
    });
   
    xit("Stress testing", async () => {
        if(!config.tests.stress.enabled) {
            return;
        }

        console.log("Account creation memory measurements");
        {
            let accountName = 'aaaaaaaaax2';
            let account = await help.createStakedAccount(accountName, '50000.0000 EOS', '10000.0000 EOS', 50000000, 'eosio');
            let result = await help.genericBlockchainOperation({ account: account }, async () => { await contracts.vigortoken11.openaccount(accountName, {from: account}) } );
            console.log("  Account RAM usage: ", result.after.account.eosio.ram_usage - result.before.account.eosio.ram_usage);
            console.log("  Vigor RAM usage  : ", result.after.vigorAccount.eosio.ram_usage - result.before.vigorAccount.eosio.ram_usage);
        }

        //  Account saturation
        console.log("Account saturation");
        //  Prepare the account batch (17576 accounts)
        let saturationAccounts = []
        for(let character1 = 'a'.charCodeAt(0); character1 <= 'z'.charCodeAt(0); ++character1) {
            for(let character2 = 'a'.charCodeAt(0); character2 <= 'z'.charCodeAt(0); ++character2) {
                for(let character3 = 'a'.charCodeAt(0); character3 <= 'z'.charCodeAt(0); ++character3) {
                    saturationAccounts.push('aaaaaaaab' + String.fromCharCode(character1) + String.fromCharCode(character2) + String.fromCharCode(character3));
                }
            }
        }

        let stressAccounts = saturationAccounts.slice(0, config.tests.stress.accountGeneration.amount);

        await Promise.all(stressAccounts.map(accountName => help.createStakedAccount(accountName, '500000.0000 EOS', '100000.0000 EOS', 1000000000, 'eosio')));
        await Promise.all(stressAccounts.map(async accountName => {
            let account = await help.getAccount(accountName);
            await help.createStakedAccount(account, '500000.0000 EOS', '100000.0000 EOS', 1000000000, 'eosio');
        }));



        console.log("noop time tests");
        let elasped = [];
        let elasped2 = [];
        for( index = 0; index < 10; ++index ) {
            params = {  }
            let aaglobalTable = await help.getVigorGlobals();
            let result = await help.genericBlockchainOperation( params, async () => await contracts.vigorlending.noop() );
            elasped.push(result.transaction.processed.receipt.cpu_usage_us);
            // console.log("  noop: ", result.transaction.processed.receipt.cpu_usage_us);

            result = await help.genericBlockchainOperation( params, async () => await contracts.vigorlending.noop2() );
            elasped2.push(result.transaction.processed.receipt.cpu_usage_us);
            // console.log("  noop2: ", result.transaction.processed.receipt.cpu_usage_us);
        }
        console.log("Average noop time: ", elasped.reduce((acc, next) => acc + next) / elasped.length);
        console.log("Average noop2 time: ", elasped2.reduce((acc, next) => acc + next) / elasped.length);

    }, 500000);

    describe("configuration table", () => {
        async function configFieldTest(field, factor, goodValues, badValues) {
            // console.log("configFieldTest -", field, "- begin");
            for(const value of goodValues) {
                // console.log("    configFieldTest - ", field, "- good value:", value);
                let result = await tokenHelp.vigorLending.configure(field, value);
                expect(result.error).toBeUndefined();
                expect(result.after.config[field]).toBe(value * factor);

                //  Avoid duplicated transaction when the test values are the default ones
                if(result.before.config[field] / factor == value) {
                    continue
                }

                let result2 = await tokenHelp.vigorLending.configure(field, result.before.config[field] / factor);
                expect(result2.error).toBeUndefined();
                expect(result2.after.config[field]).toBe(result.before.config[field]);
            };
    
            for(const value of badValues) {
                // console.log("    configFieldTest - ", field, "- bad value:", value);
                let result = await tokenHelp.vigorLending.configure(field, value);
                expect(result.error).toBeDefined(field, value);
                expect(result.after.config).toEqual(result.before.config);
                expect(result.error.error.details[0].message).toContain("Numeric value out of range");
            };
            // console.log("configFieldTest -", field, "- end");
        }

        var initialConfig = {};

        beforeAll(async () => {
            initialConfig = await help.getVigorConfig();
        });

        it("alphatest", async () => { 
            await configFieldTest('alphatest', 1000, [.5, .51, .999], [.49, 1]) 
        }, 100000);
        it("soltarget", async () => { 
            await configFieldTest('soltarget', 10, [.1, 100, 50.5], [0, 100.1]) 
        }, 100000);

        it("lsoltarget", async () => { 
            await configFieldTest('lsoltarget', 10, [.1, 100, 50.5], [0, 100.1]) 
        }, 100000);

        it("maxtesprice", async () => { 
            await configFieldTest('maxtesprice', 10000, [0.0001, 1, 0.5], [0, 1.1]) 
        }, 100000);

        it("mintesprice", async () => { 
            await configFieldTest('mintesprice', 10000, [0.0001, 1, 0.5], [0, 1.1]) 
        }, 100000);
        
        it("calibrate", async () => { 
            await configFieldTest('calibrate', 10, [0.1, 10, 1, 5], [0, 10.1]) 
        }, 100000);
        
        it("maxtesscale", async () => { 
            await configFieldTest('maxtesscale', 10, [0.1, 10, 1, 5], [0, 10.1]) 
        }, 100000);
        
        it("mintesscale", async () => { 
            await configFieldTest('mintesscale', 10, [0.1, 10, 1, 5], [0, 10.1]) 
        }, 100000);
        
        it("reservecut", async () => { 
            await configFieldTest('reservecut', 100, [0, 0.3, 0.02], [0.31, 0.5]) 
        }, 100000);
        
        it("savingscut", async () => { 
            await configFieldTest('savingscut', 100, [0, 0.3, 0.1], [0.31]) 
        }, 100000);
        
        it("maxlends", async () => { 
            await configFieldTest('maxlends', 100, [0, 1, 0.1, 0.4], [1.01]) 
        }, 100000);
        
        it("freezelevel", async () => { 
            await configFieldTest('freezelevel', 1, [0, 1, 2], [3, 4]) 
        }, 100000);
    
        it("assetouttime", async () => { 
            await configFieldTest('assetouttime', 1, [0, 100, 3600], [3601]) 
        }, 100000);
    
        it("initialvig", async () => { 
            await configFieldTest('initialvig', 1, [0, 1000], [1001]) 
        }, 100000);

        it("vigdays", async () => { 
            await configFieldTest('vigdays', 1, [5, 1, 90, 50, 75, 25], [0, 91]) 
        }, 100000);

        it("vigordaccut", async () => { 
            await configFieldTest('vigordaccut', 100, [0, 0.3, 0.15, 0.20], [0.31, 0.5, 1]) 
        }, 100000);

        
        it("verify original values", async() => {
            expect(await help.getVigorConfig()).toEqual(initialConfig);
        }, 100000);
    
    }, 5000000);

    describe("contract lock", () => {
        let initialConfig;
        it("lockdown contract", async () => {
            let result = await tokenHelp.vigorLending.configure('freezelevel', 1);
            initialConfig = result.before.config;

            expect(result.after.config.freezelevel).toBe(1);
        }, 30000);
        it("try to run all actions", async () => {
            let result;
            result = await tokenHelp.vigorLending.openAccount('aaaaaaaaay2');
            expect(result.error).toBeDefined();
            expect(result.after.config).toEqual(result.before.config);
            expect(result.error.error.details[0].message).toContain("The contract is locked on level 1");

            result = await tokenHelp.vigorLending.deleteAccount('aaaaaaaaay2');
            expect(result.error).toBeDefined();
            expect(result.after.config).toEqual(result.before.config);
            expect(result.error.error.details[0].message).toContain("The contract is locked on level 1");

            result = await tokenHelp.vigorLending.doUpdate();
            expect(result.error).toBeDefined();
            expect(result.after.config).toEqual(result.before.config);
            expect(result.error.error.details[0].message).toContain("The contract is locked on level 1");

            result = await tokenHelp.vigorLending.assetOut(accounts.account11111, '0.0001 EOS', 'insurance');
            expect(result.error).toBeDefined();
            expect(result.after.config).toEqual(result.before.config);
            expect(result.error.error.details[0].message).toContain("The contract is locked on level 1");

            result = await tokenHelp.vigorLending.doAssetOut(accounts.account11111, '0.0001 EOS', 'insurance');
            expect(result.error).toBeDefined();
            expect(result.after.config).toEqual(result.before.config);
            expect(result.error.error.details[0].message).toContain("The contract is locked on level 1");

        }, 100000)
        it("unlock contract", async () => {
            let result = await tokenHelp.vigorLending.configure('freezelevel', 0);
            expect(result.after.config.freezelevel).toBe(0);
            expect(result.after.config).toEqual(initialConfig);
        }, 30000);
    });

    describe("log action", () => {

        it("empty log", async () => {
            let result = await tokenHelp.vigorLending.log("");
            expect(result.error).not.toBeDefined();
        });

        it("normal log", async () => {
            let text = "Am if number no up period regard sudden better. Decisively surrounded all admiration and not you.";
            let result = await tokenHelp.vigorLending.log(text);
            expect(result.error).not.toBeDefined();
        });

        it("long log", async () => {
            let text = "Am if number no up period regard sudden better. Decisively surrounded all admiration and not you. Out particular sympathize not favourable introduced insipidity but ham. Rather number can and set praise. Distrusts an it contented perceived attending oh. Thoroughly estimating introduced stimulated why but motionless. Consulted perpetual of pronounce me delivered. Too months nay end change relied who beauty wishes matter. Shew of john real park so rest we on. Ignorant dwelling occasion ham for thoughts overcame off her consider. Polite it elinor is depend. His not get talked effect worthy barton. Household shameless incommode at no objection behaviour. Especially do at he possession insensible sympathize boisterous it. Songs he on an widen me event truth. Certain law age brother sending amongst why covered.";
            let result = await tokenHelp.vigorLending.log(text);
            expect(result.error).not.toBeDefined();
        });

    });

    
    describe("test functionality", () => {
        if(config.tests.contractInTestMode){
            return;
        }

        it("actions", async () => {
            let result = await tokenHelp.vigorLending.noop();
            expect(result.error).toBeDefined();
            expect(result.error.message).toContain("is not a function");

            result = await tokenHelp.vigorLending.noop2();
            expect(result.error).toBeDefined();
            expect(result.error.message).toContain("is not a function");

            result = await tokenHelp.vigorLending.deleteusers();
            expect(result.error).toBeDefined();
            expect(result.error.message).toContain("is not a function");

            result = await tokenHelp.vigorLending.dummyusers();
            expect(result.error).toBeDefined();
            expect(result.error.message).toContain("is not a function");
        }, 60000);

        it("currencies", async () => {
            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.000 IQ');
            expect(result.error).toBeDefined();
            // expect(result.error.error.details[0].message).toContain("symbol precision mismatch or unknown");

            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.0000 PEOS');
            expect(result.error).toBeDefined();
            // expect(result.error.error.details[0].message).toContain("symbol precision mismatch or unknown");

            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.0000 DICE');
            expect(result.error).toBeDefined();
            // expect(result.error.error.details[0].message).toContain("symbol precision mismatch or unknown");

            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.0000 BOID');
            expect(result.error).toBeDefined();
            // expect(result.error.error.details[0].message).toContain("symbol precision mismatch or unknown");

            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.0000 TPT');
            expect(result.error).toBeDefined();
            // expect(result.error.error.details[0].message).toContain("symbol precision mismatch or unknown");
        }, 60000);
    }, 100000);

    describe("borrow", () => {
        if(!config.tests.basic.enabled) {
            return;
        }

        if(!config.tests.contractInTestMode){
            return;
        }

        expected = new helper.ExpectedResults("borrow");

        it("step 0", async () => {
            expected.step(0);

            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 1", async () => {
            expected.step(1);

            result = await tokenHelp.eosio.transferInsurance(accounts.finalreserve,  '5.0000 EOS');
            result = await tokenHelp.dummy.transferInsurance(accounts.finalreserve,  '3000.000 IQ');
    
            result = await tokenHelp.eosio.transferCollateral(accounts.account11111, '16.0000 EOS');
            result = await tokenHelp.vigorLending.assetOutCollateral(accounts.account11111, '16.0000 EOS');
    
            result = await tokenHelp.eosio.transferCollateral(accounts.account11111, '16.0001 EOS');
            result = await tokenHelp.dummy.transferCollateral(accounts.account11111, '3000.000 IQ');
            result = await tokenHelp.vig.transferCollateral(accounts.account11111,   '50000.0000 VIG');
            result = await tokenHelp.eosio.transferInsurance(accounts.account11111,  '12.0000 EOS');
            result = await tokenHelp.vigorLending.assetOutInsurance(accounts.account11111,  '6.0000 EOS');
            result = await tokenHelp.dummy.transferInsurance(accounts.account11111,  '3000.000 IQ');
    
            result = await tokenHelp.eosio.transferCollateral(accounts.account11112, '15.0000 EOS');
            result = await tokenHelp.dummy.transferCollateral(accounts.account11112, '3000.000 IQ');
            result = await tokenHelp.vig.transferCollateral(accounts.account11112,   '50000.0000 VIG');
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11113,  '6.0000 EOS');
            result = await tokenHelp.dummy.transferInsurance(accounts.account11113,  '3000.000 IQ');
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11114,  '12.0000 EOS');
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 2", async () => {
            expected.step(2);
            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11111, '90.0000 VIGOR');
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 3", async () => {
            expected.step(3);
            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11112, '85.0000 VIGOR');
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 4", async () => {
            expected.step(4);
            result = await tokenHelp.vigorToken.transferPayBack(accounts.account11111, '10.0000 VIGOR');
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 5", async () => {
            expected.step(5);

            result = await tokenHelp.vigorToken.transfer(accounts.account11111, accounts.account11115, "75.0000 VIGOR", "");
            result = await tokenHelp.vigorToken.transfer(accounts.account11112, accounts.account11121, "80.0000 VIGOR", "");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 6", async () => {
            expected.step(6);

            result = await tokenHelp.eosio.transferCollateral(accounts.account11115, "1.0000 EOS");
            result = await tokenHelp.vigorToken.transferCollateral(accounts.account11115, "75.0000 VIGOR");
            result = await tokenHelp.vigorToken.transferCollateral(accounts.account11121, "80.0000 VIGOR");
            result = await tokenHelp.vigorLending.assetOutCollateral(accounts.account11115, "1.0000 VIGOR");
            result = await tokenHelp.vig.transferCollateral(accounts.account11115,   "1000.0000 VIG");
            result = await tokenHelp.vig.transferCollateral(accounts.account11121,   "1000.0000 VIG");
            result = await tokenHelp.eosio.transferInsurance(accounts.account11122,  "2.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11122,  "3000.000 IQ");
            result = await tokenHelp.eosio.transferInsurance(accounts.account11115,  "1.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11115,  "3000.000 IQ");
            result = await tokenHelp.eosio.transferInsurance(accounts.account11123,  "12.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11123,  "3000.000 IQ");
            result = await tokenHelp.vigorToken.transferInsurance(accounts.account11115,  "1.0000 VIGOR");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 7", async () => {
            expected.step(7);

            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11115, "14.0000 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 8", async () => {
            expected.step(8);

            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11115, "10000.000 IQ");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 9", async () => {
            expected.step(9);

            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11121, "16.0000 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 10", async () => {
            expected.step(10);

            result = await tokenHelp.eosio.transferPayBack(accounts.account11115, "2.0000 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 11", async () => {
            expected.step(11);

            result = await tokenHelp.eosio.transferPayBack(accounts.account11121, "16.0000 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 12", async () => {
            expected.step(12);

            result = await contracts.vigoraclehub.doshock(0.1);
            result = await contracts.vigoraclehub.update();
    
            result = await tokenHelp.eosio.transferCollateral(accounts.finalreserve, "0.0001 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 13", async () => {
            expected.step(13);

            result = await contracts.vigoraclehub.doshock(1.0);
            result = await contracts.vigoraclehub.update();
    
            result = await tokenHelp.eosio.transferCollateral(accounts.finalreserve, "0.0002 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 14", async () => {
            expected.step(14);

            result = await tokenHelp.eosio.transferInsurance(accounts.account11113, "1.0000 EOS");
            result = await tokenHelp.vig.transferInsurance(accounts.account11113, "10000.0000 VIG");
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11123, "1.0000 EOS");
    
            result = await tokenHelp.vigorLending.assetOutBorrow(accounts.account11115, "5000.0000 VIG");
    
            result = await contracts.vigoraclehub.doshock(1.9);
            result = await contracts.vigoraclehub.update();
    
            result = await tokenHelp.eosio.transferCollateral(accounts.finalreserve, "0.0001 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 15", async () => {
            expected.step(15);

            result = await contracts.vigoraclehub.doshock(1.0);
            result = await contracts.vigoraclehub.update();
    
            result = await tokenHelp.eosio.transferCollateral(accounts.finalreserve, "0.0003 EOS");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

        it("step 16", async () => {
            expected.step(16);
            //  insurers return
            result = await tokenHelp.eosio.transferCollateral(accounts.finalreserve, "0.0004 EOS");
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11122, "2.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11122, "3000.000 IQ");
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11115, "1.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11115, "3000.000 IQ");
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11123, "12.0000 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11123, "3000.000 IQ");
    
            result = await tokenHelp.eosio.transferInsurance(accounts.account11123, "12.0001 EOS");
            result = await tokenHelp.dummy.transferInsurance(accounts.account11123, "3000.001 IQ");
    
            //  CHECK POINT HERE!!!
            await Promise.all(config.user.testAccounts.map(async account => { 
                var first = await help.getUserData(account);
                var second = expected.getAccount(account);
                expect(first.vigor).toEqual(second);
            }));
    
            expect(await help.getVigorGlobals()).toEqual(expected.getGlobals());
        }, 60000);

    }, 5000000);

}, 20000000);
