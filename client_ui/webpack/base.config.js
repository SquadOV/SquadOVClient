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

module.exports = (env, argv) => {
    let clientSrcDir = path.resolve(__dirname, '../client_src')
    return {
        target: 'electron-renderer',
        entry: {
            app: [`${clientSrcDir}/js/app.ts`, `${clientSrcDir}/sass/app.scss`],
            login: [`${clientSrcDir}/js/loginApp.ts`, `${clientSrcDir}/sass/app.scss`],
            update: [`${clientSrcDir}/js/updateApp.ts`, `${clientSrcDir}/sass/app.scss`],
        },
        output: {
            filename: '[name].js',
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
                    exclude: /node_modules/,
                },
                {
                    test: /\.js$/,
                    use: babelLoader,
                    exclude: /node_modules/,
                },
                {
                    test: /\.vue$/,
                    use: {
                        loader: 'vue-loader',
                        options: {
                            optimizeSSR: false
                        }
                    },
                    exclude: /node_modules/,
                },
                {
                    test: /\.css$/,
                    use: cssLoaders
                },
    
                {
                    test: /\.scss$/,
                    use: [
                        'vue-style-loader',
                        ...cssLoaders,
                        {
                            loader: 'sass-loader',
                            options: { 
                                sourceMap: true,
                                implementation: require('sass'),
                                sassOptions: {
                                    indentedSyntax: false
                                }
                            }
                        }
                    ]
                },
                {
                    test: /\.sass$/,
                    use: [
                        'vue-style-loader',
                        ...cssLoaders,
                        {
                            loader: 'sass-loader',
                            options: { 
                                implementation: require('sass'),
                                sassOptions: {
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
                '@client': path.resolve(__dirname, '../client_src'),
            },
            extensions: ['.js', '.vue', '.scss', '.ts'],
        }
    }
}