const base = require('./base.config.js')
const { merge } = require('webpack-merge')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const webpack = require('webpack')
const path = require('path')

module.exports = (env, argv) => {
    return merge(base(env, argv, 'dev'), {
        mode: 'development',
        devtool: "source-map",
        plugins: [
            new webpack.DefinePlugin({
                SQUADOV_API_URL: "'http://127.0.0.1:8080'"
            })
        ],
    })
}