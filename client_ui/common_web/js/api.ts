import axios from 'axios'

import { RiotAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import {
    ValorantPlayerMatchSummary,
    ValorantMatchDetails,
    cleanValorantPlayerMatchSummary,
    cleanValorantMatchDetails,
    ValorantMatchPlayerMatchMetadata,
    cleanValorantMatchPlayerMatchMetadata,
} from '@client/js/valorant/valorant_matches'
import { AimlabTaskData, cleanAimlabTaskData } from '@client/js/aimlab/aimlab_task'
import {
    VodAssociation,
    cleanVodAssocationData,
    VodManifest,
    ValorantMatchAccessibleVods,
    HearthstoneMatchAccessibleVods,
    WowMatchAccessibleVods,
    LeagueMatchAccessibleVods,
    TftMatchAccessibleVods,
    VodMetadata,
    VodClip,
    cleanVodClipFromJson,
    ClipReact,
    ClipComment,
    cleanClipCommentFromJson
} from '@client/js/squadov/vod'
import { HearthstoneMatch, HearthstoneMatchLogs, cleanHearthstoneMatchFromJson, cleanHearthstoneMatchLogsFromJson } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneEntity } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneCardMetadata, HearthstoneBattlegroundsCardMetadata } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneGameType } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneArenaRun, cleanHearthstoneArenaRunFromJson } from '@client/js/hearthstone/hearthstone_arena'
import { HearthstoneDuelRun, cleanHearthstoneDuelRunFromJson } from '@client/js/hearthstone/hearthstone_duel'
import { HearthstoneGameAction, cleanHearthstoneGameActionFromJson } from '@client/js/hearthstone/hearthstone_actions'
import {
    Squad, cleanSquadFromJson,
    SquadMembership, cleanSquadMembershipFromJson
    SquadInvite, cleanSquadInviteFromJson
} from '@client/js/squadov/squad'
import {
    SquadOvHeartbeatResponse,
    cleanSquadOvHeartbeatResponse
} from '@client/js/squadov/user'
import {
    RiotSummoner
} from '@client/js/riot/summoner'
import { NotificationSummary } from '@client/js/squadov/notification'
import * as root from '@client/js/proto.js'
import { squadov } from '@client/js/proto'
import { longNumberToNumber } from '@client/js/long'
import { WowCharacter, WoWCharacterUserAssociation } from '@client/js/wow/character'
import {
    WowEncounter,
    cleanWowEncounterFromJson,
    WowChallenge,
    cleanWowChallengeFromJson,
    WowArena,
    cleanWowArenaFromJson,
    GenericWowMatchContainer,
    cleanGenericWowMatchContainerFromJson,
} from '@client/js/wow/matches'
import {
    SerializedWowMatchEvents,
    cleanWowMatchEventsFromJson
} from '@client/js/wow/events'
import {
    WowStatQueryParam,
    WowMatchStatContainer
} from '@client/js/wow/stats'
import {
    TftPlayerMatchSummary,
    cleanTftPlayerMatchSummaryFromJson,
    WrappedTftMatch,
    cleanWrappedTftMatchFromJson
} from '@client/js/tft/matches'
import {
    LolPlayerMatchSummary,
    cleanLolPlayerMatchSummaryFromJson,
    FullLolMatch,
    cleanFullLolMatch
} from '@client/js/lol/matches'
import {
    FeatureFlags
} from '@client/js/squadov/features'
import { TotalRecordedPlaytime } from '@client/js/squadov/playtime'
import {
    RecentMatch,
    cleanRecentMatchFromJson,
} from '@client/js/squadov/recentMatch'
import { SquadOvGames } from '@client/js/squadov/game'
import { ShareAccessTokenResponse } from '@client/js/squadov/share'
import { StatPermission } from '@client/js/stats/statLibrary'
import { uploadLocalFileToGcs } from '@client/js/gcs'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

export interface ApiData<T> {
    data: T
}

export interface HalLink {
    href: string
}

export interface HalResponse<T> {
    _links: {
        [index : string] : HalLink
    },
    data: T
}

export interface GraphqlApiData<T> {
    data: {
        data: T
    }
}

