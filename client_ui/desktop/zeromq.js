const {app} = require('electron')
const zmq = require('zeromq')
const log = require('./log.js')
// Use the ZeroMQServer for communication with the backend service.
// Note that we create both a subscriber and a publisher here because
// the backend may receive updates that it needs to communicate with the
// frontend. Using this will set two environment variables:
//
// - SQUADOV_ZEROMQ_SERVER_KEY: ZeroMQ CURVE server key to be used as the "curveSeverKey" option.
// - SQUADOV_ZEROMQ_UI_PORT: The random port used for communication from the UI to the backend service.
// - SQUADOV_ZEROMQ_SERVICE_PORT: The random port used for communication from the backend service to the UI.
class ZeroMQServerClient {
    constructor() {
        this._pub = new zmq.Publisher()
        this._sub = new zmq.Subscriber()

        // Only do this on packaged builds so we don't have to deal with this in
        // development and copy pasting the environment variable to run in VS Code or something...
        if (app.isPackaged) {
            // We're going to say that the publisher is the server and everything else
            // (including the publisher on the local service) is a client for CURVE.
            const serverKp = zmq.curveKeyPair()
            this._serverKey = process.env.SQUADOV_ZEROMQ_SERVER_KEY = serverKp.publicKey

            this._pub.curveSecretKey = serverKp.secretKey
            this._pub.curveServer = true

            this._sub.curveSecretKey = serverKp.secretKey
            this._sub.curveServer = true
        }

        this._port = 0
        this._handlers = {}
        this._started = false
    }

    async start() {
        // We do binds on both the pub and sub on the Client ui side as this is guaranteed to have
        // started first so messages can be immediately sent from the backend service when it starts.
        // If we connect the sub in the UI then we might miss some of the messages sent from the pub
        // in the backend service when it starts.
        if (!!process.env.SQUADOV_ZEROMQ_UI_PORT) {
            await this._pub.bind(`tcp://127.0.0.1:${process.env.SQUADOV_ZEROMQ_UI_PORT}`)
        } else {
            await this._pub.bind("tcp://127.0.0.1:0")
        }

        // Figure out which port the publisher bound to. Set an
        // environment variable so that the local service knows too.
        process.env.SQUADOV_ZEROMQ_UI_PORT = this._port = new URL(this._pub.lastEndpoint).port

        if (!!process.env.SQUADOV_ZEROMQ_SERVICE_PORT) {
            await this._sub.bind(`tcp://127.0.0.1:${process.env.SQUADOV_ZEROMQ_SERVICE_PORT}`)
        } else {
            await this._sub.bind(`tcp://127.0.0.1:0`)
        }
        process.env.SQUADOV_ZEROMQ_SERVICE_PORT = new URL(this._sub.lastEndpoint).port
        this._sub.subscribe()

        log.log(`Starting ZeroMQ Server on Port ${this._port}`)
        log.log(`\tExpecting Backend Service ZeroMQ Port ${process.env.SQUADOV_ZEROMQ_SERVICE_PORT}`)
        this._started = true
    }

    async run() {
        for await (const [topic, msg] of this._sub) {
            let topicStr = topic.toString('utf-8')
            let msgStr = !!msg ? msg.toString('utf-8') : ''
            if (!(topicStr in this._handlers)) {
                continue
            }

            for (let h of this._handlers[topic]) {
                h(msgStr)
            }
        }
    }

    async updateSessionId(sessionId) {
        await this._pub.send(['session-id', sessionId])
    }

    async updateRecordingPaused(paused) {
        await this._pub.send(['change-state-pause', JSON.stringify(paused)])
    }

    on(topic, handler) {
        if (!(topic in this._handlers)) {
            this._handlers[topic] = []
        }

        this._handlers[topic].push(handler)
    }

    async close() {
        if (this._started) {
            try {
                await this._sub.unbind(this._sub.lastEndpoint)
            } catch(ex) {
                console.log(ex)
            }

            try {
                await this._pub.unbind(this._pub.lastEndpoint)
            } catch(ex) {
                console.log(ex)
            }
        }

        this._sub.close()
        this._pub.close()
    }

    async requestAudioOutputOptions() {
        await this._pub.send(['request-audio-output', ''])
    }

    async requestAudioInputOptions() {
        await this._pub.send(['request-audio-input', ''])
    }
}

module.exports.ZeroMQServerClient = ZeroMQServerClient