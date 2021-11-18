/*
if (handleSquirrel()) {
    return
}

function handleSquirrel() {
    if (process.argv.length === 1) {
        return false
    }

    const {app} = require('electron')
    const ChildProcess = require('child_process')
    const path = require('path')
    const fs = require('fs')

    const appFolder = path.resolve(process.execPath, '..')
    const rootAtomFolder = path.resolve(appFolder, '..')
    const updateDotExe = path.resolve(path.join(rootAtomFolder, 'Update.exe'))
    const exeName = path.basename(process.execPath)

    const spawn = function(command, args) {
        let spawnedProcess, error

        try {
            spawnedProcess = ChildProcess.spawn(command, args, {detached: true})
        } catch (error) {}

        return spawnedProcess
    };

    const spawnUpdate = function(args) {
        return spawn(updateDotExe, args)
    };

    const squirrelEvent = process.argv[1]
    switch (squirrelEvent) {
        case '--squirrel-install':
        case '--squirrel-updated':
            // Optionally do things such as:
            // - Add your .exe to the PATH
            // - Write to the registry for things like file associations and
            //   explorer context menus

            // Uninstall the old SquadOV.
            const oldUninstaller = path.join(process.env.LOCALAPPDATA, 'Programs', 'squadov_client_ui', 'Uninstall SquadOV.exe')
            if (fs.existsSync(oldUninstaller)) {
                spawn(oldUninstaller, ['/S'])
            }

            // Install desktop and start menu shortcuts
            spawnUpdate(['--createShortcut', exeName])

            setTimeout(app.quit, 1000)
            return true

        case '--squirrel-uninstall':
            // Undo anything you did in the --squirrel-install and
            // --squirrel-updated handlers

            // Remove desktop and start menu shortcuts
            spawnUpdate(['--removeShortcut', exeName])

            setTimeout(app.quit, 1000)
            return true

        case '--squirrel-obsolete':
            // This is called on the outgoing version of your app before
            // we update to the new version - it's the opposite of
            // --squirrel-updated

            app.quit()
            return true
    }
    return false
}
*/

const {app, BrowserWindow, Menu, Tray, ipcMain, net, globalShortcut} = require('electron')
const path = require('path')
const fs = require('fs')
const {spawn} = require('child_process');
const log = require('./log.js')
const { dialog } = require('electron')
const { loginFlow } = require('./login.js')
const { setupFlow } = require('./setup.js')
const { getAppDataFolder } = require('./paths.js')

const configFile = app.isPackaged ? path.join(process.resourcesPath, 'config/config.json') : 'config/config.json'
const config = JSON.parse(fs.readFileSync(configFile))
process.env.API_SQUADOV_URL = config["API_URL"]
process.env.SQUADOV_TZDATA = !!process.env.SQUADOV_TZDATA ?
    process.env.SQUADOV_TZDATA :
    app.isPackaged ? path.join(process.resourcesPath, 'tzdata') : path.join(__dirname, '../../resources/tzdata')
process.env.SQUADOV_CSGO_RESOURCES = !!process.env.SQUADOV_CSGO_RESOURCES ?
    process.env.SQUADOV_CSGO_RESOURCES :
    app.isPackaged ? path.join(process.resourcesPath, 'csgo') : path.join(__dirname, '../../resources/csgo')
const iconPath = path.join(__dirname, 'assets/icon.ico')

if (app.isPackaged) {
    process.env.SQUADOV_VERSION = app.getVersion()
}

let win
let editorWin
let tray
let isQuitting = false

let appSettings = null
function loadAppSettings() {
    const fname = path.join(process.env.SQUADOV_USER_APP_FOLDER, 'settings.json')
    if (fs.existsSync(fname)) {
        const data = fs.readFileSync(fname)
        appSettings = JSON.parse(data)
    }

    if (app.isPackaged) {
        const appFolder = path.dirname(process.execPath)
        const updateExe = path.resolve(appFolder , '..', 'Update.exe')
        const exeName = path.basename(process.execPath)

        /*
        app.setLoginItemSettings({
            openAtLogin: appSettings.runOnStartup === true,
            path: updateExe,
            args: [
                '--processStart', `"${exeName}"`,
                '--process-start-args', '"--hidden"'
            ]
        })
        */
        app.setLoginItemSettings({
            openAtLogin: appSettings.runOnStartup === true,
            args: [
                '--hidden'
            ]
        })
    }
}

