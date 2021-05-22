const env = require('../.env.js')
const { api, rpc } = require('../eosjs')(env.keys, env.rpcURL)
const random = (min, max) => Math.floor(Math.random() * (Math.floor(max) - Math.ceil(min) + 1)) + Math.ceil(min)
const ms = require('human-to-milliseconds')
const ax = require('axios')
const pickWorker = () => env.workers[random(0, env.workers.length - 1)]
const tapos = {
  blocksBehind: 30,
  expireSeconds: 50,
  broadcast: true,
}

const actions = [
  {
    account: 'vigoraclehub',
    name: 'push',
    authorization: [{ actor: null, permission: 'active' }],
    data: null
  }
]

const feedurl = 'https://min-api.cryptocompare.com/data/price'

const feeds = {
  btcusd: feedurl + "?fsym=BTC&tsyms=USD&e=coinbase",
  eosusd: feedurl + "?fsym=EOS&tsyms=USD&e=coinbase",
  vigeos: "https://api.coingecko.com/api/v3/simple/price?ids=vig&vs_currencies=eos",
  eosusdt: "https://api.coingecko.com/api/v3/simple/price?ids=eos&vs_currencies=usd",
  eosvigor: "https://api.newdex.io/v1/price?symbol=eosio.token-eos-vigor",
  iqeos: "https://api.coingecko.com/api/v3/simple/price?ids=everipedia&vs_currencies=eos"
}


async function generateActions() {
  const worker = pickWorker()
  const btcusd = (await ax(feeds.btcusd)).data.USD
  const eosusd = (await ax(feeds.eosusd)).data.USD
  const vigeos = (await ax(feeds.vigeos)).data.vig.eos
  const eosusdt = (await ax(feeds.eosusdt)).data.eos.usd
  const eosvigor = (await ax(feeds.eosvigor)).data.data.price
  const iqeos = (await ax(feeds.iqeos)).data.everipedia.eos

  // console.log(eosvigor)
  const priceMult = 3

  const data = {
    owner: worker,
    quotes: [
      { value: parseInt(Math.round(btcusd * 10000) * 1), pair: "btcusd" },
      { value: parseInt(Math.round(eosusd * 10000) * 1), pair: "eosusd" },
      { value: parseInt(Math.round(vigeos * 100000000) * 1), pair: "vigeos" },
      { value: parseInt(Math.round(eosusdt * 10000) * 1), pair: "eosusdt" },
      { value: parseInt(Math.round(eosvigor * 10000) * 1), pair: "eosvigor" },
      { value: parseInt(Math.round(iqeos * 1000000) * 1), pair: "iqeos" },
    ]
  }

  const finalActions = actions
  finalActions[0].authorization[0].actor = worker
  finalActions[0].data = data
  return finalActions

}

async function init() {
  try {
    const actions = await generateActions()
    console.log(actions[0].data.quotes)
    // return
    const txResult = await api.transact({ actions }, tapos)
    console.log(txResult.transaction_id)
  } catch (error) {
    console.error(error.toString())
  }
}
init().catch(console.error)
setInterval(init, ms('60s'))
