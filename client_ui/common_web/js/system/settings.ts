/// #if DESKTOP
import fs from 'fs'
import path, { parse } from 'path'
import { detectComputerBaselineLevel, BaselineLevel, baselineToString } from '@client/js/system/baseline'
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
import { allGames, SquadOvGames, SquadOvWowRelease } from '@client/js/squadov/game'
import { apiClient } from '@client/js/api'
import { Keybind } from '@client/js/system/keybinds'
/// #endif

export interface SquadOvOverlay {
    enabled: boolean
    name: string
    games: SquadOvGames[]
    fabric: any
    width: number
    height: number
}

export function createEmptyOverlay(name: string): SquadOvOverlay {
    return {
        enabled: true,
        name,
        games: JSON.parse(JSON.stringify(allGames)),
        fabric: {},
        width: 1920,
        height: 1080,
    }
}

export interface SquadOvOverlaySettings {
    enabled: boolean
    layers: SquadOvOverlay[]
}

function createDefaultOverlaySettings(): SquadOvOverlaySettings {
    return {
        enabled: false,
        layers: [],
    }
}

export interface AudioDeviceSettings {
    device: string
    id: string
    volume: number
    mono: boolean
    voice: boolean
}

export function createDefaultAudioDevice(): AudioDeviceSettings {
    return {
        device: 'Default Device',
        id: '',
        volume: 1.0,
        mono: false,
        voice: false,
    }
}

export interface ProcessRecord {
    name: string
    exe: string
    ico: string
}

export interface ProcessAudioRecordSettings {
    process: ProcessRecord
    volume: number
}

export interface SquadOvRecordingSettings {
    resY: number
    fps: number
    bitrateKbps: number
    // Multiplied by 100 to get a round integer
    bandwidthLimiterMultiple: number | null
    useVideoHw2: boolean
    useHwEncoder: boolean
    useVfr4: boolean
    useWGC2: boolean
    recordMouse2: boolean
    outputDevices: AudioDeviceSettings[]
    inputDevices: AudioDeviceSettings[]
    usePushToTalk: boolean
    useWASAPIRecording3: boolean
    usePerProcessRecording: boolean
    perProcessRecordingOsCheck: boolean
    recordGameAudio: boolean
    gameAudioVolume: number
    processesToRecord: ProcessAudioRecordSettings[]
    useLocalRecording: boolean
    localRecordingLocation: string
    maxLocalRecordingSizeGb: number
    vodEndDelaySeconds: number
    overlays: SquadOvOverlaySettings
    useAudioDriftCompensation: boolean
    useVoiceBasicNoiseFilter: boolean
    voiceFilterThresholdDb: number
    useVoiceSpeechNoiseReduction: boolean
    needConfirmManualStop: boolean
    useCbr: boolean
    useNativeAspectRatio: boolean
}

export interface SquadOvKeybindSettings {
    pushToTalk: Keybind
    pushToTalk2: Keybind
    bookmark: Keybind
    clip2: Keybind
}

export interface SquadOvVideoPlaybackSettings {
    smallStepSize: number
    largeStepSize: number
}

export function computeFileFolderSizeGb(folder: string): Promise<number> {
    return new Promise((resolve, reject) => {
///#if DESKTOP
        ipcRenderer.invoke('request-folder-size', folder).then((val: IpcResponse<number>) => {
            if (val.success) {
                resolve(val.data)
            } else {
                reject('failure')
            }
        }).catch((err: any) => {
            reject(err)
        })
///#else
        resolve(0)
///#endif
    })
}

function changeLocalRecordingLocation(from: string, to: string): Promise<void> {
    return new Promise((resolve, reject) => {
///#if DESKTOP
        if (!fs.existsSync(from)) {
            resolve()
            return
        }

        ipcRenderer.invoke('change-recording-folder', {from, to}).then((val: IpcResponse<void>) => {
            if (val.success) {
                resolve()
            } else {
                reject('failure')
            }
        }).catch((err: any) => {
            reject(err)
        })
///#else
        resolve()
///#endif
    })
}

