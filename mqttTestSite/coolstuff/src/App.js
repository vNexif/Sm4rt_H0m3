import React, {useEffect, useState} from 'react';

let mqtt = require('mqtt');
let options = {
    protocol: 'mqtts',
    // clientId uniquely identifies client
    // choose any string you wish
    clientId: 'b0908853'
};
let client = mqtt.connect('mqtt://10.5.2.3:9001');

client.subscribe('painlessMesh/from/#');

function App() {

    // Sets default React state
    const [data, setData] = useState({});
    const [loading, setLoading] = useState(true);

    // useEffect(() => {
    //     document.title = "bink"
    // }, [])


    let note;
    client.on('message', function (topic, message) {
        try {

            note = JSON.parse(message);

            setData(note);
            //THIS SHIT IS NOT NEEDEED BUI
            // client.end();
        } catch (e) {
            console.log(e);
        } finally {
            setLoading(false);
        }
    });

    function ON_ONE() {
        try {
            client.publish("painlessMesh/to/476452757", "ON1");
        } catch (error) {
            console.table(error);
        }
    }

    function OFF_ONE() {
        client.publish("painlessMesh/to/476452757", "OFF1");
    }

    function ON_TWO() {
        client.publish("painlessMesh/to/476452757", "ON2");
    }

    function OFF_TWO() {
        client.publish("painlessMesh/to/476452757", "OFF2");
    }

    function Get_DHT() {
        client.publish("painlessMesh/to/476452757", "getDHT");
    }

    return (

        <div className="App">
            <header className="App-header">
                {
                    !loading ? [data].map((e, i) => (
                            <div key={i}>
                                <p>Temperature: {e.temperature}</p>
                                <p>Humidity: {e.humidity}</p>
                                <p>Heat Index: {e.heatIndex}</p>
                            </div>
                        )) :
                        <p>LOADING...</p>
                }

                <button onClick={ON_ONE}>
                    Turn ON1
                </button>

                <button onClick={OFF_ONE}>
                    Turn OFF1
                </button>

                <button onClick={ON_TWO}>
                    Turn ON2
                </button>

                <button onClick={OFF_TWO}>
                    Turn OFF2
                </button>

                <button onClick={Get_DHT}>
                    GetDHT
                </button>

            </header>
        </div>
    );
}

export default App;