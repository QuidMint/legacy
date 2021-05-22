var spawn = require('child-process-promise').spawn
const fs = require('fs-extra')
const keys = require('./.env.js').keys
const { api } = require('./eosjs')(keys, "https://jungle3.greymass.com")
const { setAbiAction, setCodeAction } = require('./encodeContractData')

const tapos = {
  blocksBehind: 20,
  expireSeconds: 30
}

/**
 * @param {string} exec native exec
 * @param {string[]} params parameters to pass to executable
 */
async function runCommand(exec, params,options) {

  var promise = spawn(exec, params, options)

  var childProcess = promise.childProcess

  console.log('[spawn] childProcess.pid: ', childProcess.pid)

  childProcess.stdout.on('data', function (data) {
    console.log('stdout: ', data.toString())
  })
  childProcess.stderr.on('data', function (data) {
    console.log('stderr: ', data.toString())
  })

  return promise
}


const methods = {
  async debug(name) {
    try {
      const exists = await fs.pathExists('../build/Makefile')
      if(!exists) {
        await fs.ensureDir('../build')
        await fs.emptyDir('../build')
        await runCommand('cmake', ['..'],{cwd:"../build"}).catch(err => {throw new Error("Error setting up makefiles, make sure cmake is isntalled! \n "+err.toString())})
      } 
      await runCommand('make', [name + '.wasm'], { cwd: "../build" })
      const authorization = [{ actor: 'vigorlending', permission: 'active' }]

      const actions = [
        setAbiAction(`../build/contracts/${name}/${name}.abi`, authorization),
        setCodeAction(`../build/contracts/${name}/${name}.wasm`, authorization)
      ]
      
      const result = await api.transact({actions}, tapos)
      console.log(result.transaction_id)

    } catch (error) {
      console.error(error.toString())
    }

  },
  async prod() {

  }
}



if (require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](...process.argv.slice(3)).catch((error) => console.error(error))
      .then((result) => console.log('Finished'))
  } else {
    console.log("Available Commands:")
    console.log(JSON.stringify(Object.keys(methods), null, 2))
  }
}