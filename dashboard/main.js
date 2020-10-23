const { app, BrowserWindow } = require('electron')
const { getSSID } = require("./utils/index")

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
    win.loadFile(__dirname+'/app/dist/index.html');
  }
  
  win.webContents.openDevTools()
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
