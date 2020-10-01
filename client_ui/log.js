const log = require('electron-log')
const path = require('path')
log.transports.file.resolvePath = (vars) => {
    return path.join(vars.appData, 'SquadOV', 'Logs', 'Client', vars.fileName)
}

module.exports = log