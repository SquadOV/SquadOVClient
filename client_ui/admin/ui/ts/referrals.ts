export enum Referral {
    Campaign,
    User
}

export interface ReferralDatum {
    code: string
    description: string
    visits: number
    downloads: number
}

import bent from 'bent'

export function getReferralBreakdown(r: Referral): Promise<ReferralDatum[]> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/referrals/${r}`).then((resp: any) => {
            const data: ReferralDatum[] = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}

export function getAvailableReferralCodes() : Promise<string[]> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/referrals`).then((resp: any) => {
            const data: string[] = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}