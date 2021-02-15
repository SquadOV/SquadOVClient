/// #if DESKTOP
import fs from 'fs'
import path from 'path'
import { detectComputerBaselineLevel, BaselineLevel, baselineToString } from '@client/js/system/baseline'
/// #endif

export interface SquadOvRecordingSettings {
    resY: number
    fps: number
    outputDevice: string
    outputVolume: number
    inputDevice: string
    inputVolume: number
}

export interface SquadOvLocalSettings {
    record: SquadOvRecordingSettings
}

function getSettingsFname() : string {
/// #if DESKTOP
    return path.join(process.env.SQUADOV_USER_APP_FOLDER!, 'settings.json')
/// #else
    return ''
/// #endif
}

export function saveLocalSettings(s: SquadOvLocalSettings) {
/// #if DESKTOP
    fs.writeFileSync(getSettingsFname(), JSON.stringify(s), {
        encoding: 'utf-8',
    })
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
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
            }
        case BaselineLevel.Medium:
            record = {
                resY: 720,
                fps: 60,
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
            }
        case BaselineLevel.High:
            record = {
                resY: 1080,
                fps: 60,
                outputDevice: 'Default Device',
                outputVolume: 1.0,
                inputDevice: 'Default Device',
                inputVolume: 1.0,
            }
    }

    return {
        record
    }
/// #else
    return {
        record: {
            resY: 1080,
            fps: 60,
            outputDevice: 'Default Device',
            outputVolume: 1.0,
            inputDevice: 'Default Device',
            inputVolume: 1.0,
        }
    }
/// #endif
}

export async function loadLocalSettings(): Promise<SquadOvLocalSettings> {
/// #if DESKTOP
    const settingsFname = getSettingsFname()
    console.log('Loading local settings...', settingsFname)
    if (!fs.existsSync(settingsFname)) {
        saveLocalSettings(await generateDefaultSettings())
    }

    let data = fs.readFileSync(settingsFname , 'utf8')
    let parsedData = JSON.parse(data)

    if (!parsedData.record.outputDevice) {
        parsedData.record.outputDevice = 'Default Device'
    }

    if (!parsedData.record.outputVolume) {
        parsedData.record.outputVolume = 1.0
    }

    if (!parsedData.record.inputDevice) {
        parsedData.record.inputDevice = 'Default Device'
    }

    if (!parsedData.record.inputVolume) {
        parsedData.record.inputVolume = 1.0
    }

    saveLocalSettings(parsedData)
    return parsedData
/// #else
    return await generateDefaultSettings()
/// #endif
}