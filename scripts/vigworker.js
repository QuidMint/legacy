const env = require('./.env')
const { api, rpc } = require('./eosjs')(env.keys, 'https://jungle3.greymass.com')

const print = (data) => console.log(data.transaction_id)
const ms = require('human-to-milliseconds')
const random = (min, max) => Math.floor(Math.random() * (Math.floor(max) - Math.ceil(min) + 1)) + Math.ceil(min)


async function doupdate(tryagain) {

  console.log('doupdate')
  const actor = 'vigorworker' + random(1, 5)
  console.log('worker:', actor)
  const transact = await api.transact({
    actions: [{
      account: 'vigorlending',
      name: 'doupdate',
      authorization: [{ actor, permission: 'active' }],
      data: {},
    }]
  }, {
    blocksBehind: 30,
    expireSeconds: 50,
    broadcast: true,

  }).catch(er => {
    console.log(er.toString())
    if (!tryagain) return
    setTimeout(() => doupdate(true), ms('10s'))
    return
  })
  if (transact) {
    console.log(transact)
    if (!tryagain) return
    setTimeout(() => doupdate(true), ms('1s'))
    for (let step = 0; step < 5; step++) {
      setTimeout(() => {
        console.log('doit', step)
        doupdate(false)
      }, ms(step * 100 + 'ms'))
    }

    return transact
  }
}
async function tick(tryagain) {

  console.log('tick')
  const actor = 'vigorworker' + random(1, 5)
  console.log('worker:', actor)
  const transact = await api.transact({
    actions: [{
      account: 'vigorlending',
      name: 'tick',
      authorization: [{ actor, permission: 'active' }],
      data: {},
    }]
  }, {
    blocksBehind: 30,
    expireSeconds: 50,
    broadcast: true,

  }).catch(er => {
    console.log(er.toString())
    if (!tryagain) return
    setTimeout(() => tick(true), ms('10s'))
    return
  })
  if (transact) {
    console.log(transact)
    if (!tryagain) return
    setTimeout(() => tick(true), ms('1s'))
    for (let step = 0; step < 5; step++) {
      setTimeout(() => {
        console.log('doit', step)
        tick(false)
      }, ms(step * 100 + 'ms'))
    }

    return transact
  }
}

doupdate(true)
tick(true)
