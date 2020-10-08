const {app, BrowserWindow} = require('electron')
const { ipcMain } = require('electron')

// Require the user to choose a password for encryption.
// Store a hash of this password in a file so we can verify it later.
// The user must type this same password every time they start the app.
// Note that this is a temporary measure while this is a client-only app -
// the real goal should be able to use Riot's official APIs.
module.exports.loginFlow =  function(win) {
    return new Promise((resolve, reject) => {
        win.loadFile('login.html')

        ipcMain.once('obtain-session', (event, [session, userId]) => {
            process.env.SQUADOV_SESSION_ID = session
            process.env.SQUADOV_USER_ID = userId
        })

        ipcMain.once('finish-login', (event, arg) => {
            resolve()
        })

        win.on('close', () => {
            reject('Failed to login.')
        })
    })
}