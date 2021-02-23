export default {
    target: 'static',
    buildModules: [
        '@nuxtjs/vuetify',
        '@nuxt/typescript-build',
    ],
    components: true,
    modules: [
        '@nuxtjs/gtm',
        'cookie-universal-nuxt'
    ],
    plugins: [
        '~/plugins/gtm'
    ],
    gtm: {
        enabled: true,
        autoInit: false
    },
    build: {
        babel: {
            plugins: [
                [
                    '@babel/plugin-proposal-decorators',
                    {
                        "legacy": true
                    }
                ],
                '@babel/plugin-proposal-class-properties',
            ]
        }
    },
    vuetify: {
        theme: {
            dark: true,
        }
    },
    generate: {
        dir: 'docs'
    }
}