import { Component } from "@angular/core";
import { BehaviorSubject, ReplaySubject } from "rxjs";
import { environment } from "src/environments/environment";

@Component({
  selector: "app-root",
  templateUrl: "./app.component.html",
  styleUrls: ["./app.component.scss"]
})
export class AppComponent {
  // connectionSubject:BehaviorSubject<any>=new BehaviorSubject<any>(null);
  connectionSubject: ReplaySubject<any> = new ReplaySubject<any>(null);
  connection$ = this.connectionSubject.asObservable();
  connection;
  constructor() {
    const self = this;
    let host;
    //if(!environment.production)host="esp.local";
    if (!environment.production) {
      host = "192.168.101.42";
    } else host = location.hostname;

    this.connection = new WebSocket("ws://" + host + ":81/", ["arduino"]);
    this.connection.onopen = function() {
      self.connectionSubject.next({ status: "connected" });
      self.connection.send("register_browser");
    };
    this.connection.onerror = function(error) {
      self.connectionSubject.next({ status: "error" });
      console.log("WebSocket Error ", error);
    };
    this.connection.onmessage = function(e) {
      console.log(e);
      console.log(e.data);
      if (e.data == "refresh") {
        self.connection.send("register_browser");
        self.connectionSubject.next({ status: "message", data: [] });
      } else {
        try {
          let data = JSON.parse(e.data);
          console.log(data);
          self.connectionSubject.next({ status: "message", data: data });
        } catch (error) {
          console.error("Data parsing error");
          console.log(e.data);
        }
      }
      // let data=JSON.parse(e.data);
      // console.log('Server data: ', data);
      // console.log('Server Id: ', data.Id);
      // console.log('Server Ip: ', data.Ip);
    };
    this.connection.onclose = function() {
      self.connectionSubject.next({ status: "closed" });
      console.log("WebSocket connection closed");
    };
  }
  onUpdate(ip) {
    const data = {
      Ip: ip,
      Action: "toggle"
    };
    this.connection.send(JSON.stringify(data));
  }
}
