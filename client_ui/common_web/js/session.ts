import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser, SquadOvHeartbeatResponse } from '@client/js/squadov/user'
import * as ls from 'local-storage'

export function checkHasSessionCookie() : boolean {
    let cookie = ls.get<any>('squadov-session')
    return !!cookie && !!cookie.sessionId
}

export function clearSessionCookie(store: any) {
    ls.remove('squadov-session')
    store.commit('setUser' , null)
    apiClient.clearSession()
}

let currentSessionExpiration: Date | null = null
let sessionRetryCount = 0
export function startSessionHeartbeat(store: any, initialSession: SquadOvHeartbeatResponse) {
    function tickSessionHeartbeat() {
        let nextTickTimeMs: number = 0
        apiClient.sessionHeartbeat(apiClient.getSessionId()!).then((resp: ApiData<SquadOvHeartbeatResponse>) => {
            apiClient.setSessionFull(resp.data.sessionId, getSessionUserId())
            currentSessionExpiration = resp.data.expiration
            nextTickTimeMs = currentSessionExpiration!.getTime() - Date.now() - 10 * 60 * 1000
            store.commit('markValidSession', true)
            sessionRetryCount = 0
        }).catch((err: any) => {
            console.log('Tick session heartbeat failure: ', err)
            nextTickTimeMs = Math.min(Math.pow(2, sessionRetryCount) + Math.random() * 1000, 15000)

            // At this point we aren't able to move forward and do anything in the app so
            // mark the current state as such in the store.
            if (new Date() > currentSessionExpiration!) {
                store.commit('markValidSession', false)
            }
            sessionRetryCount += 1
        }).finally(() => {
            setTimeout(tickSessionHeartbeat, Math.max(nextTickTimeMs, 0))
        })
    }

    currentSessionExpiration = initialSession.expiration
    setTimeout(tickSessionHeartbeat, Math.max(initialSession.expiration.getTime() - Date.now() - 10 * 60 * 1000, 0))
}

// In the web case, we need to check to see if the session is set in a cookie instead
// of loading it locally. If it's not there (or we can't grab the user info),
// we assume the user is logged out and direct them to the login page. The onValid and
// onInvalid callback functions allow us to control what to do when we encounter a
// valid or invalid session on a per-page basis.
export function loadInitialSessionFromCookies(store: any, onValid: () => void, onInvalid: () => void) {
    let cookie = ls.get<any>('squadov-session')
    if (!cookie) {
        onInvalid()
        return
    }

    apiClient.sessionHeartbeat(cookie.sessionId).then((sresp: ApiData<SquadOvHeartbeatResponse>) => {
        apiClient.setSessionFull(sresp.data.sessionId, parseInt(cookie.userId))
        getSquadOVUser(parseInt(cookie.userId)).then((uresp : ApiData<SquadOVUser>) => {
            store.commit('setUser' , uresp.data)
            startSessionHeartbeat(store, sresp.data)
            onValid()
        }).catch((err : any ) => {
            console.log('Failed to obtain user: ', err)
            onInvalid()
        })
    }).catch((err: any) => {
        console.log('Failed to refresh session', err)
        onInvalid()
    })
}

export function storeSessionCookie(sessionId: string, userId: number) {
    ls.set('squadov-session', {
        sessionId,
        userId
    })
}

export function getSessionId(): string {
    let cookie = ls.get<any>('squadov-session')
    if (!cookie) {
        return ''
    }
    return cookie.sessionId
}

export function getSessionUserId(): number {
    let cookie = ls.get<any>('squadov-session')
    if (!cookie) {
        return -1
    }
    return parseInt(cookie.userId)
}