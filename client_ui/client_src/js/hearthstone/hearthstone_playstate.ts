export enum HearthstonePlayState {
	Invalid,
	Playing,
	Winning,
	Losing,
	Won,
	Lost,
	Tied,
	Disconnected,
	Conceded
}

export function hearthstonePlayStateFromString(str: string) : HearthstonePlayState {
    if (str == 'PLAYING') {
        return HearthstonePlayState.Playing
    } else if (str == 'WINNING') {
        return HearthstonePlayState.Winning
    } else if (str == 'LOSING') {
        return HearthstonePlayState.Losing
    } else if (str == 'WON') {
        return HearthstonePlayState.Won
    } else if (str == 'LOST') {
        return HearthstonePlayState.Lost
    } else if (str == 'TIED') {
        return HearthstonePlayState.Tied
    } else if (str == 'DISCONNECTED') {
        return HearthstonePlayState.Disconnected
    } else if (str == 'CONCEDED') {
        return HearthstonePlayState.Conceded
    } else {
        return HearthstonePlayState.Invalid
    }
}