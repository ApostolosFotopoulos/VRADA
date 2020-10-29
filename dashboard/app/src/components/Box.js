import React from 'react';
import Card from '@material-ui/core/Card';
import CardContent from '@material-ui/core/CardContent';
import Typography from '@material-ui/core/Typography';
import PropTypes from 'prop-types';

function Box({
  background, color, value, label,
}) {
  return (
    <Card variant="outlined" style={{ background }}>
      <CardContent>
        <Typography style={{ color, fontSize: '2.4em' }} gutterBottom>
          { value }
        </Typography>
        <div style={{ color, textAlign: 'right', fontSize: '1.1em' }}>
          { label }
        </div>
      </CardContent>
    </Card>
  );
}

Box.propTypes = {
  background: PropTypes.string.isRequired,
  color: PropTypes.string.isRequired,
  value: PropTypes.number.isRequired,
  label: PropTypes.string.isRequired,
};
export default Box;
