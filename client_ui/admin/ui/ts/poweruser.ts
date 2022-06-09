import bent from 'bent'

export function getPowerUserCurve(start: Date, end: Date, cohortStart: Date, cohortEnd: Date, mode: number): Promise<number[]> {
    let req = bent('GET', 'json', 200)

    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/pucurve`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)
        url.searchParams.append('cohortStart', `${cohortStart.getTime()}`)
        url.searchParams.append('cohortEnd', `${cohortEnd.getTime()}`)
        url.searchParams.append('mode', `${mode}`)
        req(url.href).then((resp: any) => {
            const data: number[] = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}
