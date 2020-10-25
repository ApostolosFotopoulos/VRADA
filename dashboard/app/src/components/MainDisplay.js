import React, { useState } from 'react';
import ControlButtons from './ControlButtons';

const { ipcRenderer } = window.require('electron');

function MainDisplay() {
  const [isPlaying, setIsPlaying] = useState(false);
  const startWrite = () => {
    ipcRenderer.send('START_WRITE');
    setIsPlaying(true);
  };
  const stopWrite = () => {
    ipcRenderer.send('STOP_WRITE');
    setIsPlaying(false);
  };
  return (
    <div style={{ marginTop: '2%' }}>
      <ControlButtons
        startWrite={startWrite}
        stopWrite={stopWrite}
        isPlaying={isPlaying}
      />
    </div>
  );
}
export default MainDisplay;
