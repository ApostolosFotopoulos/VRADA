const {
  app, BrowserWindow, ipcMain, dialog,
} = require('electron');
const fs = require('fs');
const { getSSID } = require('./utils/ssid');
const UDPListener = require('./utils/UDPListener');
const { isWifiChanged } = require('./utils/wifi');

let WRITE_TO_CSV = false;
let CSV_FILENAME = '';
let UDP_PACKET = [0, 0, 0, 0, 0, 0, 0];

// Create the udp listener
const listener = new UDPListener();
const eventListener = listener.Listen();
eventListener.on('UDP_PACKETS', (packet) => {
  // Split the packet info
  if (WRITE_TO_CSV && CSV_FILENAME) {
    const packetData = packet.toString().split(';');

    // Update the UDP packet
    UDP_PACKET = [packetData[2].split(':')[1], packetData[3].split(':')[1], packetData[4].split(':')[1],
      packetData[5].split(':')[1], packetData[6].split(':')[1], packetData[7].split(':')[1], packetData[8].split(':')[1]];

    const d = `${new Date().toLocaleTimeString()},${packetData[1]},${packetData[2].split(':')[1]},${packetData[3].split(':')[1]},`
          + `${packetData[4].split(':')[1]},${packetData[5].split(':')[1]},${packetData[6].split(':')[1]},${packetData[7].split(':')[1]},`
          + `${packetData[8].split(':')[1]}\n`;
    fs.appendFileSync(CSV_FILENAME, d);
  } else {
    UDP_PACKET = [0, 0, 0, 0, 0, 0, 0];
  }
}, UDP_PACKET);

// Create the window
async function createWindow() {
  const win = new BrowserWindow({
    width: 1000,
    height: 800,
    minWidth: 1000,
    minHeight: 800,
    webPreferences: {
      nodeIntegration: true,
      devTools: process.env.NODE_ENV === 'development',
    },
  });

  if (process.env.NODE_ENV === 'development') {
    const winURL = `http://localhost:8080?ssid=${await getSSID()}`;
    win.loadURL(winURL);
    win.webContents.openDevTools();
  } else {
    const ssid = await getSSID();
    win.loadFile(`${__dirname}/build/index.html`, { query: { ssid } });
  }

  // Start the write to csv
  ipcMain.on('START_WRITE', async () => {
    try {
      WRITE_TO_CSV = true;

      // Ask for the dialog
      const res = await dialog.showSaveDialog({
        defaultPath: `${new Date().toLocaleString()}.csv`,
      });
      if (res.canceled) {
        CSV_FILENAME = '';
      } else {
        CSV_FILENAME = res.filePath;
      }
      fs.writeFileSync(CSV_FILENAME, 'Ημερομηνία,Συσκευή,Ταχύτητα,RPM,Απόσταση,Θερμίδες,Καρδιακοί Παλμοί,Ισχύς,Επίπεδο\n');
    } catch (e) {
      dialog.showErrorBox('Σφάλμα', 'Σφάλμα κατά την αποθήκευση');
    }
  });

  // Stop the write to csv
  ipcMain.on('STOP_WRITE', () => {
    WRITE_TO_CSV = false;
    CSV_FILENAME = '';
  });

  // Send data to the frontend
  ipcMain.on('FETCH_PACKET_DATA', async (e) => {
    try {
      e.reply('PACKET_DATA', UDP_PACKET);
    } catch (err) {
      dialog.showErrorBox('Σφάλμα', 'Σφάλμα κατά την μεταφορά πακέτων');
    }
  }, UDP_PACKET);

  // Interval that checks the wifi changes
  isWifiChanged(win);
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
    listener.Destroy();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
