import { HearthstoneMatchSnapshot, HearthstoneMatchSnapshotWrapper, cleanHearthstoneMatchSnapshotFromJson } from '@client/js/hearthstone/hearthstone_snapshot'
import { HearthstoneDeck, cleanHearthstoneDeckFromJson } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneMedalInfo, HearthstonePlayer } from '@client/js/hearthstone/hearthstone_player'
import { HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneCardtype } from '@client/js/hearthstone/hearthstone_cardtype'
import { HearthstoneGameAction, HearthstoneGameBlock, HearthstoneGameBlockWrapper, cleanHearthstoneGameActionFromJson } from '@client/js/hearthstone/hearthstone_actions'
import { HearthstoneGameStep } from '@client/js/hearthstone/hearthstone_game_step'
import { HearthstoneZone } from '@client/js/hearthstone/hearthstone_zone'

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

    if (gt == HearthstoneGameType.Ranked || gt == HearthstoneGameType.Casual) {
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
    latestSnapshot: HearthstoneMatchSnapshot | null
}

export function cleanHearthstoneMatchFromJson(m : HearthstoneMatch) : HearthstoneMatch {
    cleanHearthstoneMatchMetadataFromJson(m.metadata)
    if (!!m.latestSnapshot) {
        cleanHearthstoneMatchSnapshotFromJson(m.latestSnapshot)
    }
    return m
}

export interface HearthstoneMatchLogs {
    snapshots: HearthstoneMatchSnapshot[]
    actions: HearthstoneGameAction[]
    blocks: HearthstoneGameBlock[]
}

export function cleanHearthstoneMatchLogsFromJson(m: HearthstoneMatchLogs) : HearthstoneMatchLogs {
    m.snapshots.forEach(cleanHearthstoneMatchSnapshotFromJson)
    m.actions.forEach(cleanHearthstoneGameActionFromJson)
    return m
}

export class HearthstoneMatchWrapper {
    _match: HearthstoneMatch
    _logs: HearthstoneMatchLogs | null
    _snapshot: HearthstoneMatchSnapshotWrapper

    // Match Logs
    _initialSnapshot: HearthstoneMatchSnapshotWrapper | null
    _allSnapshots: HearthstoneMatchSnapshotWrapper[]
    _allBlocks: Map<string, HearthstoneGameBlockWrapper>
    _turnToSnapshot: Map<number, HearthstoneMatchSnapshotWrapper>

    constructor(m : HearthstoneMatch) {
        this._match = m
        this._logs = null
        this._snapshot = new HearthstoneMatchSnapshotWrapper(this._match.latestSnapshot)
        this._initialSnapshot = null
        this._allSnapshots = []
        this._allBlocks = new Map()
        this._turnToSnapshot = new Map()
    }

