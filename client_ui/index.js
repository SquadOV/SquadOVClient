const {app, BrowserWindow} = require('electron')
const path = require('path')
const fs = require('fs')
const {spawn} = require('child_process');
const log = require('./log.js')
const { dialog } = require('electron')
const { createVerifyEncryptionPasswordFlow } = require('./password.js')

if (app.isPackaged) {
    const { autoUpdater } = require("electron-updater")
    autoUpdater.checkForUpdatesAndNotify()
}

let win

function start() {
    win.loadFile('index.html')
}

// Start a local API server that'll be used manage our connections to the
// database that holds all the information we want to retrieve.
const { ApiServer } = require('./api_src/api')

let apiServer = new ApiServer()
function quit() {
    apiServer.close()
    app.quit()
}

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

    // DO NOT GO ANY FURTHER UNTIL WE HAVE A PASSWORD
    // SECURE ENVIRONMENT SETUP.
    await createVerifyEncryptionPasswordFlow(win)

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