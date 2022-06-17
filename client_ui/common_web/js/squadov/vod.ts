import { SquadOvGames } from "@client/js/squadov/game"

export interface VodAssociation {
    matchUuid: string
    userUuid: string
    videoUuid: string
    startTime: Date
    endTime: Date
    rawContainerFormat: string
    isClip: boolean
    isLocal: boolean
    expirationTime: Date | null
}

export interface VodTag {
    videoUuid: string
    tagId: number
    tag: string
    count: number
    isSelf: boolean
}

export interface VodClip {
    clip: VodAssociation
    manifest: VodManifest
    title: string
    description: string
    clipper: string
    game: SquadOvGames
    tm: Date
    views: number
    reacts: number
    comments: number
    favoriteReason: string | null
    isWatchlist: boolean
    accessToken: string | null
    tags: VodTag[]
    published: boolean
}

export interface VodFavoriteResponse {
    favorite: boolean
    reason: string | null
}

export interface ClipReact {
}

export interface ClipComment {
    clipUuid: string
    commentId: number
    username: string
    tm: Date
    comment: string
}

export interface ValorantMatchAccessibleVods {
    vods: VodAssociation[]
    // uuid to puuid
    userMapping: {[uuid: string] : string | undefined}
}

export interface WowMatchAccessibleVods {
    vods: VodAssociation[]
    userToId: {[uuid: string] : number | undefined }
}

export interface HearthstoneMatchAccessibleVods {
    vods: VodAssociation[]
    userToHero: {[userId: number] : string | undefined }
    userToId: {[uuid: string] : number | undefined }
}

export interface LeagueMatchAccessibleVods {
    vods: VodAssociation[]
    // uuid to participant id
    userMapping: {[uuid: string] : number | undefined}
}

export interface TftMatchAccessibleVods {
    vods: VodAssociation[]
    // uuid to puuid
    userMapping: {[uuid: string] : string | undefined}
}

export interface CsgoMatchAccessibleVods {
    vods: VodAssociation[]
    // uuid to steam id
    userMapping: {[uuid: string] : number | undefined}
}

interface VodSegment {
    uri: string
    duration: number
    segmentStart: number
    mimeType: string
}

export interface VodMetadata {
    videoUuid: string
    resX: number
    resY: number
    fps: number
    minBitrate: number
    avgBitrate: number
    maxBitrate: number
    id: string
    dataType: string
    fname: string

    bucket: string
    sessionId: string | null
}

export interface VodTrack {
    metadata: VodMetadata
    segments: VodSegment[]
    preview: string | null
}

export interface VodManifest {
    videoTracks: VodTrack[]
}

export interface VodSegmentUrl {
    url: string
    expiration: Date | null
}

export function getVideoQualityTrack(v : VodManifest, quality : string) : VodTrack | undefined {
    return v.videoTracks.find((ele : VodTrack) => ele.metadata.id == quality)
}

export function cleanVodAssocationData(v : VodAssociation) : VodAssociation {
    v.startTime = new Date(v.startTime)
    v.endTime = new Date(v.endTime)

    if (!!v.expirationTime) {
        v.expirationTime = new Date(v.expirationTime)
    }
    return v
}

export function cleanVodClipFromJson(c: VodClip) : VodClip {
    cleanVodAssocationData(c.clip)
    c.tm = new Date(c.tm)
    return c
}

export function cleanClipCommentFromJson(c: ClipComment): ClipComment {
    c.tm = new Date(c.tm)
    return c
}

export function cleanVodSegmentUrlFromJson(s: VodSegmentUrl): VodSegmentUrl {
    if (!!s.expiration) {
        s.expiration = new Date(s.expiration)
    }
    return s
}