app.on('quit', () => {
    log.log('Quitting out of SquadOV...')
})

function restore() {
    if (!!win) {
        if (!win.isVisible()) {
            win.show()
        }

        if (win.isMinimized()) {
            win.restore()
        }

        win.focus()
    }
}

const singleLock = app.requestSingleInstanceLock()
if (!singleLock) {
    app.exit(1)
} else {
    app.on('second-instance', () => {
        restore()
    })
}

function start() {
    win.loadFile('index.html')

    win.on('minimize', (e) => {
        if (!!appSettings.minimizeToTray) {
            e.preventDefault()
            win.hide()
        }

        win.webContents.send('onActiveChange', false)
    })

    win.on('close', (e) => {
        if (!isQuitting) {
            if (!!appSettings.minimizeOnClose) {
                e.preventDefault()
                win.minimize()
                win.webContents.send('onActiveChange', false)
            } else {
                quit()
            }
        }
    })

    win.on('show', (e) => {
        win.webContents.send('onActiveChange', true)
    })

    win.on('restore', (e) => {
        win.webContents.send('onActiveChange', true)
    })

    tray = new Tray(iconPath)
    const contextMenu = Menu.buildFromTemplate([
        {
            label: 'Quit SquadOV',
            click: () => {
                quit()
            }
        }
    ])

    tray.on('click', () => {
        win.show()
    })
    tray.setContextMenu(contextMenu)
}

ipcMain.on('request-app-folder', (event) => {
    event.returnValue = process.env.SQUADOV_USER_APP_FOLDER
})

// Start a local ZeroMQ server that'll be used for IPC between the electron client
// and the underlying local client service.
const { ZeroMQServerClient } = require('./zeromq')
let zeromqServer = new ZeroMQServerClient()

async function quit() {
    isQuitting = true
    await zeromqServer.close()
    app.exit(0)
}

function setAppDataFolderFromEnv() {
    process.env.SQUADOV_USER_APP_FOLDER = path.join(getAppDataFolder(), `${process.env.SQUADOV_USER_ID}`)
    log.log('Set App Data Folder: ', getAppDataFolder(), process.env.SQUADOV_USER_APP_FOLDER)
    if (!fs.existsSync(process.env.SQUADOV_USER_APP_FOLDER)) {
        fs.mkdirSync(process.env.SQUADOV_USER_APP_FOLDER, {
            recursive: true
        })
    }
}

function getSessionPath() {
    return path.join(getAppDataFolder(), 'session.json')
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

    try {
        const data = JSON.parse(fs.readFileSync(sessionPath))
        process.env.SQUADOV_SESSION_ID = data.sessionId
        process.env.SQUADOV_USER_ID = data.userId
        return true
    } catch (e) {
        log.log('Failed to read session JSON - forcing users to relog..', e)
        return false
    }
}

function saveSession() {
    let tmpPath = getSessionPath() + '.tmp'

    // Two step process so the change is more atomic so we don't run the risk
    // of corrupting the session.json file.
    fs.writeFileSync(tmpPath, JSON.stringify({
        sessionId: process.env.SQUADOV_SESSION_ID,
        userId: process.env.SQUADOV_USER_ID,
    }), {
        encoding: 'utf-8',
    })

    fs.renameSync(tmpPath, getSessionPath())
}

