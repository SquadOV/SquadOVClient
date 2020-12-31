export interface VodAssociation {
    matchUuid: string
    userUuid: string
    videoUuid: string
    startTime: Date
    endTime: Date
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

interface VodSegment {
    uri: string
    duration: number
    segmentStart: number
}

interface VodMetadata {
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

interface VodTrack {
    metadata: VodMetadata
    segments: VodSegment[]
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