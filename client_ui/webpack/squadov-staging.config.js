const base = require('./base.config.js')
const { merge } = require('webpack-merge')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const TerserPlugin = require('terser-webpack-plugin')
const webpack = require('webpack')
const path = require('path')

module.exports = (env, argv) => {
    return merge(base(env, argv), {
        mode: 'production',
        optimization: {
            minimize: true,
            minimizer: [new TerserPlugin({
                parallel: true,
            })],
        },  
        plugins: [
            new webpack.DefinePlugin({
                'API_URL': '"http://api.staging.squadov.gg"',
            })
        ],
        output: {
            path:path.resolve(__dirname, '../dist', 'production'),
            publicPath: '/dist/production',
        }
    })
}