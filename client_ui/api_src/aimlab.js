const express = require('express')

class AimlabApiServer {
    constructor(db) {
        this.db = db
    }

    createRouter() {
        const aimlabRouter = express.Router()
        aimlabRouter.get('/tasks', this.listAimlabTasks.bind(this))
        aimlabRouter.get('/tasks/:taskId', this.getAimlabTask.bind(this))
        return aimlabRouter
    }

    listAimlabTasks(req, res) { 
        this.db.all(`
        SELECT *
        FROM aimlab_tasks
        ORDER BY id DESC
        `, [], (err, rows) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(rows)
        })
    }

    getAimlabTask(req, res) {
        this.db.get(`
            SELECT *
            FROM aimlab_tasks
            WHERE id = ?
        `, [req.params['taskId']], (err, row) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(row)            
        })
    }
}

module.exports.AimlabApiServer = AimlabApiServer