import bent from 'bent'

export function getPowerUserCurve(start: Date, end: Date): Promise<number[]> {
    let req = bent('GET', 'json', 200)

    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/pucurve`)
        url.searchParams.append('start', `${start.getTime()}`)
        url.searchParams.append('end', `${end.getTime()}`)
        req(url.href).then((resp: any) => {
            const data: number[] = resp
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}
