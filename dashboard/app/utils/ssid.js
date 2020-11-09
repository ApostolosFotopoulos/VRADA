/* eslint-disable consistent-return */
const WiFiControl = require('wifi-control');

module.exports = {
  async getSSID() {
    try {
      await WiFiControl.init();
      return await WiFiControl.getIfaceState().ssid;
    } catch (e) {
      return 'ETHERNET_TYPE'; // Not correct handling.NEED FIXING TO GET THE CONNECTED INTERFACE
    }
  },
};
