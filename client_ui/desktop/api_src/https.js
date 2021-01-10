const forge = require('node-forge')

module.exports.generateSelfSignedKeyCert = function() {
    const pki = forge.pki

    let keys = pki.rsa.generateKeyPair(4096)
    let cert = pki.createCertificate()
    cert.publicKey = keys.publicKey
    cert.serialNumber = '01337'
    cert.validity.notBefore = new Date()
    cert.validity.notAfter = new Date()
    cert.validity.notAfter.setUTCFullYear(cert.validity.notAfter.getUTCFullYear() + 1)

    const attrs = [
        {
            name: 'commonName',
            value: '127.0.0.1',
        },
        {
            name: 'countryName',
            value: 'US',
        },
        {
            shortName: 'ST',
            value: 'New Jersey',
        },
        {
            shortName: 'O',
            value: 'SquadOV',
        },
        {
            shortName: 'OU',
            value: 'Client',
        },
        {
            shortName: 'E',
            value: 'mike.h.bao@gmail.com',
        },
    ]
    cert.setSubject(attrs)
    cert.setIssuer(attrs)
    cert.sign(keys.privateKey)
    return {
        key: pki.privateKeyToPem(keys.privateKey),
        cert: pki.certificateToPem(cert),
    }
}