export enum HearthstoneZone {
	Invalid,
	Play,
	Deck,
	Hand,
	Graveyard,
	RemovedFromGame,
	Setaside,
	Secret
}

export function hearthstoneZoneFromString(str: string) : HearthstoneZone {
    if (str == 'PLAY') {
        return HearthstoneZone.Play
    } else if (str == 'DECK') {
        return HearthstoneZone.Deck
    } else if (str == 'HAND') {
        return HearthstoneZone.Hand
    } else if (str == 'GRAVEYARD') {
        return HearthstoneZone.Graveyard
    } else if (str == 'REMOVEDFROMGAME') {
        return HearthstoneZone.RemovedFromGame
    } else if (str == 'SETASIDE') {
        return HearthstoneZone.Setaside
    } else if (str == 'SECRET') {
        return HearthstoneZone.Secret
    } else {
        return HearthstoneZone.Invalid
    }
}