interface LoginInput {
    username: string
    password: string
}

export interface LoginOutput {
    userId: number
    sessionId: string
    verified: boolean
}

interface RegisterInput {
    username: string
    password: string
    email: string
}

export interface CheckVerificationOutput {
    verified: boolean
}

interface ChangeForgottenPasswordInput {
    changePasswordId: string
    password: string
}

import { storeSessionCookie, getSessionId } from '@client/js/session'

interface WebsocketAuthenticationResponse {
    success: boolean
}

class ApiClient {
    _sessionId: string | null
    _tempSessionId: string | null
    _tempUserId: number | null

    constructor() {
        this._sessionId = null
        this._tempSessionId = null
        this._tempUserId = null
    }

    setSessionId(s : string) {
        this._sessionId = s
    }

    setTempSessionId(s: string | null, uid: string | null) {
        this._tempSessionId = s
        if (!!uid) {
            this._tempUserId = parseInt(uid)
        } else {
            this._tempUserId = null
        }
    }

    get hasTempSession(): boolean {
        return !!this._tempSessionId && (this._tempUserId !== null)
    }

    getSessionId(): string | null {
/// #if DESKTOP
        return this._sessionId
/// #else
        return getSessionId()
/// #endif                
    }

    getTempSessionId(): string | null {
        return this._tempSessionId
    }

    clearSession() {
        this._sessionId = null
        this._tempSessionId = null
        this._tempUserId = null
    }

    setSessionFull(s: string, userId: number) {
       this.setSessionId(s)

       // This call should be used on the desktop to store this info
       // in cookies (as an alternative for storing it on the local system)
       storeSessionCookie(s, userId)
    }

    createWebAxiosConfig() : any {
        let ret : any = {
            baseURL: SQUADOV_API_URL,
        }

        if (!!this._sessionId) {
            ret.headers = {
                'x-squadov-session-id': this._sessionId,
            }
        } else if (!!this._tempSessionId) {
            ret.headers = {
                'x-squadov-share-id': this._tempSessionId,
            }
        }

        return ret
    }

    //
    // WebSocket
    //
    connectWebsocket(path: string): Promise<WebSocket> {
        let ws = new WebSocket(SQUADOV_API_URL.replace('https', 'wss').replace('http', 'ws') + path)
        return new Promise((resolve, reject) => {
            let clearWsCallbacks = () => {
                ws.onopen = null
                ws.onmessage = null
                ws.onopen = null
            }

            let onSuccess = () => {
                clearWsCallbacks()
                resolve(ws)
            }

            let onFail = (msg: any) => {
                clearWsCallbacks()
                ws.close()
                reject(msg)
            }

            ws.onopen = () => {
                ws.send(JSON.stringify({
                    type: 'Authenticate',
                    sessionId: this._sessionId,
                }))
            }

            ws.onmessage = (e: MessageEvent) => {
                let resp: WebsocketAuthenticationResponse = JSON.parse(e.data)
                if (!!resp.success) {
                    onSuccess()
                } else {
                    onFail('WebSocket Authentication Failure')
                }
            }

            ws.onclose = () => {
                onFail('Websocket closed prematurely')
            }
        })
    }

    //
    // Web server API
    //
    login(inp : LoginInput) : Promise<ApiData<LoginOutput>> {
        return axios.post('auth/login', inp, this.createWebAxiosConfig())
    }

    register(inp : RegisterInput, params: any) : Promise<void> {
        return axios.post('auth/register', inp, {
            ...this.createWebAxiosConfig(),
            params,
        })
    }

    resendVerification() : Promise<void> {
        return axios.post('auth/verify/resend', {}, this.createWebAxiosConfig())
    }

    checkVerification() : Promise<ApiData<CheckVerificationOutput>> {
        return axios.get('auth/verify', this.createWebAxiosConfig())
    }

    forgotPassword(loginId : string) : Promise<void> {
        return axios.get('auth/forgotpw', {
            ...this.createWebAxiosConfig(),
            params: {
                loginId,
            }
        })
    }