function requestLocalRecordingCleanup(loc: string, limit: number): Promise<void> {
    return new Promise((resolve, reject) => {
///#if DESKTOP
        if (!fs.existsSync(loc)) {
            resolve()
            return
        }

        ipcRenderer.invoke('cleanup-recording-folder', {loc, limit}).then((val: IpcResponse<void>) => {
            if (val.success) {
                resolve()
            } else {
                reject('failure')
            }
        }).catch((err: any) => {
            reject(err)
        })
///#else
        resolve()
///#endif
    })
}

export async function changeLocalRecordingSettings(record: SquadOvRecordingSettings, use: boolean, loc: string, limit: number) {
    const oldLoc = record.localRecordingLocation
    const oldLimit = record.maxLocalRecordingSizeGb
    
    if (oldLoc != loc) {
        await changeLocalRecordingLocation(oldLoc, loc)
    }

    if (oldLimit > limit) {
        await requestLocalRecordingCleanup(loc, limit)
    }    
}

export interface WowDisabledInstance {
    id: number
    release: SquadOvWowRelease
}

export interface WowSettings {
    useCombatLogTimeout: boolean
    timeoutSeconds2: number
    recordArenas: boolean
    recordBattlegrounds: boolean
    recordDungeons: boolean
    recordKeystones: boolean
    recordEncounters: boolean
    recordScenarios: boolean
    minimumTimeSecondsToRecord: number
    doNotRecordInstances: WowDisabledInstance[]
    showAddonWarnings: boolean
}

function createEmptyWowSettings(): WowSettings {
    return {
        useCombatLogTimeout: true,
        timeoutSeconds2: 180,
        recordArenas: true,
        recordBattlegrounds: true,
        recordDungeons: true,
        recordKeystones: true,
        recordEncounters: true,
        recordScenarios: true,
        minimumTimeSecondsToRecord: 15,
        doNotRecordInstances: [],
        showAddonWarnings: true,
    }
}

export interface ValorantSettings {
    recordStandard: boolean
    recordSpikeRush: boolean
    recordDeathmatch: boolean
    recordOther: boolean
}

function createEmptyValorantSettings(): ValorantSettings {
    return {
        recordStandard: true,
        recordSpikeRush: true,
        recordDeathmatch: true,
        recordOther: true,
    }
}

export interface PerGameSettings {
    wow: WowSettings
    valorant: ValorantSettings,
}

function createEmptyPerGameSettings(): PerGameSettings {
    return {
        wow: createEmptyWowSettings(),
        valorant: createEmptyValorantSettings(),
    }
}

export interface SquadOvLocalSettings {
    record: SquadOvRecordingSettings
    keybinds2: SquadOvKeybindSettings
    playback: SquadOvVideoPlaybackSettings
    minimizeToTray: boolean
    minimizeOnClose: boolean
    runOnStartup: boolean
    ranSpeedCheck: boolean
    speedCheckResultMbps: number
    anonymousAnalytics: boolean
    hidePostGamePopupUntil: Date | null
    disablePostGamePopup: boolean
    disabledGames: SquadOvGames[]
    games: PerGameSettings
    useHwAccel: boolean
    instantClipLengthSeconds: number
}

function getSettingsFname() : string {
/// #if DESKTOP
    return path.join(process.env.SQUADOV_USER_APP_FOLDER!, 'settings.json')
/// #else
    return ''
/// #endif
}

function getDefaultRecordingLocation(): string {
/// #if DESKTOP
    return path.join(process.env.SQUADOV_USER_APP_FOLDER!, 'Local')
/// #else
    return ''
/// #endif
}

