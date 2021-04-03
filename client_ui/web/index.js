const express = require('express')
const app = express()
const port = 3000
const path = require('path')
const ejs = require('ejs')
const fs = require('fs')
const bent = require('bent')
app.set('view engine', 'ejs');
app.use('/dist', express.static(__dirname + '/dist'))
app.use('/assets', express.static(__dirname + '/assets'))

let configFile = process.argv[2]
let config =  fs.readFileSync(configFile , 'utf8')
let parsedConfig = JSON.parse(config)

app.get('/healthz', function (request, response) {
    response.sendStatus(200)
})

app.get('*', async function (request, response) {
    let apiUrl = parsedConfig.apiUrl
    let selfUrl = `${request.protocol}://${request.hostname}${parsedConfig.usePort ? `:${port}` : ''}${request.originalUrl}`
    let hasOEmbed = false
    let hasTwitterCard = false
    let hasOpenGraph = false
    let meta = {}

    const url = new URL(selfUrl)
    let expectingShare = false
    for (let p of url.pathname.split('/')) {
        if (p === 'share') {
            expectingShare = true
        } else if (expectingShare) {
            hasOEmbed = true
            hasTwitterCard = true
            hasOpenGraph = true

            try {
                const resp = await bent(apiUrl, 'GET', 200)(`/meta?share=${p}`)
                meta = await resp.json()
            } catch (e) {
                console.log('Failed to get share meta for: ', p)
                hasTwitterCard = false
                hasOpenGraph = false
            }
            break
        }
    }

    response.render('index', {
        oembedUrl: `${apiUrl}/oembed?url=${encodeURIComponent(selfUrl)}&format=json`,
        selfUrl,
        hasOEmbed,
        hasTwitterCard,
        hasOpenGraph,
        ...meta,
    })
})

app.listen(port, () => {
    console.log(`SquadOVWebApp listening at http://localhost:${port}`)
})