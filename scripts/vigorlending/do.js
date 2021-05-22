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
function funcName() { return funcName.caller.name }
const ax = require('axios')
const times = x => f => {
  if (x > 0) {
    f()
    times(x - 1)(f)
  }
}
const tokens = [
  ['EOS', 4, 'eosio.token'],
  ['BOID', 4, 'dummytokensx'],
  ['IQ', 3, 'dummytokensx'],
  ['PBTC', 8, 'dummytokensx'],
  ['USDT', 8, 'dummytokensx'],
  ['VIG', 4, 'vig111111111'],
  ['VIGOR', 4, 'vigortoken11']
]

async function doAction(name, data, account, auth) {
  try {
    if (!data) data = {}
    if (!account) account = contractAccount
    if (!auth) auth = account
    console.log("Do Action:", name, data)
    const authorization = [{ actor: auth, permission: 'active' }]
    const result = await api.transact({
      "delay_sec": 0,
       actions: [{ account, name, data, authorization }] }, tapos)
    const txid = result.transaction_id
    console.log(txid)
    // const error = await ax.get('https://jungle.dfuse.eosnation.io/v0/transactions/' + txid)
    // console.log(JSON.stringify(error,null,2))
    // console.log(result.transaction_id)
    return result
  } catch (error) {
    console.error(error.toString())
    if (error.json) console.error("Logs:",error.json?.error?.details[1]?.message)
  }
}

