/// #if DESKTOP
import fs from 'fs'
import path from 'path'
import { detectComputerBaselineLevel, BaselineLevel, baselineToString } from '@client/js/system/baseline'
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

export interface SquadOvRecordingSettings {
    resY: number
    fps: number
    useVideoHw: boolean
    useHwEncoder: boolean
    useVfr3: boolean
    outputDevice: string
    outputVolume: number
    inputDevice: string
    inputVolume: number
    maxUploadSpeed: number | null
    useLocalRecording: boolean
    localRecordingLocation: string
    maxLocalRecordingSizeGb: number
    useBitrate: boolean
    bitrateKbps: number
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

export interface SquadOvLocalSettings {
    record: SquadOvRecordingSettings
    minimizeToTray: boolean
    minimizeOnClose: boolean
    runOnStartup: boolean
    setupWizardRun: boolean
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
export function saveLocalSettings(s: SquadOvLocalSettings, immediate: boolean = false) {
/// #if DESKTOP
    if (inProgress) {
        return
    }

    if (saveTimer !== undefined) {
        window.clearTimeout(saveTimer)
    }

    let fn = () => {
        inProgress = true
        fs.writeFileSync(getSettingsFname(), JSON.stringify(s), {
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
                useVideoHw: true,
                useHwEncoder: true,
                useVfr3: false,
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
                maxUploadSpeed: null,
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                useBitrate: false,
                bitrateKbps: 6000,
            }
        case BaselineLevel.Medium:
            record = {
                resY: 720,
                fps: 60,
                useVideoHw: true,
                useHwEncoder: true,
                useVfr3: false,
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
                maxUploadSpeed: null,
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                useBitrate: false,
                bitrateKbps: 6000,
            }
        case BaselineLevel.High:
            record = {
                resY: 1080,
                fps: 60,
                useVideoHw: true,
                useHwEncoder: true,
                useVfr3: false,
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
                maxUploadSpeed: null,
                useLocalRecording: false,
                localRecordingLocation: getDefaultRecordingLocation(),
                maxLocalRecordingSizeGb: 100,
                useBitrate: false,
                bitrateKbps: 6000,
            }
    }

    return {
        record,
        minimizeToTray: true,
        minimizeOnClose: true,
        runOnStartup: true,
        setupWizardRun: false
    }
/// #else
    return {
        record: {
            resY: 1080,
            fps: 60,
            useVideoHw: true,
            useHwEncoder: true,
            useVfr3: false,
            outputDevice: 'Default Device',
            outputVolume: 1.0,
            inputDevice: 'Default Device',
            inputVolume: 1.0,
            maxUploadSpeed: null,
            useLocalRecording: false,
            localRecordingLocation: getDefaultRecordingLocation(),
            maxLocalRecordingSizeGb: 100,
            useBitrate: false,
            bitrateKbps: 6000,
        },
        minimizeToTray: true,
        minimizeOnClose: true,
        runOnStartup: true,
        setupWizardRun: false
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
    
    let parsedData
    try {
        parsedData = JSON.parse(data)
    } catch (e) {
        console.log('Failed to load local settings: ', e)
        fs.unlinkSync(settingsFname)
        return loadLocalSettings()
    }

    if (!parsedData.record.outputDevice) {
        parsedData.record.outputDevice = 'Default Device'
    }

    if (parsedData.record.outputVolume === undefined) {
        parsedData.record.outputVolume = 1.0
    }

    if (!parsedData.record.inputDevice) {
        parsedData.record.inputDevice = 'Default Device'
    }

    if (parsedData.record.inputVolume === undefined) {
        parsedData.record.inputVolume = 1.0
    }

    if (parsedData.minimizeToTray === undefined) {
        parsedData.minimizeToTray = true
    }

    if (parsedData.minimizeOnClose === undefined) {
        parsedData.minimizeOnClose = true
    }

    if (parsedData.runOnStartup === undefined) {
        parsedData.runOnStartup = true
    }

    if (parsedData.record.useVideoHw === undefined) {
        parsedData.record.useVideoHw = true
    }

    if (parsedData.record.useHwEncoder === undefined) {
        parsedData.record.useHwEncoder = true
    }

    if (parsedData.setupWizardRun === undefined) {
        parsedData.setupWizardRun = false
    }

    if (parsedData.record.useVfr3 === undefined) {
        parsedData.record.useVfr3 = false
    }

    if (parsedData.record.maxUploadSpeed === undefined) {
        parsedData.record.maxUploadSpeed = null
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

    if (parsedData.record.useBitrate === undefined) {
        parsedData.record.useBitrate = false
    }

    if (parsedData.record.bitrateKbps === undefined) {
        parsedData.record.bitrateKbps = 6000
    }

    saveLocalSettings(parsedData, true)
    return parsedData
/// #else
    return await generateDefaultSettings()
/// #endif
}