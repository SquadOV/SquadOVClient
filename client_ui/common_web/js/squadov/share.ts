import { SquadOvGames } from "@client/js/squadov/game";

export interface ShareAccessTokenResponse {
    fullPath: string
    key: string
    uid: number
}

export interface LinkShareData {
    isLinkShared: boolean
    shareUrl: string | null
}

export interface MatchVideoSharePermissions {
    canShare: boolean
    canClip: boolean
}

export interface MatchVideoShareConnection extends MatchVideoSharePermissions {
    id: number
    matchUuid: string | null
    videoUuid: string | null
    destUserId: number | null
    destSquadId: number | null
}

export interface AutoShareConnection {
    id: number
    sourceUserId: number
    canShare: boolean
    canClip: boolean
    destUserId: number | null
    destSquadId: number | null
    games: SquadOvGames[]
}