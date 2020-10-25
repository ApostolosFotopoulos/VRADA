const WiFiControl = require('wifi-control')
module.exports = {
  async getSSID(){
    try{
      await WiFiControl.init()
      return await WiFiControl.getIfaceState().ssid
    } catch(e){
      console.log(e)
    }
  },
}