/// #if DESKTOP
import fs from 'fs'
import path from 'path'
import { detectComputerBaselineLevel, BaselineLevel, baselineToString } from '@client/js/system/baseline'

export interface SquadOvRecordingSettings {
    resY: number
    fps: number
}

export interface SquadOvLocalSettings {
    record: SquadOvRecordingSettings
}

function getSettingsFname() : string {
    return path.join(process.env.SQUADOV_USER_APP_FOLDER!, 'settings.json')
}

export function saveLocalSettings(s: SquadOvLocalSettings) {
    fs.writeFileSync(getSettingsFname(), JSON.stringify(s), {
        encoding: 'utf-8',
    })
}

export async function generateDefaultSettings(): Promise<SquadOvLocalSettings> {
    console.log('Generating Default Settings...')
    let baseline = await detectComputerBaselineLevel()
    console.log('Detect Baseline: ', baselineToString(baseline))

    let record: SquadOvRecordingSettings
    switch (baseline) {
        case BaselineLevel.Low:
            record = {
                resY: 720,
                fps: 30
            }
        case BaselineLevel.Medium:
            record = {
                resY: 720,
                fps: 60
            }
        case BaselineLevel.High:
            record = {
                resY: 1080,
                fps: 60
            }
    }

    return {
        record
    }
}

export async function loadLocalSettings(): Promise<SquadOvLocalSettings> {
    const settingsFname = getSettingsFname()
    console.log('Loading local settings...', settingsFname)
    if (!fs.existsSync(settingsFname)) {
        saveLocalSettings(await generateDefaultSettings())
    }

    let data = fs.readFileSync(settingsFname , 'utf8')
    return JSON.parse(data)
}
/// #endif