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
    cleanClipCommentFromJson,
    VodFavoriteResponse,
    CsgoMatchAccessibleVods
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
    SquadInvite, cleanSquadInviteFromJson, SquadInviteLink, cleanSquadInviteLinkFromJson, SquadInviteLinkData
} from '@client/js/squadov/squad'
import {
    SquadOvHeartbeatResponse,
    cleanSquadOvHeartbeatResponse,
    SquadOVUserHandle,
    UserProfileBasic,
    cleanUserProfileBasicFromJson,
} from '@client/js/squadov/user'
import {
    RiotSummoner
} from '@client/js/riot/summoner'
import { NotificationSummary } from '@client/js/squadov/notification'
import * as root from '@client/js/proto.js'
import { squadov } from '@client/js/proto'
import { longNumberToNumber } from '@client/js/long'
import { WowCharacter, WoWCharacterUserAssociation, WowFullCharacter } from '@client/js/wow/character'
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
    FeatureFlags, GlobalFlags
} from '@client/js/squadov/features'
import { TotalRecordedPlaytime } from '@client/js/squadov/playtime'
import {
    RecentMatch,
    cleanRecentMatchFromJson,
    RecentMatchFilters,
    MatchFavoriteResponse
} from '@client/js/squadov/recentMatch'
import { SquadOvGames } from '@client/js/squadov/game'
import { AutoShareConnection, LinkShareData, MatchVideoShareConnection, MatchVideoSharePermissions, ShareAccessTokenResponse, ShareToProfileData } from '@client/js/squadov/share'
import { StatPermission } from '@client/js/stats/statPrimitives'
import { uploadLocalFileToCloud } from '@client/js/cloud'

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
    twoFactor: string | null
}

interface RegisterInput {
    username: string
    password: string
    email: string
    ref: string | undefined
}

export interface CheckVerificationOutput {
    verified: boolean
}

interface ChangeForgottenPasswordInput {
    changePasswordId: string
    password: string
}

import { storeSessionCookie, getSessionId } from '@client/js/session'
import { WowDeathRecap, cleanWowDeathRecapFromJson } from '@client/js/wow/deaths'
import { CsgoPlayerMatchSummary, cleanCsgoPlayerMatchSummaryFromJson } from '@client/js/csgo/summary'
import { CsgoFullMatchData, cleanCsgoFullMatchDataFromJson } from '@client/js/csgo/match'
import { MfaData } from '@client/js/squadov/mfa'
import { AimlabMatchFilters } from '@client/js/aimlab/filters'
import { HearthstoneMatchFilters } from '@client/js/hearthstone/filters'
import { CsgoMatchFilters } from '@client/js/csgo/filters'
import { LolMatchFilters } from '@client/js/lol/filters'
import { TftMatchFilters } from '@client/js/tft/filters'
import { ValorantMatchFilters } from '@client/js/valorant/filters'
import { WowMatchFilters } from '@client/js/wow/filters'
import { LocalVodsDto } from '@client/js/local/types'
import { SquadOvCommunity, CommunityFilter, cleanCommunityFromJson, CommunityRole } from '@client/js/squadov/community'
import { cleanUser2UserSubscriptionFromJson, User2UserSubscription } from './squadov/subscription'
import { LinkedAccounts, TwitchAccount } from './squadov/accounts'
import { VodDestination } from './vods/destination'

interface WebsocketAuthenticationResponse {
    success: boolean
}

class ApiClient {
    _sessionId: string | null
    _tempSessionId: string | null
    _tempUserId: number | null
    _accessToken: string | null
    _useAccessToken: boolean
    _localApiPort: number | null

