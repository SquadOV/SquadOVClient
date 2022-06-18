export interface FeatureFlags {
    maxRecordPixelY: number
    maxRecordFps: number
    allowRecordUpload: boolean
    allowWowCombatLogUpload: boolean
    enableUserProfiles: boolean
    maxBitrateKbps: number
    mandatoryWatermark: boolean
    watermarkMinSize: number
    vodPriority: number
    earlyAccess: boolean
    vodRetention: number | null
    maxSquadSize: number | null
    maxClipSeconds: number
}

export interface GlobalFlags {
    disableRegistration: boolean
}