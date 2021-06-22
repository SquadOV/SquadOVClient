import { RiotAccountData } from "@client/js/valorant/valorant_account";

export interface TwitchAccount {
    twitchUserId: number
    twitchName: string
}

export interface LinkedAccounts {
    twitch: TwitchAccount[]
    riot: RiotAccountData[]
}