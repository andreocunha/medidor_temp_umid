// server.js
const app = require('express')()
const server = require('http').createServer(app)
const { send } = require('process');
const WebSocket = require('ws')
var port = process.env.PORT || 4000;

const wss = new WebSocket.Server({ server })

let dados = '0,0,Segunda-00:00'

wss.on('connection', ws => {
    console.log('Nova conexão')
    ws.send(dados);

    ws.on('message', message => {
        dados = message
    })

    // every 100ms examine the socket and send more data
    // only if all the existing data was sent out
    setInterval(() => {
        if (ws.bufferedAmount == 0) {
        ws.send(dados);
        }
    }, 100);

})

app.get('/', (req, res) => res.json({ info: dados }))

server.listen(port, function () {
    console.log('Rodando em localhost:4000')
})
  