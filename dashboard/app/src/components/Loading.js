import React from 'react';
import Grid from '@material-ui/core/Grid';
import CircularProgress from '@material-ui/core/CircularProgress';
import '../static/css/Loading.css';

function Loading() {
  return (
    <div className="loading-div">
      <div>
        <Grid container>
          <Grid item xs={12}>
            <CircularProgress />
          </Grid>
        </Grid>
      </div>
    </div>
  );
}

export default Loading;
