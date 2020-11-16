import { HearthstoneMatchSnapshot, HearthstoneMatchSnapshotWrapper, cleanHearthstoneMatchSnapshotFromJson } from '@client/js/hearthstone/hearthstone_snapshot'
import { HearthstoneDeck, cleanHearthstoneDeckFromJson } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneMedalInfo, HearthstonePlayer } from '@client/js/hearthstone/hearthstone_player'
import { HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneCardtype } from '@client/js/hearthstone/hearthstone_cardtype'

export interface HearthstoneMatchLogMetadata {
    snapshotIds: string[]
    numActions: number
}

export enum HearthstoneGameType {
    Unknown = 0,
    VsAi = 1,
    VsFriend = 2,
    Tutorial = 4,
    Arena = 5,
    TestAiVsAi = 6,
    Ranked = 7,
    Casual = 8,
    TavernBrawl = 0x10,
    Tb1pVsAi = 17,
    Tb2pCoop = 18,
    FsgBrawlVsFriend = 19,
    FsgBrawl = 20,
    FsgBrawl1pVsAi = 21,
    FsgBrawl2pCoop = 22,
    Battlegrounds = 23,
    BattlegroundsFriendly = 24,
    Reserved1822 = 26,
    Reserved1823 = 27,
    PvpDrPaid = 28,
    PvpDr = 29
}

export enum HearthstoneFormatType {
    Unknown,
    Wild,
    Standard
}

export function gameTypeToString(gt : HearthstoneGameType) : string {
    switch (gt) {
        case HearthstoneGameType.Ranked:
            return "Constructed Ranked"
        case HearthstoneGameType.Casual:
        case HearthstoneGameType.VsAi:
        case HearthstoneGameType.VsFriend:
        case HearthstoneGameType.Tutorial:
            return "Constructed Casual"
        case HearthstoneGameType.Arena:
            return "Arena"
        case HearthstoneGameType.TavernBrawl:
        case HearthstoneGameType.Tb1pVsAi:
        case HearthstoneGameType.Tb2pCoop:
        case HearthstoneGameType.FsgBrawlVsFriend:
        case HearthstoneGameType.FsgBrawl:
        case HearthstoneGameType.FsgBrawl1pVsAi:
        case HearthstoneGameType.FsgBrawl2pCoop:
            return "Brawl"
        case HearthstoneGameType.Battlegrounds:
        case HearthstoneGameType.BattlegroundsFriendly:
            return "Battlegrounds"
        case HearthstoneGameType.PvpDrPaid:
        case HearthstoneGameType.PvpDr:
            return "Duels"
        default:
            return 'Unknown'
    }
}

export function formatTypeToString(ft: HearthstoneFormatType) : string {
    switch (ft) {
        case HearthstoneFormatType.Unknown:
            return 'Unknown'
        case HearthstoneFormatType.Standard:
            return 'Standard'
        case HearthstoneFormatType.Wild:
            return 'Wild'
    }
}

export function isGameTypeConstructed(gt: HearthstoneGameType) : boolean {
    return gt == HearthstoneGameType.Ranked || gt == HearthstoneGameType.Casual
}

export function isGameTypeRanked(gt: HearthstoneGameType) : boolean {
    return gt == HearthstoneGameType.Ranked
}

export function constructGameTypeString(gt: HearthstoneGameType, ft: HearthstoneFormatType) : string {
    let base = gameTypeToString(gt)

    if (ft != HearthstoneFormatType.Unknown) {
        base += ` - ${formatTypeToString(ft)}`
    }

    return base
}

export interface HearthstoneMatchMetadata {
    gameType: HearthstoneGameType
    formatType: HearthstoneFormatType
    scenarioId: number
    matchTime: Date
    elapsedSeconds: number
    deck: HearthstoneDeck | null
    players: { [id: number] : HearthstonePlayer }
}

function cleanHearthstoneMatchMetadataFromJson(m: HearthstoneMatchMetadata) : HearthstoneMatchMetadata {
    m.matchTime = new Date(m.matchTime)
    if (!!m.deck) {
        cleanHearthstoneDeckFromJson(m.deck)
    }
    return m
}

export interface HearthstoneMatch {
    matchUuid: string
    metadata: HearthstoneMatchMetadata
    logMetadata: HearthstoneMatchLogMetadata
    latestSnapshot: HearthstoneMatchSnapshot | null
}

