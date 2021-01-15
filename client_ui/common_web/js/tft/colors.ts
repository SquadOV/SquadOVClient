import { Color } from '@client/js/color'

export function getTftCostColor(cost: number): Color {
    switch (cost) {
        default:
        case 1:
            return {
                r: 49,
                g: 65,
                b: 76
            }
        case 2:
            return {
                r: 18,
                g: 118,
                b: 57
            }
        case 3:
            return {
                r: 38,
                g: 113,
                b: 170
            }
        case 4:
            return {
                r: 236,
                g: 8,
                b: 173
            }
        case 5:
            return {
                r: 219,
                g: 119,
                b: 8
            }
    }
}