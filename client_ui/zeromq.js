const {app} = require('electron')
const zmq = require('zeromq')
const log = require('./log.js')
// Use the ZeroMQServer for communication with the backend service.
// Note that we create both a subscriber and a publisher here because
// the backend may receive updates that it needs to communicate with the
// frontend. Using this will set two environment variables:
//
// - SQUADOV_ZEROMQ_SERVER_KEY: ZeroMQ CURVE server key to be used as the "curveSeverKey" option.
// - SQUADOV_ZEROMQ_PORT: The random port used to connect to the ZeroMQ server.
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

            const clientKp  = zmq.curveKeyPair()
            this._sub.curveSecretKey = clientKp.secretKey
            this._sub.curvePublicKey = clientKp.publicKey
            this._sub.curveServerKey = serverKp.publicKey
        }

        this._port = 0
        this._handlers = {}
    }

    async start() {
        if (!!process.env.SQUADOV_ZEROMQ_PORT) {
            await this._pub.bind(`tcp://127.0.0.1:${process.env.SQUADOV_ZEROMQ_PORT}`)
        } else {
            await this._pub.bind("tcp://127.0.0.1:0")
        }

        // Figure out which port the publisher bound to. Set an
        // environment variable so that the local service knows too.
        process.env.SQUADOV_ZEROMQ_PORT = this._port = new URL(this._pub.lastEndpoint).port
        await this._sub.connect(`tcp://127.0.0.1:${this._port}`)
        this._sub.subscribe('session-id')

        log.log(`Starting ZeroMQ Server on Port ${this._port}`)
        if (!app.isPackaged) {
            log.log(`\tZeroMQ Key: ${process.env.SQUADOV_ZEROMQ_SERVER_KEY}`)
        }
    }

    async run() {
        for await (const [topic, msg] of this._sub) {
            let msgStr = msg.toString('utf-8')
            if (!(topic in this._handlers)) {
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

    on(topic, handler) {
        if (!(topic in this._handlers)) {
            this._handlers[topic] = []
        }

        this._handlers[topic].push(handler)
    }

    close() {
        this._sub.close()
        this._pub.unbind(this._pub.lastEndpoint)
    }
}

module.exports.ZeroMQServerClient = ZeroMQServerClient