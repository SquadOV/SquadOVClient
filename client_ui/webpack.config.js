const path = require('path');
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const VueLoaderPlugin = require('vue-loader/lib/plugin')
const VuetifyLoaderPlugin = require('vuetify-loader/lib/plugin')
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin')

const cssLoaders = [
    {
        loader: MiniCssExtractPlugin.loader
    },
    'css-loader'
]

const babelLoader = {
    loader: 'babel-loader',
    options: {
        presets: ['@babel/preset-env'],
        plugins: [
            ['@babel/plugin-proposal-decorators', {'legacy': true}]
        ]
    }
}

let config = {
    target: 'electron-renderer',
    mode: 'development',
    entry: {
        app: ['./client_src/js/app.ts', './client_src/sass/app.scss'],
        password: ['./client_src/js/passwordApp.ts', './client_src/sass/app.scss'],
    },
    output: {
        filename: '[name].js',
        path: path.resolve(__dirname, 'dist'),
        publicPath: '/dist',
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: [
                    "cache-loader",
                    "thread-loader",
                    babelLoader,
                    {
                        loader: 'ts-loader',
                        options: {
                            appendTsSuffixTo: [/\.vue$/],
                            happyPackMode: true,
                        }
                    }
                ],
            },
            {
                test: /\.js$/,
                use: babelLoader,
            },
            {
                test: /\.vue$/,
                use: {
                    loader: 'vue-loader',
                    options: {
                        optimizeSSR: false
                    }
                }
            },
            {
                test: /\.css$/,
                use: cssLoaders
            },

            {
                test: /\.scss$/,
                use: [
                    ...cssLoaders,
                    {
                        loader: 'sass-loader',
                        options: { 
                            sourceMap: true,
                            implementation: require('sass'),
                            sassOptions: {
                                fiber: require('fibers'),
                                indentedSyntax: false
                            }
                        }
                    }
                ]
            },
            {
                test: /\.sass$/,
                use: [
                    ...cssLoaders,
                    {
                        loader: 'sass-loader',
                        options: { 
                            sourceMap: true,
                            implementation: require('sass'),
                            sassOptions: {
                                fiber: require('fibers'),
                                indentedSyntax: true
                            }
                        }
                    }
                ]
            },
            {
                test: /\.(ttf|eot|svg|woff|woff2|png)?$/,
                use: 'url-loader',
            },
        ]
    },
    plugins: [
        new VueLoaderPlugin(),
        new VuetifyLoaderPlugin(),
        new MiniCssExtractPlugin({
            filename: '[name].css'
        }),
        new ForkTsCheckerWebpackPlugin({
            typescript: {
                extensions: {
                    vue: true
                },
            },
        }),
    ],
    resolve: {
        alias: {
            'vue$': 'vue/dist/vue.esm.js',
            '@client': path.resolve(__dirname, 'client_src'),
        },
        extensions: ['.js', '.vue', '.scss', '.ts'],
    }
}

module.exports = (env, argv) => {
    if (argv.mode === 'production') {
        config.mode = 'production'
        config.output.path = path.resolve(__dirname, 'dist', 'production')
        config.output.publicPath = '/dist/production'
    }
    return config
}