export default {
    target: 'static',
    buildModules: [
        '@nuxtjs/vuetify',
        '@nuxt/typescript-build',
    ],
    modules: [
        '@nuxtjs/gtm',
        '@nuxtjs/axios',
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
    },
    publicRuntimeConfig: {
        apiUrl: process.env.SQUADOV_API_URL || 'https://api.squadov.gg'
    }
}