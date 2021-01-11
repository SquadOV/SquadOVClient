const express = require('express')
const app = express()
const port = 3000
const path = require('path')
const ejs = require('ejs')
app.set('view engine', 'ejs');
app.use('/dist', express.static(__dirname + '/dist'))

app.get('*', function (request, response) {
  response.render('index')
});

app.listen(port, () => {
  console.log(`SquadOVWebApp listening at http://localhost:${port}`)
})