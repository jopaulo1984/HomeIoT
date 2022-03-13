
function sendCommand(cmd, response) {
    RequestPOST("http://192.168.1.87:30128/", `cmd=${cmd}`, function (rtext) {
        response(rtext);
    });
}

