import { HearthstoneCardRace } from '@client/js/hearthstone/hearthstone_cardtype'
import { HearthstoneZone } from '@client/js/hearthstone/hearthstone_zone'

export interface HearthstoneDeckCardCount {
    normal: number
    golden: number
}

export interface HearthstoneDeckSlot {
    index: number
    cardId: string
    owned: boolean
    count: HearthstoneDeckCardCount
}

export enum HearthstoneCardRarity {
	Invalid,
	Common,
	Free,
	Rare,
	Epic,
	Legendary
}

export interface HearthstoneCardMetadata {
    cardId: string
    name: string
    cost: number
    rarity: HearthstoneCardRarity
}

export interface HearthstoneBattlegroundsCardMetadata {
    base: HearthstoneCardMetadata
    tavernLevel: number
    cardRace: HearthstoneCardRace | null
}

export interface HearthstoneDeckSlotWithMetadata extends HearthstoneDeckSlot {
    metadata: HearthstoneCardMetadata
}

export interface HearthstoneDeckZoneCardData extends HearthstoneDeckSlotWithMetadata {
    zone: HearthstoneZone
}

export interface HearthstoneDeck {
    name: string
    deckId: number
    heroCard: string
    heroPremium: number
    deckType: number
    createDate: Date
    isWild: boolean
    slots: HearthstoneDeckSlot[]
}

export function cleanHearthstoneDeckFromJson(d : HearthstoneDeck) : HearthstoneDeck {
    d.createDate = new Date(d.createDate)
    return d
}