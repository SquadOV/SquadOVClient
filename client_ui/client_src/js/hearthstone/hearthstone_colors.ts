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

export function getRarityColor(rarity: HearthstoneCardRarity): Color {
    switch (rarity) {
        default:
        case HearthstoneCardRarity.Invalid:
        case HearthstoneCardRarity.Free:
        case HearthstoneCardRarity.Common:
            return {
                r: 105,
                g: 105,
                b: 105
            }
        case HearthstoneCardRarity.Rare:
            return {
                r: 56,
                g: 142,
                b: 254
            }
        case HearthstoneCardRarity.Epic:
            return {
                r: 169,
                g: 48,
                b: 205
            }
        case HearthstoneCardRarity.Legendary:
            return {
                r: 251,
                g: 142,
                b: 4
            }
    }
}