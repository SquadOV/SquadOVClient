export interface User2UserSubscription {
    id: number
    sourceUserId: number
    destUserId: number
    isTwitch: boolean
    lastChecked: Date
}

export function cleanUser2UserSubscriptionFromJson(s: User2UserSubscription): User2UserSubscription {
    s.lastChecked = new Date(s.lastChecked)
    return s
}