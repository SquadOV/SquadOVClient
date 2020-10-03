const crypto = require('crypto')

function createAes256KeyFromUserPassword() {
    const pw = process.env.SQUADOV_PASSWORD
    const hash = crypto.createHash('sha256')
    hash.update(pw)
    return Buffer.from(hash.digest('base64'), 'base64')
}

module.exports.encryptPassword = function(pw) {
    const key = createAes256KeyFromUserPassword()
    const iv = crypto.randomBytes(32).toString('base64')
    let cipher = crypto.createCipheriv('aes-256-gcm', key, iv)

    let enc = cipher.update(pw, 'utf8', 'base64')
    enc += cipher.final('base64')

    const tag = cipher.getAuthTag().toString('base64')
    return `${enc}.${iv}.${tag}`
}

module.exports.decryptPassword = function(pw) {
    const key = createAes256KeyFromUserPassword()
    const [enc, iv, tag] = pw.split('.')
    let cipher = crypto.createDecipheriv('aes-256-gcm', key, iv)
    cipher.setAuthTag(Buffer.from(tag, 'base64'))
    let dec = cipher.update(enc, 'base64', 'utf8')
    dec += cipher.final('utf8')
    return dec
}