    constructor() {
        this._sessionId = null
        this._tempSessionId = null
        this._tempUserId = null
        this._localApiPort = null
        this._accessToken = null
        this._useAccessToken = false
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

    setAccessToken(s: string | null) {
        this._accessToken = s 
    }

    accessToken(s: string | null | undefined = null): ApiClient {
        let newToken = s || this._accessToken
        if (!!newToken) {
            let ret: ApiClient = new ApiClient()
            ret._sessionId = this._sessionId
            ret._tempSessionId = this._tempSessionId
            ret._tempUserId = this._tempUserId
            ret._localApiPort = this._localApiPort
            ret.setAccessToken(newToken)
            ret._useAccessToken = true
            return ret
        } else {
            return this
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
        this._accessToken = null
    }

    setSessionFull(s: string, userId: number) {
       this.setSessionId(s)

       // This call should be used on the desktop to store this info
       // in cookies (as an alternative for storing it on the local system)
       storeSessionCookie(s, userId)
    }

    createWebAxiosConfig(useTempIfAvailable: boolean = false) : any {
        let ret : any = {
            baseURL: SQUADOV_API_URL,
        }

        // Always use access token when available.
        if (!!this._accessToken && this._useAccessToken) {
            ret.headers = {
                'x-squadov-access-token': this._accessToken,
            }
        } else if (!!this._tempSessionId && useTempIfAvailable) {
            ret.headers = {
                'x-squadov-share-id': this._tempSessionId,
            }
        } else if (!!this._sessionId) {
            ret.headers = {
                'x-squadov-session-id': this._sessionId,
            }
        }

        return ret
    }

    createLocalAxiosConfig(): Promise<any> {
        return new Promise( async (resolve) => {
            if (this._localApiPort === null) {
                this._localApiPort = await ipcRenderer.invoke('request-local-api-port')
            }

            resolve({
                baseURL: `http://localhost:${this._localApiPort}/`,
                headers: {
                    'x-squadov-api-key': process.env.SQUADOV_API_KEY
                }
            })
        })
    }

    getIpAddress(): Promise<ApiData<string>> {
        return axios.get('https://api.ipify.org').then((resp: ApiData<string>) => {
            resp.data = resp.data.trim()
            return resp
        })
    }

    secondaryIdentify(ip: string, anonId: string): Promise<void> {
        return axios.post('v1/users/me/identify', {
            ip,
            anonId,
        }, this.createWebAxiosConfig())
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

    finishMfaLogin(mfaId: string, code: string): Promise<ApiData<LoginOutput>> {
        return axios.post('auth/login/mfa', {
            id: mfaId,
            code,
        }, this.createWebAxiosConfig())
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

    editMyUsername(username: string): Promise<void> {
        return axios.post('v1/users/me/profile/username', {
            username
        }, this.createWebAxiosConfig())
    }

    editMyEmail(email: string): Promise<void> {
        return axios.post('v1/users/me/profile/email', {
            email
        }, this.createWebAxiosConfig())
    }

    changeForgottenPassword(input: ChangeForgottenPasswordInput) : Promise<void> {
        return axios.post('auth/forgotpw/change', input, this.createWebAxiosConfig())
    }

    changePassword(currentPw: string, newPw: string): Promise<void> {
        return axios.post('v1/users/me/changepw', {
            currentPw,
            newPw,
        }, this.createWebAxiosConfig())
    }

    verifyPassword(pw: string): Promise<void> {
        return axios.post('v1/users/me/pw/verify', {
            password: pw
        }, this.createWebAxiosConfig())
    }

    request2FAQrCodeUrl(): Promise<ApiData<MfaData>> {
        return axios.get('v1/users/me/2fa/qr', this.createWebAxiosConfig())
    }

    enable2fa(code: string, secret: string): Promise<ApiData<string[]>> {
        return axios.post('v1/users/me/2fa', {
            code,
            secret,
        }, this.createWebAxiosConfig())
    }

    disable2fa(code: string): Promise<void> {
        return axios.delete(`v1/users/me/2fa?code=${code}`, this.createWebAxiosConfig())
    }

    check2faStatus(): Promise<ApiData<boolean>> {
        return axios.get('v1/users/me/2fa', this.createWebAxiosConfig())
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

    getMySquadMates(squads: number[] | undefined): Promise<ApiData<SquadOVUserHandle[]>> {
        return axios.get(`v1/users/me/squadmates`, {
            params: {
                squads
            },
            ...this.createWebAxiosConfig()
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
        return axios.get(`v1/vod/match/${matchUuid}/user/id/${userId}`, this.createWebAxiosConfig(true)).then((resp : ApiData<VodAssociation>) => {
            cleanVodAssocationData(resp.data)
            return resp
        })
    }

    findVodFromVideoUuid(videoUuid: string) : Promise<ApiData<VodAssociation>> {
        return axios.get(`v1/vod/${videoUuid}/assoc`, this.createWebAxiosConfig(true)).then((resp : ApiData<VodAssociation>) => {
            cleanVodAssocationData(resp.data)
            return resp
        })
    }

    getVodManifest(videoUuid: string) : Promise<ApiData<VodManifest>> {
        return axios.get(`v1/vod/${videoUuid}`, this.createWebAxiosConfig(true))
    }

    getVodSegment(url : string) : Promise<ApiData<string>> {
        return axios.get(url, {
            ...this.createWebAxiosConfig(true),
        })
    }

    allAimlabTaskData(params : {next : string | null, userId : number, start : number, end : number, filters: AimlabMatchFilters}) : Promise<ApiData<HalResponse<AimlabTaskData[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/aimlab/user/${params.userId!}`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<AimlabTaskData[]>>) => {
            resp.data.data.forEach(cleanAimlabTaskData)
            return resp
        })
    }

    getAimlabTaskData(uuid : string, userId: number) : Promise<ApiData<AimlabTaskData>> {
        return axios.get(`v1/aimlab/user/${userId}/match/${uuid}/task`, this.createWebAxiosConfig(true)).then((resp : ApiData<AimlabTaskData>) => {
            cleanAimlabTaskData(resp.data)
            return resp
        })
    }

    listValorantMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number, filters: ValorantMatchFilters}) : Promise<ApiData<HalResponse<ValorantPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/valorant/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
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
        return axios.get(`v1/valorant/match/${matchUuid}`, this.createWebAxiosConfig(true)).then((resp : ApiData<ValorantMatchDetails>) => {
            cleanValorantMatchDetails(resp.data)
            return resp
        })
    }

    getValorantMatchPlayerMetadata(matchUuid: string, puuid: string) : Promise<ApiData<ValorantMatchPlayerMatchMetadata>> {
        return axios.get(`v1/valorant/match/${matchUuid}/metadata/${puuid}`, this.createWebAxiosConfig(true)).then((resp : ApiData<ValorantMatchPlayerMatchMetadata>) => {
            cleanValorantMatchPlayerMatchMetadata(resp.data)
            return resp
        })
    }

    getValorantMatchAccessibleVods(matchUuid: string): Promise<ApiData<ValorantMatchAccessibleVods>> {
        return axios.get(`v1/valorant/match/${matchUuid}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<ValorantMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    getHearthstoneMatchAccessibleVods(matchId: string): Promise<ApiData<HearthstoneMatchAccessibleVods>> {
        return axios.get(`v1/hearthstone/match/${matchId}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<HearthstoneMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    listHearthstoneMatchesForPlayer(params : {next : string | null, userId : number, start : number, end : number, filter : HearthstoneGameType[], aux: HearthstoneMatchFilters}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/match`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                    filter: JSON.stringify(params.filter),
                    ...params.aux,
                }
            })
    }

    listHearthstoneMatchesForArenaRun(collectionUuid: string, userId: number, filters: HearthstoneMatchFilters): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}/matches`, {
            params: {
                ...filters
            },
            ...this.createWebAxiosConfig()
        })
    }

    listHearthstoneMatchesForDuelRun(collectionUuid: string, userId: number, filters: HearthstoneMatchFilters): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}/matches`, {
            params: {
                ...filters
            },
            ...this.createWebAxiosConfig()
        })
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
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<HearthstoneDuelRun>) => {
            cleanHearthstoneDuelRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneArenaRun(collectionUuid: string, userId: number): Promise<ApiData<HearthstoneArenaRun>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<HearthstoneArenaRun>) => {
            cleanHearthstoneArenaRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatch(matchId: string, userId: number) : Promise<ApiData<HearthstoneMatch>> {
        return axios.get(`v1/hearthstone/user/${userId}/match/${matchId}`, this.createWebAxiosConfig(true)).then((resp : ApiData<HearthstoneMatch>) => {
            cleanHearthstoneMatchFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatchLogs(matchId: string, userId: number) : Promise<ApiData<HearthstoneMatchLogs>> {
        return axios.get(`v1/hearthstone/user/${userId}/match/${matchId}/logs`, {
            responseType: 'arraybuffer',
            ...this.createWebAxiosConfig(true)
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
        return axios.post(`v1/hearthstone/cards`, cards, this.createWebAxiosConfig(true))
    }

    getHearthstoneBattlegroundsCardsForTavernLevel(level: number): Promise<ApiData<HearthstoneBattlegroundsCardMetadata[]>> {
        return axios.get(`v1/hearthstone/cards/battlegrounds/tavern/${level}`, this.createWebAxiosConfig(true))
    }

    graphqlRequest(req : string) : Promise<GraphqlApiData<any>> {
        return axios.post('graphql', {
            query: req
        }, this.createWebAxiosConfig(true))
    }

    listRiotValorantAccounts(userId: number): Promise<ApiData<RiotAccountData[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/valorant`, this.createWebAxiosConfig())
    }

    listRiotTftAccounts(userId: number): Promise<ApiData<RiotSummoner[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/tft`, this.createWebAxiosConfig())
    }

    listTftMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number, filters: TftMatchFilters}) : Promise<ApiData<HalResponse<TftPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/tft/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<TftPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanTftPlayerMatchSummaryFromJson)
            return resp
        })
    }

    getTftMatch(matchUuid: string): Promise<ApiData<WrappedTftMatch>> {
        return axios.get(`v1/tft/match/${matchUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<WrappedTftMatch>) => {
            cleanWrappedTftMatchFromJson(resp.data)
            return resp
        })
    }

    getTftMatchAccessibleVods(matchUuid: string): Promise<ApiData<TftMatchAccessibleVods>> {
        return axios.get(`v1/tft/match/${matchUuid}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<TftMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    listRiotLolAccounts(userId: number): Promise<ApiData<RiotSummoner[]>> {
        return axios.get(`v1/users/${userId}/accounts/riot/lol`, this.createWebAxiosConfig())
    }

    listLolMatchesForPlayer(params : {next : string | null, userId: number, puuid : string, start : number, end : number, filters: LolMatchFilters}) : Promise<ApiData<HalResponse<LolPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/lol/user/${params.userId}/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<LolPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanLolPlayerMatchSummaryFromJson)
            return resp
        })
    }

