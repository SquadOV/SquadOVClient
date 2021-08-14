import { Oembed } from './oembed'

export interface Vod {
    videoUuid: string
    userUuid: string
    matchUuid: string
    isClip: boolean
    isLocal: boolean
    isProcessed: boolean
    startTime: Date
    endTime: Date | null
    resX: number
    resY: number
    fps: number
    shareToken: string | null
    oembed: Oembed
}

import bent from 'bent'

export function searchForVod(video: string, user: string): Promise<Vod[]> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/search/vod`)
        url.searchParams.append('video', video)
        url.searchParams.append('user', user)

        req(url.href).then((resp: any) => {
            let data: Vod[] = resp
            data.forEach((v: Vod) => {
                v.startTime = new Date(v.startTime)
                if (!!v.endTime) {
                    v.endTime = new Date(v.endTime)
                }
            })
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}