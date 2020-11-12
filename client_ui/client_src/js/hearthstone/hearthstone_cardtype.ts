export enum HearthstoneCardtype {
    Invalid = 0,
	Game = 1,
	Player = 2,
	Hero = 3,
	Minion = 4,
	Spell = 5,
	Enchantment = 6,
	Weapon = 7,
	Item = 8,
	Token = 9,
	HeroPower = 10,
	Blank = 11,
	GameModeButton = 12,
	MoveMinionHoverOverTarget = 22
}

export function hearthstoneCardTypeFromString(str: string) : HearthstoneCardtype {
    if (str == 'GAME') {
        return HearthstoneCardtype.Game
    } else if (str == 'PLAYER') {
        return HearthstoneCardtype.Player
    } else if (str == 'HERO') {
        return HearthstoneCardtype.Hero
    } else if (str == 'MINION') {
        return HearthstoneCardtype.Minion
    } else if (str == 'SPELL') {
        return HearthstoneCardtype.Spell
    } else if (str == 'ENCHANTMENT') {
        return HearthstoneCardtype.Enchantment
    } else if (str == 'WEAPON') {
        return HearthstoneCardtype.Weapon
    } else if (str == 'ITEM') {
        return HearthstoneCardtype.Item
    } else if (str == 'TOKEN') {
        return HearthstoneCardtype.Token
    } else if (str == 'HERO_POWER') {
        return HearthstoneCardtype.HeroPower
    } else if (str == 'BLANK') {
        return HearthstoneCardtype.Blank
    } else if (str == 'GAME_MODE_BUTTON') {
        return HearthstoneCardtype.GameModeButton
    } else if (str == 'MOVE_MINION_HOVER_TARGET') {
        return HearthstoneCardtype.MoveMinionHoverOverTarget
    } else {
        return HearthstoneCardtype.Invalid
    }
}