export enum Metrics {
    ActiveUsers,
    Churn,
    ReferralUsers,
    ReferralCampaigns,
    Games,
    Registrations,
    SquadInvites,
    Squads,
    Vods,
    LostUsers,
    AverageAge,
}

export interface MetricDatum {
    tm: Date
    data: any
    sub: string[]
}

import bent from 'bent'
import { Interval } from '@client/ts/interval'

export function getMetricData(m: Metrics, interval: Interval, start: Date, end: Date): Promise<MetricDatum[]> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/metrics/${m}?interval=${interval}&start=${start.getTime()}&end=${end.getTime()}`).then((resp: any) => {
            const data: MetricDatum[] = resp
            data.forEach((ele: MetricDatum) => {
                ele.tm = new Date(ele.tm)
            })
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}