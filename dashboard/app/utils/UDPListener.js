/* eslint-disable no-underscore-dangle */
const dgram = require('dgram');
const events = require('events');

/**
* @class Listens for udp packets and transmit them to the electron app.
*/
class UDPListener {
  constructor(p, i) {
    this.port = p || 12345;
    this.ip = i || '0.0.0.0';
  }

  Listen() {
    /**
    * Create the event listener.
    */
    this.eventEmitter = new events.EventEmitter();

    /**
    * Create the socket that listens to the traffic.
    */
    this.socket = dgram.createSocket('udp4');
    this.socket.bind(this.port, this.ip);

    /**
    * When a udp packet arrives then trigger an event that
    * alerts the electron app.
    */
    const _this = this;
    this.socket.on('message', (packet) => {
      if (_this.eventEmitter) {
        _this.eventEmitter.emit('UDP_PACKETS', packet);
      }
    });
    return this.eventEmitter;
  }

  Destroy() {
    /**
    * Destroy the resources that we used
    */
    delete this.eventEmitter;
    delete this.socket;
  }
}

module.exports = UDPListener;
