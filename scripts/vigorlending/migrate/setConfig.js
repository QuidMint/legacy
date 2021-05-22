const env = require('../../.env.js')

const { api } = require('../eosjs')(env.keys, env.rpcURL)


async function init() {
  try {
    
  } catch (error) {
    console.error(error)
  }
}
init()