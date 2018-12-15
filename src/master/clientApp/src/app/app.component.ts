import { Component } from '@angular/core';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {
  constructor(){
    var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
    connection.onopen = function () {
      connection.send('Connect ' + new Date());
    };
    connection.onerror = function (error) {
      console.log('WebSocket Error ', error);
    };
    connection.onmessage = function (e) {
      console.log('Server: ', e.data);
    };
    connection.onclose = function () {
      console.log('WebSocket connection closed');
    };
  }
  
}
