import { cert, initializeApp } from 'firebase-admin/app';
import { getFirestore } from 'firebase-admin/firestore';
var serviceAccount = require('./service-account-file.json');

initializeApp({
    credential: cert(serviceAccount),
    databaseURL: "https://doantn-b19c6-default-rtdb.firebaseio.com/"
});

const firestore = getFirestore();

export { firestore };