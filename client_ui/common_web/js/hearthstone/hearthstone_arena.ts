import { HearthstoneDeck, cleanHearthstoneDeckFromJson } from '@client/js/hearthstone/hearthstone_deck'

export interface HearthstoneArenaRun {
    arenaUuid: string
    deck: HearthstoneDeck | null
    wins: number
    loss: number
    timestamp: Date
}

export function cleanHearthstoneArenaRunFromJson(r : HearthstoneArenaRun): HearthstoneArenaRun {
    r.timestamp = new Date(r.timestamp)
    if (!!r.deck) {
        cleanHearthstoneDeckFromJson(r.deck)
    }
    return r
}