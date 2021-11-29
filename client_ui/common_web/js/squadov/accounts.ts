import { RiotAccountData } from "@client/js/valorant/valorant_account";

export interface DiscordAccount {
    id: string
    username: string
    discriminator: string
    avatar: string | null
}

export interface TwitchAccount {
    twitchUserId: number
    twitchName: string
}

export interface LinkedAccounts {
    twitch: TwitchAccount[]
    riot: RiotAccountData[]
    discord: DiscordAccount[]
}