    logout() : Promise<void> {
        return axios.post('auth/logout', {}, this.createWebAxiosConfig())
    }
    
    verifyEmail(verificationId : string) : Promise<void> {
        return axios.post('auth/verify', {
            verificationId: verificationId,
        }, this.createWebAxiosConfig())
    }

    changeForgottenPassword(input: ChangeForgottenPasswordInput) : Promise<void> {
        return axios.post('auth/forgotpw/change', input, this.createWebAxiosConfig())
    }

    sessionHeartbeat(sessionId: string): Promise<ApiData<SquadOvHeartbeatResponse>> {
        return axios.post('auth/session/heartbeat', {
            sessionId
        }, this.createWebAxiosConfig()).then((resp: ApiData<SquadOvHeartbeatResponse>) => {
            cleanSquadOvHeartbeatResponse(resp.data)
            return resp
        })
    }

    getNotificationSummary(): Promise<ApiData<NotificationSummary>> {
        return axios.get(`v1/users/me/notifications`, this.createWebAxiosConfig())
    }

    getSquad(squadId: number): Promise<ApiData<Squad>> {
        return axios.get(`v1/squad/${squadId}/profile`, this.createWebAxiosConfig()).then((resp: ApiData<Squad>) => {
            cleanSquadFromJson(resp.data)
            return resp
        })
    }

    getUserSquadMembership(squadId: number, userId: number): Promise<ApiData<SquadMembership>> {
        return axios.get(`v1/squad/${squadId}/membership/${userId}`, this.createWebAxiosConfig()).then((resp: ApiData<SquadMembership>) => {
            cleanSquadMembershipFromJson(resp.data)
            return resp
        })
    }

    getUserSquads(userId: number): Promise<ApiData<SquadMembership[]>> {
        return axios.get(`v1/users/${userId}/squads`, this.createWebAxiosConfig()).then((resp : ApiData<SquadMembership[]>) => {
            resp.data.forEach(cleanSquadMembershipFromJson)
            return resp
        })
    }

    getSquadUsers(squadId: number): Promise<ApiData<SquadMembership[]>> {
        return axios.get(`v1/squad/${squadId}/membership`, this.createWebAxiosConfig()).then((resp : ApiData<SquadMembership[]>) => {
            resp.data.forEach(cleanSquadMembershipFromJson)
            return resp
        })
    }

    getSquadInvites(squadId: number): Promise<ApiData<SquadInvite[]>> {
        return axios.get(`v1/squad/${squadId}/invite`, this.createWebAxiosConfig()).then((resp : ApiData<SquadInvite[]>) => {
            resp.data.forEach(cleanSquadInviteFromJson)
            return resp
        })
    }

    getUserSquadInvites(userId: number): Promise<ApiData<SquadInvite[]>> {
        return axios.get(`v1/users/${userId}/squads/invites`, this.createWebAxiosConfig()).then((resp : ApiData<SquadInvite[]>) => {
            resp.data.forEach(cleanSquadInviteFromJson)
            return resp
        })
    }

    acceptSquadInvite(squadId: number, inviteUuid: string): Promise<void> {
        return axios.post(`v1/squad/${squadId}/invite/${inviteUuid}/accept`, {}, this.createWebAxiosConfig())
    }

    rejectSquadInvite(squadId: number, inviteUuid: string): Promise<void> {
        return axios.post(`v1/squad/${squadId}/invite/${inviteUuid}/reject`, {}, this.createWebAxiosConfig())
    }

    publicAcceptSquadInvite(squadId: number, inviteUuid: string, sig: string): Promise<void> {
        return axios.post(`public/squad/${squadId}/invite/${inviteUuid}/accept`, {}, {
            ...this.createWebAxiosConfig(),
            params: {
                sig
            },
        })
    }

    publicRejectSquadInvite(squadId: number, inviteUuid: string, sig: string): Promise<void> {
        return axios.post(`public/squad/${squadId}/invite/${inviteUuid}/reject`, {}, {
            ...this.createWebAxiosConfig(),
            params: {
                sig
            },
        })
    }

    deleteSquad(squadId: number): Promise<void> {
        return axios.delete(`v1/squad/${squadId}/admin`, this.createWebAxiosConfig())
    }