function updateSession(sessionId, sendIpc) {
    process.env.SQUADOV_SESSION_ID = sessionId
    saveSession()

    log.log('UPDATE SESSION: ' , sessionId)
    if (!!sendIpc) {
        zeromqServer.updateSessionId(sessionId)
    }

    if (!!win) {
        win.webContents.send('update-session', sessionId)
    }

    if (!!editorWin) {
        editorWin.webContents.send('update-session', sessionId)
    }
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

// This gets called when the app starts. It's needed to pass the USER_APP_FOLDER
// env variable to the app since we can't rely on the environment variable being 
// set properly the first time the user starts the app.
ipcMain.handle('request-app-folder', () => {
    return process.env.SQUADOV_USER_APP_FOLDER
})

// This event gets called whenever the user wants to resume/pause recording.
ipcMain.on('change-state-pause', (event, paused) => {
    zeromqServer.updateRecordingPaused(paused)
})

ipcMain.on('request-restart', () => {
    restart()
})

ipcMain.handle('request-vod-clip', async (event, {task, source, start, end}) => {
    return await zeromqServer.performVodClip(task, source, start, end)
})

ipcMain.handle('request-cloud-upload', async (event, {task, file, destination}) => {
    return await zeromqServer.performCloudUpload(task, file, destination)
})

ipcMain.on('open-vod-editor', (event, {videoUuid, game}) => {
    if (!editorWin) {
        editorWin = new BrowserWindow({
            width: 1600,
            height: 900,
            frame: false,
            webPreferences: {
                nodeIntegration: true,
                contextIsolation: false,
                webSecurity: app.isPackaged,
            },
            icon: iconPath
        })
    
        if (!app.isPackaged) {
            editorWin.webContents.toggleDevTools()
        }

        editorWin.setMenu(null)
        editorWin.setMenuBarVisibility(false)
        editorWin.on('close', () => {
            editorWin = null
        })
    }

    editorWin.loadURL(`file://${__dirname}/index.html#editor/${videoUuid}?game=${game}`)
    editorWin.show()
})

ipcMain.on('close-vod-editor', (event) => {
    if (!!editorWin) {
        if (editorWin.isDevToolsOpened()) {
            editorWin.closeDevTools()
        }

        editorWin.close()
        editorWin = null
    }
})

ipcMain.on('redirect-to-route', (event, route) => {
    if (!!win) {
        win.webContents.send('redirect-to-route', route)
    }
})

ipcMain.on('open-path-window', (event, path) => {
    let pathWin = new BrowserWindow({
        width: 1600,
        height: 900,
        frame: false,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            webSecurity: app.isPackaged,
        },
        icon: iconPath
    })
    if (!app.isPackaged) {
        pathWin.webContents.toggleDevTools()
    }

    pathWin.setMenu(null)
    pathWin.setMenuBarVisibility(false)
    pathWin.loadURL(`file://${__dirname}/index.html${path}`)
    pathWin.show()
})

async function requestOutputDevices() {
    await zeromqServer.requestAudioOutputOptions()
}

async function requestInputDevices() {
    await zeromqServer.requestAudioInputOptions()
}

ipcMain.on('request-output-devices', async () => {
    await requestOutputDevices()
})

ipcMain.on('request-input-devices', async () => {
    await requestInputDevices()
})

zeromqServer.on('change-running-games', (games) => {
    win.webContents.send('change-running-games', JSON.parse(games))
})

zeromqServer.on('change-recording-games', (games) => {
    win.webContents.send('change-recording-games', JSON.parse(games))
})

zeromqServer.on('post-game-notification', (game) => {
    let data = JSON.parse(game)
    data.tm = new Date(data.tm)
    win.webContents.send('post-game-notification', data)
})

zeromqServer.on('respond-output-devices', (r) => {
    win.webContents.send('respond-output-devices', JSON.parse(r))
})

zeromqServer.on('respond-input-devices', (r) => {
    win.webContents.send('respond-input-devices', JSON.parse(r))
})

totalCloseCount = 0
function startClientService() {
    if (totalCloseCount > 5) {
        log.log('Close count exceeded threshold - preventing automatic reboot. Please restart SquadOV.')
        if (!!win) {
            win.webContents.on('did-finish-load', () => {
                win.webContents.send('service-error')    
            })
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

    log.log("SPAWN PROCESS: " + exePath)
    let child = spawn(exePath, {
        cwd: path.dirname(exePath)
    })

    child.stdout.on('data', (data) => {
        console.log(`SERVICE: ${data}`)
    })

    child.stderr.on('data', (data) => {
        console.log(`SERVICE: ${data}`)
    })
    child.on('close', (code) => {

        log.log('Unexpected close of client service: ', code)
        totalCloseCount += 1
        win.webContents.send('reset-state')
        setTimeout(() => {
            startClientService()
        }, 1000)
    })
}

function startAutoupdater() {
    const { autoUpdater } = require("electron-updater")
    autoUpdater.autoDownload = false
    autoUpdater.autoInstallOnAppQuit = false
    autoUpdater.logger = log
    autoUpdater.channel = app.commandLine.hasSwitch('beta') ? 'beta' : 'latest'

    setInterval(() => {
        // This event is for when an update is available and we're past
        // the initial start-up workflow. In this case we need to indicate
        // to the user that an update is available and have them restart.
        autoUpdater.checkForUpdates().then((resp) => {
            console.log(`SquadOV Found Update to Version  ${resp.updateInfo.version} vs ${app.getVersion()}`)
            if (resp.updateInfo.version != app.getVersion()) {
                win.webContents.send('main-update-downloaded', resp.updateInfo.version)
            }
        })
    }, 1 * 60 * 1000)

    let updateWindow = new BrowserWindow({
        width: 300,
        height: 300,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
        },
        frame: false,
        resizable: false,
        movable: false,
        icon: iconPath
    })

    if (!app.isPackaged) {
        updateWindow.webContents.toggleDevTools()
    }
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

        autoUpdater.once('update-available', (info) => {
            log.log('Index Update Available: ', info)
            autoUpdater.downloadUpdate().then(() => {
                autoUpdater.quitAndInstall(false, true)
            }).catch((err) => {
                console.log('Failed to update: ', err)
            })
            updateWindow.webContents.send('update-update-available', info)            
        })

        autoUpdater.on('download-progress', (progress) => {
            log.log('Index Download Progress: ', progress)
            updateWindow.webContents.send('update-download-progress', progress)
        })
        autoUpdater.checkForUpdates()
    })
}

