const mqtt = require('mqtt');
const express = require('express');
const app = express();
const request = require('request');
const path = require('path');

app.use(express.static('public'));

const options = {
    host: 'mqtt://m13.cloudmqtt.com',
    username: 'tgrisrwn',
    password: 'GCLhF8pDVQsd',
    clientId: 'webapp',
    port: 13015,
    keepalive: 60,
    reconnectPeriod: 500,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    encoding: 'utf8'
};

const client = mqtt.connect('mqtt://m13.cloudmqtt.com', options);

let leituraSensores = {
    dht_u: 'off',
    dht_t: 'off',
    luz: 'off',
    solo: 'off',
};

let notificacoes = {
    dht_u: false,
    dht_t: false,
    luz: false,
    solo: false
};

let notificacoesLock = {
    dht_u: true,
    dht_t: true,
    luz: true,
    solo: true
};

client.on('connect', () => {
    console.log('Conectado ao servidor MQTT');
    client.subscribe('SENSORS');
});
 
client.on('message', (topic, message) => {
    let data = message.toString().split('_');
    leituraSensores.dht_u = Number(data[0]) || 'off';
    leituraSensores.dht_t = Number(data[1]) || 'off';
    if (!Number(data[2])) {
        leituraSensores.luz = 'off';
    } else if (data[2] < 341) {
        leituraSensores.luz = 'baixa';
    } else if (data[2] < 642) {
        leituraSensores.luz = 'média';
    } else {
        leituraSensores.luz = 'alta';
    }

    if (!Number(data[3])) {
        leituraSensores.solo = 'off';
    } else if (data[3] < 341) {
        leituraSensores.solo = 'alta';
    } else if (data[3] < 642) {
        leituraSensores.solo = 'média';
    } else {
        leituraSensores.solo = 'baixa';
    }
    if (notificacoes.dht_t && !notificacoesLock.dht_t && Number(leituraSensores.dht_t) >= Number(notificacoes.dht_t)) {
        sendPush('Temperatura', `A temperatura ao redor de sua planta está acima de ${notificacoes.dht_t} °C`);
        notificacoesLock.dht_t = true;
    }
    if (notificacoes.dht_u && !notificacoesLock.dht_u && Number(leituraSensores.dht_u) >= Number(notificacoes.dht_u)) {
        sendPush('Umidade do ar', `A umidade do ar ao redor de sua planta está acima de ${notificacoes.dht_u} %`);
        notificacoesLock.dht_u = true;
    }
    if (notificacoes.luz && !notificacoesLock.luz && leituraSensores.luz == notificacoes.luz) {
        sendPush('Luminosidade', `Sua planta está recebendo luminosidade ${leituraSensores.luz}`);
        notificacoesLock.luz = true;
    }
    if (notificacoes.solo && !notificacoesLock.solo && leituraSensores.solo == notificacoes.solo) {
        sendPush('Umidade do solo', `O solo da sua planta está com umidade ${leituraSensores.solo}`);
        notificacoesLock.solo = true;
    }
    console.log('Sensores atualizados', leituraSensores);
});

app.get('/:sensor', (req, res) => {
    let value = leituraSensores[req.params.sensor] || 'off';
    res.send(value.toString());
});

app.get('/TOGGLE/:sensor/:value', (req, res) => {
    switch(req.params.sensor) {
        case 'dht_t':
            client.publish('TOGGLE', `pin2_${(req.params.value == "true") ? 'on' : 'off'}`);
            break;
        case 'dht_u':
            client.publish('TOGGLE', `pin1_${(req.params.value == "true") ? 'on' : 'off'}`);
            break;
        case 'luz':
            client.publish('TOGGLE', `pin3_${(req.params.value == "true") ? 'on' : 'off'}`);
            break;
        case 'solo':
            client.publish('TOGGLE', `pin4_${(req.params.value == "true") ? 'on' : 'off'}`);
            break;
    }
    res.sendStatus(200);
});

// A cada 10 min desbloqueia as notificações
setInterval(function() {
    notificacoesLock.dht_u = false;
    notificacoesLock.dht_t = false;
    notificacoesLock.luz = false;
    notificacoesLock.solo = false;
}, 600000);

function sendPush(title, message) {
    request({
        url: 'https://api.pushbullet.com/v2/pushes',
        method: 'POST',
        headers: {
          'Access-Token': process.env.ACCESS_TOKEN,
          'Content-Type': 'application/json'
        },
        json: {
          'title': title,
          'body': message,
          'type': 'note',
        }
    });
} 

app.get('/notify/:sensor/:option', (req, res) => {
    if(req.params.option) {
        notificacoes[req.params.sensor] = req.params.option;
        notificacoesLock[req.params.sensor] = false;
    } else {
        notificacoes[req.params.sensor] = false;
        notificacoesLock[req.params.sensor] = true;
    }
    res.sendStatus(200);
});


app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'view.html'));
});

app.listen(process.env.PORT || 3000, function() {
    console.log('Servidor iniciado');
});