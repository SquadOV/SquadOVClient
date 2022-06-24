export enum EPricingTier {
    Basic = 'BASIC',
    Silver = 'SILVER',
    Gold = 'GOLD',
    Diamond = 'DIAMOND'
}

export enum EProFeature {
    Support,
    SquadSize,
    Watermark,
    Resolution,
    Fps,
    Bitrate,
    Codec,
    VodRetention,
    ClipLength,
    SeparateAudioTracks,
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

export function getNextHighestTier(tier: EPricingTier | null): EPricingTier | null {
    if (!tier) {
        return null
    }

    switch (tier) {
        case EPricingTier.Basic:
            return EPricingTier.Silver
        case EPricingTier.Silver:
            return EPricingTier.Gold
        case EPricingTier.Gold:
            return EPricingTier.Diamond
        case EPricingTier.Diamond:
            return null
    }
}

export enum Currency {
    Usd = 'usd',
    Euro = 'eur',
}

export function currencySymbol(c: Currency) : string {
    switch (c) {
        case Currency.Usd:
            return '$'
        case Currency.Euro:
            return '€'
    }
}

export function formatCurrency(amount: number, c: Currency) : string {
    let s = currencySymbol(c)
    switch (c) {
        case Currency.Usd:
        case Currency.Euro:
            return `${s}${amount.toFixed(2)}`
    }
}