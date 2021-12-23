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
    Clips,
    WatchTime,
}

export interface MetricDatum {
    tm: Date
    data: any
    sub: string[]
}

import bent from 'bent'
import { Interval } from '@client/ts/interval'

export function getMetricData(m: Metrics, interval: Interval, start: Date, end: Date, extra: any): Promise<MetricDatum[]> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/metrics/${m}`)
        url.searchParams.append('interval', `${interval}`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)

        if (!!extra) {
            for (const [key, value] of Object.entries(extra)) {
                url.searchParams.append(key, `${value}`)
            }
        }

        req(url.href).then((resp: any) => {
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