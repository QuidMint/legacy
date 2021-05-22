const { JsonRpc, Api } = require('eosjs')
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig')
const { TextDecoder, TextEncoder } = require('util')
const key = require('./.env.mn').STABLEQUAN11
const signatureProvider = new JsSignatureProvider([key])
const fetch = require('node-fetch')
var rpc = new JsonRpc('https://eos.greymass.com', { fetch })
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() })
const ms = require('human-to-milliseconds')
const tapos = {
  blocksBehind: 20,
  expireSeconds: 30
}
require('colors')
const printError = (message) => console.error("\n", message.red.bold, "\n")
/**
 * Get the top 21 Vigor Custodians
 */
async function getCustodians() {
  return (await rpc.get_table_rows({
    json: true,
    code: "daccustodia1",
    scope: "daccustodia1",
    table: "custodians",
    limit: -1
  })).rows.map(el => {
    return {
      actor: el.cust_name,
      permission: "active"
    }
  })
}
/**
 * @returns {object} action to sign 
 * @param {string} proposalName name of msig proposal
 * @param {array} actionstoSign array of actions to sign
 * @param {object} authorization auth of the msig proposer
 * @param {string} expires how long until the msig expires, formatted like '1d' or '1w'
 */
async function createmsig(proposalName, actionstoSign, authorization,expires) {
  if (!proposalName) return printError("Missing Proposal Name")
  if (!actionstoSign) return printError("Missing Actions")
  if (!authorization) return printError("Missing Authorization")
  if (!expires) expires = '3d'


  const serialized = await api.serializeActions(actionstoSign)
  const requested = await getCustodians()
  const expiration = new Date(Date.now() + ms(expires)).toISOString().split('.')[0]
  console.log("expires: ",expiration)
  const msigData = {
    proposer: authorization[0].actor,
    proposal_name: proposalName,
    requested,
    trx: {
      expiration,
      ref_block_num: 0,
      ref_block_prefix: 0,
      max_net_usage_words: 0,
      max_cpu_usage_ms: 0,
      delay_sec: 0,
      context_free_actions: [],
      actions: serialized,
      transaction_extensions: []
    }
  }
  console.log(msigData.trx.actions[0].authorization)
  console.log(authorization)
  const proposeAction = {
    actions: [{
      account: 'eosio.msig',
      name: 'propose',
      authorization,
      data: msigData,
    }]
  }
  
  return proposeAction

}

module.exports = createmsig