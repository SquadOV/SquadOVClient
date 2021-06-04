export function getTftCostColor(cost: number): string {
    switch (cost) {
        default:
        case 1:
            return 'color-tft-1-cost'
        case 2:
            return 'color-tft-2-cost'
        case 3:
            return 'color-tft-3-cost'
        case 4:
            return 'color-tft-4-cost'
        case 5:
            return 'color-tft-5-cost'
    }
}