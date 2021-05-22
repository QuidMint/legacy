const { api } = require('./eosjs')(null, "https://eos.dfuse.eosnation.io")
const fs = require('fs-extra')
async function fullTable(code, scope, table, limit) {
  var items = []
  const loop = async (lower_bound) => {
    const result = await api.rpc.get_table_rows({ code, scope, table, limit, lower_bound })
    items = items.concat(result.rows)
    console.log(items.length)
    if (result.more) return loop(result.next_key)
  }
  await loop()
  return items
}


const methods = {
  async findAccountActions(account) {
    const bailouts = await fullTable("vigorlending", "vigorlending", "bailout", 1000)
    console.log(bailouts.length)
    const userEntries = bailouts.filter(el => el.usern == account)
    console.log(userEntries)
  },
  async findBailoutActions(bailoutid) {
    const bailouts = await fullTable("vigorlending", "vigorlending", "bailout", 1000)
    console.log(bailouts.length)
    const userEntries = bailouts.filter(el => el.bailoutid == bailoutid)
    console.log(userEntries)
  }
}

if (require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](process.argv[3], process.argv[4], process.argv[5], process.argv[6]).catch((error) => console.error(error))
      .then((result) => console.log('Finished'))
  } else {
    console.log("Available Commands:")
    console.log(JSON.stringify(Object.keys(methods), null, 2))
  }
}