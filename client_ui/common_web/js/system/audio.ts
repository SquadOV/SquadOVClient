export interface SingleAudioDevice {
    name: string
    id: string
}

export function createDefaultSingleAudioDevice(): SingleAudioDevice {
    return {
        name: 'System Default Device',
        id: ''
    }
}

export interface AudioDeviceListingResponse {
    options: SingleAudioDevice[]
    default: SingleAudioDevice
}