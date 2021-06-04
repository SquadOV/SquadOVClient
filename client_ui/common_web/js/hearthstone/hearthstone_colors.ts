import { Color } from '@client/js/color'
import { HearthstoneCardRarity } from '@client/js/hearthstone/hearthstone_deck'
export function getSameTeamColor(): Color {
    return {
        r: 76,
        g: 175,
        b: 80
    }
}

export function getOpposingTeamColor(): Color {
    return {
        r: 255,
        g: 82,
        b: 82
    }
}

export function getNeutralTeamColor(): Color {
    return {
        r: 245,
        g: 245,
        b: 220
    }
}

export function getRarityColor(rarity: HearthstoneCardRarity): string {
    switch (rarity) {
        default:
        case HearthstoneCardRarity.Invalid:
        case HearthstoneCardRarity.Free:
        case HearthstoneCardRarity.Common:
            return 'color-hs-common'
        case HearthstoneCardRarity.Rare:
            return 'color-hs-rare'
        case HearthstoneCardRarity.Epic:
            return 'color-hs-epic'
        case HearthstoneCardRarity.Legendary:
            return 'color-hs-legendary'
    }
}