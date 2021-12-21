const express = require('express')
const app = express()
const port = 3000
const path = require('path')
const ejs = require('ejs')
const fs = require('fs')
const bent = require('bent')
const formurlencoded = require('form-urlencoded').default
const auth = require('./js/auth')
const api = require('./js/api')
const cookieParser = require('cookie-parser')

let configFile = process.argv[2]
let config =  fs.readFileSync(configFile , 'utf8')
let parsedConfig = JSON.parse(config)

let apiServer = new api.ApiServer(parsedConfig)

app.set('view engine', 'ejs')
app.use(cookieParser())
app.use(express.json())
app.get('/robots.txt', function (req, res) {
    res.type('text/plain');
    res.send("User-agent: *\nDisallow: /")
})

app.use('/dist', express.static(path.join(__dirname, 'dist')))
app.use('/assets', express.static(path.join(__dirname, 'assets')))
app.get('/healthz', function (request, response) {
    response.sendStatus(200)
})

app.get('/login', function(request, response) {
    redirectUri = `${parsedConfig.url}/oauth`
    response.redirect(`${parsedConfig.oauth.faUrl}/oauth2/authorize?client_id=${parsedConfig.oauth.clientId}&redirect_uri=${redirectUri}&response_type=code&tenantId=${parsedConfig.oauth.tenantId}&scope=openid`)
})

app.get('/oauth', async function(request, response) {
    // Exchange authorization code to ensure it was a proper login.
    tokenClient = bent(parsedConfig.oauth.faUrl, 'POST', 200, {
        'Content-Type': 'application/x-www-form-urlencoded'
    })
    
    body = formurlencoded({
        'client_id': parsedConfig.oauth.clientId,
        'client_secret': parsedConfig.oauth.clientSecret,
        'code': request.query.code,
        'grant_type': 'authorization_code',
        'redirect_uri': `${parsedConfig.url}/oauth`,
    })
    tokenClient('/oauth2/token', body).then(async (resp) => {
        // Set a cookie to remember the user for this session. Don't keep the cookie around,
        // next time just force the user to relog.
        const body = await resp.json()
        auth.storeAuthCookie(parsedConfig, body.access_token, response)

        response.redirect(`${parsedConfig.url}`)
    }).catch(async (err) => {
        response.sendStatus(403)
    })
})

let apiRouter = express.Router()
apiRouter.use(async function (request, response, next) {
    if (await auth.verifyAuthCookie(parsedConfig, request)) {        
        next()
    } else {
        response.sendStatus(403)
    }
})

apiRouter.get('/metrics/:metric', async function(request, response) {
    metric = parseInt(request.params.metric)
    interval = parseInt(request.query.interval)
    start = new Date(parseInt(request.query.start))
    end = new Date(parseInt(request.query.end))

    data = await apiServer.getMetrics(metric, interval, start, end, request.query)
    response.status(200).json(data)
})


apiRouter.get('/cohorts/:cohort', async function(request, response) {
    cohort = parseInt(request.params.cohort)
    interval = parseInt(request.query.interval)
    start = new Date(parseInt(request.query.start))
    end = new Date(parseInt(request.query.end))
    period = parseInt(request.query.period)
    length = parseInt(request.query.length)

    data = await apiServer.getCohort(cohort, interval, start, end, period, length)
    response.status(200).json(data)
})

apiRouter.get('/kpi/:kpi', async function(request, response) {
    kpi = parseInt(request.params.kpi)
    data = await apiServer.getKpi(kpi)
    response.status(200).json(data)
})

apiRouter.get('/referrals', async function (request, response) {
    data = await apiServer.getAvailableReferrals()
    response.status(200).json(data)
})

apiRouter.get('/referrals/:referral', async function(request, response) {
    referral = parseInt(request.params.referral)
    data = await apiServer.getReferralBreakdown(referral)
    response.status(200).json(data)
})

apiRouter.post('/campaign', async function(request, response) {
    code = request.body.code
    desc = request.body.desc
    await apiServer.createCampaign(code, desc)
    response.status(204).send()
})

apiRouter.get('/funnel', async function (request, response) {
    start = new Date(parseInt(request.query.start))
    end = new Date(parseInt(request.query.end))
    codes = JSON.parse(request.query.codes)
    organicOnly = !!parseInt(request.query.organicOnly)
    verifiedOnly = !!parseInt(request.query.verifiedOnly)

    data = await apiServer.getFunnelData(start, end, codes, verifiedOnly)
    response.status(200).json(data)
})

apiRouter.get('/pucurve', async function (request, response) {
    start = new Date(parseInt(request.query.start))
    end = new Date(parseInt(request.query.end))

    data = await apiServer.getPowerUserCurve(start, end)
    response.status(200).json(data)
})

apiRouter.get('/search/user', async function (request, response) {
    search = request.query.search
    data = await apiServer.searchForUser(search)
    response.status(200).json(data)
})

apiRouter.get('/search/vod', async function (request, response) {
    video = request.query.video.trim()
    user = request.query.user.trim()
    data = await apiServer.searchForVod(video, user)
    response.status(200).json(data)
})

apiRouter.get('/search/game/:game', async function (request, response) {
    game = request.params.game
    data = undefined
    if (game == 'aimlab') {
        data = await apiServer.searchAimLab(request.query)
    } else if (game == 'csgo') {
        data = await apiServer.searchCsgo(request.query)
    } else if (game == 'hearthstone') {
        data = await apiServer.searchHearthstone(request.query)
    } else if (game == 'lol') {
        data = await apiServer.searchLol(request.query)
    } else if (game == 'tft') {
        data = await apiServer.searchTft(request.query)
    } else if (game == 'valorant') {
        data = await apiServer.searchValorant(request.query)
    } else if (game == 'wow') {
        data = await apiServer.searchWow(request.query)
    }
    response.status(200).json(data)
})


app.use('/api', apiRouter)
app.get('*', async function (request, response) {
    // No auth cookie? Go to login page otherwise grant access to the SPA.
    if (await auth.verifyAuthCookie(parsedConfig, request)) {
        response.render('index')
    } else {
        response.redirect(`${parsedConfig.url}/login`)
    }
})


app.listen(port, () => {
    console.log(`SquadOV Admin listening at http://localhost:${port}`)
})