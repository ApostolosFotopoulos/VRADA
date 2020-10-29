import React, { useState, useEffect } from 'react';
import queryString from 'query-string';
import Loading from './Loading';
import WrongAP from './WrongAP';
import MainDisplay from './MainDisplay';
import '../static/css/App.css';

function App() {
  const [isLoading, setLoading] = useState(true);
  const [isConnectedToVrada, setIsConnectedToVrada] = useState(false);

  const checkTheWifi = () => {
    const url = queryString.parse(window.location.search);
    if (url.ssid.toUpperCase().includes('VRADA')) {
      return true;
    }
    return false;
  };

  useEffect(() => {
    if (checkTheWifi()) {
      setLoading(false);
      setIsConnectedToVrada(true);
    } else {
      setLoading(false);
      setIsConnectedToVrada(false);
    }
  }, []);

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
