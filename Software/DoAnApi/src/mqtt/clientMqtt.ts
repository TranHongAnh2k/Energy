import moment from 'moment';
import mqtt from 'mqtt';
import { firestore } from '../firebase/clientApp';
const host = 'driver.cloudmqtt.com';
const port = '18643';
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;
const connectUrl = `mqtt://${host}:${port}`
const client = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    username: 'cqbfckol',
    password: 'mpSkyZ4D1N6f',
    reconnectPeriod: 1000,
})
const listMacAdress: string[] = []
client.on('connect', async () => {
    try {
        console.log('Connected');
        const collectionSnapshot = await firestore.collection("mac_address").get();
        collectionSnapshot.forEach((snap) => {
            const obj = JSON.parse(JSON.stringify(snap.data()));
            listMacAdress.push(obj.value.toString())
            client.subscribe([obj.value], () => {
                console.log('Sub ' + obj.value)
            });
        })

    } catch (error) {
        console.log(error);
    }
})

client.on('message', (topic, payload) => {
    const isDevice = listMacAdress.find((mac: string) => mac === topic.toString())
    console.log(isDevice);

    if (isDevice) {
        environment(payload.toString(), topic.toString())
    }
})



async function environment(payload: string, mac: string) {
    try {
        const objPlayload = JSON.parse(payload);
        console.log(objPlayload);
        if (Number(objPlayload.current) < 0 || Number(objPlayload.voltage) < 0 || Number(objPlayload.energy) < 0) return
        const date = new Date()
        const month = date.getMonth()
        const res = await firestore.collection(mac).doc(month.toString())
        res.update({
            voltage: objPlayload.voltage.toFixed(3),
            current: objPlayload.current.toFixed(3),
            energy: objPlayload.energy.toFixed(3),
            month: month,
        });

        console.log("ESPs/environment/" + mac + " add done id doc:" + res.id);
    } catch (error) {
        console.log(error);

    }

}

export { client };
