export enum Cohorts {
    InstalledUsers,
    ActiveUsers,
    RecordingUsers
}

export interface CohortDatumContainer {
    tm: Date
    data: any[]
    size: number
}

import bent from 'bent'
import { Interval } from '@client/ts/interval'

export function getCohortData(m: Cohorts, interval: Interval, start: Date, end: Date, period: Interval, length: number): Promise<CohortDatumContainer[]> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/cohorts/${m}?interval=${interval}&start=${start.getTime()}&end=${end.getTime()}&period=${period}&length=${length}`).then((resp: any) => {
            const data: CohortDatumContainer[] = resp
            data.forEach((ele: CohortDatumContainer) => {
                ele.tm = new Date(ele.tm)
            })
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}