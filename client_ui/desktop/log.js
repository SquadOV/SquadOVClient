const log = require('electron-log')
const path = require('path')
const { getAppDataFolder } = require('./paths.js')

log.transports.file.resolvePath = (vars) => {
    return path.join(getAppDataFolder(vars.appData), 'Logs', 'Client', vars.fileName)
}

module.exports = log