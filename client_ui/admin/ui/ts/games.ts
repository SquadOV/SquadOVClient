export interface Game {
    matchUuid: string
    userUuid: string
    videoUuid: string | null
    time: Date
}

export interface AimlabGame extends Game {
    task: string
    score: number
}

export interface CsgoGame extends Game {
    map: string
    mode: string
    hasDemo: boolean
}

export interface HearthstoneGame extends Game {
}

export interface LolGame extends Game {
    riotId: String
}

export interface TftGame extends Game {
    riotId: String
}

export interface ValorantGame extends Game {
    map: string
    riotId: String
    mode: string
    isRanked: boolean
}

export interface WowGame extends Game {
    gameVersion: string
    arena: {
        type: string
    } | null,
    challenge: {
        name: string
        keystone: number
        success: boolean
    } | null,
    encounter: {
        name: string
        difficulty: number
        success: boolean
    } | null
}

import bent from 'bent'

export function searchForGames<T extends Game>(gn: string, data: any): Promise<T[]> {
    let req = bent('GET', 'json', 200)
    return new Promise((resolve, reject) => {
        let url = new URL(`${window.location.origin}/api/search/game/${gn}`)
        if (!!data) {
            for (const [key, value] of Object.entries(data)) {
                url.searchParams.append(key, `${value}`)
            }
        }
        req(url.href).then((resp: any) => {
            let data: T[] = resp
            data.forEach((ele: T) => {
                ele.time = new Date(ele.time)
            })
            resolve(data)
        }).catch((err: any) => {
            const txt = err.text()
            reject(txt)
        })
    })
}