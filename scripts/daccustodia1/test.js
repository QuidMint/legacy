const env = require('../.env.js')
// const accts = require('../accounts')
// const acct = (name) => accts[env.network][name]
const { api } = require('../eosjs')(env.keys, env.rpcURL)
const tapos = { blocksBehind: 12, expireSeconds: 30 }
const whitelistData = require('./whitelistData.js')
const contractAccount = 'vigorlending'
const fullKick = require('./fullKick')
const initConfig = require('./initConfig')
const customConfig = require('./customConfig')
const vigorlending = require('./do')
require('colors')
const printError = (message) => console.error("\n", message.red.bold, "\n")

const ax = require('axios')
const tokens = [
  ['EOS', 4, 'eosio.token'],
  ['IQ', 3, 'dummytokensx'],
  ['PBTC', 8, 'dummytokensx'],
  ['USDT', 8, 'dummytokensx'],
  ['VIG', 4, 'vig111111111'],
  ['VIGOR', 4, 'vigortoken11']
]


async function doAction(name, data, account, auth,fail) {
  try {
    if (!data) data = {}
    if (!account) account = contractAccount
    if (!auth) auth = account
    console.log("Do Action:",name,data)
    const authorization = [{ actor: auth, permission: 'active' }]
    const result = await api.transact({ actions: [{ account, name, data, authorization }] }, tapos)
    console.log(result.transaction_id)
    return result
  } catch (error) {
    console.error(error.toString())
    if (error.json) console.error(error.json?.error?.details[1]?.message)
    if (!fail) return doAction(name, data, account, auth)
    else throw(new Error(error.toString()))
  }
}
const getToken = (symbol) => {
  const [sym, precision, contract] = tokens.find(el => el[0] == symbol)
  return { sym, precision, contract }
}

const depositToken = (action, from) => {
      // console.log("Deposit:", action)
      return doAction('transfer', { from, to: contractAccount, quantity: `${action.quantity.toFixed(action.token.precision)} ${action.token.sym}`, memo: action.memo }, action.token.contract, from)
}

const runDeposits = async (depositActions, accountName) => {
  for (const action of depositActions) {
    await depositToken(action,accountName)
  }
}
const getFaucet = async (accountName) => {
  console.log("Pinging Faucet...")
  const result = await ax.get('https://api.boid.com/vigFaucet/' + accountName).catch(err => console.error(err.toString()))
  console.log(result?.data?.txid)
}
const getUser = async (accountName) => {
  return (await api.rpc.get_table_rows({ code: "vigorlending", scope: "vigorlending", table: "user", lower_bound: accountName, upper_bound: accountName, limit: 1 })).rows[0]
}

