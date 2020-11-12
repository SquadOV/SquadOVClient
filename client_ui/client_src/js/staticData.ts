const STATIC_CONTENT_BASE_URL = 'https://us-central1.content.squadov.gg'

class StaticDataClient {

    constructor() {
    
    }

    getHearthstoneCardImageUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/card.png`
    }

    getHearthstoneCardPortraitUrl(cardId: string) : string {
        return `${STATIC_CONTENT_BASE_URL}/hearthstone/Cards/${cardId}/portrait.png`
    }
}

export let staticClient = new StaticDataClient()