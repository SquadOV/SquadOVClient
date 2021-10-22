export interface FunnelData {
    view: number
    reg: number
    login: number
    download: number
    install: number
    record: number
    active: number
}

import bent from 'bent'

export function getFunnelData(start: Date, end: Date, codes: string[], organicOnly: boolean): Promise<FunnelData> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/funnel`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)
        url.searchParams.append('codes', JSON.stringify(codes))
        url.searchParams.append('organicOnly', organicOnly ? '1' : '0')

        req(url.href).then((resp: any) => {
            const data: FunnelData = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}