    editSquad(squadId: number, name: string): Promise<void> {
        return axios.put(`v1/squad/${squadId}/admin`, {
            squadName: name
        }, this.createWebAxiosConfig())
    }

    leaveSquad(squadId: number): Promise<void> {
        return axios.post(`v1/squad/${squadId}/leave`, {}, this.createWebAxiosConfig())
    }
    
    createSquad(squadName: string, squadGroup: string): Promise<void> {
        return axios.post(`v1/squad`, {
            squadName,
            squadGroup
        }, this.createWebAxiosConfig())
    }

    sendSquadInvite(squadId: number, usernames: string[], emails: string[]): Promise<void> {
        return axios.post(`v1/squad/${squadId}/invite`, {
            usernames,
            emails,
        }, this.createWebAxiosConfig())
    }

    revokeSquadInvite(squadId: number, inviteUuid: string) : Promise<void> {
        return axios.post(`v1/squad/${squadId}/admin/invite/${inviteUuid}/revoke`, {}, this.createWebAxiosConfig())
    }

    kickSquadMember(squadId: number, userId: number): Promise<void> {
        return axios.delete(`v1/squad/${squadId}/admin/membership/${userId}`, this.createWebAxiosConfig())
    }

    findVodFromMatchUserId(matchUuid : string, userId: number) : Promise<ApiData<VodAssociation>> {
        return axios.get(`v1/vod/match/${matchUuid}/user/id/${userId}`, this.createWebAxiosConfig()).then((resp : ApiData<VodAssociation>) => {
            cleanVodAssocationData(resp.data)
            return resp
        })
    }

    getVodManifest(videoUuid: string) : Promise<ApiData<VodManifest>> {
        return axios.get(`v1/vod/${videoUuid}`, this.createWebAxiosConfig())
    }

    getVodSegment(url : string) : Promise<ApiData<string>> {
        return axios.get(url, {
            ...this.createWebAxiosConfig(),
        })
    }

