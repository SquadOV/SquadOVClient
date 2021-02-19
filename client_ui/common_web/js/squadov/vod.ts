import { SquadOvGames } from "@client/js/squadov/game"

export interface VodAssociation {
    matchUuid: string
    userUuid: string
    videoUuid: string
    startTime: Date
    endTime: Date
    rawContainerFormat: string
    isClip: boolean
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
    minBitrate: number
    avgBitrate: number
    maxBitrate: number
    id: string
    dataType: string
    fname: string
}

export interface VodTrack {
    metadata: VodMetadata
    segments: VodSegment[]
    preview: string | null
}

export interface VodManifest {
    videoTracks: VodTrack[]
}

export function getVideoQualityTrack(v : VodManifest, quality : string) : VodTrack | undefined {
    return v.videoTracks.find((ele : VodTrack) => ele.metadata.id == quality)
}

export function cleanVodAssocationData(v : VodAssociation) : VodAssociation {
    v.startTime = new Date(v.startTime)
    v.endTime = new Date(v.endTime)
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