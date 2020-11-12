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