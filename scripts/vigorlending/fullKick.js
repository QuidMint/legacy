function init(usern) {
  try {
    const contract = contractAccount
    const authorization = [{ actor: contractAccount, permission: 'active' }]
    const account = contract
    const actions = [
      {
        account: contract,
        name: "kick",
        data: { usern: 'vigorworker2' },
        authorization
      },
      {
        account: contract,
        name: "kick",
        data: { usern: 'vigorworker3' },
        authorization
      },
    ]

    return actions
  } catch (error) {
    console.error(error)
  }
}
module.exports = init