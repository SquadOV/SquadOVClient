const path = require('path');
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const VueLoaderPlugin = require('vue-loader/lib/plugin')
const VuetifyLoaderPlugin = require('vuetify-loader/lib/plugin')
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin')
const HtmlWebpackPlugin = require('html-webpack-plugin')

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

module.exports = (env, argv, subfolder) => {
    const clientSrcDir = path.resolve(__dirname, '../')
    const target = env.target
    const isDesktop = (target === 'electron-renderer')

    let entry = {
        app: [`${clientSrcDir}/js/app.ts`, `${clientSrcDir}/sass/app.scss`]
    }

    if (isDesktop) {
        Object.assign(entry, {
            login: [`${clientSrcDir}/js/loginApp.ts`, `${clientSrcDir}/sass/app.scss`],
            password: [`${clientSrcDir}/js/passwordApp.ts`, `${clientSrcDir}/sass/app.scss`],
            update: [`${clientSrcDir}/js/updateApp.ts`, `${clientSrcDir}/sass/app.scss`],
            sessionError: [`${clientSrcDir}/js/sessionError.ts`, `${clientSrcDir}/sass/app.scss`],
        })
    }

    const opts = {
        DESKTOP: isDesktop
    }

    let baseFilename = isDesktop ? '[name]' : '[name].[contenthash]'
    let chunkBaseFilename = isDesktop ? '[id]' : '[id].[chunkhash]'

    return {
        target,
        entry,
        output: {
            filename: `${baseFilename}.js`,
            chunkFilename: `${chunkBaseFilename}.js`,
            path:path.resolve(__dirname, '../dist', subfolder, target),
            publicPath: `/dist/${subfolder}/${target}`,
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
                        },
                        {
                            loader: 'ifdef-loader',
                            options: opts
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
                    use: [
                        {
                            loader: 'vue-loader',
                            options: {
                                optimizeSSR: false
                            }
                        },
                        {
                            loader: 'ifdef-loader',
                            options: opts
                        }
                    ],
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
                filename: `${baseFilename}.css`,
                chunkFilename: `${chunkBaseFilename}.css`
            }),
            new ForkTsCheckerWebpackPlugin({
                typescript: {
                    extensions: {
                        vue: true
                    },
                },
            }),
            new HtmlWebpackPlugin({
                template: path.resolve(__dirname, 'loader.ejs'),
                minify: false,
                inject: false,
                filename: 'loader.tmpl'
            })
        ],
        resolve: {
            alias: {
                'vue$': 'vue/dist/vue.esm.js',
                '@client': path.resolve(__dirname, '../'),
            },
            extensions: ['.js', '.vue', '.scss', '.ts'],
        }
    }
}