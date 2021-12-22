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

export function createCampaign(code: string, desc: string): Promise<void> {
    let req = bent('POST', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/campaign`, { code, desc }).then(() => {
            resolve()
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}

export interface ReferralFlowData {
    steps: string[]
    links: {
        source: string
        target: string
        value: number
    }[]
}

export function getReferralFlow(start: Date, end: Date): Promise<ReferralFlowData> {
    let req = bent('GET', 'json', 200)

    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/referralflow`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)
        req(url.href).then((resp: any) => {
            const data: ReferralFlowData = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}