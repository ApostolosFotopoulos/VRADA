import React from 'react';
import Grid from '@material-ui/core/Grid';
import '../static/css/WrongAP.css';

function WrongAP() {
  return (
    <div className="warning-div">
      <div>
        <Grid container>
          <Grid item xs={12}>
            <h1>VRADA</h1>
            <h3>Please connect to the VRADA AP</h3>
          </Grid>
        </Grid>
      </div>
    </div>
  );
}
export default WrongAP;
