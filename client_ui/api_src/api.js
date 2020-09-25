const express = require('express')
const crypto = require('crypto')
const sqlite3 = require('sqlite3')
const path = require('path')
const fs = require('fs')
const { migrateDb } = require('./migrate.js')
const {app} = require('electron')
const { ValorantApiServer } = require('./valorant.js')

const checkApiKey = (req, res, next) => {
    const key = process.env.SQUADOV_API_KEY
    const auth = req.get('Authorization')

    if (`Bearer ${key}` !== auth) {
        res.status(401).json({ error: 'Invalid API Key.' })
        return
    }

    next()
}

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
    
        this.db = new sqlite3.Database(fname, sqlite3.OPEN_READONLY, (err) => {
            if (!!err) throw err
        })

        this.valorant = new ValorantApiServer(this.db)
    }

    constructor() {
        this.apiOptions = {
            apiPort: -1,
            apiKey: crypto.randomBytes(128).toString('base64')
        }
        process.env.SQUADOV_API_KEY = this.apiOptions.apiKey
    }

    async start(onStart) {
        await this.initBackendDatabase()

        const restApp = express()
        restApp.use(checkApiKey)

        restApp.use('/valorant', this.valorant.createRouter())

        this.server = restApp.listen(0, () => {
            process.env.SQUADOV_API_PORT = this.apiOptions.apiPort = this.server.address().port
            console.log(`Starting API Server on Port ${process.env.SQUADOV_API_PORT} with Key ${process.env.SQUADOV_API_KEY}`)
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