const env = require('../.env.js')
// const accts = require('../accounts')
// const acct = (name) => accts[env.network][name]
const { api } = require('../eosjs')(env.keys, env.rpcURL)
const tapos = { blocksBehind: 6, expireSeconds: 10 }
const addPairData = require('./addPairData.js')
const contractAccount = 'vigoraclehub'

async function doAction(name, data, auth) {
  try {
    if (!data) data = {}
    const contract = auth
    const authorization = [{ actor: auth, permission: 'active' }]
    const account = contract
    const result = await api.transact({ actions: [{ account, name, data, authorization }] }, tapos)
    console.log(result)
    return result
  } catch (error) {
    console.error(error.toString())
  }
}

async function addpair() {
  doAction('addpair', addPairData, contractAccount).then(result => console.log(result))

}
async function switchpair(name, active) {
  doAction('switchpair', { name, active: Boolean(active) }, contractAccount).then(result => console.log(result))
}


const methods = { addpair, switchpair }
module.exports = methods

if (process.argv[2]) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](process.argv[3], process.argv[4], process.argv[5], process.argv[6]).catch((error) => console.log(error))
      .then((result) => console.log('Finished'))
  }
}