const methods = {

  async whitelist() {
    await doAction('whitelist', whitelistData, contractAccount).then(result => console.log(result))

  },
  async configure(key, value) {
    await doAction('configure', { key, value }, contractAccount).then(result => console.log(result))

  },
  async kick(usern) {
    await doAction('kick', { usern,delay_sec:0 }, contractAccount).then(result => console.log(result))
  },
  async fullkick(usern) {
    try {
      const actions = fullKick(usern)
      const result = await api.transact({ actions }, tapos)
      console.log(result)
    } catch (error) {
      console.error(error.toString())
    }
  },
  async deleteacnt(owner) {
    await doAction('dodeleteacnt', { owner }, contractAccount).then(result => console.log(result))
  },
  async returncol(usern) {
    await doAction('returncol', { usern }, contractAccount).then(result => console.log(result))
  },

  async returnins(usern) {
    await doAction('returnins', { usern }, contractAccount).then(result => console.log(result))
  },
  async predoupdate(step) {
    await doAction('predoupdate', { step })
  },
  async doupdate() {
    await doAction('doupdate')
  },
  async setconfig() {
    const config = require('./migrate/mnConfig.json')
    await doAction('setconfig',{config})
  },
  async tick() {
    await doAction('tick')
  },
  async liquidate(usern) {
    await doAction('liquidate', { usern })
  },
  async liquidateup(usern) {
    await doAction('liquidateup', { usern })
  },
  async bailout(usern) {
    await doAction('bailout', { usern })
  },
  async bailoutup(usern) {
    await doAction('bailoutup', { usern })
  },
  async dbgresetall(numrows) {
    if (!numrows) numrows = 500
    for (var i = 0; i < 40; i++) { await doAction('dbgresetall', { numrows }) }
  },
  async syncConfig(config) {
    if (!config) {
      const rpc2 = require('../eosjs')(null, "https://eos.greymass.com").rpc
      const query = { json: true, code: "vigorlending", table: "config", scope: "vigorlending" }
      const [mnConfig, tnConfig] = await Promise.all([(await rpc2.get_table_rows(query)).rows[0], (await api.rpc.get_table_rows(query)).rows[0]])
      const actions = initConfig(mnConfig, tnConfig, contractAccount)
      if (actions.length == 0) return console.log("All config params match")
      const result = await api.transact({ actions }, tapos).catch(el => console.error(el.toString()))
      if (result) console.log(result.transaction_id)
    }
  },
  async syncWhitelist(config) {
    if (!config) {
      const rpc2 = require('../eosjs')(null, "https://eos.greymass.com").rpc
      const query = { json: true, code: "vigorlending", table: "whitelist", scope: "vigorlending" }
      const whitelist = (await rpc2.get_table_rows(query)).rows
      // console.log(whitelist)
      function getTknContract(token) {
        const symbol = token.sym.split(',')[1]
        console.log(symbol)
        const [sym,precision,contract] = tokens.find(el => el[0] == symbol)
        console.log(contract)
        return [sym, precision, contract]
      }
      for (const token of whitelist) {
        const [symb, precision, contract] = getTknContract(token)
        const sym = `${precision},${symb}`
        console.log(sym)
        await doAction('whitelist', { sym, contract: contract, feed: token.feed, assetin: token.assetin, assetout: token.assetout, maxlends: token.maxlends })
      }
      return
    }
  },
  async fundstoFaucet(token,quantity) {
    for ([symbol, precision, contract] of tokens) {
      if (token && token != symbol) continue
      if (!quantity) quantity = (await api.rpc.get_currency_balance(contract, contractAccount, symbol))[0]
      else quantity = `${parseFloat(quantity).toFixed(precision)} ${token}`
      console.log(quantity)
      if (parseFloat(quantity) > 0) await doAction('transfer', { from: contractAccount, to: 'dummytokensx', quantity, memo: "refund faucet" }, contract, contractAccount)
    }
  },
  async unstakeRex(percent) {
    if (!percent) percent = 100
    const rexbal = (await api.rpc.get_table_rows({ code: 'eosio', scope: "eosio", table: "rexbal", lower_bound: contractAccount, upper_bound: contractAccount })).rows[0].matured_rex
    if (rexbal > 0) await doAction('sellrex', { from: contractAccount, rex: `${((rexbal / 10000) * (percent/100)).toFixed(4)} REX` }, 'eosio', contractAccount)
    const rexliq = (await api.rpc.get_table_rows({ code: 'eosio', scope: "eosio", table: "rexfund", lower_bound: contractAccount, upper_bound: contractAccount })).rows[0].balance
    console.log(rexliq)
    if (parseFloat(rexliq) > 0) await doAction('withdraw', { owner: contractAccount, amount: rexliq }, 'eosio', contractAccount)
  },
  async openaccount(owner) {
    await doAction('openaccount', { owner })
  },
  async dbgclrqueue(numrows) {
    if (!numrows) numrows = 1000
    await doAction('dbgclrqueue', { numrows })
  },
  async clearconfig() {
    
    await doAction('clearconfig')
  },
  async freezelevel(value) {
    await doAction('freezelevel', { value})
  },
  async setCustomConfig() {
    for ([key, value] of Object.entries(customConfig)) {
      console.log(key, value)
      await doAction('configure', { key, value })
    }
  },
  async dbggivevig() {
    await doAction(funcName(), { quantity:1, usrn:"vigorworker2", memo:"insurance"},null,'vigorworker2')
  },
  async fullReset() {
    await this.dbgresetall()
    await this.unstakeRex(20) 
    await this.fundstoFaucet()
    await this.syncConfig()
    await this.setCustomConfig()
    await this.syncWhitelist()
    await this.openaccount('vigordacfund')
    await this.openaccount('finalreserve')
  },
  async multiKick() {
    const contract = contractAccount
    const authorization = [{ actor: contractAccount, permission: 'active' }]
    const actions = [
      {
        account: contract,
        name: "kick",
        data: { usern: 'vigorworker4' },
        authorization
      },
      {
        account: contract,
        name: "kick",
        data: { usern: 'vigorworker5' },
        authorization
      },
    ]
    const result = await api.transact({ actions }, tapos)
    console.log(result)
  }
}

module.exports = methods

if (process.argv[2] && require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](process.argv[3], process.argv[4], process.argv[5], process.argv[6]).catch((error) => console.log(error))
      .then((result) => console.log('Finished'))
  }
}