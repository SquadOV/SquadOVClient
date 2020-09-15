const CURRENT_DB_VERSION = 1

async function migrateDb(db) {
    return new Promise(resolve => {
        db.get('PRAGMA user_version', [], (err, row) => {
            if (!!err) {
                console.log('poop prgrama')
                throw err
            }
    
            const currentVersion = row.user_version
            console.log(`Migrating DB from v${currentVersion} to v${CURRENT_DB_VERSION}`)
            db.serialize(() => {
                db.run('BEGIN EXCLUSIVE TRANSACTION;')
                if (currentVersion < 1) {
    
                    db.run(`
    CREATE TABLE valorant_accounts (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        puuid VARCHAR NOT NULL,
        username VARCHAR NOT NULL,
        tag VARCHAR NOT NULL
    );
                    `)
                }
    
                db.run(`PRAGMA user_version = ${CURRENT_DB_VERSION}`)
                db.exec('COMMIT TRANSACTION;')
            })
            resolve()
        })
        
    })

}

exports.migrateDb = migrateDb