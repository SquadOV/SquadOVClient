const { ipcMain } = require('electron')

module.exports.setupFlow =  function(win) {
    return new Promise((resolve, reject) => {
        let finish = false
        ipcMain.once('finish-setup', (event, arg) => {
            finish = true
            win.close()
            resolve()
        })

        win.on('close', () => {
            if (!finish) {
                reject()
            }
        })
    })
}