let inProgress: boolean = false
let saveTimer: number | undefined = undefined
export function saveLocalSettings(s: SquadOvLocalSettings, immediate: boolean = false, ignoreInProgress: boolean = false) {
/// #if DESKTOP
    if (inProgress && !ignoreInProgress) {
        return
    }

    if (saveTimer !== undefined) {
        window.clearTimeout(saveTimer)
    }

    inProgress = true
    let fn = () => {
        fs.writeFileSync(getSettingsFname(), JSON.stringify(s, null, 4), {
            encoding: 'utf-8',
        })
    
        ipcRenderer.invoke('reload-app-settings').finally(() => {
            inProgress = false
        })
        saveTimer = undefined
    }

    if (!immediate) {
        saveTimer = window.setTimeout(fn, 500)
    } else {
        fn()
    }
/// #endif
}

export async function generateDefaultSettings(): Promise<SquadOvLocalSettings> {
/// #if DESKTOP
    console.log('Generating Default Settings...')
    let baseline = await detectComputerBaselineLevel()
    console.log('Detect Baseline: ', baselineToString(baseline))

    let record: SquadOvRecordingSettings
    switch (baseline) {
        case BaselineLevel.Low:
            record = {
                resY: 720,
                fps: 30,
                bitrateKbps: 6000,
                bandwidthLimiterMultiple: 200,
                useVideoHw2: true,
                useHwEncoder: true,
                useVfr4: true,
                useWGC2: true,
                recordMouse2: false,
                outputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: false,
                    }
                ],
                inputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: true,
                    }
                ],
                usePushToTalk: false,
                useWASAPIRecording3: true,
                usePerProcessRecording: false,
                perProcessRecordingOsCheck: false,
                recordGameAudio: false,
                gameAudioVolume: 1.0,
                processesToRecord: [],
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                vodEndDelaySeconds: 0,
                overlays: createDefaultOverlaySettings(),
                useAudioDriftCompensation: true,
                useVoiceBasicNoiseFilter: false,
                voiceFilterThresholdDb: -60,
                useVoiceSpeechNoiseReduction: false,
                needConfirmManualStop: true,
                useCbr: false,
                useNativeAspectRatio: false,
            }
        case BaselineLevel.Medium:
            record = {
                resY: 720,
                fps: 60,
                bitrateKbps: 6000,
                bandwidthLimiterMultiple: 200,
                useVideoHw2: true,
                useHwEncoder: true,
                useVfr4: true,
                useWGC2: true,
                recordMouse2: false,
                outputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: false,
                    }
                ],
                inputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: true,
                    }
                ],
                usePushToTalk: false,
                useWASAPIRecording3: true,
                usePerProcessRecording: false,
                perProcessRecordingOsCheck: false,
                recordGameAudio: false,
                gameAudioVolume: 1.0,
                processesToRecord: [],
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                vodEndDelaySeconds: 0,
                overlays: createDefaultOverlaySettings(),
                useAudioDriftCompensation: true,
                useVoiceBasicNoiseFilter: false,
                voiceFilterThresholdDb: -60,
                useVoiceSpeechNoiseReduction: false,
                needConfirmManualStop: true,
                useCbr: false,
                useNativeAspectRatio: false,
            }
        case BaselineLevel.High:
            record = {
                resY: 1080,
                fps: 60,
                bitrateKbps: 6000,
                bandwidthLimiterMultiple: 200,
                useVideoHw2: true,
                useHwEncoder: true,
                useVfr4: true,
                useWGC2: true,
                recordMouse2: false,
                outputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: false,
                    }
                ],
                inputDevices: [
                    {
                        device: 'Default Device',
                        id: '',
                        volume: 1.0,
                        mono: false,
                        voice: true,
                    }
                ],
                usePushToTalk: false,
                useWASAPIRecording3: true,
                usePerProcessRecording: false,
                perProcessRecordingOsCheck: false,
                recordGameAudio: false,
                gameAudioVolume: 1.0,
                processesToRecord: [],
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                vodEndDelaySeconds: 0,
                overlays: createDefaultOverlaySettings(),
                useAudioDriftCompensation: true,
                useVoiceBasicNoiseFilter: false,
                voiceFilterThresholdDb: -60,
                useVoiceSpeechNoiseReduction: false,
                needConfirmManualStop: true,
                useCbr: false,
                useNativeAspectRatio: false,
            }
    }

    return {
        record,
        keybinds2: {
            pushToTalk: {
                keys: [],
                mustHold: false,
                holdSeconds: 0,
            },
            pushToTalk2: {
                keys: [],
                mustHold: false,
                holdSeconds: 0,
            },
            bookmark: {
                keys: [120],
                mustHold: true,
                holdSeconds: 1,
            },
            clip2: {
                keys: [18, 83],
                mustHold: true,
                holdSeconds: 1,
            },
        },
        playback: {
            smallStepSize: 5000,
            largeStepSize: 10000,
        },
        minimizeToTray: true,
        minimizeOnClose: true,
        runOnStartup: true,
        ranSpeedCheck: false,
        speedCheckResultMbps: 0,
        anonymousAnalytics: true,
        hidePostGamePopupUntil: null,
        disablePostGamePopup: false,
        disabledGames: [],
        games: createEmptyPerGameSettings(),
        useHwAccel: true,
        instantClipLengthSeconds: 15,
    }
