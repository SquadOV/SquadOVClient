export interface FeatureFlags {
    maxRecordPixelY: number
    maxRecordFps: number
    allowRecordUpload: boolean
    allowWowCombatLogUpload: boolean
    enableUserProfiles: boolean
    maxBitrateKbps: number
    mandatoryWatermark: boolean
    watermarkMinSize: number
}

export interface GlobalFlags {
    disableRegistration: boolean
}