var methods
try {
  methods = {
    async borrowAll(accountName) {
      for (const token of tokens) {
        await this.borrow(token[0],accountName)
      }
    },
    async borrow(symbol, accountName) {
      const token = tokens.find(el => el[0] == symbol.toUpperCase())
      if (!token) return printError("Invalid token symbol: " + symbol)
      var available
        try {
          await doAction('assetout', { memo: "borrow", usern: accountName, assetout: (99999999).toFixed(token[1]) + ` ${ token[0]}` }, contractAccount, accountName,true)
        } catch (error) {
          const msg = error.toString().split("borrow")
          // console.log(msg)
          available = parseFloat(msg[1])
          console.log(available)
          if (available == null) return methods.borrow(symbol, accountName)
        }
      if (available > 0) await doAction('assetout', { memo: "borrow", usern: accountName, assetout: available.toFixed(token[1]) + ` ${token[0]}` }, contractAccount, accountName)
      else printError(`Max ${token[0]} borrow reached`)
    },
    async depositall(memo,accountName) {
      // await vigorlending.openaccount(accountName)
      for ([symbol, precision, contract] of tokens) {
        console.log(symbol)
        quantity = (await api.rpc.get_currency_balance(contract, accountName, symbol))[0]
        console.log(quantity)
        if (parseFloat(quantity) > 0 ) await depositToken({ token: getToken(symbol), memo, quantity: parseFloat(quantity) }, accountName)
        else continue
      }
    },
    async setup1(accountName) {
      if (!accountName) return printError(`Error: Must specify target account name`)
      await getFaucet(accountName)
      await vigorlending.openaccount(accountName)
      await runDeposits([
        { token: getToken('VIG'), memo: "collateral", quantity: 30000 },
        { token: getToken('IQ'), memo: "collateral", quantity: 10000 },
        { token: getToken('PBTC'), memo: "collateral", quantity: 0.001 },
        { token: getToken('USDT'), memo: "collateral", quantity: 1000 },
        { token: getToken('VIG'), memo: "insurance", quantity: 10000 },
        { token: getToken('IQ'), memo: "insurance", quantity: 10000 },
        { token: getToken('PBTC'), memo: "insurance", quantity: 0.001 },
        // { token: getToken('VIGOR'), memo: "savings", quantity: 1000 }
      ], accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "444.1000 VIGOR" }, contractAccount, accountName)
      await depositToken({ token: getToken('VIGOR'), memo: "collateral", quantity: 100 }, accountName )
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "24.100 IQ" }, contractAccount, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "24.1000 VIG" }, contractAccount, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "0.00010000 PBTC" }, contractAccount, accountName)
      // await vigorlending.kick(accountName)
    },
    async setup2(accountName) {
      if (!accountName) return printError(`Error: Must specify target account name`)
      getFaucet(accountName).catch(er => console.error(er.toString()))
      getFaucet(accountName).catch(er => console.error(er.toString()))
      await getFaucet(accountName).catch(er => console.error(er.toString()))
      await vigorlending.openaccount(accountName)
      await runDeposits([
        { token: getToken('VIG'), memo: "collateral", quantity:280000 },
        { token: getToken('IQ'), memo: "collateral", quantity: 40000 },
        { token: getToken('PBTC'), memo: "collateral", quantity: 0.00155 },
        { token: getToken('USDT'), memo: "collateral", quantity: 6000 },
        { token: getToken('VIG'), memo: "insurance", quantity: 1800 },
        { token: getToken('IQ'), memo: "insurance", quantity: 10000 },
        { token: getToken('PBTC'), memo: "insurance", quantity: 0.001 },
        // { token: getToken('VIGOR'), memo: "savings", quantity: 1000 }
      ], accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "4000.1000 VIGOR" }, contractAccount, accountName)
      await depositToken({ token: getToken('VIGOR'), memo: "collateral", quantity: 400 }, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "400.100 IQ" }, contractAccount, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "140000.1000 VIG" }, contractAccount, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "0.00200000 PBTC" }, contractAccount, accountName)
      // await vigorlending.kick(accountName)
    },
    async basicBorrowandBail1(accountName) {
      if (!accountName) return printError(`Error: Must specify target account name`)
      await vigorlending.openaccount(accountName)
      await getFaucet(accountName).catch(er => console.error(er.toString()))
      await vigorlending.returncol(accountName)
      await depositToken({ token: getToken('VIG'), memo: "vigfees", quantity: 5000 }, accountName)
      await depositToken({ token: getToken('USDT'), memo: "collateral", quantity: 500 }, accountName)
      await doAction('assetout', { memo: "borrow", usern: accountName, assetout: "100.0000 VIGOR" }, contractAccount, accountName)
      const userBefore = await getUser(accountName)
      await vigorlending.bailout(accountName)
      
      var global = {}
      while (global.step != 0) {
        console.log(global.step)
        global = (await api.rpc.get_table_rows({ table: "globalstats", json: true, code: contractAccount, scope: contractAccount, limit: 1 })).rows[0]
      }
      const userAfter = await getUser(accountName)

      const results = {
        collValueBefore: parseFloat(userBefore.valueofcol).toFixed(2),
        collValueAfter: parseFloat(userAfter.valueofcol).toFixed(2),
      }
      const difference = parseFloat(results.collValueAfter) - parseFloat(results.collValueBefore)
      const percentChange = (difference / parseFloat(results.collValueBefore)) * 100

      console.log(JSON.stringify(results,null,2))
      console.log("Difference: ",difference)
      console.log("Percent Difference: ",percentChange)

    }, async basicBorrow2(accountName) {
      if (!accountName) return printError(`Error: Must specify target account name`)
      await getFaucet(accountName).catch(er => console.error(er))
      await depositToken({ token: getToken('VIG'), memo: "collateral", quantity: 500000 }, accountName)
    }, async globalCheck(accountName) {
      if (!accountName) return printError(`Error: Must specify target account name`)
      await vigorlending.returncol(accountName)

      var globalBefore = {}
      console.log("Waiting for full update...")

      while (globalBefore.step != 0) {
        globalBefore = (await api.rpc.get_table_rows({ table: "globalstats", json: true, code: contractAccount, scope: contractAccount, limit: 1 })).rows[0]
      }

      const before = {
        col: parseFloat(globalBefore.valueofcol),
        ins: parseFloat(globalBefore.valueofins),
        debt: parseFloat(globalBefore.totaldebt),
        cryptoDebt: parseFloat(globalBefore.l_valueofcol),
        cryptoCol: parseFloat(globalBefore.l_totaldebt)
      }
      console.log(before)

      await this.basicBorrowandBail1(accountName)

      var globalAfter = {}
      console.log("Waiting for full update...")

      while (globalAfter.step != 0) {
        globalAfter = (await api.rpc.get_table_rows({ table: "globalstats", json: true, code: contractAccount, scope: contractAccount, limit: 1 })).rows[0]
        // console.log(globalAfter.step)
      }

      const after = {
        col: parseFloat(globalAfter.valueofcol),
        ins: parseFloat(globalAfter.valueofins),
        debt: parseFloat(globalAfter.totaldebt),
        cryptoDebt: parseFloat(globalAfter.l_valueofcol),
        cryptoCol: parseFloat(globalAfter.l_totaldebt)
      }
      console.log(after)
      const diff = {
        col: before.col - after.col,
        ins: before.ins - after.ins,
        debt: before.debt - after.debt,
        cryptoDebt: before.cryptoDebt - after.cryptoDebt,
        cryptoCol: before.cryptoCol - after.cryptoCol
      }
      console.log(diff)
      
    },
    async kickMany() {
      try {
        const toKick = [
          'vigorworker1',
          'vigorworker2',
          'vigorworker3',
          'vigorworker4',
          'vigorworker5'
        ]
        const authorization = [{ actor: contractAccount, permission: 'active' }]
        const actions = []

        toKick.forEach((el,i) => {
          const action = {
            authorization,
            account: contractAccount,
            name: 'kick',
            data: { usern: el,delay_sec:i*30 }
          }
          actions.push(action)
        })
        console.log(actions);
        return
        const result = await api.transact({ actions }, tapos)
        console.log(result.transaction_id)
      } catch (error) {
        console.log(error)
      }
    
    }
  }
} catch (error) {
  console.log(error.toString())
}


module.exports = methods

if (require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](process.argv[3], process.argv[4], process.argv[5], process.argv[6]).catch((error) => console.error(error))
      .then((result) => console.log('Finished'))
  } else {
    console.log("Available Commands:")
    console.log(JSON.stringify(Object.keys(methods),null,2))
  }
}