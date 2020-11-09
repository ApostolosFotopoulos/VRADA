/* eslint-disable array-callback-return */
const {
  app, BrowserWindow, ipcMain, dialog,
} = require('electron');
const fs = require('fs');
const { getSSID } = require('./utils/ssid');
const UDPListener = require('./utils/UDPListener');

let WRITE_TO_CSV = false;
let CSV_FILENAME = '';
let UDP_PACKET = [0, 0, 0, 0, 0, 0, 0];
const UDP_PACKETS = [];
let UDP_METADATA = UDP_PACKET.map(() => ({ sum: 0, instances: 0 }));

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

    // Push to the packets
    UDP_PACKETS.push(d);

    console.log(d);

    // Update the meta data of the packets
    UDP_METADATA = UDP_METADATA.map((it, idx) => ({
      sum: Number(packetData[idx + 2].split(':')[1]) > 0 ? Number(it.sum) + Number(packetData[idx + 2].split(':')[1]) : Number(it.sum),
      instances: Number(packetData[idx + 2].split(':')[1]) > 0 ? Number(it.instances) + 1 : Number(it.instances),
    }));
    console.log(UDP_METADATA);
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
    const winURL = 'http://localhost:8080';
    win.loadURL(winURL);
    win.webContents.openDevTools();
  } else {
    win.loadFile(`${__dirname}/build/index.html`);
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
      fs.writeFileSync(CSV_FILENAME, '\ufeffΗμερομηνία,Συσκευή,Ταχύτητα,RPM,Απόσταση,Θερμίδες,Καρδιακοί Παλμοί,Ισχύς,Επίπεδο\n', 'utf8');
    } catch (e) {
      dialog.showErrorBox('Σφάλμα', 'Σφάλμα κατά την αποθήκευση');
    }
  });

  // Stop the write to csv
  ipcMain.on('STOP_WRITE', () => {
    WRITE_TO_CSV = false;
    if (UDP_PACKETS.length > 0) {
      fs.appendFileSync(CSV_FILENAME, `\ufeff-,-,${UDP_METADATA.map((i) => ((i.sum / i.instances) || 0).toFixed(2)).join(',')}\n`, 'utf8');
      UDP_PACKETS.map((i) => {
        fs.appendFileSync(CSV_FILENAME, `\ufeff${i}`, 'utf8');
      });
    }
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

  // Send the status of the wifi to the frontend
  ipcMain.on('FETCH_WIFI_STATUS', async (e) => {
    try {
      e.reply('WIFI_STATUS', await getSSID());
    } catch (err) {
      dialog.showErrorBox('Σφάλμα', 'Σφάλμα στην σύνδεση');
    }
  });
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
