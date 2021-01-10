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

export enum HearthstoneCardRace {
	Invalid = 0,
	Bloodelf = 1,
	Dranei = 2,
	Dwarf = 3,
	Gnome = 4,
	Goblin = 5,
	Human = 6,
	NightElf = 7,
	Orc = 8,
	Tauren = 9,
	Troll = 10,
	Undead = 11,
	Worgen = 12,
	Goblin2 = 13,
	Murloc = 14,
	Demon = 0xF,
	Scourge = 0x10,
	Mechanical = 17,
	Elemental = 18,
	Ogre = 19,
	Pet = 20,
	Totem = 21,
	Nerubian = 22,
	Pirate = 23,
	Dragon = 24,
	Blank = 25,
	All = 26,
	Egg = 38
}

export function cardRaceToString(r: HearthstoneCardRace) : string {
    switch (r) {
        case HearthstoneCardRace.Pet:
            return 'Beast'
        case HearthstoneCardRace.Demon:
            return 'Demon'
        case HearthstoneCardRace.Dragon:
            return 'Dragon'
        case HearthstoneCardRace.Elemental:
            return 'Elemental'
        case HearthstoneCardRace.Mechanical:
            return 'Mech'
        case HearthstoneCardRace.Murloc:
            return 'Murloc'
        case HearthstoneCardRace.Pirate:
            return 'Pirate'
        case HearthstoneCardRace.Invalid:
            return 'Neutral'
    }
    return 'Other'
}