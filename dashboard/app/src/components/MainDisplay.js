import React, { useState, useEffect } from 'react';
import Grid from '@material-ui/core/Grid';
import ControlButtons from './ControlButtons';
import Box from './Box';
import preferences from '../static/preferences.json';

const { ipcRenderer } = window.require('electron');

function MainDisplay() {
  const [isPlaying, setIsPlaying] = useState(false);
  const [packetData, setPacketData] = useState([0, 0, 0, 0, 0, 0, 0]);
  const [details] = useState(preferences);
  const [macAddress, setMacAddress] = useState('00:0c:bf:26:c1:1d');
  const [keepTrack, setKeepTrack] = useState(null);
  const [initialRender, setInitialRender] = useState(true);

  const startWrite = () => {
    ipcRenderer.send('START_WRITE');
    setIsPlaying(true);
  };

  const stopWrite = () => {
    ipcRenderer.send('STOP_WRITE');
    setIsPlaying(false);
  };

  const changeMacAddress = (e) => {
    clearInterval(keepTrack);
    setMacAddress(e.target.value);
  };

  useEffect(() => {
    setKeepTrack(setInterval(() => {
      ipcRenderer.send('FETCH_PACKET_DATA', macAddress);
    }, 100, ipcRenderer, macAddress));
    if (initialRender) {
      ipcRenderer.on('PACKET_DATA', (e, d) => {
        setPacketData(d);
      });
      setInitialRender(false);
    }
  }, [macAddress]);

  const renderBoxes = () => packetData.map((e, idx) => (
    <Grid item xs={4} key={`grid-item-${details[idx].label}`}>
      <Box
        background={details[idx].background}
        color={details[idx].color}
        value={e}
        label={details[idx].label}
      />
    </Grid>
  ));
  return (
    <div style={{ marginTop: '2%' }}>
      <ControlButtons
        startWrite={startWrite}
        stopWrite={stopWrite}
        isPlaying={isPlaying}
        macAddress={macAddress}
        changeMacAddress={changeMacAddress}
      />
      <Grid container spacing={4} style={{ marginTop: '2%', padding: '2%' }}>
        {renderBoxes()}
      </Grid>
    </div>
  );
}
export default MainDisplay;
