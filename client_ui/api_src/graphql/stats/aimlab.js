class AimlabStatQueryBuilder {
    constructor(task, params) {
        this.task = task
        this.mode = params.mode
        this.group = params.group
        this.groupFn = params.groupFn
        this.sort = params.sort
    }

    get xColumn() {
        if (this.group == 'AGDATE') {
            return 'date(createDate)'   
        } else if (this.group == 'AGDATETIME') {
            return 'datetime(createDate)'
        } else if (this.group == 'AGTIME') {
            return 'time(createDate)'
        } else if (this.group == 'AGVERSION') {
            return 'version'
        } else {
            return 'ROW_NUMBER() OVER (ORDER BY datetime(createDate) ASC)'
        }
    }

    get groupSql() {
        if (!this.group) {
            return 'GROUP BY id'
        }
        return `GROUP BY ${this.xColumn}`
    }

    get sortSql() {
        if (!this.sort || !this.group) {
            // Technically nort sorting by ID but more in line with what the user
            // would expect.
            return 'ORDER BY datetime(createDate) ASC'
        }
        return `ORDER BY ${this.xColumn} ${this.sort}`
    }

    combinedColumn(col) {
        if (!this.groupFn) {
            return col
        }

        if (this.groupFn == 'AVG') {
            return `AVG(${col})`
        } else if (this.groupFn == 'MIN') {
            return `MIN(${col})`
        } else if (this.groupFn == 'MAX') {
            return `MAX(${col})`
        }
        return col
    }

    run(db) {
        return new Promise((resolve, reject) => {
            db.all(this.sql(), this.params(), (err, rows) => {
                if (!!err) {
                    reject(err)
                } else {
                    resolve(rows)
                }
            })
        })
    }
}

class AimlabSparDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score,
                ${this.combinedColumn(`json_extract(rawData, '$.killTotal')`)} AS kill,
                ${this.combinedColumn(`1000.0 / json_extract(rawData, '$.killsPerSec')`)} AS ttk,
                ${this.combinedColumn(`json_extract(rawData, '$.accTotal')`)} AS acc
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `

        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabDetectionDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabDecisionshotDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabTrackDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabErbDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabLinetraceDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

class AimlabPentakillDataQueryBuilder extends AimlabStatQueryBuilder {
    sql() {
        let str = `
            SELECT
                ${this.xColumn} AS x,
                ${this.combinedColumn('score')} AS score
            FROM aimlab_tasks
            WHERE taskName = '${this.task}' AND mode = ?
            ${this.groupSql}
            ${this.sortSql}
        `
        return str
    }

    params() {
        return [this.mode]
    }
}

function createResolveAimlabSparData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabSparDataQueryBuilder(task, args)
        return q.run(context.db)
    }
}

function createResolveAimlabDetectionData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabDetectionDataQueryBuilder(task, args)
        return q.run(context.db)
    }
}

function createResolveAimlabDecisionshotData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabDecisionshotDataQueryBuilder(task, args)
        return q.run(context.db)
    }    
}

function createResolveAimlabTrackData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabTrackDataQueryBuilder(task, args)
        return q.run(context.db)
    }    
}

function createResolveAimlabErbData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabErbDataQueryBuilder(task, args)
        return q.run(context.db)
    }    
}

function createResolveAimlabLinetraceData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabLinetraceDataQueryBuilder(task, args)
        return q.run(context.db)
    }    
}

function createResolveAimlabPentakillData(task) {
    return (obj, args, context, info) => {
        let q = new AimlabPentakillDataQueryBuilder(task, args)
        return q.run(context.db)
    }    
}

module.exports.resolvers = {
    AimlabStats: {
        gridshot: createResolveAimlabSparData('gridshot'),
        spidershot: createResolveAimlabSparData('spidershot'),
        microshot: createResolveAimlabSparData('microshot'),
        sixshot: createResolveAimlabSparData('sixshot'),
        microflex: createResolveAimlabSparData('microflex'),
        motionshot: createResolveAimlabSparData('motionshot'),
        multishot: createResolveAimlabSparData('multishot'),
        detection: createResolveAimlabDetectionData('detection'),
        decisionshot: createResolveAimlabDecisionshotData('decisionshot'),
        strafetrack: createResolveAimlabTrackData('strafetrack'),
        circletrack: createResolveAimlabTrackData('circletrack'),
        strafeshot: createResolveAimlabErbData('strafeshot'),
        circleshot: createResolveAimlabErbData('circleshot'),
        linetrace: createResolveAimlabLinetraceData('linetrace'),
        multilinetrace: createResolveAimlabLinetraceData('multilinetrace'),
        pentakill: createResolveAimlabPentakillData('pentakill'),
    }
}