    getLolMatch(matchUuid: string): Promise<ApiData<FullLolMatch>> {
        return axios.get(`v1/lol/match/${matchUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<FullLolMatch>) => {
            cleanFullLolMatch(resp.data)
            return resp
        })
    }

    getLolMatchAccessibleVods(matchUuid: string): Promise<ApiData<LeagueMatchAccessibleVods>> {
        return axios.get(`v1/lol/match/${matchUuid}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<LeagueMatchAccessibleVods>) => {
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
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/characters`, this.createWebAxiosConfig(true)).then((resp: ApiData<WowCharacter[]>) => {
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

    listWoWEncountersForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number, filters: WowMatchFilters}): Promise<ApiData<HalResponse<WowEncounter[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/encounters`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowEncounter[]>>) => {
            resp.data.data.forEach(cleanWowEncounterFromJson)
            return resp
        })
    }

    listWoWChallengesForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number, filters: WowMatchFilters}): Promise<ApiData<HalResponse<WowChallenge[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/challenges`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowChallenge[]>>) => {
            resp.data.data.forEach(cleanWowChallengeFromJson)
            return resp
        })
    }

    listWoWArenasForCharacter(params : {next : string | null, userId : number, guid: string, start : number, end : number, filters: WowMatchFilters}): Promise<ApiData<HalResponse<WowArena[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/wow/users/${params.userId}/characters/${params.guid}/arena`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<WowArena[]>>) => {
            resp.data.data.forEach(cleanWowArenaFromJson)
            return resp
        })
    }
    
    listWoWMatchCharacterAssociations(userId: number, matchUuid: string): Promise<ApiData<WoWCharacterUserAssociation[]>> {
        return axios.get(`v1/wow/match/${matchUuid}/users/${userId}/characters`, this.createWebAxiosConfig(true))
    }

    getWoWMatch(userId: number, matchUuid: string): Promise<ApiData<GenericWowMatchContainer>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<GenericWowMatchContainer>) => {
            cleanGenericWowMatchContainerFromJson(resp.data)
            return resp
        })
    }

    getWoWMatchEvents(userId: number, matchUuid: string): Promise<ApiData<SerializedWowMatchEvents>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/events`, this.createWebAxiosConfig(true)).then((resp: ApiData<SerializedWowMatchEvents>) => {
            cleanWowMatchEventsFromJson(resp.data)
            return resp
        })
    }

    getWoWMatchStats(userId: number, matchUuid: string, endpoint: string, query: WowStatQueryParam): Promise<ApiData<WowMatchStatContainer>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/stats/${endpoint}`, {
            ...this.createWebAxiosConfig(true),
            params: query,
        })
    }

    getWoWMatchAccessibleVods(matchUuid: string): Promise<ApiData<WowMatchAccessibleVods>> {
        return axios.get(`v1/wow/match/${matchUuid}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<WowMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    getWoWCharacterArmoryLink(name: string, guid: string): Promise<ApiData<string>> {
        return axios.get(`v1/wow/characters/armory`, {
            ...this.createWebAxiosConfig(true),
            params: {
                characterName: name,
                characterGuid: guid,
            }
        })
    }
    
    getWowDeathRecap(userId: number, matchUuid: string, eventId: number, seconds: number): Promise<ApiData<WowDeathRecap>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/death/${eventId}`, {
            params: {
                seconds,
            },
            ...this.createWebAxiosConfig(true)
        }).then((resp: ApiData<WowDeathRecap>) => {
            cleanWowDeathRecapFromJson(resp.data)
            return resp
        })
    }

    getWowFullCharacter(userId: number, matchUuid: string, guid: string): Promise<ApiData<WowFullCharacter>> {
        return axios.get(`v1/wow/users/${userId}/match/${matchUuid}/characters/${guid}`, this.createWebAxiosConfig(true))
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

    markUserDownload(): Promise<void> {
        return axios.post(`v1/users/me/download`, {}, this.createWebAxiosConfig())
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

    listMyRecentMatches(params : {next : string | null, start : number, end : number, filters : RecentMatchFilters, profileId: number | undefined}): Promise<ApiData<HalResponse<RecentMatch[]>>> {
        let cfg = this.createWebAxiosConfig()
        let promise = !!params.next ?
            axios.get(params.next, cfg) :
            (params.profileId !== undefined) ?
                axios.get(`profile/${params.profileId}/matches`, {
                    ...cfg,
                    params: {
                        start: params.start!,
                        end: params.end!,
                        ...params.filters,
                    }
                }) :
                axios.get(`v1/users/me/recent`, {
                    ...cfg,
                    params: {
                        start: params.start!,
                        end: params.end!,
                        ...params.filters,
                    }
                })

        return promise.then((resp : ApiData<HalResponse<RecentMatch[]>>) => {
            resp.data.data.forEach(cleanRecentMatchFromJson)
            return resp
        })
    }

    getMatchDataFromVideoUuid(videoUuid: string): Promise<ApiData<RecentMatch>> {
        return axios.get(`v1/vod/${videoUuid}/match`, this.createWebAxiosConfig()).then((resp: ApiData<RecentMatch>) => {
            cleanRecentMatchFromJson(resp.data)
            return resp
        })
    }

    getGenericSharePermissions(matchUuid: string | undefined | null, videoUuid: string | undefined | null, game: SquadOvGames | undefined | null): Promise<ApiData<MatchVideoSharePermissions>> {
        return axios.post(`v1/share/permissions`, {
            matchUuid,
            videoUuid,
            game
        }, this.createWebAxiosConfig())
    }

    getMatchShareConnections(matchUuid: string): Promise<ApiData<MatchVideoShareConnection[]>> {
        return axios.get(`v1/match/${matchUuid}/share/internal`, {
            ...this.createWebAxiosConfig()
        })
    }

    getMatchShareUrl(matchUuid: string): Promise<ApiData<LinkShareData>> {
        return axios.get(`v1/match/${matchUuid}/share/public`, this.createWebAxiosConfig())
    }

    deleteMatchShareUrl(matchUuid: string): Promise<void> {
        return axios.delete(`v1/match/${matchUuid}/share/public`, this.createWebAxiosConfig())
    }

    createMatchShareUrl(matchUuid: string, fullPath: string, game: SquadOvGames, graphqlStats: StatPermission[] | undefined): Promise<ApiData<LinkShareData>> {
        return axios.post(`v1/match/${matchUuid}/share/public`, {
            fullPath,
            game,
            graphqlStats,
        }, this.createWebAxiosConfig())
    }

    getClipShareConnections(clipUuid: string): Promise<ApiData<MatchVideoShareConnection[]>> {
        return axios.get(`v1/clip/${clipUuid}/share/internal`, {
            ...this.createWebAxiosConfig()
        })
    }

    createClipShareUrl(clipUuid: string, fullPath: string): Promise<ApiData<LinkShareData>> {
        return axios.post(`v1/clip/${clipUuid}/share/public`, {
            fullPath,
        }, this.createWebAxiosConfig())
    }

    getClipShareUrl(clipUuid: string): Promise<ApiData<LinkShareData>> {
        return axios.get(`v1/clip/${clipUuid}/share/public`, this.createWebAxiosConfig())
    }

    deleteClipShareUrl(clipUuid: string): Promise<void> {
        return axios.delete(`v1/clip/${clipUuid}/share/public`, this.createWebAxiosConfig())
    }

    exchangeShareAccessToken(accessTokenId: string): Promise<ApiData<ShareAccessTokenResponse>> {
        return axios.post(`public/share/${accessTokenId}/exchange`, {}, this.createWebAxiosConfig())
    }

    deleteRiotAccount(userId: number, puuid: string): Promise<void> {
        return axios.delete(`v1/users/${userId}/accounts/riot/generic/${puuid}`, this.createWebAxiosConfig())
    }

    resyncRiotAccount(userId: number, puuid: string): Promise<ApiData<void>> {
        return axios.post(`v1/users/${userId}/accounts/riot/generic/${puuid}`, {}, this.createWebAxiosConfig())
    }

    deleteVod(vodUuid: string): Promise<ApiData<void>> {
        return axios.delete(`v1/vod/${vodUuid}`, this.createWebAxiosConfig())
    }

    getVodUploadDestination(vodUuid: string): Promise<ApiData<VodDestination>> {
        return axios.get(`v1/vod/${vodUuid}/upload`, this.createWebAxiosConfig())
    }

    createClip(parentVodUuid: string, clipPath: string, association: VodAssociation, metadata: VodMetadata, title: string, description: string, game: SquadOvGames) : Promise<ApiData<string>> {
        interface ClipResponse {
            uuid: string
            destination: VodDestination
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
            let uploadData = await uploadLocalFileToCloud(clipPath, resp.data.destination, resp.data.uuid)

            association.videoUuid = resp.data.uuid
            metadata.videoUuid = resp.data.uuid
            metadata.bucket = resp.data.destination.bucket
            metadata.sessionId = resp.data.destination.session
            await this.associateVod(association, metadata, uploadData.session, uploadData.parts)
            return {
                data: resp.data.uuid
            }
        })
    }

    associateVod(association: VodAssociation, metadata: VodMetadata, sessionUri: string, parts: string[] | undefined): Promise<void> {
        return axios.post(`v1/vod/${association.videoUuid}`, {
            association,
            metadata,
            sessionUri,
            parts
        }, this.createWebAxiosConfig())
    }

    listClips(params : {next : string | null, matchUuid : string | undefined, start : number, end : number, filters : RecentMatchFilters, profileId?: number}): Promise<ApiData<HalResponse<VodClip[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            (params.profileId !== undefined) ?
                axios.get(`profile/${params.profileId}/clips`, {
                    ...this.createWebAxiosConfig(),
                    params: {
                        start: params.start!,
                        end: params.end!,
                        matchUuid: params.matchUuid,
                        ...params.filters,
                    }
                }) :
                axios.get('v1/clip', {
                    ...this.createWebAxiosConfig(),
                    params: {
                        start: params.start!,
                        end: params.end!,
                        matchUuid: params.matchUuid,
                        ...params.filters,
                    }
                })

        return promise.then((resp : ApiData<HalResponse<VodClip[]>>) => {
            resp.data.data.forEach(cleanVodClipFromJson)
            return resp
        })
    }

    deleteVods(uuid: string[]): Promise<void> {
        return axios.post(`v1/vod/bulkDelete`, {
            vods: uuid,
        }, this.createWebAxiosConfig())
    }

    deleteClips(clips: string[]): Promise<void> {
        return axios.post(`v1/clip/bulkDelete`, {
            vods: clips,
        }, this.createWebAxiosConfig())
    }

    getClip(clipUuid: string): Promise<ApiData<VodClip>> {
        return axios.get(`v1/clip/${clipUuid}`, this.createWebAxiosConfig(true)).then((resp: ApiData<VodClip>) => {
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
        return axios.get(`v1/clip/${clipUuid}/view`, this.createWebAxiosConfig(true))
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

    myReferralLink(): Promise<ApiData<string>> {
        return axios.get('v1/users/me/referral', this.createWebAxiosConfig())
    }

    isMatchFavorite(matchUuid: string): Promise<ApiData<MatchFavoriteResponse>> {
        return axios.get(`v1/match/${matchUuid}/favorite`, this.createWebAxiosConfig())
    }

    addMatchFavorite(matchUuid: string, reason: string): Promise<void> {
        return axios.post(`v1/match/${matchUuid}/favorite`, {
            reason,
        }, this.createWebAxiosConfig())
    }

    removeMatchFavorite(matchUuid: string): Promise<void> {
        return axios.delete(`v1/match/${matchUuid}/favorite`, this.createWebAxiosConfig())
    }

    isVodFavorite(videoUuid: string): Promise<ApiData<VodFavoriteResponse>> {
        return axios.get(`v1/vod/${videoUuid}/list/favorite`, this.createWebAxiosConfig())
    }

    addVodFavorite(videoUuid: string, reason: string): Promise<void> {
        return axios.post(`v1/vod/${videoUuid}/list/favorite`, {
            reason,
        }, this.createWebAxiosConfig())
    }

    removeVodFavorite(videoUuid: string): Promise<void> {
        return axios.delete(`v1/vod/${videoUuid}/list/favorite`, this.createWebAxiosConfig())
    }

    isVodWatchlist(videoUuid: string): Promise<ApiData<boolean>> {
        return axios.get(`v1/vod/${videoUuid}/list/watch`, this.createWebAxiosConfig())
    }

    addVodWatchlist(videoUuid: string): Promise<void> {
        return axios.post(`v1/vod/${videoUuid}/list/watch`, {}, this.createWebAxiosConfig())
    }

    removeVodWatchlist(videoUuid: string): Promise<void> {
        return axios.delete(`v1/vod/${videoUuid}/list/watch`, this.createWebAxiosConfig())
    }

    listCsgoMatchesForPlayer(params : {next : string | null, userId: number, start : number, end : number, filters: CsgoMatchFilters}) : Promise<ApiData<HalResponse<CsgoPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/csgo/user/${params.userId}/match`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                    ...params.filters,
                }
            })

        return promise.then((resp : ApiData<HalResponse<CsgoPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanCsgoPlayerMatchSummaryFromJson)
            return resp
        })
    }

    getCsgoMatchData(matchUuid: string, userId: number): Promise<ApiData<CsgoFullMatchData>> {
        return axios.get(`v1/csgo/user/${userId}/match/${matchUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<CsgoFullMatchData>) => {
            cleanCsgoFullMatchDataFromJson(resp.data)
            return resp
        })
    }

    getCsgoMatchAccessibleVods(matchUuid: string): Promise<ApiData<CsgoMatchAccessibleVods>> {
        return axios.get(`v1/csgo/match/${matchUuid}/vods`, this.createWebAxiosConfig(true)).then((resp: ApiData<CsgoMatchAccessibleVods>) => {
            resp.data.vods.forEach(cleanVodAssocationData)
            return resp
        })
    }

    createShareConnection(req: MatchVideoShareConnection, game: SquadOvGames | undefined): Promise<ApiData<MatchVideoShareConnection[]>> {
        return axios.post('v1/share', {
            conn: req,
            game,
        }, this.createWebAxiosConfig())
    }

    deleteShareConnection(connId: number): Promise<void> {
        return axios.delete(`v1/share/conn/${connId}`, this.createWebAxiosConfig())
    }

    editShareConnection(connId: number, canShare: boolean, canClip: boolean) {
        return axios.post(`v1/share/conn/${connId}`, {
            canShare,
            canClip
        }, this.createWebAxiosConfig())
    }

    getAutoShareConnections(): Promise<ApiData<AutoShareConnection[]>> {
        return axios.get('v1/share/auto', this.createWebAxiosConfig())
    }

    newAutoShareConnection(conn: AutoShareConnection): Promise<ApiData<AutoShareConnection>> {
        return axios.post('v1/share/auto', conn, this.createWebAxiosConfig())
    }

    deleteAutoShareConnection(connId: number): Promise<void> {
        return axios.delete(`v1/share/auto/${connId}`, this.createWebAxiosConfig())
    }

    editAutoShareConnection(connId: number, conn: AutoShareConnection): Promise<void> {
        return axios.post(`v1/share/auto/${connId}`, conn, this.createWebAxiosConfig())
    }

    listCommunities(filter: CommunityFilter): Promise<ApiData<SquadOvCommunity[]>> {
        return axios.get('v1/community', {
            ...this.createWebAxiosConfig(),
            params: filter,
        }).then((resp: ApiData<SquadOvCommunity[]>) => {
            resp.data.forEach(cleanCommunityFromJson)
            return resp
        })
    }

    createCommunity(c: SquadOvCommunity): Promise<ApiData<SquadOvCommunity>> {
        return axios.post('v1/community', {
            community: c,
        }, this.createWebAxiosConfig()).then((resp: ApiData<SquadOvCommunity>) => {
            cleanCommunityFromJson(resp.data)
            return resp
        })
    }

    getCommunityFromSlug(slug: string): Promise<ApiData<SquadOvCommunity>> {
        return axios.get(`public/community/slug/${slug}`, this.createWebAxiosConfig()).then((resp: ApiData<SquadOvCommunity>) => {
            cleanCommunityFromJson(resp.data)
            return resp
        })
    }

    getMyCommunityRolesFromSlug(slug: string): Promise<ApiData<CommunityRole[]>> {
        return axios.get(`v1/community/slug/${slug}/role`, this.createWebAxiosConfig())
    }

    getMyCommunitySubscription(slug: string): Promise<ApiData<User2UserSubscription[]>> {
        return axios.get(`v1/community/slug/${slug}/sub`, this.createWebAxiosConfig()).then((resp: ApiData<User2UserSubscription[]>) => {
            resp.data.forEach(cleanUser2UserSubscriptionFromJson)
            return resp
        })
    }

    getLinkedTwitchAcccount(): Promise<ApiData<TwitchAccount[]>> {
        return axios.get(`v1/users/me/accounts/twitch`, this.createWebAxiosConfig())
    }

    getAllLinkedAccounts(): Promise<ApiData<LinkedAccounts>> {
        return axios.get(`v1/users/me/accounts`, this.createWebAxiosConfig())
    }

    deleteMyLinkedTwitchAccount(twitchUserId: number): Promise<void> {
        return axios.delete(`v1/users/me/accounts/twitch/${twitchUserId}`, this.createWebAxiosConfig())
    }

    getTwitchOauthAuthorizeUrl(): Promise<ApiData<string>> {
        return axios.get(`v1/users/me/oauth/twitch`, this.createWebAxiosConfig())
    }

    submitTwitchOauthAuthorization(code: string, state: string, redirectUrl: string) : Promise<void> {
        return axios.post(`auth/oauth/twitch`, {code, state, redirectUrl}, this.createWebAxiosConfig())
    }
    
    getGlobalAppFeatures(): Promise<ApiData<GlobalFlags>> {
        return axios.get('/public/flags', this.createWebAxiosConfig())
    }

    getSquadInviteLinksForUser(squadId: number, userId: number): Promise<ApiData<SquadInviteLink[]>> {
        return axios.get(`v1/users/${userId}/squads/${squadId}/links`, this.createWebAxiosConfig()).then((resp: ApiData<SquadInviteLink[]>) => {
            resp.data.forEach(cleanSquadInviteLinkFromJson)
            return resp
        })
    }

    createSquadInviteLinkForUser(squadId: number, userId: number): Promise<ApiData<SquadInviteLink>> {
        return axios.post(`v1/users/${userId}/squads/${squadId}/links`, {}, this.createWebAxiosConfig()).then((resp: ApiData<SquadInviteLink>) => {
            cleanSquadInviteLinkFromJson(resp.data)
            return resp
        })
    }

    modifySquadInviteLink(link: SquadInviteLink): Promise<void> {
        return axios.put(`v1/users/${link.userId}/squads/${link.squadId}/links/${link.id}`, link, this.createWebAxiosConfig())
    }

    deleteSquadInviteLink(squadId: number, userId: number, id: string): Promise<void> {
        return axios.delete(`v1/users/${userId}/squads/${squadId}/links/${id}`, this.createWebAxiosConfig())
    }

    getSquadInviteLinkData(id: string): Promise<ApiData<SquadInviteLinkData>> {
        return axios.get(`public/link/${id}`, this.createWebAxiosConfig())
    }

    acceptSquadInviteLink(id: string): Promise<void> {
        return axios.post(`v1/link/${id}/accept`, {}, this.createWebAxiosConfig())
    }

    getBasicUserProfileFromId(userId: number): Promise<ApiData<UserProfileBasic | null>> {
        return axios.get(`profile`, {
            params: {
                id: userId,
            },
            ...this.createWebAxiosConfig()
        }).then((resp: ApiData<UserProfileBasic | null>) => {
            if (!!resp.data) {
                cleanUserProfileBasicFromJson(resp.data)
            }
            return resp
        })
    }

    getBasicUserProfileFromSlug(slug: string): Promise<ApiData<UserProfileBasic | null>> {
        return axios.get(`profile`, {
            params: {
                slug,
            },
            ...this.createWebAxiosConfig()
        }).then((resp: ApiData<UserProfileBasic | null>) => {
            if (!!resp.data) {
                cleanUserProfileBasicFromJson(resp.data)
            }
            return resp
        })
    }

    updateBasicProfileData(p: UserProfileBasic, coverPhoto: File | null, profilePhoto: File | null): Promise<void> {
        let form = new FormData()
        if (!!coverPhoto) {
            form.append('coverPhoto', coverPhoto)
        }

        if (!!profilePhoto) {
            form.append('profilePhoto', profilePhoto)
        }
        
        if (!!p.misc) {
            form.append('description', p.misc.description)
            form.append('facebook', p.misc.links.facebook || '')
            form.append('instagram', p.misc.links.instagram || '')
            form.append('twitch', p.misc.links.twitch || '')
            form.append('youtube', p.misc.links.youtube || '')
            form.append('snapchat', p.misc.links.snapchat || '')
            form.append('twitter', p.misc.links.twitter || '')
            form.append('tiktok', p.misc.links.tiktok || '')
        }

        form.append('displayName', p.displayName)

        let cfg = this.createWebAxiosConfig()
        cfg.headers['Content-Type'] = 'multipart/form-data'
        return axios.post(`v1/users/me/profile/data`, form, cfg)
    }

    updateBasicProfileAccess(p: UserProfileBasic): Promise<void> {
        return axios.post(`v1/users/me/profile/access`, {
            slug: p.linkSlug,
            misc: p.miscAccess,
            achievements: p.achievementAccess,
            matches: p.matchAccess,
        }, this.createWebAxiosConfig())
    }

    createUserProfile(slug: string): Promise<ApiData<UserProfileBasic>> {
        return axios.post(`v1/users/me/profile`, {
            slug,
        }, this.createWebAxiosConfig()).then((resp: ApiData<UserProfileBasic>) => {
            cleanUserProfileBasicFromJson(resp.data)
            return resp
        })
    }

    checkIfMatchClipSharedToProfile(matchUuid: string | undefined, clipUuid: string | undefined): Promise<ApiData<ShareToProfileData>> {
        return axios.get(`v1/share/profile`, {
            ...this.createWebAxiosConfig(),
            params: {
                matchUuid,
                clipUuid,
            }
        })
    }

    shareMatchClipToProfile(matchUuid: string | undefined, clipUuid: string | undefined): Promise<void> {
        return axios.post(`v1/share/profile`, {
            matchUuid,
            clipUuid,
        }, this.createWebAxiosConfig())
    }

    deleteMatchClipFromProfile(matchUuid: string | undefined, clipUuid: string | undefined): Promise<void> {
        return axios.delete(`v1/share/profile`, {
            ...this.createWebAxiosConfig(),
            params: {
                matchUuid,
                clipUuid,
            }
        })
    }

    getSentryDsn(): Promise<ApiData<string>> {
        return axios.get('v1/sentry/web', this.createWebAxiosConfig())
    }

    // Local API
    async listLocalVods(start: number, end: number): Promise<ApiData<LocalVodsDto>> {
        return axios.get('api/vods', {
            ...await this.createLocalAxiosConfig(),
            params: {
                start,
                end
            }
        })
    }
}

export let apiClient = new ApiClient()

/// #if DESKTOP
ipcRenderer.on('update-session', (event : any, message : string) => {
    apiClient.setSessionId(message)
})
/// #endif