    addLogs(logs: HearthstoneMatchLogs) {
        this._logs = logs
        this._initialSnapshot = (this._logs.snapshots.length > 0) ? new HearthstoneMatchSnapshotWrapper(this._logs.snapshots[0]) : null
        let previousIndex = -1
        for (let l of this._logs.snapshots) {
            let wrapper = new HearthstoneMatchSnapshotWrapper(l)
            let turn = wrapper.currentTurn
            let step = wrapper.step

            // MainNext handles all the usual turns and FinalGameover handles the last turn.
            if (step == HearthstoneGameStep.MainNext || step == HearthstoneGameStep.FinalGameover) {
                this._turnToSnapshot.set(turn, wrapper)
            } else {
                continue
            }
            
            if (l.auxData?.lastActionIndex !== undefined) {
                wrapper.addActions(this._logs.actions.slice(previousIndex + 1, l.auxData.lastActionIndex))
                previousIndex = l.auxData.lastActionIndex
            }
            this._allSnapshots.push(wrapper)
        }

        // Assign blocks to snapshots. Note that blocks come in order of the *last action index*
        // and each block should only be assignable to a single snapshot so we use the last action index
        // to figure out which snapshot a block belongs to.
        let snapshotIdx = 0
        for (let b of this._logs.blocks) {
            while (snapshotIdx < this._allSnapshots.length && b.endActionIndex > this._allSnapshots[snapshotIdx]._snapshot?.auxData?.lastActionIndex!) {
                ++snapshotIdx
            }

            if (snapshotIdx == this._allSnapshots.length) {
                console.log('Unable to find snapshot for block: ', b.blockId, b.endActionIndex)
                break
            }

            let snapshot = this._allSnapshots[snapshotIdx]
            let wrapper = new HearthstoneGameBlockWrapper(b, snapshot, this._snapshot)
            wrapper.addActions(this._logs.actions.slice(b.startActionIndex, b.endActionIndex + 1))
            snapshot.addBlock(wrapper)
            this._allBlocks.set(b.blockId, wrapper)
        }

        // Note that blocks may also have *parent blocks* in which case they should just be nested under the block that already exists.
        // We take a second pass to handle this to ensure that we've recorded every block already.
        for (let b of this._logs.blocks) {
            if (!b.parentBlock) {
                continue
            }

            let parent = this._allBlocks.get(b.parentBlock)
            let child = this._allBlocks.get(b.blockId)

            if (!parent || !child) {
                continue
            }

            parent.addSubBlock(child)
        }
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
            if (e.controller == playerId 
                && e.cardType == HearthstoneCardtype.Hero
                // Needs to also check the graveyard since for battlegrounds if we lose, the latest snapshot will
                // stick the user's hero there (since they hit 0 HP).
                && (e.zone == HearthstoneZone.Play || e.zone == HearthstoneZone.Graveyard)
                // In the very specific case where the current match is for Battlegrounds we want to check for the PLAYER_ID
                // tag as that'll be set on heroes that were chosen.
                && ((this.isBattlegrounds && e.playerId > 0) || !this.isBattlegrounds)
            ) {
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

    snapshotForTurn(turn: number): HearthstoneMatchSnapshotWrapper | undefined {
        if (!this._turnToSnapshot.has(turn)) {
            return undefined
        }
        return this._turnToSnapshot.get(turn)
    }

    isTurnForCurrentPlayer(turn : number) : boolean {
        if (!this.currentPlayerId) {
            return false
        }

        let ss = this.snapshotForTurn(turn)
        if (!ss) {
            console.log('No snapshot for turn: ', turn)
            return false
        }

        return ss.currentPlayerId == this.currentPlayerId
    }

    // Number of seconds since the start of the match that the block was triggered
    matchTimeForBlock(b : HearthstoneGameBlockWrapper) : number {
        if (!b.blockTime) {
            return 0
        }
        return Math.round((b.blockTime.getTime() - this._match.metadata.matchTime.getTime()) / 1000)
    }

    computeDeckForPlayer(p: number): HearthstoneDeck | undefined {
        // We can use the first snapshot to determine all the cards that are in a player's deck and then
        // we can use the final snapshot to determine which cards they are.
        if (!this._initialSnapshot) {
            return undefined
        }

        let player = this.player(p)
        if (!player) {
            return undefined
        }

        // For each card, how many are there in the deck.
        let cardCount : Map<string, number> = new Map()
        for (let entity of this._initialSnapshot.entitiesForPlayerId(p)) {
            // It's important we check for the deck here because it's possible that the game
            // creates other cards that aren't a part of the user's constructed deck in the beginning
            // (e.g. the hero card). Note that we need to allow for cards in the hand already too
            // as the game will put cards in the player's hand almost immediately.
            if (entity.zone != HearthstoneZone.Deck && entity.zone != HearthstoneZone.Hand) {
                continue
            }

            // It's critical that we pull the same entity from the final snapshot to determine the card ID.
            // If there is no card ID then we *do not know* what card this is so we just ignore it.
            let cardId = this._snapshot.entity(entity._entity.entityId)?.cardId
            if (!cardId || cardId == '') {
                continue
            }

            if (cardCount.has(cardId)) {
                cardCount.set(cardId, cardCount.get(cardId)! + 1)
            } else {
                cardCount.set(cardId, 1)
            }
        }

        // Now we just need to translate all this data into our own deck format.
        return {
            name: `${player.name}'s Deck`,
            deckId: 0,
            heroCard: '',
            heroPremium: 0,
            deckType: 0,
            createDate: new Date(),
            isWild: false,
            slots: Array.from(cardCount.keys()).map((cardId : string) => {
                return {
                    index: 0,
                    cardId: cardId,
                    owned: true,
                    count: {
                        normal: cardCount.get(cardId)!,
                        golden: 0
                    }
                }
            })
        }
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

    get currentPlayerDeck(): HearthstoneDeck | undefined {
        if (!!this._match.metadata.deck) {
            return this._match.metadata.deck
        }

        if (!this.currentPlayerId) {
            return undefined
        }
        return this.computeDeckForPlayer(this.currentPlayerId)
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

    get opposingPlayerDeck(): HearthstoneDeck | undefined {
        if (!this.opposingPlayerId) {
            return undefined
        }
        return this.computeDeckForPlayer(this.opposingPlayerId)
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

    get isBattlegrounds() : boolean {
        return this._match.metadata.gameType == HearthstoneGameType.Battlegrounds ||
        this._match.metadata.gameType == HearthstoneGameType.BattlegroundsFriendly
    }
}