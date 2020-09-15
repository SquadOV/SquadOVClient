const express = require('express')
const crypto = require('crypto')
const sqlite3 = require('sqlite3')
const path = require('path')
const fs = require('fs')
const { migrateDb } = require('./migrate.js')
const {app} = require('electron')

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
    }

    constructor() {
        this.apiOptions = {
            apiPort: -1,
            apiKey: crypto.randomBytes(128).toString('base64')
        }
        process.env.SQUADOV_API_KEY = this.apiOptions.apiKey

        this.initBackendDatabase()
    }

    start(onStart) {
        const restApp = express()
        restApp.use(checkApiKey)

        const valorantRouter = express.Router()
        valorantRouter.get('/accounts', this.listValorantAccounts.bind(this))

        restApp.use('/valorant', valorantRouter)

        this.server = restApp.listen(0, () => {
            process.env.SQUADOV_API_PORT = this.apiOptions.apiPort = this.server.address().port
            console.log(`Starting API Server on Port ${this.apiOptions.apiPort} with Key ${this.apiOptions.apiKey}`)
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

    listValorantAccounts(req, res) { 
        this.db.all(`
        SELECT *
        FROM valorant_accounts
        `, [], (err, rows) => {
            if (!!err) res.status(500).json({ 'error': err })
            res.json(rows)
        })
    }
}

exports.ApiServer = ApiServer