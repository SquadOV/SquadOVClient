export interface User {
    id: number
    username: string
    email: string
    verified: boolean
    registrationTime: Date
    lastSession: Date | null
    lastActive: Date | null
    lastVod: Date | null
    accounts: {
        linkedRiot: string[]
        linkedTwitch: string[]
        wowChars: string[]
        steamAccounts: string[]
    },
    hw: {
        os: string
        cpu: string
        ramKb: number
        gpu0: string
        gpu1: string
    } | null
}


import bent from 'bent'

export function searchForUser(search: string): Promise<User | null> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/search/user`)
        url.searchParams.append('search', search)

        req(url.href).then((resp: any) => {
            const data: User | null = resp
            if (!!data) {
                data.registrationTime = new Date(data.registrationTime)
                if (!!data.lastSession) {
                    data.lastSession = new Date(data.lastSession)
                }

                if (!!data.lastActive) {
                    data.lastActive = new Date(data.lastActive)
                }

                if (!!data.lastVod) {
                    data.lastVod = new Date(data.lastVod)
                }
            }
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}