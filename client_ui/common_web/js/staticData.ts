const STATIC_CONTENT_BASE_URL = 'https://us-central1.content.squadov.gg'
const SUPPORTED_WOW_PATCHES = new Set([
    '1.13.7',
    '2.5.2',
    '9.0.2',
    '9.0.5',
    '9.1.0',
    '9.1.5',
])
const LATEST_WOW_PATCH = '9.1.5'
const LATEST_WOW_VANILLA_PATCH = '1.13.7'
const LATEST_WOW_TBC_PATCH = '2.5.2'

import { SquadOvWowRelease as WowGameRelease } from '@client/js/squadov/game'
export { WowGameRelease }

export function wowGameReleaseFromString(s: string | undefined): WowGameRelease {
    if (s == 'vanilla') {
        return WowGameRelease.Vanilla
    } else if (s == 'tbc') {
        return WowGameRelease.Tbc
    } else {
        return WowGameRelease.Retail
    }
}

function wowGameReleaseToStaticFolder(r: WowGameRelease) : string {
    switch (r) {
        case WowGameRelease.Retail:
            return 'content'
        case WowGameRelease.Vanilla:
            return 'content-vanilla'
        case WowGameRelease.Tbc:
            return 'content-tbc'
    }
}

export function getSupportedWowPatch(p: string): string {
    if (SUPPORTED_WOW_PATCHES.has(p)) {
        return p
    } else {
        const versionParts = p.split('.')
        if (versionParts[0] === '1') {
            return LATEST_WOW_VANILLA_PATCH
        } else if (versionParts[0] === '2') {
            return LATEST_WOW_TBC_PATCH
        } else {
            return LATEST_WOW_PATCH
        }
    }
}

class StaticDataClient {

    constructor() {
    
    }

    getHearthstoneCardImageUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/card.png`
    }

    getHearthstoneCardPortraitUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/portrait.png`
    }

    getWowClassIconUrl(wowPatch: string, classId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/class/${classId}/icon.png`
    }

    getWowClassDataUrl(wowPatch: string, classId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/class/${classId}/data.json`
    }

    getWowSpecsDataUrl(wowPatch: string, specId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/specs/${specId}/data.json`
    }

    getWowSpecsIconUrl(wowPatch: string, specId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/specs/${specId}/icon.png`
    }

    getWowInstanceBackgroundUrl(wowPatch: string, instanceId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/instances/${instanceId}/background.png`
    }

    getWowInstanceDataUrl(wowPatch: string, instanceId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/instances/${instanceId}/data.json`
    }

    getWowDifficultyDataUrl(wowPatch: string, diffId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/difficulty/${diffId}/data.json`
    }

    getWowSpellDataUrl(wowPatch: string, spellId: number) : string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/spells/${spellId}/data.json`
    }

    getWowSpellIconUrl(wowPatch: string, spellId: number) : string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/spells/${spellId}/icon.png`
    }

    getWowItemDataUrl(wowPatch: string, itemId: number) : string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/items/${itemId}/data.json`
    }

    getWowItemIconUrl(wowPatch: string, itemId: number) : string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/items/${itemId}/icon.png`
    }

    getWowCovenantDataUrl(wowPatch: string, id: number): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/covenants/${id}/data.json`
    }

    getWowSoulbindDataUrl(wowPatch: string, id: number): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/soulbinds/${id}/data.json`
    }

    getWowConduitDataUrl(wowPatch: string, id: number): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/conduits/${id}/data.json`
    }

    getWowTalentDataUrl(wowPatch: string, id: number): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/talents/${id}/data.json`
    }

    getWowTalentIconUrl(wowPatch: string, id: number): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${getSupportedWowPatch(wowPatch)}/talents/${id}/icon.png`
    }

    getWowContentArenaUrl(r: WowGameRelease): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${wowGameReleaseToStaticFolder(r)}/arenas.json`
    }

    getWowContentDungeonsUrl(r: WowGameRelease): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${wowGameReleaseToStaticFolder(r)}/dungeons.json`
    }

    getWowContentRaidsUrl(r: WowGameRelease): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${wowGameReleaseToStaticFolder(r)}/raids.json`
    }

    getWowContentEncountersUrl(r: WowGameRelease): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${wowGameReleaseToStaticFolder(r)}/encounters.json`
    }

    getWowContentInstancesUrl(r: WowGameRelease): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${wowGameReleaseToStaticFolder(r)}/instances.json`
    }

    getTftLittleLegendIcon(id: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/companions/${id}/portrait.png`
    }
    
    getTftTraitIcon(set: string, trait: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/traits/${trait}/icon.png`
    }

    getTftTraitData(set: string, trait: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/traits/${trait}/data.json`
    }

    getTftUnitIcon(set: string, unit: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/champions/${unit}/icon.png`
    }

    getTftUnitData(set: string, unit: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/champions/${unit}/data.json`
    }

    getTftItemIcon(set: string, item: number): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/items/${item}/icon.png`
    }

    getTftItemData(set: string, item: number): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/full/items/${item}/data.json`
    }
}

export let staticClient = new StaticDataClient()