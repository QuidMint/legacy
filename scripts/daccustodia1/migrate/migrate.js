const constructMsig2 = require('../../constructMsig2')
const key = require('../../.env.mn.js').STABLEQUAN11
const { api } = require('../../eosjs')([key], "https://eos.greymass.com")
const tapos = {
  blocksBehind: 20,
  expireSeconds: 30
}
const proposerAuth = [{ actor: 'stablequan11', permission: 'active' }]

const methods = {
  async step1() {
    const name = 'zerovotedfix1'
    const expires = '240h'
    try {
      const actions = [
        {
          account: 'eosio.msig',
          name: 'approve',
          authorization: [{ actor: 'vigordacltd1', permission: 'active' }],
          data: {
            level: {actor:'vigordacltd1',permission:'active'},
            proposal_hash: 'e9fd84741ab81439095bd851f9bae9fcf6dd379fc05d26c13fd06c19642e9262',
            proposal_name: 'zerovotedfix',
            proposer: 'stablequan11'
          }
        }
      ]
      const transaction = await constructMsig2(name, actions, proposerAuth, expires)
      const result = await api.transact(transaction,tapos)
      console.log(result)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
  },
  async step2() {
    const name = 'zerovotedfix'
    const expires = '240h'
    const { setAbiAction, setCodeAction} = require('../../encodeContractData')
    const authorization = [{ actor: 'daccustodia1', permission: 'active' }]
    try {
      const actions = [
        setAbiAction("/home/gg/DAC/daccontracts/daccustodia1/output/mainnet/daccustodian/daccustodian.abi", authorization),
        setCodeAction("/home/gg/DAC/daccontracts/daccustodia1/output/mainnet/daccustodian/daccustodian.wasm", authorization)
      ]
       const transaction = await constructMsig(name, actions, proposerAuth, expires)
       const result = await api.transact(transaction, tapos)

       console.log(result)
    } catch (error) {
      console.error(error.toString())
    }
  },
  async step3() {
    const name = 'zerovotedfix1'
    const expires = '240h'
    const config = require('./mnConfig.json')
    const { setAbiAction, setCodeAction } = require('../../encodeContractData')
    const authorization = [{ actor: 'vigorlending', permission: 'active' }]
    try {
      const actions = [

        // {
        //   account: 'vigorlending',
        //   name: 'clearconfig',
        //   authorization: [{ actor: 'vigorlending', permission: 'active' }],
        //   data: {}
        // },
        // setAbiAction("./contracts/vigorlending.abi", authorization),
        // setCodeAction("./contracts/vigorlending.wasm", authorization),
        {
          account: 'vigorlending',
          name: 'setconfig',
          authorization: [{ actor: 'vigorlending', permission: 'active' }],
          data: { config }
        },
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      const result = await api.transact(transaction, tapos)
      console.log(result)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
  },
  async step4() {
    const name = 'vigorupstep4'
    const expires = '12h'
    try {
      const actions = [
        {
          account: 'vigorlending',
          name: 'configure',
          authorization: [{ actor: 'vigorlending', permission: 'active' }],
          data: {
            key: "freezelevel",
            value: 0
          }
        }
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      const result = await api.transact(transaction, tapos)
      console.log(result)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
  },
  async step5() {
    const name = 'vigorupstep5'
    const expires = '12h'
    try {
      const actions = [
        {
          account: 'vigorlending',
          name: 'configure',
          authorization: [{ actor: 'vigorlending', permission: 'active' }],
          data: {
            key: "freezelevel",
            value:0
          }
        }
      ]
      const transaction = await constructMsig(name, actions, proposerAuth, expires)
      const result = await api.transact(transaction, tapos)
      console.log(result)
    } catch (error) {
      console.error(error.toString())
      // console.error(error)

    }
  },
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