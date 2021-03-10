const bent = require('bent')

exports.storeAuthCookie = function(config, accessToken, response) {
    response.cookie('squadOvAdminJwt', accessToken, {
        secure: config.secure,
        httpOnly: true,
        sameSite: 'Strict',
    })
}

exports.verifyAuthCookie = async function(config, request) {
    return new Promise((resolve, reject) => {
        let token = request.cookies.squadOvAdminJwt
        if (!token)  {
            console.log('Admin cookie not found.')
            resolve(false)
        } else {
            tokenClient = bent(config.oauth.faUrl, 'GET', 200, {
                'Authorization': `Bearer ${token}`
            })

            tokenClient('/api/jwt/validate').then((resp) => {
                resolve(true)
            }).catch(async (err) => {
                resolve(false)
            })
        }
    })
}