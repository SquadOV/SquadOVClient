const path = require('path')
const fs = require('fs-extra')

const iconUrl = path.join(__dirname, 'assets', 'icon.ico')

const srcRelease = path.join(__dirname, '..', '..', 'build', 'bin', 'x64', 'Release')
const srcConfig = path.join(__dirname, 'config')

const dstRelease = path.join(__dirname, '..', '..', 'resources', 'service')
const dstConfig = path.join(__dirname, '..', '..', 'resources', 'config')

fs.rmdirSync(dstRelease, {
    recursive: true
})
fs.rmdirSync(dstConfig, {
    recursive: true
})

fs.copySync(srcRelease, dstRelease, {
    filter: (src, dst) => {
        return !/.*\.pdb$/.test(src)
    }
})
fs.copySync(srcConfig, dstConfig, {
    filter: (src, dst) => {
        return !/config\/(?!squadov\.json)/.test(src)
    }
})

module.exports = {
    packagerConfig: {
        appCopyright: 'Copyright © GRCHive, Inc. 2020-2021',
        arch: ['x64'],
        asar: true,
        executableName: 'SquadOV',
        extraResource: [
            path.join(__dirname, '..', '..', 'resources', 'tzdata'),
            dstRelease,
            dstConfig
        ],
        ignore: [
            'node_modules/zeromq/build/libzmq',
            'node_modules/zeromq/build/Release/zeromq.iobj',
            'node_modules/zeromq/build/Release/zeromq.pdb',
        ],
        icon: iconUrl,
        name: 'SquadOV',
        platform: ['win32'],
        win32metadata: {
            CompanyName: 'GRCHive, Inc.',
            FileDescription: 'SquadOV. Better VODs, better match details.',
            InternalName: 'SquadOV',
            ProductName: 'SquadOV'
        }
    },
    makers: [
        {
            name: '@electron-forge/maker-squirrel',
            config: {
                certificateFile: process.env.CSC_LINK,
                certificatePassword: process.env.CSC_KEY_PASSWORD,
                authors: 'GRCHive, Inc.',
                description: 'SquadOV. Better VODs, better match details.',
                exe: 'SquadOV.exe',
                iconUrl:  iconUrl,
                name: 'squadov.squadov.app',
                setupExe: 'SquadOV_Setup.exe',
//                remoteReleases: 'https://us-central1.content.squadov.gg/builds',
                setupIcon:  iconUrl
            }
        }
    ]
}