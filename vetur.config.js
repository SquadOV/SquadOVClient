// vetur.config.js
/** @type {import('vls').VeturConfig} */
module.exports = {
    // **optional** default: `{}`
    // override vscode settings
    // Notice: It only affects the settings used by Vetur.
    settings: {
      "vetur.useWorkspaceDependencies": true,
      "vetur.experimental.templateInterpolationService": true
    },
    // support monorepos
    projects: [
      './client_ui/common_web',
      './client_ui/landing',
      './client_ui/support',
      './client_ui/admin'
    ]
  }
  