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