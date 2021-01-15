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

export interface TftCompanion {
    contentId: string,
    skinId: number,
    species: string
}

export function cleanTftPlayerMatchSummaryFromJson(s: TftPlayerMatchSummary) : TftPlayerMatchSummary {
    s.gameDatetime = new Date(s.gameDatetime)
    return s
}

function tftSet35PatchRelease() : Date {
    let dt = new Date()
    dt.setUTCFullYear(2020, 6, 10)
    dt.setUTCHours(6)
    return dt
}

export function getTftSetNumber(major: number, tm: Date): string {
    if (major == 3) {
        if (tm >= tftSet35PatchRelease()) {
            return `${major}.5`
        } else {
            return `${major}`
        }
    } else {
        return `${major}`
    }
}