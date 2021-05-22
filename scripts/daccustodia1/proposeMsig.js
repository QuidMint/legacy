// const encodeContractData = require('../../encodeContractData')
const constructMsig = require('../constructMsig')
const key = require('../.env.mn.js').keys.STABLEQUAN11
const { api } = require('../eosjs')([key], "https://eos.dfuse.eosnation.io")
var crypto = require('crypto')
const fs = require('fs-extra')
const tapos = {
  blocksBehind: 20,
  expireSeconds: 30
}
const proposerAuth = [{ actor: 'stablequan11', permission: 'active' }]

function shuffleArray(array) {
  for (var i = array.length - 1; i > 0; i--) {
    var j = Math.floor(Math.random() * (i + 1))
    var temp = array[i]
    array[i] = array[j]
    array[j] = temp
  } return array
}

async function pickDustAcct() {
  /** @type {object[]} */
  const users = shuffleArray((await api.rpc.get_table_rows({ json: true, code: 'vigorlending', table: 'user', scope: 'vigorlending', limit: -1 })).rows)
  const usr = users.find(el => {
    if (
      parseFloat(el.savings) < 1 &&
      parseFloat(el.svalueofinsx) < 1 &&
      parseFloat(el.reputation.reputation) < 1 &&
      parseFloat(el.debt) < 1 &&
      parseFloat(el.valueofcol) < 1 &&
      parseFloat(el.valueofins) < 1 &&
      parseFloat(el.pcts) < 0.1 &&
      parseFloat(el.l_valueofcol) < 1
    ) return true
    else return false
  })
  console.log(usr)
  return usr
}

const methods = {
  /**
   * 
   * @param {string=} account  vigor user account name 
   */
  async kick(account) {
    if (!account) account = (await pickDustAcct()).usern
    if (!account) return console.error("No dust account found")
    console.log("kicking", account)
    const name = 'kick' + account.substr(0, 8)
    const expires = '96h'
    try {
      const actions = [
        {
          account: 'vigorlending',
          name: 'kick',
          authorization: [{ actor: 'vigorlending', permission: 'med' }],
          data: {
            usern: account
          }
        }
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      transaction.actions.push(
        {
          account: 'eosio.msig',
          name: 'approve',
          authorization: proposerAuth,
          data: {
            level: proposerAuth[0],
            proposal_name: name,
            proposer: proposerAuth[0].actor
          }
        }
      )
      let result
      const url = 'https://bloks.io/msig/' + proposerAuth[0].actor + '/' + name

      try {
        result = await api.transact(transaction, tapos)
      } catch (error) {
        console.log(error.toString())
        if (error.toString().match('proposal with the same name exists')) {
          console.log(url)
          const proposal = await api.rpc.get_table_rows({ "json": true, "code": "eosio.msig", "scope": "9014855587803054608", "table": "proposal", "table_key": "", "lower_bound": "kickn4sdygjq", "upper_bound": "", "index_position": 1, "key_type": "name", "limit": 1, "reverse": false, "show_payer": false })
          console.log(proposal.rows[0])
          fs.ensureFileSync('./msigProposalsExisting.txt')
          const existing = fs.readFileSync('./msigProposalsExisting.txt').toString()
          if (existing.match(url)) return
          else fs.appendFileSync('./msigProposalsExisting.txt', '\n' + url)
          return
        }
        else return this.kick(account)
      }
      console.log(result.transaction_id)
      console.log(url)
      return url
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
  },
  async kickMany(count) {
    let i = 0
    let urls = []
    while (i < count) {
      const result = await this.kick()
      if (result) {
        urls.push(result)
        fs.appendFileSync('./msigProposals.txt', '\n' + result)
        i++
      }
    }
    console.log(urls)
  },
  async bailout(account) {
    console.log("Bailout:", account)
    const name = 'bail' + account.substr(0, 8)
    const expires = '96h'
    try {
      const actions = [
        {
          account: 'vigorlending',
          name: 'bailout',
          authorization: [{ actor: 'vigorlending', permission: 'med' }],
          data: {
            usern: account
          }
        }
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      transaction.actions.push(
        {
          account: 'eosio.msig',
          name: 'approve',
          authorization: proposerAuth,
          data: {
            level: proposerAuth[0],
            proposal_name: name,
            proposer: proposerAuth[0].actor
          }
        }
      )
      let result
      try {
        result = await api.transact(transaction, tapos)
      } catch (error) {
        console.log(error.toString())
        if (error.toString().match('proposal with the same name exists')) return
        else return this.kick(account)
      }
      console.log(result.transaction_id)
      const url = 'https://bloks.io/msig/' + proposerAuth[0].actor + '/' + name
      console.log(url)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)
    }
  },
  async bailoutcr(value) {
    console.log("configure bailoutcr:", value)
    const name = 'adjbailoutcr'
    const expires = '96h'
    try {
      const actions = [
        {
          account: 'vigorlending',
          name: 'configure',
          authorization: [{ actor: 'vigorlending', permission: 'active' }],
          data: {
            key: 'bailoutcr',
            value: value
          }
        },
        {
          account: 'vigorlending',
          name: 'configure',
          authorization: [{ actor: 'vigorlending', permission: 'active' }],
          data: {
            key: 'bailoutupcr',
            value:value
          }
        }
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      transaction.actions.push(
        {
          account: 'eosio.msig',
          name: 'approve',
          authorization: proposerAuth,
          data: {
            level: proposerAuth[0],
            proposal_name: name,
            proposer: proposerAuth[0].actor
          }
        }
      )
      let result
      try {
        result = await api.transact(transaction, tapos)
      } catch (error) {
        console.log(error.toString())
        if (error.toString().match('proposal with the same name exists')) return
        else return this.kick(account)
      }
      console.log(result.transaction_id)
      const url = 'https://bloks.io/msig/' + proposerAuth[0].actor + '/' + name
      console.log(url)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)
    }
  },
  /**
   * 
   * @param {string} msigName 
   */
  async payout(msigName) {
    if (!msigName) return console.error("Must specify msigName")
    console.log("Creating Proposal:", msigName)
    const name = msigName.toLowerCase()
    const expires = '96h'
    let actions = []
    const tkn = { account: "vig111111111", precision: 4, symbol: "VIG" }
    const xferAuth = [{ actor: "vigordacfund", permission: "active" }]
    const payoutUsers = [
      ['johnatboid11', 1e6],
      ['touchman1313', 1e6],
      ['xxxshadowxxx', 1e6],
      ['corts1111111', 2e5],
      ['maxmaxmaxmac', 2e5],
    ]
    payoutUsers.forEach(([to,amount]) => {
      actions.push({
        account:tkn.account, name: "transfer",
        data: {
          from: "vigordacfund", to,
          "quantity": amount.toFixed(tkn.precision) + " " + tkn.symbol,
          "memo": "Vigor work payout " + msigName
        }, authorization: xferAuth
      })
    })

    try {
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      transaction.actions.push(
        {
          account: 'eosio.msig',
          name: 'approve',
          authorization: proposerAuth,
          data: {
            level: proposerAuth[0],
            proposal_name: name,
            proposer: proposerAuth[0].actor
          }
        }
      )
      let result
      const url = 'https://bloks.io/msig/' + proposerAuth[0].actor + '/' + name

      try {
        result = await api.transact(transaction, tapos)
      } catch (error) {
        console.log(error.toString())
      }
      console.log(result.transaction_id)
      console.log(url)
      return url
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
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