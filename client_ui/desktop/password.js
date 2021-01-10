const {app, BrowserWindow} = require('electron')
const { ipcMain } = require('electron')
const log = require('./log.js')

// Require the user to choose a password for encryption.
// Store a hash of this password in a file so we can verify it later.
// The user must type this same password every time they start the app.
// Note that this is a temporary measure while this is a client-only app -
// the real goal should be able to use Riot's official APIs.
module.exports.createMigrationFlow =  function(win) {
    return new Promise((resolve, reject) => {
        ipcMain.once('finish', (event) => {
            resolve()
        })
        
        win.loadFile('password.html')
        win.on('close', () => {
            resolve()
        })
    })
}