const axios = require('axios').default
const axiosCookieJarSupport = require('axios-cookiejar-support').default
const tough = require('tough-cookie')
const jwtDecode = require('jwt-decode')

class RiotRsoUser {
    constructor(username, tag, puuid) {
        this.username = username
        this.tag = tag
        this.puuid = puuid
    }
}

class RiotRsoToken {
    constructor(token, entitlements, expires) {
        this.token = token
        this.entitlements = entitlements
        this.expires = new Date()
        this.expires.setTime(this.expires.getTime() + (expires * 1000))
    }
}

class RiotRsoTokenRetriever {
    constructor(username, password) {
        this._username = username
        this._password = password
    }

    obtain() {
        return new Promise(async (resolve, reject) => {
            const authInst = axios.create({
                baseURL: 'https://auth.riotgames.com/api/v1/authorization',
            })
            axiosCookieJarSupport(authInst);
            const cookieJar = new tough.CookieJar();

            try {
                await authInst.post('', {
                    'client_id': 'play-valorant-web-prod',
                    'redirect_uri': 'https://playvalorant.com/opt_in',
                    'nonce': '1',
                    'response_type': 'token id_token',
                }, {
                    jar: cookieJar,
                    withCredentials: true,
                })

                const authResp = await authInst.put('', {
                    'type': 'auth',
                    'username': this._username,
                    'password': this._password,
                }, {
                    jar: cookieJar,
                    withCredentials: true,
                })

                // The URI holds the access token as part of the URL fragment.
                const uri = new URL(authResp.data.response.parameters.uri)
                // The result is just the same as a normal query params so just slash out the # and use it as URLSearchParams.
                const params = new URLSearchParams(uri.hash.slice(1))
                const accessToken = params.get('access_token')
                const jwtAccess = jwtDecode(accessToken)

                const accessExpires = params.get('expires_in')
                
                // Next we need to grab the entitlements token.
                const entitleResp = await axios.post('https://entitlements.auth.riotgames.com/api/token/v1', {}, {
                    headers: {
                        'Authorization': `Bearer ${accessToken}`
                    }
                })

                const entitlementToken = entitleResp.data.entitlements_token

                let ret = {
                    rso: new RiotRsoToken(accessToken, entitlementToken, accessExpires),
                    puuid: jwtAccess['sub']
                }
                resolve(ret)
            } catch (err) {
                // TOO MUCH INFO GETS DUMPED HERE SO DON'T PASS BACK ERR.
                console.log('Failed to obtain token: ', err)
                reject('Failed to obtain RSO Token')
            }
        })
    }
}

module.exports.RiotRsoTokenRetriever = RiotRsoTokenRetriever