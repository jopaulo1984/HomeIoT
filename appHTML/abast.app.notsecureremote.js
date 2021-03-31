
var REMOTE_IP = "191.43.80.15";

function sendCommand(cmd, response) {
    RequestPOST(`http://${REMOTE_IP}:30128/`, `cmd=${cmd}`, function (rtext) {
        response(rtext);
    });
}
