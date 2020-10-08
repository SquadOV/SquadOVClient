const {app, BrowserWindow, ipcMain, session} = require('electron')
const path = require('path')
const fs = require('fs')
const {spawn} = require('child_process');
const log = require('./log.js')
const { dialog } = require('electron')
const { loginFlow } = require('./login.js')
const { createVerifyEncryptionPasswordFlow} = require('./password.js')

if (app.isPackaged) {
    const { autoUpdater } = require("electron-updater")
    autoUpdater.checkForUpdatesAndNotify()
}

let win

function start() {
    win.loadFile('index.html')
}

ipcMain.on('request-app-folder', (event) => {
    event.returnValue = process.env.SQUADOV_USER_APP_FOLDER
})

// Start a local API server that'll be used manage our connections to the
// database that holds all the information we want to retrieve.
const { ApiServer } = require('./api_src/api');
const { fixSchemaAst } = require('graphql-tools');

let apiServer = new ApiServer()
function quit() {
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

ipcMain.on('logout', () => {
    const sessionPath = getSessionPath()
    if (fs.existsSync(sessionPath)) {
        fs.unlinkSync(sessionPath)
    }
    app.relaunch()
    quit()    
})

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

ipcMain.on('obtain-session', (event, [session, userId]) => {
    process.env.SQUADOV_SESSION_ID = session
    process.env.SQUADOV_USER_ID = userId
    saveSession()
})

ipcMain.on('refresh-session', (event, session) => {
    process.env.SQUADOV_SESSION_ID = session
    saveSession()
})

ipcMain.on('request-session', (event) => {
    event.returnValue = {
        sessionId: process.env.SQUADOV_SESSION_ID,
        userId: process.env.SQUADOV_USER_ID,
    }
})

app.on('ready', async () => {
    win= new BrowserWindow({
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

    apiServer.start(() => {
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

        start()
    })
})