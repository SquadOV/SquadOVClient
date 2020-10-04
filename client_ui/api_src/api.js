const express = require('express')
const crypto = require('crypto')
const sqlite3 = require('sqlite3')
const path = require('path')
const fs = require('fs')
const { migrateDb } = require('./migrate.js')
const {app} = require('electron')
const { ValorantApiServer } = require('./valorant.js')
const { AimlabApiServer } = require('./aimlab.js')
const { createGraphqlEndpoint } = require('./graphql/graphql.js')
const log = require('../log.js')
const { generateSelfSignedKeyCert } = require('./https')
const https = require('https')
const { ipcMain } = require('electron')

const checkApiKey = (req, res, next) => {
    const key = process.env.SQUADOV_API_KEY
    const auth = req.get('Authorization')

    // Allow the key to come in either the authorization header
    // or as a URL param.
    if (`Bearer ${key}` !== auth && req.query.key !== key) {
        res.status(401).json({ error: 'Invalid API Key.' })
        return
    }

    next()
}

ipcMain.on('apiServer', (event) => {
    event.returnValue = [process.env.SQUADOV_API_PORT, process.env.SQUADOV_API_KEY]
})

class ApiServer {
    async initBackendDatabase() {
        const dirname = path.join(app.getPath('appData'), 'SquadOV')
        if (!fs.existsSync(dirname)) {
            fs.mkdir(dirname, {
                recursive: true,
                mode: 0o755,
            }, (err) => {
                if (!!err) throw err
            })
        }
    
        const fname = path.join(dirname, 'squadov.db')
    
        let initDb = new sqlite3.Database(fname, sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE, (err) => {
            if (!!err) throw err
        })
        await migrateDb(initDb)
        initDb.close()
    
        this.db = new sqlite3.Database(fname, sqlite3.OPEN_READWRITE, (err) => {
            if (!!err) throw err
        })

        this.valorant = new ValorantApiServer(this.db)
        this.aimlab = new AimlabApiServer(this.db)
    }

    constructor() {
        this.apiOptions = {
            apiPort: -1,
            apiKey: !!process.env.SQUADOV_API_KEY ? process.env.SQUADOV_API_KEY : crypto.randomBytes(128).toString('base64')
        }
        process.env.SQUADOV_API_KEY = this.apiOptions.apiKey
    }

    async start(onStart) {
        await this.initBackendDatabase()

        const restApp = express()
        restApp.use(express.json())
        restApp.use(checkApiKey)

        restApp.use('/valorant', this.valorant.createRouter())
        restApp.use('/aimlab', this.aimlab.createRouter())
        restApp.use('/graphql', createGraphqlEndpoint(this.db))

        let port = !!process.env.SQUADOV_API_PORT ? parseInt(process.env.SQUADOV_API_PORT) : 0
        const tlsOptions = generateSelfSignedKeyCert()
        this.server = https.createServer(tlsOptions, restApp)
        this.server.listen({
            port: port,
        }, () => {
            process.env.SQUADOV_API_PORT = this.apiOptions.apiPort = this.server.address().port
            log.log(`Starting API Server on Port ${process.env.SQUADOV_API_PORT} with Key ${process.env.SQUADOV_API_KEY}`)
            onStart()
        })
    }

    close() {
        if (!!this.server) {
            this.server.close()
        }

        if (!!this.db) {
            this.db.close()
        }
    }
}

exports.ApiServer = ApiServer