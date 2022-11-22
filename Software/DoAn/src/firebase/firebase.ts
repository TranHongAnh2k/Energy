import app from 'firebase/app';
import 'firebase/auth';
import 'firebase/firebase-firestore';
import FirebaseConfig from './firebaseConfig';

class Firebase {
	private db: app.firestore.Firestore;
	static instance: Firebase;

	constructor() {
		app.initializeApp(FirebaseConfig);
		this.db = app.firestore();
		console.log('firebase initializeApp');
	}

	async getData(mac:string) {
		const collectionSnapshot = await this.db.collection(mac).get()
		const db:any = []
		collectionSnapshot.forEach((snap) => {
            const obj = JSON.parse(JSON.stringify(snap.data()));
			db.push(obj)
        })
		return db
	}
	async getMacAddress() {
		const collectionSnapshot = await this.db.collection('mac_address').get()
		const db:any = []
		collectionSnapshot.forEach((snap) => {
            const obj = JSON.parse(JSON.stringify(snap.data()));
			db.push(obj)
        })
		return db
	}

}
export default function getInstanceFirebase() {
	if (!Firebase.instance) {
		Firebase.instance = new Firebase();
	}
	return Firebase.instance;
}
