const base = require('./base.config.js')
const { merge } = require('webpack-merge')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const TerserPlugin = require('terser-webpack-plugin')
const webpack = require('webpack')
const path = require('path')

module.exports = (env, argv) => {
    return merge(base(env, argv, 'production'), {
        mode: 'production',
        optimization: {
            minimize: true,
            minimizer: [new TerserPlugin({
                parallel: true,
            })],
        },
        plugins: [
            new webpack.DefinePlugin({
                SQUADOV_API_URL: "'https://api.squadov.gg'",
                SQUADOV_SEGMENT_KEY: "'9CbNGTfu0j4UK0VwZvtb8jvyF3pz1wsk'",
                SQUADOV_GOOGLE_CLIENT_ID: "'989563532134-99nrfg2shd496mnma0g47ba37pondkb2.apps.googleusercontent.com'" //change to your own google app id
            })
        ],
    })
}