export interface ActivityDatum {
    x: number | string
    y: number
}

import bent from 'bent'

export function getActivityScatter(start: Date, end: Date, mode: number): Promise<ActivityDatum[]> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/activity?start=${start.getTime()}&end=${end.getTime()}&mode=${mode}`).then((resp: any) => {
            const data: ActivityDatum[] = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}