let currentSessionExpiration = null
let sessionRetryCount = 0
let oldServerTimestamp = null

function getServerTimestamp() {
    return oldServerTimestamp
}

function setServerTimestamp(d) {
    oldServerTimestamp = d
}

let retryTimeout = null

const http = require('http')
const https = require('https')
function startSessionHeartbeat(onBeat) {
    try {
        log.log('Performing session heartbeat...')
        const url = new URL(`${process.env.API_SQUADOV_URL}/auth/session/heartbeat`)
        const options = {
            method: 'POST',
            hostname: url.hostname,
            path: url.pathname,
            port: url.port,
            protocol: url.protocol,
            timeout: 7000,
            headers: {
                'Content-Type': 'application/json'
            }
        }

        const request = (options.protocol == 'https:') ? https.request(url, options) : http.request(options)
        const sentSessionId = process.env.SQUADOV_SESSION_ID
        request.write(JSON.stringify({
            sessionId: sentSessionId
        }))

        let retrySessionHeartbeat = (connError) => {
            let timeoutMs = Math.min(Math.pow(2, sessionRetryCount) + Math.random() * 1000, 15000)
            // We want to retry a few times up until the session is expired
            // to try and get a new session just in case the user's internet
            // is spotty (or their computer went to sleep or something). In the case
            // where we aren't able to obtain a valid session before the session
            // expires, we want to put the app into an error state that that keeps trying
            // to update the session but is other not functional.
            if (!!currentSessionExpiration && !connError) {
                // In the case where we're passed the expiration we need to restart and force the user to re-log.
                if (new Date() > currentSessionExpiration) {
                    logout()
                    return
                }
            } else {
                // This is the first session heartbeat or some connection error. Need to display some sort of loading error
                // screen so that the user isn't just greeted with nothing.
                if (!win.webContents.getURL().includes('sessionError.html')) {
                    win.loadFile('sessionError.html')
                }
            }

            // We don't want multiple different paths of retrying. Only a singular retry loop should ever happen at the same time.
            if (!!retryTimeout) {
                log.log('...Ignoring session heartbeat retry. One already exists.')
            } else {
                log.log(`\tRetrying heartbeat in ${timeoutMs}ms`)
                retryTimeout = setTimeout(() => {
                    log.log('...Retry Heartbeat')
                    retryTimeout = null
                    // We need to push onBeat forward just in case this is the first session heartbeat.
                    startSessionHeartbeat(onBeat, false)
                }, timeoutMs)
                sessionRetryCount += 1
            }
        }

        request.on('error', (err) => {
            log.log('Error in Sesssion Heartbeat: ', err)
            retrySessionHeartbeat(true)
        })

        request.on('abort', (err) => {
            log.log('Session Heartbeat Aborted: ', err)
            retrySessionHeartbeat(true)
        })

        request.on('timeout', () => {
            // Note that we don't want to abort the request JUST IN CASE the server
            // actually responds later. What we want to do in that case is to be smart
            // about whether the incoming session ID is newer than the one we have stored.
            log.log('Session Heartbeat Timeout!')
            retrySessionHeartbeat(true)
        })

        request.on('response', (resp) => {
            if (resp.statusCode == 401) {
                log.log('Sesssion Heartbeat Unauthorized: ', resp.statusCode, resp.statusMessage)
                logout()
            } else if (resp.statusCode != 200) {
                log.log('Bad response code: ', resp.statusCode)
                retrySessionHeartbeat(false)
            } else {
                let body = ''
                resp.on('data', (chunk) => {
                    body += chunk.toString()
                })
                
                resp.on('end', () => {
                    let respBody = JSON.parse(body)

                    // We assume that every time we create a new session we'll get an expiration date in the future.
                    // Thus, if we have two requests A and B that both send a session heartbeat request. The one that
                    // last hit the server (and is thus the more authoritative one) is the one with the latest expiration
                    // time. Therefore, we can ignore the older refresh request if it comes after a newer request.
                    let serverExpiration = new Date(respBody.expiration)
                    let serverTimestamp = new Date(respBody.serverTime)

                    let oldTimestamp = getServerTimestamp()
                    if (!!oldTimestamp && serverExpiration < oldTimestamp) {
                        log.log('Found old session refresh...ignoring.')
                        return
                    }
                    setServerTimestamp(serverTimestamp)

                    updateSession(respBody.sessionId, true)
                    sessionRetryCount = 0

                    if (!!onBeat) {
                        onBeat()
                    }

                    // Need to make sure we recover from the session error page when we successfully reconnect
                    // to the server.
                    if (!!win && !win.webContents.getURL().includes('index.html')) {
                        win.loadFile('index.html')
                    }
                    
                    currentSessionExpiration = new Date(Date.now() + respBody.expiresIn * 1000)

                    // Preemptively refresh the session 10 minutes before it's due to expire.
                    setTimeout(() => {
                        startSessionHeartbeat()
                    }, Math.max((respBody.expiresIn - 10 * 60) * 1000, 0))
                })
            }
        })
        request.end()
    } catch (e) {
        log.log('Failed to perform session heartbeat: ', e)
        logout()
    }
}