/// #else
    return {
        record: {
            resY: 1080,
            fps: 60,
            bitrateKbps: 6000,
            bandwidthLimiterMultiple: 200,
            useVideoHw2: true,
            useHwEncoder: true,
            useVfr4: true,
            useWGC2: true,
            recordMouse2: false,
            outputDevices: [
                {
                    device: 'Default Device',
                    id: '',
                    volume: 1.0,
                    mono: false,
                    voice: false,
                }
            ],
            inputDevices: [
                {
                    device: 'Default Device',
                    id: '',
                    volume: 1.0,
                    mono: false,
                    voice: true,
                }
            ],
            usePushToTalk: false,
            useWASAPIRecording3: true,
            usePerProcessRecording: false,
            perProcessRecordingOsCheck: false,
            recordGameAudio: false,
            gameAudioVolume: 1.0,
            processesToRecord: [],
            useLocalRecording: false,
            localRecordingLocation: getDefaultRecordingLocation(),
            maxLocalRecordingSizeGb: 100,
            vodEndDelaySeconds: 0,
            overlays: createDefaultOverlaySettings(),
            useAudioDriftCompensation: true,
            useVoiceBasicNoiseFilter: false,
            voiceFilterThresholdDb: -60,
            useVoiceSpeechNoiseReduction: false,
            needConfirmManualStop: true,
            useCbr: false,
            useNativeAspectRatio: false,
        },
        keybinds2: {
            pushToTalk: {
                keys: [],
                mustHold: false,
                holdSeconds: 0,
            },
            pushToTalk2: {
                keys: [],
                mustHold: false,
                holdSeconds: 0,
            },
            bookmark: {
                keys: [120],
                mustHold: true,
                holdSeconds: 1,
            },
            clip2: {
                keys: [18, 83],
                mustHold: true,
                holdSeconds: 1,
            },
        },
        playback: {
            smallStepSize: 5000,
            largeStepSize: 10000,
        },
        minimizeToTray: true,
        minimizeOnClose: true,
        runOnStartup: true,
        ranSpeedCheck: false,
        speedCheckResultMbps: 0,
        anonymousAnalytics: true,
        hidePostGamePopupUntil: null,
        disablePostGamePopup: false,
        disabledGames: [],
        games: createEmptyPerGameSettings(),
        useHwAccel: true,
        instantClipLengthSeconds: 15,
    }
/// #endif
}

