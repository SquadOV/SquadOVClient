import { HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneMatchSnapshotWrapper } from '@client/js/hearthstone/hearthstone_snapshot'
import { HearthstoneCardtype } from '@client/js/hearthstone/hearthstone_cardtype'

export enum ActionType {
    CreateGame,
    CreatePlayer,
    FullEntity,
    ShowEntity,
    TagChange
}

export interface HearthstoneGameAction {
    tm: Date
    actionType: ActionType,
    currentBlockId: string
    realEntityId: number
    tags: { [nm : string] : string | undefined }
    attributes: { [nm : string] : string | undefined }
}

export enum BlockType {
    Invalid = 0,
    Attack = 1,
    Joust = 2,
    Power = 3,
    Trigger = 5,
    Deaths = 6,
    Play = 7,
    Fatigue = 8,
    Ritual = 9,
    RevealCard = 10,
    GameReset = 11,
    MoveMinion = 12
}

export interface HearthstoneGameBlock {
    blockId: string
    startActionIndex: number
    endActionIndex: number
    blockType: BlockType
    parentBlock: string | null
    entityId: number
}

export class HearthstoneGameBlockWrapper {
    _gameBlock: HearthstoneGameBlock
    _actions: HearthstoneGameAction[]
    _snapshot: HearthstoneMatchSnapshotWrapper
    _finalSnapshot: HearthstoneMatchSnapshotWrapper

    _parent: HearthstoneGameBlockWrapper | null
    _children: HearthstoneGameBlockWrapper[]

    // Need the final snapshot as that's a more reliable way of obtaining information about entities.
    constructor(gb: HearthstoneGameBlock, parentSnapshot: HearthstoneMatchSnapshotWrapper, finalSnapshot: HearthstoneMatchSnapshotWrapper) {
        this._gameBlock = gb
        this._actions = []
        this._snapshot = parentSnapshot
        this._finalSnapshot = finalSnapshot
        this._parent = null
        this._children = []
    }

    addActions(a : HearthstoneGameAction[]) {
        this._actions = a
    }

    addSubBlock(b : HearthstoneGameBlockWrapper) {
        b._parent = this
        this._children.push(b)
    }

    get blockType(): BlockType {
        return this._gameBlock.blockType
    }

    get isSpell(): boolean {
        if (!this.subject) {
            return false
        }
        return this.subject.cardType == HearthstoneCardtype.Spell
    }

    get isOffensive(): boolean {
        if (!this.subject) {
            return false
        }

        return this.targets.some((ele: HearthstoneEntityWrapper) => {
            return ele.controller != this.subject?.controller
        })
    }

    get subject(): HearthstoneEntityWrapper | undefined {
        return this._finalSnapshot.entity(this._gameBlock.entityId)
    }

    get targets(): HearthstoneEntityWrapper[] {
        // The target list is dependent on the kind of block this is.
        // If this block is a "PLAY" block then it depends on whether or not a 
        // "POWER" block exists within the block. An "ATTACK" block is more straightforward
        // in that we just need to look for a change to the PROPOSED_DEFENDER tag.
        let ret: HearthstoneEntityWrapper[] = []
        if (this._gameBlock.blockType == BlockType.Attack) {
            for (let a of this._actions) {
                if (a.actionType == ActionType.TagChange && 'PROPOSED_DEFENDER' in a.tags) {
                    let defender = parseInt(a.tags['PROPOSED_DEFENDER']!)
                    if (defender > 0) {
                        let entity = this._finalSnapshot.entity(defender)
                        if (!!entity) {
                            ret.push(entity)
                        }
                    }
                }
            }
        } else if (this._gameBlock.blockType == BlockType.Play) {
            for (let sub of this._children) {
                if (sub.blockType == BlockType.Power) {
                    ret.push(...sub.targets)
                }
            }
        }
        return ret
    }

    get isRoot() : boolean {
        return !this._gameBlock.parentBlock
    }
}