export function cleanHearthstoneMatchFromJson(m : HearthstoneMatch) : HearthstoneMatch {
    cleanHearthstoneMatchMetadataFromJson(m.metadata)
    if (!!m.latestSnapshot) {
        cleanHearthstoneMatchSnapshotFromJson(m.latestSnapshot)
    }
    return m
}

export class HearthstoneMatchWrapper {
    _match: HearthstoneMatch
    _snapshot: HearthstoneMatchSnapshotWrapper

    constructor(m : HearthstoneMatch) {
        this._match = m
        this._snapshot = new HearthstoneMatchSnapshotWrapper(this._match.latestSnapshot)
    }

    entity(entityId: number): HearthstoneEntityWrapper | undefined {
        return this._snapshot.entity(entityId)
    }

    playerEntity(playerId: number) : HearthstoneEntityWrapper | undefined {
        let entityId = this._snapshot.playerIdToEntityId(playerId)
        if (!entityId) {
            return undefined
        }
        return this.entity(entityId)
    }

    playerHeroEntity(playerId: number): HearthstoneEntityWrapper | undefined {
        let entities = this._snapshot.entitiesForPlayerId(playerId)
        for (let e of entities) {
            if (e.controller == playerId && e.cardType == HearthstoneCardtype.Hero) {
                return e
            }
        }
        return undefined
    }

    playerHeroCard(playerId: number): string | undefined {
        return this.playerHeroEntity(playerId)?.cardId
    }

    medalsForPlayer(playerId: number): HearthstoneMedalInfo | undefined {
        if (this._match.metadata.formatType == HearthstoneFormatType.Standard) {
            return this.player(playerId)?.medalInfo.standard
        } else {
            return this.player(playerId)?.medalInfo.wild
        }
    }

    player(playerId: number): HearthstonePlayer | undefined {
        return this._match.metadata.players[playerId]
    }

    get currentPlayerId(): number | undefined {
        for (let [pid, player] of Object.entries(this._match.metadata.players)) {
            let typedPlayer: HearthstonePlayer = player
            if (!typedPlayer.local) {
                continue
            }
            return parseInt(pid)
        }
        return undefined
    }

    get currentPlayer(): HearthstonePlayer | undefined {
        if (!this.currentPlayerId) {
            return undefined
        }
        return this.player(this.currentPlayerId)
    }

    get currentPlayerEntity(): HearthstoneEntityWrapper | undefined {
        if (!this.currentPlayerId) {
            return undefined
        }

        return this.playerEntity(this.currentPlayerId)
    }

    get currentPlayerHeroEntity(): HearthstoneEntityWrapper | undefined {
        if (!this.currentPlayerId) {
            return undefined
        }

        return this.playerHeroEntity(this.currentPlayerId)
    }

    get opposingPlayerId(): number | undefined {
        for (let [pid, player] of Object.entries(this._match.metadata.players)) {
            let typedPlayer: HearthstonePlayer = player
            if (typedPlayer.local) {
                continue
            }
            return parseInt(pid)
        }
        return undefined
    }

    get opposingPlayer(): HearthstonePlayer | undefined {
        if (!this.opposingPlayerId) {
            return undefined
        }
        return this.player(this.opposingPlayerId)
    }

    get opposingPlayerEntity(): HearthstoneEntityWrapper | undefined {
        if (!this.opposingPlayerId) {
            return undefined
        }

        return this.playerEntity(this.opposingPlayerId)
    }

    get opposingPlayerHeroEntity(): HearthstoneEntityWrapper | undefined {
        if (!this.opposingPlayerId) {
            return undefined
        }

        return this.playerHeroEntity(this.opposingPlayerId)
    }

    get matchTime(): Date {
        return this._match.metadata.matchTime
    }

    get displayedGameMode(): string {
        return constructGameTypeString(this._match.metadata.gameType, this._match.metadata.formatType)
    }

    get numTurns() : number {
        return this._snapshot.currentTurn
    }

    get elapsedSeconds() : number {
        return this._match.metadata.elapsedSeconds
    }

    get isRanked() : boolean {
        return isGameTypeRanked(this._match.metadata.gameType)
    }
}