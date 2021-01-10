const base = require('./base.config.js')
const { merge } = require('webpack-merge')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const webpack = require('webpack')
const path = require('path')

module.exports = (env, argv) => {
    return merge(base(env, argv), {
        mode: 'development',
        devtool: "source-map",
        plugins: [
        ],
        output: {
            path: path.resolve(__dirname, '../dist'),
            publicPath: '/dist',
        }
    })
}