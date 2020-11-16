import { HearthstoneEntity, HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'

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

export class HearthstoneMatchSnapshotWrapper {
    _snapshot: HearthstoneMatchSnapshot | null
    _entities: Map<number, HearthstoneEntityWrapper>
    constructor(s: HearthstoneMatchSnapshot | null) {
        this._snapshot = s
        this._entities = new Map()
    }

    entity(entityId: number) : HearthstoneEntityWrapper | undefined {
        if (!this._snapshot) {
            return undefined
        }

        if (this._entities.has(entityId)) {
            return this._entities.get(entityId)
        }

        let wrapper = new HearthstoneEntityWrapper(this._snapshot.entities[entityId])
        this._entities.set(entityId, wrapper)
        return wrapper
    }

    entitiesForPlayerId(playerId: number): HearthstoneEntityWrapper[] {
        if (!this._snapshot) {
            return []
        }

        let allEid = []
        for (let [eid, entity] of Object.entries(this._snapshot.entities)) {
            allEid.push(parseInt(eid))
        }
        return allEid.map((id: number) => {
            return this.entity(id)
        }).filter((ele: HearthstoneEntityWrapper | undefined) => {
            if (!ele) {
                return false
            }
            return ele.controller == playerId
        }) as HearthstoneEntityWrapper[]
    }

    playerIdToEntityId(playerId: number) : number | undefined {
        if (!this._snapshot) {
            return undefined
        }
        return this._snapshot.playerIdToEntityId[playerId]
    }

    get currentTurn() : number {
        let numTurns = this._snapshot?.auxData?.currentTurn
        return !!numTurns ? numTurns: 0
    }
}