var express = require("express");
var mDnsSd = require("node-dns-sd");
var app = express();

mDnsSd
.discover({
  name: "esp.local"
})
.then(device_list => {
  console.log(JSON.stringify(device_list, null, "  "));
//   res.send(device_list);
})
.catch(error => {
  console.error(error);
});
// app.get("/mdns", function(req, res) {


// });

// app.listen(4201);
