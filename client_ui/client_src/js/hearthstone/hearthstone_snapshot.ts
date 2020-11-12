import { HearthstoneEntity } from '@client/js/hearthstone/hearthstone_entity'

export interface HearthstoneMatchSnapshotAuxData {
    currentTurn: number
    step: number
    currentPlayerId: number
    lastActionId: number
}

export interface HearthstoneMatchSnapshot {
    uuid: string
    tm: Date | null
    gameEntityId: number
    playerNameToPlayerId: { [nm : string] : number }
    playerIdToEntityId: { [id: number] : number }
    entities: { [id: number] : HearthstoneEntity }
    auxData: HearthstoneMatchSnapshotAuxData | null
}

export function cleanHearthstoneMatchSnapshotFromJson(m : HearthstoneMatchSnapshot) : HearthstoneMatchSnapshot {
    if (!!m.tm) {
        m.tm = new Date(m.tm)
    }
    return m
}