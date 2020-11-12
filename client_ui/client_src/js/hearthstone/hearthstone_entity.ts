import { HearthstoneZone, hearthstoneZoneFromString } from '@client/js/hearthstone/hearthstone_zone'
import { HearthstoneCardtype, hearthstoneCardTypeFromString } from '@client/js/hearthstone/hearthstone_cardtype'

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

// Attributes
const CARD_ID = 'CardID'

export class HearthstoneEntityWrapper {
    _entity: HearthstoneEntity

    constructor(entity: HearthstoneEntity) {
        this._entity = entity
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
}