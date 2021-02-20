import { HearthstoneDeck, cleanHearthstoneDeckFromJson } from '@client/js/hearthstone/hearthstone_deck'

export interface HearthstoneDuelRun {
    duelUuid: string
    heroCard: string | null
    deck: HearthstoneDeck | null
    wins: number
    loss: number
    timestamp: Date
    rating: number | null
}

export function cleanHearthstoneDuelRunFromJson(r : HearthstoneDuelRun): HearthstoneDuelRun {
    r.timestamp = new Date(r.timestamp)
    if (!!r.deck) {
        cleanHearthstoneDeckFromJson(r.deck)
    }
    return r
}