import { HearthstoneZone, hearthstoneZoneFromString } from '@client/js/hearthstone/hearthstone_zone'
import { HearthstoneCardtype, hearthstoneCardTypeFromString } from '@client/js/hearthstone/hearthstone_cardtype'
import { HearthstonePlayState, hearthstonePlayStateFromString } from '@client/js/hearthstone/hearthstone_playstate'
import { HearthstoneGameStep, hearthstoneGameStepFromString } from '@client/js/hearthstone/hearthstone_game_step'

export interface HearthstoneEntity {
    entityId: number
    tags: { [nm : string] : string | undefined }
    attributes: { [nm : string] : string | undefined }
}

// Tags
const ZONE_POSITION = 'ZONE_POSITION'
const ZONE = 'ZONE'
const CONTROLLER = 'CONTROLLER'
const CARDTYPE = 'CARDTYPE'
const PLAYSTATE = 'PLAYSTATE'
const HEALTH = 'HEALTH'
const DAMAGE = 'DAMAGE'
const STEP = 'STEP'
const SPAWN_TIME_COUNT = 'SPAWN_TIME_COUNT'
const PLAYER_ID = 'PLAYER_ID'
const PLAYER_LEADERBOARD_PLACE = 'PLAYER_LEADERBOARD_PLACE'

// Attributes
const CARD_ID = 'CardID'

export class HearthstoneEntityWrapper {
    _entity: HearthstoneEntity

    constructor(entity: HearthstoneEntity) {
        this._entity = entity
    }

    get step(): HearthstoneGameStep {
        let raw = this._entity.tags[STEP]
        if (!raw) {
            return HearthstoneGameStep.Invalid
        }
        return hearthstoneGameStepFromString(raw)
    }

    get health(): number {
        let hp = this._entity.tags[HEALTH]
        if (!hp) {
            return 0
        }
        return parseInt(hp)
    }

    get damage(): number {
        let dmg = this._entity.tags[DAMAGE]
        if (!dmg) {
            return 0
        }
        return parseInt(dmg)
    }

    get remainingHealth(): number {
        return this.health - this.damage
    }

    get zone() : HearthstoneZone {
        let zone = this._entity.tags[ZONE]
        if (!zone) {
            return HearthstoneZone.Invalid
        }
        return hearthstoneZoneFromString(zone)
    }

    get zonePosition() : number {
        let pos = this._entity.tags[ZONE_POSITION]
        if (!pos) {
            return -1
        }
        return parseInt(pos)
    }

    get controller() : number {
        let ctrl = this._entity.tags[CONTROLLER]
        if (!ctrl) {
            return -1
        }
        return parseInt(ctrl)
    }

    get cardId() : string {
        let id = this._entity.attributes[CARD_ID]
        if (!id) {
            return ''
        }
        return id
    }

    get cardType() : HearthstoneCardtype {
        let typ = this._entity.tags[CARDTYPE]
        if (!typ) {
            return HearthstoneCardtype.Invalid
        }
        return hearthstoneCardTypeFromString(typ)
    }

    get playState() : HearthstonePlayState {
        let state = this._entity.tags[PLAYSTATE]
        if (!state) {
            return HearthstonePlayState.Invalid
        }
        return hearthstonePlayStateFromString(state)
    }
    
    get spawnTimeCount(): number {
        let data = this._entity.tags[SPAWN_TIME_COUNT]
        if (!data) {
            return 0
        }
        return parseInt(data)
    }

    get playerId(): number {
        let data = this._entity.tags[PLAYER_ID]
        if (!data) {
            return 0
        }
        return parseInt(data)
    }

    get leaderboardPlace(): number {
        let data = this._entity.tags[PLAYER_LEADERBOARD_PLACE]
        if (!data) {
            return 0
        }
        return parseInt(data)
    }
}