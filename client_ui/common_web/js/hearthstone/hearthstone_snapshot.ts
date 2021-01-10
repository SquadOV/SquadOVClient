import { HearthstoneEntity, HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneGameStep } from '@client/js/hearthstone/hearthstone_game_step'
import { HearthstoneGameBlockWrapper, BlockType, HearthstoneGameAction } from '@client/js/hearthstone/hearthstone_actions'

export enum BattlegroundsRoundWinState {
    Win,
    Loss,
    Draw
}

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
    _actions: HearthstoneGameAction[]
    _isBattlegrounds: boolean
    constructor(s: HearthstoneMatchSnapshot | null, isBattlegrounds: boolean) {
        this._snapshot = s
        this._entities = new Map()
        this._blocks = []
        this._actions = []
        this._isBattlegrounds = isBattlegrounds
    }

    // A subset of the blocks that correspond to player actions.
    get gameBlocks(): HearthstoneGameBlockWrapper[] {
        return this._blocks.map((ele : HearthstoneGameBlockWrapper) => {
            return ele.gameBlocks
        }).flat()
    }

    get snapshotTime(): Date | undefined {
        if (!this._snapshot || !this._snapshot.tm) {
            return undefined
        }

        // The snapshot happens at the end of the turn.
        // The beginning of the turn (block 0) isn't necessarily when the player draws the card.
        // Thus, we must search for the action at which we go to the "MAIN_READY" step. Note that
        // we use MAIN_READY here instead of MAIN_START as it translates to battlegrounds as well
        // while MAIN_START is not used in the action phase of battlegrounds. Note that for SHOP turns
        // (odd numbers) in Battlegrounds, we need to look for other actions...
        let turn = this._snapshot?.auxData?.currentTurn
        let isBattlegroundsShop = this._isBattlegrounds && !!turn && turn % 2 !== 0

        for (let a of this._actions) {
            if (isBattlegroundsShop) {
                // From what I can see, the first thing the game does is set tag=1196 to value=0 on the board
                // so we can look for that...
                if (!('1196' in a.tags)) {
                    continue
                }

                let val = a.tags['1196']
                if (val === '0') {
                    return a.tm
                }
            } else {
                if (a.realEntityId != this._snapshot.gameEntityId) {
                    continue
                }

                if (!('STEP' in a.tags)) {
                    continue
                }
    
                let step = a.tags['STEP']
                if (step === 'MAIN_READY') {
                    return a.tm
                }
            }
        }

        // If we really reach here then it's probably some edge case like the first shop round on battlegrounds
        // and resorting to the first action is probably OK.
        return (this._actions.length > 0 ) ? this._actions[0].tm : undefined
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

    addActions(actions: HearthstoneGameAction[]) {
        this._actions = actions
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
        }).sort((a: HearthstoneEntityWrapper | undefined, b: HearthstoneEntityWrapper | undefined) => {
            if (a === undefined || b === undefined) {
                return 0
            }
            return a._entity.entityId - b._entity.entityId
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

    battlegroundsWinState(currentPlayerHeroEntityId: number): BattlegroundsRoundWinState {
        // Need to see if the player's hero entity is either the target of or is the subject of any attacks.
        // Subject - WIN!
        // Target - Loss!
        // Doesn't Exist - Draw!
        let blocks = this.gameBlocks.filter((ele: HearthstoneGameBlockWrapper) => {
            return ele.subject?._entity.entityId === currentPlayerHeroEntityId ||
                !!ele.targets.find((ele : HearthstoneEntityWrapper) => {
                    return ele._entity.entityId === currentPlayerHeroEntityId
                })
        })

        if (blocks.length === 0) {
            return BattlegroundsRoundWinState.Draw
        } else if (blocks[0].subject?._entity.entityId === currentPlayerHeroEntityId) {
            return BattlegroundsRoundWinState.Win
        } else {
            return BattlegroundsRoundWinState.Loss
        }
    }
}