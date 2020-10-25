import React from 'react';
import Grid from '@material-ui/core/Grid';
import Button from '@material-ui/core/Button';
import PlayIcon from '@material-ui/icons/PlayArrow';
import PauseIcon from '@material-ui/icons/Pause';
import PropTypes from 'prop-types';

function ControlButtons({ startWrite, stopWrite, isPlaying }) {
  return (
    <div>
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
        </Grid>
      </Grid>
    </div>
  );
}

ControlButtons.propTypes = {
  startWrite: PropTypes.func.isRequired,
  stopWrite: PropTypes.func.isRequired,
  isPlaying: PropTypes.bool.isRequired,
};

export default ControlButtons;
