const STATIC_CONTENT_BASE_URL = 'https://us-central1.content.squadov.gg'
const SUPPORTED_WOW_PATCHES = new Set([
    '9.0.2',
    '9.0.5',
    '9.1.0',
])
const LATEST_WOW_PATCH = '9.1.0'

export function getSupportedWowPatch(p: string): string {
    if (SUPPORTED_WOW_PATCHES.has(p)) {
        return p
    } else {
        return LATEST_WOW_PATCH
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

    getWowContentArenaUrl(): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/content/arenas.json`
    }

    getWowContentDungeonsUrl(): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/content/dungeons.json`
    }

    getWowContentRaidsUrl(): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/content/raids.json`
    }

    getWowContentEncountersUrl(): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/content/encounters.json`
    }

    getTftLittleLegendIcon(id: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/companions/${id}/portrait.png`
    }
    
    getTftTraitIcon(set: string, trait: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/traits/${trait}/icon.png`
    }

    getTftTraitData(set: string, trait: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/traits/${trait}/data.json`
    }

    getTftUnitIcon(set: string, unit: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/champions/${unit}/icon.png`
    }

    getTftUnitData(set: string, unit: string): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/champions/${unit}/data.json`
    }

    getTftItemIcon(set: string, item: number): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/items/${item}/icon.png`
    }

    getTftItemData(set: string, item: number): string {
        return `${STATIC_CONTENT_BASE_URL}/tft/set${set}/items/${item}/data.json`
    }
}

export let staticClient = new StaticDataClient()