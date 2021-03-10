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
    const clientSrcDir = path.resolve(__dirname, '../ui')

    let entry = {
        app: [`${clientSrcDir}/ts/app.ts`, `${clientSrcDir}/sass/app.scss`]
    }

    let baseFilename = '[name].[contenthash]'
    let chunkBaseFilename = '[id].[chunkhash]'

    return {
        target: 'web',
        entry,
        output: {
            filename: `${baseFilename}.js`,
            chunkFilename: `${chunkBaseFilename}.js`,
            path:path.resolve(__dirname, '../dist'),
            publicPath: `/dist/`,
        },
        module: {
            rules: [
                {
                    test: /\.tsx?$/,
                    use: [
                        babelLoader,
                        {
                            loader: 'ts-loader',
                            options: {
                                appendTsSuffixTo: [/\.vue$/],
                                happyPackMode: true,
                            }
                        },
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
                        ...cssLoaders,
                        {
                            loader: 'sass-loader',
                            options: { 
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
                        ...cssLoaders,
                        {
                            loader: 'sass-loader',
                            options: { 
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
                filename: 'css.ejs',
                inject: false,
                template: path.join(__dirname, 'css.template.ejs'),
                minify: false,
            }),
            new HtmlWebpackPlugin({
                filename: 'js.ejs',
                inject: false,
                template: path.join(__dirname, 'js.template.ejs'),
                minify: false,
            }),
        ],
        resolve: {
            alias: {
                'vue$': 'vue/dist/vue.esm.js',
                '@client': path.resolve(__dirname, '../ui'),
            },
            extensions: ['.js', '.vue', '.scss', '.ts'],
        }
    }
}