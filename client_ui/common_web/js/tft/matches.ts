import { TftTrait } from '@client/js/tft/traits'
import { TftUnit } from '@client/js/tft/units'

export interface TftPlayerMatchSummary {
    matchUuid: string,
    gameDatetime: Date,
    gameLength: number,
    gameVariation: string | null,
    gameVersion: string,
    queueId: number,
    tftSetNumber: number,
    companion: TftCompanion,
    level: number,
    placement: number,
    traits: TftTrait[],
    units: TftUnit[],
    hasVod: boolean
}

export interface WrappedTftMatch {
    data: TftMatch
    puuidToName: { [puuid: string] : string | undefined }
}

export interface TftMatch {
    info: TftMatchInfo
}

export interface TftMatchInfo {
    gameDatetime: Date
    gameLength: number
    gameVariation: string | null
    gameVersion: string
    queueId: number
    tftSetNumber: number
    participants: TftParticipant[]
}

export interface TftParticipant {
    goldLeft: number
    lastRound: number
    level: number
    placement: number
    playersEliminated: number
    puuid: string
    timeEliminated: number
    totalDamageToPlayers: number
    traits: TftTrait[]
    units: TftUnit[]
    companion: TftCompanion
}

export interface TftCompanion {
    contentId: string,
    skinId: number,
    species: string
}

export function cleanWrappedTftMatchFromJson(w: WrappedTftMatch) : WrappedTftMatch {
    w.data.info.gameDatetime = new Date(w.data.info.gameDatetime)
    return w
}

export function cleanTftPlayerMatchSummaryFromJson(s: TftPlayerMatchSummary) : TftPlayerMatchSummary {
    s.gameDatetime = new Date(s.gameDatetime)
    return s
}

function checkCanBeVersionSet3MidSet(version: string) : boolean {
    let tokens = version.split('.')
    let major = parseInt(tokens[0])
    let minor = parseInt(tokens[1])
    return (major == 10 && (minor >= 12 || minor <= 18))
}

export function getTftSetNumber(major: number, version: string): string {
    if (major == 3) {
        if (checkCanBeVersionSet3MidSet(version)) {
            return `${major}.5`
        } else {
            return `${major}`
        }
    } else {
        return `${major}`
    }
}