app.on('ready', async () => {
    await zeromqServer.start()
    zeromqServer.run()

    win = new BrowserWindow({
        width: 1600,
        height: 900,
        frame: false,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            webSecurity: app.isPackaged
        },
        icon: iconPath
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

    if (!app.commandLine.hasSwitch('hidden')) {
        win.show()
    }

    if (app.commandLine.hasSwitch('debug')) {
        process.env.SQUADOV_DEBUG = '1'
    }
    
    log.log('Loading Session...')
    if (!loadSession()) {
        // Needing to login overrides the hidden flag as we don't display the tray icon until much later.
        // Maybe that needs to change instead?
        win.show()

        // DO NOT GO ANY FURTHER UNTIL WE HAVE SUCCESSFULLY LOGGED IN.
        log.log('Start Login Flow...')
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

    win.hide()

    // Do setup flow - will get the user setup with the config file and a host of other things to ensure their settings
    // are good to go for using SquadOV.
    let setupWindow = new BrowserWindow({
        width: 300,
        height: 300,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
        },
        frame: false,
        resizable: false,
        movable: false,
        icon: iconPath
    })

    if (!app.isPackaged) {
        setupWindow.webContents.toggleDevTools()
    }
    setupWindow.loadFile('setup.html')
    setupWindow.show()

    log.log('Starting Setup Flow...')
    try {
        await setupFlow(setupWindow)
    } catch (ex) {
        log.log('User exited out of setup...')
        quit()
        return
    }

    // For simplicity, we only have the Electron app refresh the session ID instead of having everyone refreshing the session ID
    // themselves this way we can avoid race conditions where multiple people try to refresh the same session at the same time
    // and we end up with an invalid session ID.
    log.log('Start Session Heartbeat...')
    startSessionHeartbeat(async () => {
        if (!setupWindow.isDestroyed()) {
            setupWindow.close()
        }
        
        win.show()

        // Note that all this stuff should be in the session heartbeat callback because we don't really want to 
        // start the UI and the client service (2 things that require querying the API) until we have a valid session.
        // This is particularly relevant when we're loading the session from disk.
        start()

        if (!parseInt(process.env.SQUADOV_MANUAL_SERVICE)) {
            startClientService()
            await backendReady
            
            await requestOutputDevices()
            await requestInputDevices()
        }
    })
})

app.on('window-all-closed', () => {
    quit()
})

ipcMain.handle('reload-app-settings', async () => {
    loadAppSettings()
    await zeromqServer.reloadAppSettings()
    return true
})

ipcMain.handle('request-folder-size', async (event, folder) => {
    try {
        return {
            success: true,
            data: await zeromqServer.requestFolderSize(folder)
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.handle('change-recording-folder', async (event, {from, to}) => {
    try {
        return {
            success: true,
            data: await zeromqServer.changeRecordingFolder(from, to)
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.handle('cleanup-recording-folder', async (event, {loc, limit}) => {
    try {
        return {
            success: true,
            data: await zeromqServer.cleanupRecordingFolder(loc, limit)
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.on('request-vod-local-download', async (event, uuid) => {
    await zeromqServer.requestVodDownload(uuid)
})

ipcMain.handle('check-vod-local', async (event, uuid) => {
    try {
        return {
            success: true,
            data: await zeromqServer.checkForLocalVod(uuid)
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.handle('delete-vod-local', async (event, uuid) => {
    try {
        return {
            success: await zeromqServer.deleteLocalVod(uuid),
            data: null,
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.handle('request-user-folder-selection', async (event, defaultPath) => {
    let ret = await dialog.showOpenDialog({
        title: 'Select a Folder',
        defaultPath,
        properties: ['openDirectory', 'promptToCreate']
    })

    if (ret.canceled || ret.filePaths.length === 0) {
        return defaultPath
    } else {
        return ret.filePaths[0]
    }
})

ipcMain.handle('request-key-code-char', async (event, keyCode) => {
    try {
        return {
            success: true,
            data: await zeromqServer.requestKeyCodeChar(keyCode)
        }
    } catch(ex) {
        return {
            success: false,
            data: ex,
        }
    }
})

ipcMain.on('request-restore', () => {
    restore()
})

zeromqServer.on('vod-download-progress', (resp) => {
    let parsedResp = JSON.parse(resp)
    if (!!win) {
        win.webContents.send('vod-download-progress', parsedResp)
    }
})

zeromqServer.on('cloud-upload-progress', (resp) => {
    let parsedResp = JSON.parse(resp)
    if (!!win) {
        win.webContents.send('cloud-upload-progress', parsedResp)
    }
})

zeromqServer.on('preview-stream-status', (resp) => {
    let parsedResp = JSON.parse(resp)
    if (!!win) {
        win.webContents.send('preview-stream-status', parsedResp)
    }
})

let apiPort = null
zeromqServer.on('on-local-api-port', (resp) => {
    let port = JSON.parse(resp)
    apiPort = port
})

ipcMain.handle('request-local-api-port', (event) => {
    event.returnValue = apiPort
    return apiPort
})

ipcMain.on('minimize', (event) => {
    let window = BrowserWindow.fromWebContents(event.sender)
    window.minimize()
})

ipcMain.on('maximize', (event) => {
    let window = BrowserWindow.fromWebContents(event.sender)
    if (window.isMaximized()) {
        window.unmaximize()
    } else {
        window.maximize()
    }
})

ipcMain.on('closeWindow', (event) => {
    let window = BrowserWindow.fromWebContents(event.sender)
    if (event.sender.isDevToolsOpened()) {
        event.sender.closeDevTools()
    }
    
    window.close()
})

ipcMain.handle('start-record-preview', (event, game) => {
    let mediaRootPath = path.join(process.env.SQUADOV_USER_APP_FOLDER, 'HLS')

    if (fs.existsSync(mediaRootPath)) {
        fs.rmSync(mediaRootPath, {
            recursive: true,
            force: true,
        })
    }
    
    fs.mkdirSync(mediaRootPath)

    // Tell C++ to send the game recording to the appropriate path.
    zeromqServer.startGameRecordingStream(path.join(mediaRootPath, 'index.m3u8'), game)
})

ipcMain.on('stop-record-preview', (event) => {
    zeromqServer.stopGameRecordingStream()
})

ipcMain.on('reload-record-preview', (event) => {
    zeromqServer.reloadGameRecordingStream()
})

ipcMain.on('enable-record-preview-overlays', (event, enabled) => {
    zeromqServer.enablePreviewGameRecordingStream(enabled)
})