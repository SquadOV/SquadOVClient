import { SquadOvActivity } from "@client/js/squadov/status"

export function compareString(a: string, b: string): number {
    if (a < b) {
        return -1
    } else if (a > b) {
        return 1
    } else {
        return 0
    }
}

const ACTIVITY_TO_CMP_INDEX = new Map([
    [SquadOvActivity.Online, 2],
    [SquadOvActivity.InGame, 1],
    [SquadOvActivity.Recording, 0],
    [SquadOvActivity.Offline, 3],
])

export function compareUserStatus(a: SquadOvActivity, b: SquadOvActivity): number {
    let aIdx = ACTIVITY_TO_CMP_INDEX.get(a)
    let bIdx = ACTIVITY_TO_CMP_INDEX.get(b)

    if (aIdx == undefined || bIdx === undefined) {
        return 0
    }

    return aIdx - bIdx
}