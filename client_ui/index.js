const {app, BrowserWindow, ipcMain, net} = require('electron')
const path = require('path')
const fs = require('fs')
const {spawn} = require('child_process');
const log = require('./log.js')
const { dialog } = require('electron')
const { loginFlow } = require('./login.js')
const { createVerifyEncryptionPasswordFlow} = require('./password.js')

const configFile = app.isPackaged ? path.join(process.resourcesPath, 'config/config.json') : 'config/config.json'
const config = JSON.parse(fs.readFileSync(configFile))
process.env.API_SQUADOV_URL = config["API_URL"]

let win

function start() {
    win.loadFile('index.html')
}

ipcMain.on('request-app-folder', (event) => {
    event.returnValue = process.env.SQUADOV_USER_APP_FOLDER
})

// Start a local ZeroMQ server that'll be used for IPC between the electron client
// and the underlying local client service.
const { ZeroMQServerClient } = require('./zeromq')
let zeromqServer = new ZeroMQServerClient()

// Start a local API server that'll be used manage our connections to the
// database that holds all the information we want to retrieve.
const { ApiServer } = require('./api_src/api');

let apiServer = new ApiServer()
async function quit() {
    zeromqServer.close()
    apiServer.close()
    app.quit()
}

app.on('certificate-error', (event, contents, url, error, certificate, callback) => {
    if (url.startsWith(`https://127.0.0.1:${process.env.SQUADOV_API_PORT}`)) {
        event.preventDefault()
        callback(true)
    } else {
        callback(false)
    }
})

function setAppDataFolderFromEnv() {
    process.env.SQUADOV_USER_APP_FOLDER = path.join(app.getPath('appData'), 'SquadOV', `${process.env.SQUADOV_USER_ID}`)

    if (!fs.existsSync(process.env.SQUADOV_USER_APP_FOLDER)) {
        fs.mkdirSync(process.env.SQUADOV_USER_APP_FOLDER, {
            recursive: true
        })

        // Check if there's an existing installation. If so, migrate everything into this new folder.
        // Note that *eveyrthing* here excludes the log folder. That should stay in the non-user specified folder.
        let oldInstallPath = path.join(app.getPath('appData'), 'SquadOV')
        let oldDbPath = path.join(oldInstallPath, 'squadov.db')
        if (fs.existsSync(oldDbPath)) {
            fs.renameSync(
                oldDbPath,
                path.join(process.env.SQUADOV_USER_APP_FOLDER, 'squadov.db'),
            )
        }

        let oldVerifyPath = path.join(oldInstallPath, 'verify.bcrypt')
        if (fs.existsSync(oldVerifyPath)) {
            fs.renameSync(
                oldVerifyPath,
                path.join(process.env.SQUADOV_USER_APP_FOLDER, 'verify.bcrypt'),
            )
        }

        let oldRecordPath = path.join(oldInstallPath, 'Record')
        if (fs.existsSync(oldRecordPath)) {
            fs.renameSync(
                oldRecordPath,
                path.join(process.env.SQUADOV_USER_APP_FOLDER, 'Record'),
            )
        }
    }
}

function getSessionPath() {
    return path.join(app.getPath('appData'), 'SquadOV', 'session.json')
}

function restart() {
    app.relaunch()
    quit()
}

function logout() {
    const sessionPath = getSessionPath()
    if (fs.existsSync(sessionPath)) {
        fs.unlinkSync(sessionPath)
    }
    restart()
}

ipcMain.on('logout', logout)

// Returns true is a session was loaded successfuly.
function loadSession() {
    const sessionPath = getSessionPath()
    if (!fs.existsSync(sessionPath)) {
        return false
    }

    const data = JSON.parse(fs.readFileSync(sessionPath))
    process.env.SQUADOV_SESSION_ID = data.sessionId
    process.env.SQUADOV_USER_ID = data.userId
    return true
}

function saveSession() {
    fs.writeFileSync(getSessionPath(), JSON.stringify({
        sessionId: process.env.SQUADOV_SESSION_ID,
        userId: process.env.SQUADOV_USER_ID,
    }), {
        encoding: 'utf-8',
    })
}

