export enum EPricingTier {
    Basic = 'BASIC',
    Silver = 'SILVER',
    Gold = 'GOLD',
    Diamond = 'DIAMOND'
}

export const PRICING_ORDER = {
    [EPricingTier.Basic]: 1,
    [EPricingTier.Silver]: 2,
    [EPricingTier.Gold]: 3,
    [EPricingTier.Diamond]: 4,
}

type PricingGrid = {
    [tier in EPricingTier]: number
}

export interface Discount {
    percent: number
    reason: string
}

export interface FullPricingInfo {
    pricing: PricingGrid
    discounts: Discount[]
}

export function computePricePerMonth(grid: FullPricingInfo, tier: EPricingTier): number {
    let basePrice = grid.pricing[tier]
    let totalDiscount = 0
    for (let d of grid.discounts) {
        totalDiscount += d.percent
    }
    return basePrice * (1.0 - totalDiscount)
}