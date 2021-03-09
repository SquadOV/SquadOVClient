const express = require('express')
const app = express()
const port = 3000
const path = require('path')
const ejs = require('ejs')
app.set('view engine', 'ejs')
app.get('/robots.txt', function (req, res) {
    res.type('text/plain');
    res.send("User-agent: *\nDisallow: /")
})

app.use('/dist', express.static(path.join(__dirname, 'dist')))
app.use('/assets', express.static(path.join(__dirname, 'assets')))
app.get('/healthz', function (request, response) {
    response.sendStatus(200)
})

app.get('*', function (request, response) {
    response.render('index')
})

app.listen(port, () => {
    console.log(`SquadOV Admin listening at http://localhost:${port}`)
})