function updateSession(sessionId, sendIpc) {
    process.env.SQUADOV_SESSION_ID = sessionId
    saveSession()

    log.log('UPDATE SESSION: ' , sessionId)
    if (!!sendIpc) {
        zeromqServer.updateSessionId(sessionId)
    }
    win.webContents.send('update-session', sessionId)
}


zeromqServer.on('session-id', (sessionId) => {
    updateSession(sessionId, false)
})

let backendReady = new Promise((resolve, reject) => {
    zeromqServer.on('on-ready', () => {
        resolve()
    })
})


// This is the initial session obtainment from logging in. Loading it from storage will
// directly call loadSession(). This event ONLY happens in the Login UI.
ipcMain.on('obtain-session', (event, [session, userId]) => {
    process.env.SQUADOV_SESSION_ID = session
    process.env.SQUADOV_USER_ID = userId
    saveSession()
})

// This event gets called when the UI obtains a new session ID from the API server.
// This can happen when/if the session expires and the API server refreshes it and gives us
// a new one.
ipcMain.on('refresh-session', (event, session) => {
    updateSession(session, true)
})

// This event gets called when the primary app UI loads up for the first time
// and needs to retrieve what the current session ID and user ID are.
ipcMain.handle('request-session', () => {
    return {
        sessionId: process.env.SQUADOV_SESSION_ID,
        userId: process.env.SQUADOV_USER_ID,
    }
})

totalCloseCount = 0
function startClientService() {
    if (totalCloseCount > 5) {
        console.log('Close count exceeded threshold - preventing automatic reboot. Please restart SquadOV.')
        if (!!win) {
            win.webContents.send('service-error')
        }
        return
    }

    // Start auxiliary service that'll handle waiting for games to run and
    // collecting the relevant information and sending it to the database.
    // Search for the proper executable file.
    //  1) The file specified by the environment variable: SQUADOV_SERVICE_EXE
    //  2) Relative to the current working directory: resources/service/squadov_client_service.exe
    let exeName = ''
    if (process.platform == 'win32') {
        exeName = 'squadov_client_service.exe'
    }

    let exePath = process.env.SQUADOV_SERVICE_EXE
    if (!fs.existsSync(exePath)) {
        exePath = path.join(path.dirname(app.getPath('exe')), 'resources', 'service', exeName)
        if (!fs.existsSync(exePath)) {
            dialog.showMessageBoxSync({
                type: 'error',
                title: 'SquadOV Installation Error',
                message: 'Failed to find the client service executable for SquadOV. Please check that SquadOV was installed correctly. Reinstall if necessary.'
            })

            quit()
            return
        }
    }

    log.log("SPAWN PROCESS: " + exePath);
    let child = spawn(exePath)
    child.stdout.on('data', (data) => {
        console.log(`SERVICE: ${data}`)
    })

    child.stderr.on('data', (data) => {
        console.log(`SERVICE: ${data}`)
    })

    child.on('close', (code) => {
        console.log('Unexpected close of client service: ', code)
        totalCloseCount += 1
        setTimeout(() => {
            startClientService()
        }, 500)
    })
}

function startAutoupdater() {
    const { autoUpdater } = require("electron-updater")
    autoUpdater.autoDownload = true
    autoUpdater.autoInstallOnAppQuit = true
    autoUpdater.logger = log

    // This event is for when an update is available and we're past
    // the initial start-up workflow. In this case we need to indicate
    // to the user that an update is available and have them restart.
    autoUpdater.on('update-downloaded', (info) => {
        win.webContents.send('main-update-downloaded', info.version)
    })

    // 30 minute update check interval to give us some buffer to grow for now.
    // IF Github has a limit of 5000 API calls per hour and an update check uses
    // up to 3 calls per check, then doing it every 30 minutes will allow us to grow up to
    // 833 active users.
    // TODO: REPLACE GITHUB RELEASES WITH A MORE SUSTAINABLE SOLUTION.
    setInterval(() => {
        autoUpdater.checkForUpdates()
    }, 30 * 60 * 1000)

    let updateWindow = new BrowserWindow({
        width: 300,
        height: 300,
        webPreferences: {
            nodeIntegration: true,
        },
        frame: false,
        resizable: false,
        movable: false,
    })
    updateWindow.webContents.toggleDevTools()
    updateWindow.loadFile('update.html')
    updateWindow.show()
    updateWindow.on('close', () => {
        quit()
    })

    return new Promise((resolve, reject) => {
        autoUpdater.once('update-not-available', () => {
            autoUpdater.removeAllListeners('update-available')
            autoUpdater.removeAllListeners('download-progress')
            updateWindow.removeAllListeners('close')
            updateWindow.close()
            resolve()
        })

        autoUpdater.on('update-available', (info) => {
            updateWindow.webContents.send('update-update-available', info)
        })

        autoUpdater.on('download-progress', (progress) => {
            updateWindow.webContents.send('update-download-progress', progress)
        })

        autoUpdater.once('update-downloaded', () => {
            restart()
        })
        autoUpdater.checkForUpdates()
    })
}

