const fs = require('fs-extra')

async function init(){
  try {
    const rpc2 = require('../../eosjs')(null, "https://eos.greymass.com").rpc
    const query = { json: true, code: "vigorlending", table: "config", scope: "vigorlending" }
    const config = (await rpc2.get_table_rows(query)).rows[0]
    fs.writeJSONSync('./mnConfig.json',config)
  } catch (error) {
    console.error(error)
  }
}
init()