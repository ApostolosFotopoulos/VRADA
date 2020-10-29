/* eslint-disable consistent-return */
const { getSSID } = require('./ssid');

module.exports = {
  isWifiChanged: async (win) => {
    try {
      let oldWifi = await getSSID();
      return setInterval(async () => {
        try {
          const newWifi = await getSSID();
          if (oldWifi !== newWifi) {
            if (process.env.NODE_ENV === 'development') {
              const winURL = `http://localhost:8080?ssid=${newWifi}`;
              win.loadURL(winURL);
            } else {
              win.loadFile(`${__dirname}/app/dist/index.html?ssid=${newWifi}`);
            }
          }
          oldWifi = newWifi;
        } catch (e) {
          console.log(e);
        }
      }, 1000, oldWifi, win);
    } catch (e) {
      console.log(e);
    }
  },
};