function startSessionHeartbeat(onBeat) {
    const url = `${process.env.API_SQUADOV_URL}/auth/session/heartbeat`
    log.log('Starting session heartbeat...', url)
    const request = net.request({
        method: 'POST',
        url
    })
    request.setHeader('Content-Type', 'application/json')
    request.write(JSON.stringify({
        sessionId: process.env.SQUADOV_SESSION_ID
    }))
    request.on('response', (resp) => {
        if (resp.statusCode != 200) {
            log.log('Sesssion Heartbeat Failure: ', resp.statusCode, resp.statusMessage)
            logout()
        } else {
            let body = ''
            resp.on('data', (chunk) => {
                body += chunk.toString()
            })
            
            resp.on('end', () => {
                let respBody = JSON.parse(body)

                updateSession(respBody.sessionId, true)
                if (!!onBeat) {
                    onBeat()
                }

                let expiration = new Date(respBody.expiration)

                // Preemptively refresh the session 10 minutes before it's due to expire.
                setTimeout(() => {
                    startSessionHeartbeat()
                }, Math.max(expiration.getTime() - Date.now() - 10 * 60 * 1000, 0))
            })
        }
    })
    request.end()
}

app.on('ready', async () => {
    await zeromqServer.start()
    zeromqServer.run()

    win = new BrowserWindow({
        width: 1280,
        height: 720,
        webPreferences: {
            nodeIntegration: true,
        }
    })

    if (!app.isPackaged) {
        win.webContents.toggleDevTools()
    }

    win.setMenu(null)
    win.setMenuBarVisibility(false)
    win.hide()

    if (app.isPackaged) {
        await startAutoupdater()
    }

    win.show()
    if (!loadSession()) {
        // DO NOT GO ANY FURTHER UNTIL WE HAVE SUCCESSFULLY LOGGED IN.
        try {
            await loginFlow(win)
        } catch (ex) {
            log.log('User chose not to login...good bye: ', ex)
            quit()
            return
        }
        log.log(`OBTAINED SESSION: ${process.env.SQUADOV_SESSION_ID} - USER ID: ${process.env.SQUADOV_USER_ID}`)
        saveSession()
    }

    // Set the environment variable SQUADOV_USER_APP_FOLDER to specify which folder to store *ALL* this user's data in.
    setAppDataFolderFromEnv()

    // For simplicity, we only have the Electron app refresh the session ID instead of having everyone refreshing the session ID
    // themselves this way we can avoid race conditions where multiple people try to refresh the same session at the same time
    // and we end up with an invalid session ID.
    startSessionHeartbeat(async () => {
        // Note that all this stuff should be in the session heartbeat callback because we don't really want to 
        // start the UI and the client service (2 things that require querying the API) until we have a valid session.
        // This is particularly relevant when we're loading the session from disk.

        // THEN HAVE THE USER VERIFY THEIR LOCAL PASSWORD.
        // This is *different* form their account password. It cannot be reset.
        try {
            await createVerifyEncryptionPasswordFlow(win)
        } catch(ex) {
            log.log('User chose not to input/create password...good bye: ', ex)
            quit()
            return
        }
        log.log(`OBTAINED ENCRYPTION PASSWORD`)
        
        apiServer.start(async () => {
            if (!parseInt(process.env.SQUADOV_MANUAL_SERVICE)) {
                startClientService()
                await backendReady
            }
        })
        start()
    })
})

app.on('window-all-closed', () => {
    quit()
})