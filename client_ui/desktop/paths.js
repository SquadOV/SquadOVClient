const {app} = require('electron')
const path = require('path')

module.exports.getAppDataFolder = function(appData) {
    return path.join(!!appData ? appData : app.getPath('appData'), 'SquadOV' + (!!process.env.SQUADOV_APPDATA_SUFFIX ? process.env.SQUADOV_APPDATA_SUFFIX : ''))
}