    allAimlabTaskData(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<AimlabTaskData[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/aimlab/user/${params.userId!}`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<AimlabTaskData[]>>) => {
            resp.data.data.forEach(cleanAimlabTaskData)
            return resp
        })
    }

    getAimlabTaskData(uuid : string, userId: number) : Promise<ApiData<AimlabTaskData>> {
        return axios.get(`v1/aimlab/user/${userId}/match/${uuid}/task`, this.createWebAxiosConfig()).then((resp : ApiData<AimlabTaskData>) => {
            cleanAimlabTaskData(resp.data)
            return resp
        })
    }

    listValorantMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number}) : Promise<ApiData<HalResponse<ValorantPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/valorant/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<ValorantPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanValorantPlayerMatchSummary)
            return resp
        })
    }

    getValorantPlayerStats(userId: number, puuid : string) : Promise<ApiData<ValorantPlayerStatsSummary>> {
        return axios.get(`v1/valorant/user/${userId}/accounts/${puuid}/stats`, this.createWebAxiosConfig())
    }

    getValorantMatchDetails(matchUuid : string) : Promise<ApiData<ValorantMatchDetails>> {
        return axios.get(`v1/valorant/match/${matchUuid}`, this.createWebAxiosConfig()).then((resp : ApiData<ValorantMatchDetails>) => {
            cleanValorantMatchDetails(resp.data)
            return resp
        })
    }

    getValorantMatchPlayerMetadata(matchUuid: string, puuid: string) : Promise<ApiData<ValorantMatchPlayerMatchMetadata>> {
        return axios.get(`v1/valorant/match/${matchUuid}/metadata/${puuid}`, this.createWebAxiosConfig()).then((resp : ApiData<ValorantMatchPlayerMatchMetadata>) => {
            cleanValorantMatchPlayerMatchMetadata(resp.data)
            return resp
        })
    }

    getValorantMatchAccessibleVods(matchUuid: string, userId: number): Promise<ApiData<ValorantMatchAccessibleVods>> {
        return axios.get(`v1/valorant/match/${matchUuid}/user/${userId}/vods`, this.createWebAxiosConfig()).then((resp: ApiData<ValorantMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    getHearthstoneMatchAccessibleVods(matchId: string, userId: number): Promise<ApiData<HearthstoneMatchAccessibleVods>> {
        return axios.get(`v1/hearthstone/user/${userId}/match/${matchId}/vods`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    listHearthstoneMatchesForPlayer(params : {next : string | null, userId : number, start : number, end : number, filter : HearthstoneGameType[]}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/match`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                    filter: JSON.stringify(params.filter)
                }
            })
    }

    listHearthstoneMatchesForArenaRun(collectionUuid: string, userId: number): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}/matches`, this.createWebAxiosConfig())
    }

    listHearthstoneMatchesForDuelRun(collectionUuid: string, userId: number): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}/matches`, this.createWebAxiosConfig())
    }

    listHearthstoneArenaRunsForPlayer(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/arena`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                }
            })
    }

    listHearthstoneDuelRunsForPlayer(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/duels`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                }
            })
    }

    getHearthstoneDuelRun(collectionUuid: string, userId: number): Promise<ApiData<HearthstoneDuelRun>> {
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneDuelRun>) => {
            cleanHearthstoneDuelRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneArenaRun(collectionUuid: string, userId: number): Promise<ApiData<HearthstoneArenaRun>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneArenaRun>) => {
            cleanHearthstoneArenaRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatch(matchId: string, userId: number) : Promise<ApiData<HearthstoneMatch>> {
        return axios.get(`v1/hearthstone/user/${userId}/match/${matchId}`, this.createWebAxiosConfig()).then((resp : ApiData<HearthstoneMatch>) => {
            cleanHearthstoneMatchFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatchLogs(matchId: string, userId: number) : Promise<ApiData<HearthstoneMatchLogs>> {
        return axios.get(`v1/hearthstone/user/${userId}/match/${matchId}/logs`, {
            responseType: 'arraybuffer',
            ...this.createWebAxiosConfig()
        }).then((resp: ApiData<ArrayBuffer>) => {
            let buf = new Uint8Array(resp.data)
            
            let gameLog: squadov.hearthstone.game_state.HearthstoneSerializedGameLog  = root.squadov.hearthstone.game_state.HearthstoneSerializedGameLog.decode(buf)
            return {
                data: {
                    snapshots: gameLog.snapshots.map((ele: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot) => {
                        let entities : { [id: number] : HearthstoneEntity } = {}
                        for (let [eid, entity] of Object.entries(ele.entities!)) {
                            entities[parseInt(eid)] = {
                                entityId: entity.entityId!,
                                tags: JSON.parse(entity.tags!),
                                attributes: JSON.parse(entity.attributes!)
                            }
                        }

                        return {
                            uuid: ele.uuid!,
                            tm: !!ele.tm ? new Date(longNumberToNumber(ele.tm) * 1000) : null,
                            gameEntityId: ele.gameEntityId!,
                            playerNameToPlayerId: ele.playerNameToPlayerId!,
                            playerIdToEntityId: ele.playerIdToEntityId!,
                            entities: entities,
                            auxData: (!!ele.auxData) ? {
                                currentTurn: ele.auxData.currentTurn!,
                                step: ele.auxData.step!,
                                currentPlayerId: ele.auxData.currentPlayerId!,
                                lastActionIndex: longNumberToNumber(ele.auxData.lastActionIndex!),
                            } : null
                        }
                    }),
                    actions: JSON.parse(gameLog.actions).map((ele: HearthstoneGameAction) => {
                        return cleanHearthstoneGameActionFromJson(ele)
                    }),
                    blocks: gameLog.blocks.map((ele: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock) => {
                        return {
                            blockId: ele.blockId!,
                            startActionIndex: ele.startActionIndex!,
                            endActionIndex: ele.endActionIndex!,
                            blockType: ele.blockType!,
                            parentBlock: ele.parentBlock!,
                            entityId: ele.entityId!
                        }
                    })
                }
            }
        })
    }

    getBulkHearthstoneCardMetadata(cards: string[]) : Promise<ApiData<HearthstoneCardMetadata[]>> {
        return axios.post(`v1/hearthstone/cards`, cards, this.createWebAxiosConfig())
    }

    getHearthstoneBattlegroundsCardsForTavernLevel(level: number): Promise<ApiData<HearthstoneBattlegroundsCardMetadata[]>> {
        return axios.get(`v1/hearthstone/cards/battlegrounds/tavern/${level}`, this.createWebAxiosConfig())
    }

    graphqlRequest(req : string) : Promise<GraphqlApiData<any>> {
        return axios.post('graphql', {
            query: req
        }, this.createWebAxiosConfig())
    }

    listRiotValorantAccounts(userId: number): Promise<ApiData<RiotAccountData[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/valorant`, this.createWebAxiosConfig())
    }

    listRiotTftAccounts(userId: number): Promise<ApiData<RiotSummoner[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/tft`, this.createWebAxiosConfig())
    }

    listTftMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number}) : Promise<ApiData<HalResponse<TftPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/tft/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<TftPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanTftPlayerMatchSummaryFromJson)
            return resp
        })
    }

    getTftMatch(matchUuid: string): Promise<ApiData<WrappedTftMatch>> {
        return axios.get(`v1/tft/match/${matchUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<WrappedTftMatch>) => {
            cleanWrappedTftMatchFromJson(resp.data)
            return resp
        })
    }

    getTftMatchAccessibleVods(matchUuid: string, userId: number): Promise<ApiData<TftMatchAccessibleVods>> {
        return axios.get(`v1/tft/match/${matchUuid}/user/${userId}/vods`, this.createWebAxiosConfig()).then((resp: ApiData<TftMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    listRiotLolAccounts(userId: number): Promise<ApiData<RiotSummoner[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/lol`, this.createWebAxiosConfig())
    }

    listLolMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number}) : Promise<ApiData<HalResponse<LolPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/lol/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<LolPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanLolPlayerMatchSummaryFromJson)
            return resp
        })
    }

    getLolMatch(matchUuid: string): Promise<ApiData<FullLolMatch>> {
        return axios.get(`v1/lol/match/${matchUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<FullLolMatch>) => {
            cleanFullLolMatch(resp.data)
            return resp
        })
    }

    getLolMatchAccessibleVods(matchUuid: string, userId: number): Promise<ApiData<LeagueMatchAccessibleVods>> {
        return axios.get(`v1/lol/match/${matchUuid}/user/${userId}/vods`, this.createWebAxiosConfig()).then((resp: ApiData<LeagueMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    getValorantAccount(userId: number, puuid : string) : Promise<ApiData<RiotAccountData>> {  
        return axios.get(`v1/users/${userId}/accounts/riot/valorant/puuid/${puuid}`, this.createWebAxiosConfig())
    }

    listWoWCharacters(userId: number): Promise<ApiData<WowCharacter[]>> {
        return axios.get(`v1/wow/users/${userId}/characters`, this.createWebAxiosConfig()).then((resp: ApiData<WowCharacter[]>) => {
            resp.data.sort((a: WowCharacter, b: WowCharacter) => {
                if (a.name < b.name) {
                    return -1
                } else if (a.name > b.name) {
                    return 1
                }
                return 0
            })
            return resp
        })
    }

    listWoWCharactersForMatch(matchUuid: string, userId: number): Promise<ApiData<WowCharacter[]>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/characters`, this.createWebAxiosConfig()).then((resp: ApiData<WowCharacter[]>) => {
            resp.data.sort((a: WowCharacter, b: WowCharacter) => {
                if (a.name < b.name) {
                    return -1
                } else if (a.name > b.name) {
                    return 1
                }
                return 0
            })
            return resp
        })
    }

    listWoWEncountersForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number}): Promise<ApiData<HalResponse<WowEncounter[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/encounters`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowEncounter[]>>) => {
            resp.data.data.forEach(cleanWowEncounterFromJson)
            return resp
        })
    }

    listWoWChallengesForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number}): Promise<ApiData<HalResponse<WowChallenge[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/challenges`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowChallenge[]>>) => {
            resp.data.data.forEach(cleanWowChallengeFromJson)
            return resp
        })
    }

    listWoWArenasForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number}): Promise<ApiData<HalResponse<WowArena[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/arena`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowArena[]>>) => {
            resp.data.data.forEach(cleanWowArenaFromJson)
            return resp
        })
    }
    
    listWoWMatchCharacterAssociations(userId: number, matchUuid: string): Promise<ApiData<WoWCharacterUserAssociation[]>> {
        return axios.get(`v1/wow/match/${matchUuid}/users/${userId}/characters`, this.createWebAxiosConfig())
    }

    getWoWMatch(userId: number, matchUuid: string): Promise<ApiData<GenericWowMatchContainer>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<GenericWowMatchContainer>) => {
            cleanGenericWowMatchContainerFromJson(resp.data)
            return resp
        })
    }

    getWoWMatchEvents(userId: number, matchUuid: string): Promise<ApiData<SerializedWowMatchEvents>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/events`, this.createWebAxiosConfig()).then((resp: ApiData<SerializedWowMatchEvents>) => {
            cleanWowMatchEventsFromJson(resp.data)
            return resp
        })
    }

    getWoWMatchStats(userId: number, matchUuid: string, endpoint: string, query: WowStatQueryParam): Promise<ApiData<WowMatchStatContainer>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/stats/${endpoint}`, {
            ...this.createWebAxiosConfig(),
            params: query,
        })
    }

    getWoWMatchAccessibleVods(userId: number, matchUuid: string): Promise<ApiData<WowMatchAccessibleVods>> {
        return axios.get(`v1/wow/match/${matchUuid}/users/${userId}/vods`, this.createWebAxiosConfig()).then((resp: ApiData<WowMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    submitBugReport(title: string, description: string, logs: Blob) : Promise<void> {
        let form = new FormData()
        form.append('title', title)
        form.append('description', description)
        form.append('logs', logs)

        let cfg = this.createWebAxiosConfig()
        cfg.headers['Content-Type'] = 'multipart/form-data'

        return axios.post(`v1/bug`, form, cfg)
    }

    markUserActive(): Promise<void> {
        return axios.post(`v1/users/me/active`, {}, this.createWebAxiosConfig())
    }

    getRiotRsoAuthorizeUrl(userId: number): Promise<ApiData<string>> {
        return axios.get(`v1/users/${userId}/oauth/rso`, this.createWebAxiosConfig())
    }

    submitRsoAuthorization(code: string, state: string) : Promise<void> {
        return axios.post(`auth/oauth/riot`, {code, state}, this.createWebAxiosConfig())
    }

    getFeatureFlags(userId: number): Promise<ApiData<FeatureFlags>> {
        return axios.get(`v1/users/${userId}/features`, this.createWebAxiosConfig())
    }

    getRecentPlaytime(seconds: number): Promise<ApiData<TotalRecordedPlaytime>> {
        return axios.get('v1/users/me/playtime', {
            ...this.createWebAxiosConfig(),
            params: {
                seconds
            },
        })
    }

    listMyRecentMatches(params : {next : string | null, start : number, end : number}): Promise<ApiData<HalResponse<RecentMatch[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/users/me/recent`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<RecentMatch[]>>) => {
            resp.data.data.forEach(cleanRecentMatchFromJson)
            return resp
        })
    }

    getMatchShareUrl(matchUuid: string, fullPath: string, game: SquadOvGames, graphqlStats: StatPermission[] | undefined): Promise<ApiData<string>> {
        return axios.post(`v1/match/${matchUuid}/share`, {
            fullPath,
            game,
            graphqlStats,
        }, this.createWebAxiosConfig())
    }

    getClipShareUrl(clipUuid: string, fullPath: string): Promise<ApiData<string>> {
        return axios.post(`v1/clip/${clipUuid}/share`, {
            fullPath,
        }, this.createWebAxiosConfig())
    }

    exchangeShareAccessToken(accessTokenId: string): Promise<ApiData<ShareAccessTokenResponse>> {
        return axios.post(`public/share/${accessTokenId}/exchange`, {}, this.createWebAxiosConfig())
    }

    deleteRiotAccount(userId: number, puuid: string): Promise<ApiData<void>> {
        return axios.delete(`v1/users/${userId}/accounts/riot/generic/${puuid}`, this.createWebAxiosConfig())
    }

    resyncRiotAccount(userId: number, puuid: string): Promise<ApiData<void>> {
        return axios.post(`v1/users/${userId}/accounts/riot/generic/${puuid}`, {}, this.createWebAxiosConfig())
    }

    deleteVod(vodUuid: string): Promise<ApiData<void>> {
        return axios.delete(`v1/vod/${vodUuid}`, this.createWebAxiosConfig())
    }

    createClip(parentVodUuid: string, clipPath: string, association: VodAssociation, metadata: VodMetadata, title: string, description: string, game: SquadOvGames) : Promise<ApiData<string>> {
        interface ClipResponse {
            uuid: string
            uploadPath: string
        }
        // First post responds with the clip uuid and upload path.
        return axios.post(`v1/vod/${parentVodUuid}/clip`, {
            title,
            description,
            game
        }, this.createWebAxiosConfig()).then(async (resp: ApiData<ClipResponse>) => {
            // Once we have the clip vod uuid we can upload the VOD to GCS. We can consider
            // doing a resumable upload here but I think the clip should be small enough to just upload it
            // all in one go.
            let sessionUri = await uploadLocalFileToGcs(clipPath, resp.data.uploadPath)

            association.videoUuid = resp.data.uuid
            metadata.videoUuid = resp.data.uuid
            await this.associateVod(association, metadata, sessionUri)
            return {
                data: resp.data.uuid
            }
        })
    }

    associateVod(association: VodAssociation, metadata: VodMetadata, sessionUri: string): Promise<void> {
        return axios.post(`v1/vod/${association.videoUuid}`, {
            association,
            metadata,
            sessionUri,
        }, this.createWebAxiosConfig())
    }

    listClips(params : {next : string | null, matchUuid : string | undefined, start : number, end : number}): Promise<ApiData<HalResponse<VodClip[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get('v1/clip', {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    matchUuid: params.matchUuid,
                }
            })

        return promise.then((resp : ApiData<HalResponse<VodClip[]>>) => {
            resp.data.data.forEach(cleanVodClipFromJson)
            return resp
        })
    }

    getClip(clipUuid: string): Promise<ApiData<VodClip>> {
        return axios.get(`v1/clip/${clipUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<VodClip>) => {
            cleanVodClipFromJson(resp.data)
            return resp
        })
    }

    getClipReacts(clipUuid: string): Promise<ApiData<ClipReact[]>> {
        return axios.get(`v1/clip/${clipUuid}/react`, this.createWebAxiosConfig())
    }

    reactToClip(clipUuid: string): Promise<void> {
        return axios.post(`v1/clip/${clipUuid}/react`, {}, this.createWebAxiosConfig())
    }

    removeReactToClip(clipUuid: string): Promise<void> {
        return axios.delete(`v1/clip/${clipUuid}/react`, this.createWebAxiosConfig())
    }

    markClipView(clipUuid: string): Promise<void> {
        return axios.post(`v1/clip/${clipUuid}/view`, {}, this.createWebAxiosConfig())
    }

    getClipComments(params : {next : string | null, clipUuid : string , start : number, end : number}): Promise<ApiData<HalResponse<ClipComment[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/clip/${params.clipUuid}/comments`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<ClipComment[]>>) => {
            resp.data.data.forEach(cleanClipCommentFromJson)
            return resp
        })
    }

    addNewClipComment(clipUuid: string, comment: string): Promise<ApiData<ClipComment>> {
        return axios.post(`v1/clip/${clipUuid}/comments`, { comment }, this.createWebAxiosConfig()).then((resp: ApiData<ClipComment>) => {
            cleanClipCommentFromJson(resp.data)
            return resp
        })
    }
}

export let apiClient = new ApiClient()

/// #if DESKTOP
ipcRenderer.on('update-session', (event : any, message : string) => {
    apiClient.setSessionId(message)
})
/// #endif