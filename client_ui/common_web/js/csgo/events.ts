import { SteamAccount } from '@client/js/steam/account'

export enum CsgoEventSource {
    Gsi,
    Demo
}

export enum CsgoBombStatus {
    Planted,
    Defused,
    Exploded,
    Unknown
}

export enum CsgoBombSite {
    SiteA,
    SiteB,
    SiteUnknown
}

export function bombSiteToString(c: CsgoBombSite): string {
    if (c == CsgoBombSite.SiteA) {
        return 'A'
    } else if (c == CsgoBombSite.SiteB) {
        return 'B'
    } else {
        return ''
    }
}

export enum CsgoTeam {
    CT,
    Terrorist,
    Spectate
}

export function getCsgoOppositeTeam(t: CsgoTeam): CsgoTeam {
    if (t == CsgoTeam.CT) {
        return CsgoTeam.Terrorist
    } else if (t == CsgoTeam.Terrorist) {
        return CsgoTeam.CT
    } else {
        return CsgoTeam.Spectate
    }
}

export enum CsgoRoundWinReason {
    TargetBombed = 1,
    BombDefused = 7,
    CTWin,
    TWin,
    TSurrender = 17,
    CTSurrender,
    Unknown,
}

export enum CsgoWeapon {
    Unknown,
    Knife,
    Bomb,
    Taser,
    // Pistols
    P2000,
    Usps,
    Glock,
    P250,
    FiveSeven,
    Tec9,
    Cz75,
    DuelBerettas,
    Deagle,
    R8,
    // SMG
    Mp9,
    Mac10,
    PpBizon,
    Mp7,
    Ump45,
    P90,
    Mp5,
    // Rifle
    Famas,
    Galil,
    M4a4,
    M4a1s,
    Ak47,
    Aug,
    Sg553,
    Scout,
    Awp,
    Scar20,
    G3sg1,
    // Heavy
    Nova,
    Xm1014,
    Mag7,
    SawedOff,
    M249,
    Negev,
    // Grenades
    He,
    Flashbang,
    Smoke,
    Decoy,
    Molotov,
    Incendiary,
}

export function getCsgoPrimaryWeapon(w: CsgoWeapon[]): CsgoWeapon {
    let candidates = w.filter((a: CsgoWeapon) => a >= CsgoWeapon.Mp9 && a <= CsgoWeapon.Negev)
    if (candidates.length > 0) {
        return candidates[0]
    } else {
        return getCsgoSidearmWeapon(w)
    }
}

export function getCsgoSidearmWeapon(w: CsgoWeapon[]): CsgoWeapon {
    let candidates = w.filter((a: CsgoWeapon) => a >= CsgoWeapon.P2000 && a <= CsgoWeapon.R8)
    if (candidates.length > 0) {
        return candidates[0]
    } else {
        return CsgoWeapon.Knife
    }
}

export function getCsgoSecondaryWeapons(w: CsgoWeapon[]): CsgoWeapon[] {
    return w.filter((a: CsgoWeapon) => a >= CsgoWeapon.He || a == CsgoWeapon.Taser)
}

export enum CsgoHitgroup {
    Generic = 0,
    Head,
    Chest,
    Stomach,
    LeftArm,
    RightArm,
    LeftLeg,
    RightLeg,
    Gear
}

export interface CsgoRoundDamage {
    containerId: number
    roundNum: number
    tm: Date
    receiver: number
    attacker: number | null
    remainingHealth: number
    remainingArmor: number
    damageHealth: number
    damageArmor: number
    weapon: CsgoWeapon
    hitgroup: CsgoHitgroup
}

export function cleanCsgoRoundDamageFromJson(d: CsgoRoundDamage): CsgoRoundDamage {
    d.tm = new Date(d.tm)
    return d
}

export interface CsgoRoundKill {
    containerId: number
    roundNum: number
    tm: Date
    victim: number | null
    killer: number | null
    assister: number | null
    flashAssist: boolean | null
    headshot: boolean | null
    smoke: boolean | null
    blind: boolean | null
    wallbang: boolean | null
    noscope: boolean | null
    weapon: CsgoWeapon | null
}

export function cleanCsgoRoundKillFromJson(k: CsgoRoundKill): CsgoRoundKill {
    k.tm = new Date(k.tm)
    return k
}

export interface CsgoRoundPlayerStats {
    containerId: number
    roundNum: number
    userId: number
    kills: number
    deaths: number
    assists: number
    mvp: boolean
    equipmentValue: number | null
    money: number | null
    headshotKills: number | null
    utilityDamage: number | null
    enemiesFlashed: number | null
    damage: number | null
    armor: number | null
    hasDefuse: boolean | null
    hasHelmet: boolean | null
    team: CsgoTeam
    weapons: CsgoWeapon[]
}

export interface CsgoEventRound {
    containerId: number
    roundNum: number
    tmRoundStart: Date | null
    tmRoundPlay: Date | null
    tmRoundEnd: Date | null
    bombState: CsgoBombStatus | null
    tmBombPlant: Date | null
    bombPlantUser: number | null
    bombPlantSite: CsgoBombSite | null
    tmBombEvent: Date | null
    bombEventUser: number | null
    winningTeam: CsgoTeam | null
    roundWinReason: CsgoRoundWinReason | null
    roundMvp: number | null
    playerStats: CsgoRoundPlayerStats[]
    kills: CsgoRoundKill[]
    damage: CsgoRoundDamage[]
}

export function cleanCsgoEventRoundFromJson(e: CsgoEventRound): CsgoEventRound {
    if (!!e.tmRoundStart) {
        e.tmRoundStart = new Date(e.tmRoundStart)
    }

    if (!!e.tmRoundPlay) {
        e.tmRoundPlay = new Date(e.tmRoundPlay)
    }

    if (!!e.tmRoundEnd) {
        e.tmRoundEnd = new Date(e.tmRoundEnd)
    }

    if (!!e.tmBombPlant) {
        e.tmBombPlant = new Date(e.tmBombPlant)
    }

    if (!!e.tmBombEvent) {
        e.tmBombEvent = new Date(e.tmBombEvent)
    }

    e.kills.forEach(cleanCsgoRoundKillFromJson)
    e.damage.forEach(cleanCsgoRoundDamageFromJson)
    return e
}

export interface CsgoEventPlayer {
    containerId: number
    userId: number
    steamAccount: SteamAccount
    kills: number
    deaths: number
    assists: number
    mvps: number
}

export interface CsgoEventContainer {
    id: number
    viewUuid: string
    eventSource: CsgoEventSource,
    rounds: CsgoEventRound[],
    players: CsgoEventPlayer[]
}

export function cleanCsgoEventContainerFromJson(e: CsgoEventContainer): CsgoEventContainer {
    e.rounds.forEach(cleanCsgoEventRoundFromJson)
    return e
}