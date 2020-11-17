import { HearthstoneEntity, HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneGameStep } from '@client/js/hearthstone/hearthstone_game_step'
import { HearthstoneGameBlockWrapper, BlockType } from '@client/js/hearthstone/hearthstone_actions'

export interface HearthstoneMatchSnapshotAuxData {
    currentTurn: number
    step: HearthstoneGameStep
    currentPlayerId: number
    lastActionIndex: number
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
    _blocks: HearthstoneGameBlockWrapper[]
    constructor(s: HearthstoneMatchSnapshot | null) {
        this._snapshot = s
        this._entities = new Map()
        this._blocks = []
    }

    // A subset of the blocks that correspond to player actions
    get gameBlocks(): HearthstoneGameBlockWrapper[] {
        return this._blocks.filter((ele : HearthstoneGameBlockWrapper) => {
            let typ =  ele._gameBlock.blockType
            return (typ == BlockType.Attack) || (typ == BlockType.Play)
        })
    }

    get snapshotTime(): Date | undefined {
        if (!this._snapshot || !this._snapshot.tm) {
            return undefined
        }

        return this._snapshot.tm
    }

    get gameEntity(): HearthstoneEntityWrapper | undefined {
        if (!this._snapshot) {
            return undefined
        }

        return this.entity(this._snapshot.gameEntityId)
    }

    addBlock(block: HearthstoneGameBlockWrapper) {
        if (!block.isRoot) {
            return
        }
        this._blocks.push(block)
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

    get currentPlayerId() : number {
        let st = this._snapshot?.auxData?.currentPlayerId
        return !!st ? st : -1
    }

    get currentTurn() : number {
        let numTurns = this._snapshot?.auxData?.currentTurn
        return !!numTurns ? numTurns: 0
    }

    get step(): HearthstoneGameStep {
        let st = this._snapshot?.auxData?.step
        return !!st ? st : HearthstoneGameStep.Invalid
    }
}