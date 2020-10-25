const { app, BrowserWindow, ipcMain,dialog } = require('electron')
const { fstat } = require('fs')
const { getSSID } = require("./utils/ssid")
const UDPListener = require('./utils/UDPListener')
const { isWifiChanged } = require('./utils/wifi')
const fs = require('fs')

var WRITE_TO_CSV = false
var CSV_FILENAME = ""

// Create the udp listener
const eventListener = new UDPListener().Listen()
eventListener.on('udp-packets',(packet)=>{
  // Split the packet info
  if(WRITE_TO_CSV && CSV_FILENAME){
    const packetData = packet.toString().split(";")
    console.log({
      "Speed":packetData[2].split(":")[1],
      "rpm":packetData[3].split(":")[1],
      "distance":packetData[4].split(":")[1],
      "calories":packetData[5].split(":")[1],
      "hf":packetData[6].split(":")[1],
      "power":packetData[7].split(":")[1],
      "level":packetData[8].split(":")[1],
    })
    let d =`${new Date().toLocaleTimeString()},${packetData[1]},${packetData[2].split(":")[1]},${packetData[3].split(":")[1]},`
          +`${packetData[4].split(":")[1]},${packetData[5].split(":")[1]},${packetData[6].split(":")[1]},${packetData[7].split(":")[1]},`
          +`${packetData[8].split(":")[1]}\n`
    fs.appendFileSync(CSV_FILENAME,d)
  }
})



async function createWindow () {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    minWidth: 800,
    minHeight: 600,
    webPreferences: {
      nodeIntegration: true,
    },
  })

  if(process.env.NODE_ENV === 'development'){
    const winURL = `http://localhost:8080?ssid=${await getSSID()}`
    win.loadURL(winURL)
  } else {
    win.loadFile(__dirname+`/app/dist/index.html?ssid=${await getSSID()}`);
  }
  
  win.webContents.openDevTools()

  // Start the write to csv
  ipcMain.on('START_WRITE',async ()=>{
    try{
      console.log("Start to write.....")
      WRITE_TO_CSV = true

      // Ask for the dialog
      let res = await dialog.showSaveDialog({
        defaultPath: new Date().toLocaleString() + ".csv"
      })
      if(res.canceled){
        CSV_FILENAME = ""
      } else {
        CSV_FILENAME = res.filePath
      }
      fs.writeFileSync(CSV_FILENAME,"Ημερομηνία,Συσκευή,Ταχύτητα,RPM,Απόσταση,Θερμίδες,Καρδιακοί Παλμοί,Ισχύς,Επίπεδο\n")
    } catch(e){
      console.log(e)
      dialog.showErrorBox("Σφάλμα", "Σφάλμα κατά την αποθήκευση");
    }
  })

  // Stop the write to csv
  ipcMain.on('STOP_WRITE',()=>{
    console.log("Stopped the writing....")
    WRITE_TO_CSV = false
    CSV_FILENAME = ""
  })
  
  // Interval that checks the wifi changes 
  isWifiChanged(win)
}

app.whenReady().then(createWindow)

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow()
  }
})
