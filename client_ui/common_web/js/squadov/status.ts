import { SquadOvGames } from '@client/js/squadov/game'
import { Store } from 'vuex'
import { RootState } from '@client/js/vuex/state'
import { apiClient } from '@client/js/api'
import { gameShorthandToGame } from '@client/js/squadov/game'

export enum SquadOvActivity {
    Online,
    InGame,
    Recording,
    Offline
}

export interface TrackedUserStatus {
    activity: SquadOvActivity
    game: SquadOvGames[]
}

export interface TrackedUserStatusContainer {
    status: { [userId: number] : TrackedUserStatus }
}

export class TrackedUserStatsManager {
    _store: Store<RootState>
    _userId: number | undefined
    _connection: WebSocket | null
    _ready: boolean
    _subscribed: Set<number>
    _messageQueue: string[]
    _reconnectCount: number

    constructor(store: Store<RootState>) {
        this._store = store
        this._store.watch((st: RootState) => {
            return st.currentUser?.id
        }, (userId: number | undefined) => {
            this.changeCurrentUserId(userId)
        })

        this._userId = undefined
        this._connection = null
        this._ready = false
        this._subscribed = new Set()
        this._messageQueue = []
        this._reconnectCount = 0
    }

    changeCurrentUserId(userId: number | undefined) {
        if (this._userId === userId) {
            return
        }

        if (this._userId !== undefined) {
            this.disconnect()
        }

        if (userId !== undefined) {
            this.connect(userId)   
        }
    }

    connect(userId: number, cb?: () => void) {
        this._userId = userId

        console.log('Trying to connect to User Activity Websocket...')
        apiClient.connectWebsocket(`/ws/status/${userId}`).then((resp: WebSocket) => {
            console.log('...Connected to User Activity Websocket')
            this._connection = resp
            this._connection.onmessage = (e: MessageEvent) => {
                this.receiveMessage(e)
            }

            this._connection.onclose = (e: CloseEvent) => {
                console.log('User Activty Websocket Close: ', e.code)
                if (e.code != 1001) {
                    if (!!this._connection) {
                        this._connection.close()
                    }
                    this._connection = null
                    this.reconnect()
                }
            }
            
            this._reconnectCount = 0
            this.flushMessageQueue()
            if (!!cb) {
                cb()
            }
        }).catch((err : any) => {
            console.log('Failed to connect websocket: ', err)
            this._reconnectCount += 1
            setTimeout(() => {
                this.reconnect()
            }, Math.min(Math.pow(2, this._reconnectCount) + Math.random() * 1000, 15000))
        })
    }

    reconnect() {
        if (this._userId === undefined) {
            return
        }
        console.log('Attempting websocket reconnection...')
        this.connect(this._userId, () => {
            // Once we reconnect we must resub and resend our current status.
            this.subscribeTo(Array.from(this._subscribed))
            this.refreshCurrentUserStatus()
        })
    }

    flushMessageQueue() {
        if (!this._connection) {
            return
        }
        for (let m of this._messageQueue) {
            this._connection.send(m)
        }
        this._messageQueue = []
    }

    sendMessage(m: string) {
        if (!!this._connection) {
            this._connection.send(m)
        } else {
            this._messageQueue.push(m)
        }
    }

    subscribeTo(userIds: number[]) {
        if (userIds.length === 0) {
            return
        }

        console.log('Subscribe to user status: ', userIds)
        let final = userIds.filter((ele: number) => !this._subscribed.has(ele))
        final.forEach((ele: number) => this._subscribed.add(ele))

        let msg = JSON.stringify({
            type: 'Subscribe',
            users: final,
        })
        this.sendMessage(msg)
    }

    unsubscribeFrom(userIds: number[]) {
        if (userIds.length === 0) {
            return
        }

        console.log('Unsubscribe from user status: ', userIds)
        let final = userIds.filter((ele: number) => this._subscribed.has(ele))
        final.forEach((ele: number) => this._subscribed.delete(ele))

        let msg = JSON.stringify({
            type: 'Unsubscribe',
            users: final,
        })
        this.sendMessage(msg)
    }

    receiveMessage(e: MessageEvent) {
        let status: TrackedUserStatusContainer = JSON.parse(e.data)
        for (let [userId, st] of Object.entries(status.status)) {
            this._store.commit('setUserActivityStatus', {
                userId: parseInt(userId),
                status: st,
            })
        }
    }

    disconnect() {
        if (!!this._connection) {
            this._connection.close()
        }
        this._userId = undefined
        this._ready = false
    }

    changeUserStatus(activity: SquadOvActivity, games: SquadOvGames[]) {
        let msg = JSON.stringify({
            type: 'StatusChange',
            activity,
            game: games,
        })
        this.sendMessage(msg)
    }

    refreshCurrentUserStatus() {
        if (this._store.state.currentState.recordingGames.length > 0) {
            this.changeUserStatus(SquadOvActivity.Recording, this._store.state.currentState.recordingGames.map(gameShorthandToGame))
        } else if (this._store.state.currentState.runningGames.length > 0) {
            this.changeUserStatus(SquadOvActivity.InGame, this._store.state.currentState.runningGames.map(gameShorthandToGame))
        } else {
            this.changeUserStatus(SquadOvActivity.Online, [])
        }
    }
}