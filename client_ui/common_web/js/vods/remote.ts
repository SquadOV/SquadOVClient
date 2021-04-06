export enum RCMessageType {
    StopAndDestroy
}

export interface RCMessagePacket {
    type: RCMessageType
    data: any
}

export class VodRemoteControlContext {
    _uuid: string
    _channel: BroadcastChannel
    _messageListeners: ((a: RCMessagePacket) => void)[]

    constructor(uuid: string) {
        this._uuid = uuid
        this._channel = new BroadcastChannel(`squadov-vod-rc-${uuid}`)
        this._channel.addEventListener('message', (e: MessageEvent) => { this.handleMessages(e) })
        this._messageListeners = []
    }

    addListener(m: (a: RCMessagePacket) => void) {
        this._messageListeners.push(m)
    }

    handleMessages(e: MessageEvent) {
        for (let m of this._messageListeners) {
            m(e.data)
        }
    }

    stopAndDestroy() {
        this._channel.postMessage({
            type: RCMessageType.StopAndDestroy,
            data: undefined,
        })
    }
}