(function() {
  'use strict';

class Lock {

  constructor() {
    this.device = null;
    this.onDisconnected = this.onDisconnected.bind(this);
  }
  
  request() {
    let options = {
      "filters": [{
        "name": "HC-05"
      }],
      "optionalServices": [0xFF02]
    };
    return navigator.bluetooth.requestDevice(options)
    .then(device => {
      this.device = device;
      this.device.addEventListener('gattserverdisconnected', this.onDisconnected);
    });
  }
  
  connect() {
    if (!this.device) {
      return Promise.reject('Device is not connected.');
    }
    return this.device.gatt.connect();
  }
  
  readColor() {
    return this.device.gatt.getPrimaryService(0xFF02)
    .then(service => service.getCharacteristic(0xFFFC))
    .then(characteristic => characteristic.readValue());
  }

  writeColor(data) {
    return this.device.gatt.getPrimaryService(0xFF02)
    .then(service => service.getCharacteristic(0xFFFC))
    .then(characteristic => characteristic.writeValue(data));
  }

  disconnect() {
    if (!this.device) {
      return Promise.reject('Device is not connected.');
    }
    return this.device.gatt.disconnect();
  }

  onDisconnected() {
    console.log('Device is disconnected.');
  }
}

var lock = new Lock();

document.querySelector('button').addEventListener('click', event => {
  lock.request()
  .then(_ => lock.connect())
  .then(_ => { /* Do something with lock... */})
  .catch(error => { console.log(error) });
});


