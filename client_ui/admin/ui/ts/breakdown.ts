export enum Breakdown {
    Registration,
    WatchTimeGame,
    WatchTimeUser,
    ActiveUserGame,
}

export interface BreakdownDatumContainer {
    headers: string[]
    data: any[][]
}

import bent from 'bent'

export function getBreakdownData(b: Breakdown, start: Date, end: Date, extra: any): Promise<BreakdownDatumContainer> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/breakdown/${b}`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)

        if (!!extra) {
            for (const [key, value] of Object.entries(extra)) {
                url.searchParams.append(key, `${value}`)
            }
        }

        req(url.href).then((resp: any) => {
            const data: BreakdownDatumContainer = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}