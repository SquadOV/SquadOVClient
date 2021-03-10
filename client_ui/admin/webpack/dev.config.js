const base = require('./base.config.js')
const { merge } = require('webpack-merge')
const webpack = require('webpack')

module.exports = (env, argv) => {
    return merge(base(env, argv, 'dev'), {
        mode: 'development',
        devtool: "source-map",
        plugins: [
        ],
    })
}