const STATIC_CONTENT_BASE_URL = 'https://us-central1.content.squadov.gg'
const WOW_PATCH = '9.0.2'

class StaticDataClient {

    constructor() {
    
    }

    getHearthstoneCardImageUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/card.png`
    }

    getHearthstoneCardPortraitUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/portrait.png`
    }

    getWowClassIconUrl(classId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/class/${classId}/icon.png`
    }

    getWowClassDataUrl(classId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/class/${classId}/data.json`
    }

    getWowSpecsDataUrl(specId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/specs/${specId}/data.json`
    }

    getWowSpecsIconUrl(specId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/specs/${specId}/icon.png`
    }

    getWowInstanceBackgroundUrl(instanceId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/instances/${instanceId}/background.png`
    }

    getWowInstanceDataUrl(instanceId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/instances/${instanceId}/data.json`
    }

    getWowDifficultyDataUrl(diffId: number | string): string {
        return `${STATIC_CONTENT_BASE_URL}/wow/${WOW_PATCH}/difficulty/${diffId}/data.json`
    }
}

export let staticClient = new StaticDataClient()