export enum Kpi {
    ViralCoefficient,
    TotalUsers,
    TotalVods,
    TotalMatches
}
import bent from 'bent'

export function kpiName(kpi: Kpi): string {
    switch (kpi) {
        case Kpi.ViralCoefficient:
            return 'Viral Coefficient'
        case Kpi.TotalUsers:
            return 'Total Users'
        case Kpi.TotalVods:
            return 'Total VODs'
        case Kpi.TotalMatches:
            return 'Total Matches'
    }
}

export function kpiIcon(kpi: Kpi): string {
    switch (kpi) {
        case Kpi.ViralCoefficient:
            return 'mdi-bug'
        case Kpi.TotalUsers:
            return 'mdi-account'
        case Kpi.TotalVods:
            return 'mdi-video'
        case Kpi.TotalMatches:
            return 'mdi-sword-cross'
    }
}

export function getKpi(kpi: Kpi): Promise<number> {
    let req = bent('GET', 'json', 200, window.location.origin)
    return new Promise((resolve, reject) => {
        req(`/api/kpi/${kpi}`).then((resp: any) => {
            resolve(resp)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}