export async function loadLocalSettings(): Promise<SquadOvLocalSettings> {
/// #if DESKTOP
    const settingsFname = getSettingsFname()
    console.log('Loading local settings...', settingsFname)
    if (!fs.existsSync(settingsFname)) {
        saveLocalSettings(await generateDefaultSettings(), true)
    }

    let data = fs.readFileSync(settingsFname , 'utf8')
    
    let parsedData: SquadOvLocalSettings
    try {
        parsedData = JSON.parse(data)
    } catch (e) {
        console.error('Failed to load local settings: ', e)
        fs.unlinkSync(settingsFname)
        return loadLocalSettings()
    }

    try {
        if (parsedData.minimizeToTray === undefined) {
            parsedData.minimizeToTray = true
        }

        if (parsedData.minimizeOnClose === undefined) {
            parsedData.minimizeOnClose = true
        }

        if (parsedData.runOnStartup === undefined) {
            parsedData.runOnStartup = true
        }

        if (parsedData.record.useVideoHw2 === undefined) {
            parsedData.record.useVideoHw2 = false
        }

        if (parsedData.record.useHwEncoder === undefined) {
            parsedData.record.useHwEncoder = true
        }

        if (parsedData.ranSpeedCheck === undefined) {
            parsedData.ranSpeedCheck = false
        }

        if (parsedData.speedCheckResultMbps === undefined) {
            parsedData.speedCheckResultMbps = 0
        }

        if (parsedData.record.useVfr4 === undefined) {
            parsedData.record.useVfr4 = true
        }

        if (parsedData.record.useWGC2 === undefined) {
            parsedData.record.useWGC2 = true
        }

        if (parsedData.record.recordMouse2 === undefined) {
            parsedData.record.recordMouse2 = false
        }
        
        if (parsedData.record.useLocalRecording === undefined) {
            parsedData.record.useLocalRecording = false
        }

        if (parsedData.record.localRecordingLocation === undefined) {
            parsedData.record.localRecordingLocation = getDefaultRecordingLocation()
        }

        if (parsedData.record.maxLocalRecordingSizeGb === undefined) {
            parsedData.record.maxLocalRecordingSizeGb = 100
        }

        if (parsedData.record.usePushToTalk === undefined) {
            parsedData.record.usePushToTalk = false   
        }

        if (parsedData.keybinds2 === undefined) {
            parsedData.keybinds2 = {
                pushToTalk: {
                    //@ts-ignore
                    keys: parsedData.keybinds?.pushToTalk || [],
                    mustHold: false,
                    holdSeconds: 0,
                },
                pushToTalk2: {
                    //@ts-ignore
                    keys: parsedData.keybinds?.pushToTalk2 || [],
                    mustHold: false,
                    holdSeconds: 0,
                },
                bookmark: {
                    //@ts-ignore
                    keys: parsedData.keybinds?.bookmark || [120],
                    mustHold: true,
                    holdSeconds: 1,
                },
                clip2: {
                    //@ts-ignore
                    keys: parsedData.keybinds?.clip2 || [18, 83],
                    mustHold: true,
                    holdSeconds: 1,
                },
            }
        }

        if (parsedData.record.vodEndDelaySeconds === undefined) {
            parsedData.record.vodEndDelaySeconds = 0
        }

        if (parsedData.playback === undefined) {
            parsedData.playback = {
                smallStepSize: 5000,
                largeStepSize: 10000,
            }
        }

        if (parsedData.anonymousAnalytics === undefined) {
            parsedData.anonymousAnalytics = true
        }

        if (parsedData.disabledGames === undefined) {
            parsedData.disabledGames = []
        }

        if (parsedData.record.overlays === undefined) {
            parsedData.record.overlays = createDefaultOverlaySettings()
        }

        if (parsedData.record.useAudioDriftCompensation === undefined) {
            parsedData.record.useAudioDriftCompensation = true
        }

        if (parsedData.games === undefined) {
            parsedData.games = createEmptyPerGameSettings()
        }

        if (parsedData.record.outputDevices === undefined) {
            parsedData.record.outputDevices = []
        }

        for (let d of parsedData.record.outputDevices) {
            if (d.voice === undefined) {
                d.voice = false
            }
        }

        if (parsedData.record.inputDevices === undefined) {
            parsedData.record.inputDevices = []
        }

        for (let d of parsedData.record.inputDevices) {
            if (d.voice === undefined) {
                d.voice = true
            }
        }
        
        if (parsedData.record.useVoiceBasicNoiseFilter === undefined) {
            parsedData.record.useVoiceBasicNoiseFilter = false
        }

        if (parsedData.record.voiceFilterThresholdDb === undefined) {
            parsedData.record.voiceFilterThresholdDb = -60
        }

        if (parsedData.record.useVoiceSpeechNoiseReduction === undefined) {
            parsedData.record.useVoiceSpeechNoiseReduction = false
        }

        if (parsedData.games.wow.recordArenas === undefined) {
            parsedData.games.wow.recordArenas = true
            parsedData.games.wow.recordKeystones = true
            parsedData.games.wow.recordEncounters = true
        }

        if (parsedData.games.valorant === undefined) {
            parsedData.games.valorant = createEmptyValorantSettings()
        }

        if (parsedData.games.wow.timeoutSeconds2 === undefined) {
            parsedData.games.wow.timeoutSeconds2 = 180
        }

        if (parsedData.games.wow.recordDungeons === undefined) {
            parsedData.games.wow.recordDungeons = true
        }

        if (parsedData.games.wow.recordBattlegrounds === undefined) {
            parsedData.games.wow.recordBattlegrounds = true
        }

        if (parsedData.games.wow.minimumTimeSecondsToRecord === undefined) {
            parsedData.games.wow.minimumTimeSecondsToRecord = 15
        }

        if (parsedData.games.wow.doNotRecordInstances === undefined) {
            parsedData.games.wow.doNotRecordInstances = []
        }
    
        if (parsedData.record.useWASAPIRecording3 === undefined) {
            parsedData.record.useWASAPIRecording3 = true
        }

        if (parsedData.record.usePerProcessRecording === undefined) {
            parsedData.record.usePerProcessRecording = false
        }

        if (parsedData.record.recordGameAudio === undefined) {
            parsedData.record.recordGameAudio = true
        }
        
        if (parsedData.record.processesToRecord === undefined) {
            parsedData.record.processesToRecord = []
        }

        if (parsedData.record.gameAudioVolume === undefined) {
            parsedData.record.gameAudioVolume = 1.0
        }

        if (parsedData.record.perProcessRecordingOsCheck === undefined) {
            parsedData.record.perProcessRecordingOsCheck = false
        }

        if (parsedData.games.wow.recordScenarios === undefined) {
            parsedData.games.wow.recordScenarios = true
        }

        if (parsedData.record.needConfirmManualStop === undefined) {
            parsedData.record.needConfirmManualStop = true
        }
        
        if (parsedData.hidePostGamePopupUntil === undefined) {
            parsedData.hidePostGamePopupUntil = null
        }

        if (parsedData.disablePostGamePopup === undefined) {
            parsedData.disablePostGamePopup = false
        }

        if (parsedData.record.bitrateKbps === undefined) {
            parsedData.record.bitrateKbps = 6000
        }

        if (parsedData.useHwAccel === undefined) {
            parsedData.useHwAccel = true
        }

        if (parsedData.instantClipLengthSeconds === undefined) {
            parsedData.instantClipLengthSeconds = 15
        }

        if (parsedData.games.wow.showAddonWarnings === undefined) {
            parsedData.games.wow.showAddonWarnings = true
        }

        if (parsedData.record.useCbr === undefined) {
            parsedData.record.useCbr = false
        }

        if (parsedData.record.useNativeAspectRatio === undefined) {
            parsedData.record.useNativeAspectRatio = false
        }

        if (parsedData.record.bandwidthLimiterMultiple === undefined) {
            parsedData.record.bandwidthLimiterMultiple = 200
        }
    } catch (ex) {
        console.log('Failed to migrate config file...regenerating: ', ex)
        parsedData = await generateDefaultSettings()
    }

    if (!!parsedData.hidePostGamePopupUntil) {
        parsedData.hidePostGamePopupUntil = new Date(parsedData.hidePostGamePopupUntil)
    }

    // THIS IS DUPLICATED WITH C++?
    if (parsedData.speedCheckResultMbps < 16.0) {
        apiClient.enableTransferAccel()
    }

    saveLocalSettings(parsedData, true)
    return parsedData
/// #else
    return await generateDefaultSettings()
/// #endif
}