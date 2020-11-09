import React, { useState, useEffect } from 'react';
import Loading from './Loading';
import WrongAP from './WrongAP';
import MainDisplay from './MainDisplay';
import '../static/css/App.css';

const { ipcRenderer } = window.require('electron');

function App() {
  const [isLoading, setLoading] = useState(true);
  const [isConnectedToVrada, setIsConnectedToVrada] = useState(false);

  useEffect(() => {
    setInterval(() => {
      ipcRenderer.send('FETCH_WIFI_STATUS');
    }, 1000, ipcRenderer);
    ipcRenderer.on('WIFI_STATUS', (e, d) => {
      console.log(d)
      if ((d && d.includes('VRADA')) || (d && d.includes('ETHERNET_TYPE'))) {
        setLoading(false);
        setIsConnectedToVrada(true);
      } else {
        setLoading(false);
        setIsConnectedToVrada(false);
      }
    });
  });

  return (
    <div className="App">
      {
        isLoading
          ? <Loading isLoading={isLoading} />
          : (
            <div>
              {
              isConnectedToVrada
                ? <MainDisplay />
                : <WrongAP />
            }
            </div>
          )
      }
    </div>
  );
}

export default App;
