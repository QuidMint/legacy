function init(mnConfig, tnConfig, contractAccount) {
  try {
    if (!tnConfig) throw new Error("Testnet config fetch error")
    if (!mnConfig) throw new Error("Mainnet config fetch error")

    let actions = []
    const authorization = [{ actor: contractAccount, permission: 'active' }]

    const factors = {
      alphatest: 1000,
      soltarget: 10,
      lsoltarget: 10,
      maxtesprice: 10000,
      mintesprice: 10000,
      calibrate: 10,
      maxtesscale: 10,
      mintesscale: 10,
      reservecut: 100,
      savingscut: 100,
      maxlends: 100,
      maxdisc: 100,
      mincollat: 100,
      datac: 100,
      vigordaccut: 100
    }

    function handleValue(key, value) {
      if (factors[key]) {
        return value / factors[key]
      } else return value
    }

    for ([key, value] of Object.entries(mnConfig)) {
      if (tnConfig[key] == value) continue
      console.log(key, value, tnConfig[key])
      actions.push(
        {
          account: contractAccount,
          name: "configure",
          data: { key: key, value: handleValue(key,value) },
          authorization
        }
      )

    }
    return actions
  } catch (error) {
    console.error(error)
  }
}
module.exports = init