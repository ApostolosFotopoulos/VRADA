import React from 'react';
import Grid from '@material-ui/core/Grid';
import Button from '@material-ui/core/Button';
import PlayIcon from '@material-ui/icons/PlayArrow';
import PauseIcon from '@material-ui/icons/Pause';
import PropTypes from 'prop-types';
import Select from '@material-ui/core/Select';
import MenuItem from '@material-ui/core/MenuItem';

function ControlButtons({
  startWrite, stopWrite, isPlaying, macAddress, changeMacAddress,
}) {
  return (
    <Grid container>
      <Grid item xs={12} style={{ textAlign: 'center' }}>
        <Button
          variant="contained"
          style={{ margin: 2, background: isPlaying ? 'transparent' : '#2ea44f' }}
          onClick={startWrite}
          disabled={isPlaying}
        >
          <PlayIcon style={{ color: '#fff' }} />
        </Button>
        <Button
          variant="contained"
          style={{ margin: 2, background: isPlaying ? '#d73a49' : 'transparent' }}
          onClick={stopWrite}
          disabled={!isPlaying}
        >
          <PauseIcon style={{ color: '#fff' }} />
        </Button>
        <Select
          labelId="demo-controlled-open-select-label"
          id="demo-controlled-open-select"
          value={macAddress}
          style={{ color: '#fff', margin: 2, marginLeft: 5 }}
          disabled={isPlaying}
          onChange={changeMacAddress}
        >
          <MenuItem value="00:0c:bf:26:c1:1d">00:0c:bf:26:c1:1d</MenuItem>
          <MenuItem value="00:0c:bf:26:c1:1e">00:0c:bf:26:c1:1e</MenuItem>
        </Select>
      </Grid>
    </Grid>
  );
}

ControlButtons.propTypes = {
  startWrite: PropTypes.func.isRequired,
  stopWrite: PropTypes.func.isRequired,
  isPlaying: PropTypes.bool.isRequired,
  macAddress: PropTypes.string.isRequired,
  changeMacAddress: PropTypes.func.isRequired,
};

export default ControlButtons;
