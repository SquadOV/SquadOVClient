const express = require('express')
const app = express()
const port = 3000
const path = require('path')
const ejs = require('ejs')
const fs = require('fs')
app.set('view engine', 'ejs');
app.use('/dist', express.static(__dirname + '/dist'))
app.use('/assets', express.static(__dirname + '/assets'))

let configFile = process.argv[2]
let config =  fs.readFileSync(configFile , 'utf8')
let parsedConfig = JSON.parse(config)

app.get('/healthz', function (request, response) {
    response.sendStatus(200)
})

app.get('*', function (request, response) {
    let apiUrl = parsedConfig.apiUrl
    let oembedUrl = `${request.protocol}://${request.hostname}${parsedConfig.usePort ? `:${port}` : ''}${request.originalUrl}`
    let hasOEmbed = false

    const url = new URL(oembedUrl)
    let expectingShare = false
    for (let p of url.pathname.split('/')) {
        if (p === 'share') {
            expectingShare = true
        } else if (expectingShare) {
            hasOEmbed = true
            break
        }
    }

    response.render('index', {
        apiUrl,
        oembedUrl: encodeURIComponent(oembedUrl),
        hasOEmbed,
    })
})

app.listen(port, () => {
    console.log(`SquadOVWebApp listening at http://localhost:${port}`)
})