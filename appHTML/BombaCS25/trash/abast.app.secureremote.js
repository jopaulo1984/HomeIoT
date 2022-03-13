const AUT_MODE = "OFF";
const MAN_MODE = "ON";

var darea = null;
var cxdagua = null;
var cxnivel = null;
var cisterna = null;
var vazao = null;
var bomba = null;
var bmbestd = null;
var tubo1 = null;
var tubo2 = null;
var menu = null;
var autled = null;
var manled = null;
var lockled = null;
var vcount = null;

var bfont = { name: "Monospace", size: 14 };
var enteste = false;
var updating = false;

var infos;
var opmode;

function RequestPOST(url, dados, response) {
    var xhttp = new XMLHttpRequest();	
    xhttp.open("POST", "https://condominioparaty.com.br/iot/" + url, true);
    xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            response(xhttp.responseText);
        }
    };

    xhttp.send(dados); //`token=pEka15G61&device=t0R27g6/BombaCasa25&cmd=${cmd}`
}

function newWaitWindow(msg) {
    var message = newJPLabel({text: msg});
    message.style.fontSize = "125%";
    message.style.fontFamily = "'Monospace', 'Consolas'";
    var dlg = newJPDialog({title: "  ", content: message});
    dlg.show();
    return dlg;
}

function sendCommand(cmd, response) {
    RequestPOST("send.php", `token=pEka15G61&device=t0R27g6/BombaCasa25&cmd=${cmd}`, function (rtext) {
        response(rtext);
    });
}

function updateInfos(onupdated) {

    var response = function (rtext) {

        try {
            var temp = JSON.parse(rtext);
            infos = temp;
        } catch {
            console.log('Não foi possível obter as informações do sistema.');
        }

        onupdated && onupdated();

    }

    RequestPOST("get.php", "token=pEka15G61&device=t0R27g6/BombaCasa25", response);

}

function showInfos() {

    var wait = newWaitWindow("Carregando informações...");

    updateInfos(function () {

        var tab = newElement("table");
        tab.className = "table";

        Object.keys(infos).forEach(function (key) {
            var tr = newElement("tr", {parent: tab});
            var th = newElement("td", {parent: tr});
            var td = newElement("td", {parent: tr});
            th.innerText = key;
            td.innerText = infos[key];
        });

        //var bok = newJPDialogButton();
        var win = newJPDialog({title: 'Informações', content: tab, buttons: [newJPButton({text: 'Ok', result: ResponseResult.OK})]});
        win.showModal(function (sender, result) {
            sender.destroy();
        });

        wait.destroy();

    });

}

function getONOFF(value) {
    if (value) return 'ON';
    return 'OFF';
}

function getONOFFColor(value) {
    if (value) return '#dd3c3c';
    return '#cccccc';
}

function getFlowColor(value) {
    if (value === 0) return '#888888';
    if (0 < value && value < 10) return '#8888dd';
    if (value >= 10) return '#88ddff';
}

function getCxHeight(value) {
    if (value === 2) return 88;
    if (value === 1) return 50;
    return 18;
}

function update() {

    updating = true;

    updateInfos(function () {

        var hlevel = getCxHeight(infos['level']);
        
        darea.itemconfig(bomba, { fillstyle: getONOFFColor(infos['pump']) });
        darea.itemconfig(bmbestd, { text: 'Estado: ' + getONOFF(infos['pump']) });
        darea.itemconfig(vazao, { text: 'Fluxo: ' + infos['flowrate'].toFixed(1) + ' L/m' });
        darea.itemconfig(vcount, { text: 'Volume: ' + infos['volumecount'] + ' L' });
        darea.itemconfig(tubo1, { strokestyle: getFlowColor(infos['flowrate']) });
        darea.itemconfig(tubo2, { strokestyle: getFlowColor(infos['flowrate']) });
        darea.itemconfig(cxnivel, { y: 109 - hlevel, h: hlevel });
        darea.itemconfig(lockled, { fillstyle: infos['state'] == 2 ? 'red' : '#cccccc' });

        var btnbmb = document.getElementById('btnbmb');

        btnbmb.innerText = infos['pump'] == 1 ? "Desligar Bomba" : "Ligar Bomba";
        btnbmb.style.backgroundColor = infos['pump'] == 1 ? "#ffcccc" : "#ccffcc";

        setbtnmode(infos['manual']);

        updating = false;

        setTimeout(update, 2000);

    });

    //updatemode();

}

function draw() {

    //cisterna
    darea.createRectangle(10, 210, 100, 100);
    darea.createText(10, 200, { text: "CISTERNA", font: bfont });
    darea.createRectangle(11, 240, 98, 68, { strokestyle: '#88ddff', fillstyle: '#88ddff' });

    //bomba
    bomba = darea.createCircle(200, 180, 20, { fillstyle: "#cccccc" });
    darea.createText(150, 210, { text: "BOMBA", font: bfont });
    bmbestd = darea.createText(150, 225, { text: "Estado: ?", font: bfont });
    vazao = darea.createText(150, 240, { text: "Vazão: ?", font: bfont });
    vcount = darea.createText(150, 255, { text: "Volume: ?", font: bfont });
    lockled = darea.createCircle(240 , 190, 5, { fillstyle: "#cccccc" });
    darea.createText(250, 195, { text: "Bloqueada", font: bfont });

    //cx d'agua
    cxdagua = darea.createRectangle(300, 10, 100, 100);
    cxnivel = darea.createRectangle(301, 108, 98, 0, { strokestyle: '#88ddff', fillstyle: '#88ddff' });
    darea.createText(300, 125, { text: "CX. D'ÁGUA", font: bfont });

    //tubulacoes
    tubo1 = darea.createPolyline([{ x: 90, y: 300 }, { x: 90, y: 180 }, { x: 200, y: 180 }], { linewidth: 4, strokestyle: '#cccccc' });
    tubo2 = darea.createPolyline([{ x: 200, y: 160 }, { x: 240, y: 160 }, { x: 240, y: 20 }, { x: 320, y: 20 }], { linewidth: 4, strokestyle: '#cccccc' });

    //sinotico
    var x1 = 300;
    var x2 = 310;
    darea.createText(x1 - 5, 250, { text: "Modo", font: bfont });
    autled = darea.createCircle(x1 , 265, 5, { fillstyle: "#cccccc" });
    darea.createText(x2, 270, { text: "Automático", font: bfont });
    manled = darea.createCircle(x1 , 285, 5, { fillstyle: "#cccccc" });
    darea.createText(x2, 290, { text: "Manual", font: bfont });

    update();

}

function aguardaUpdate() {
    if (!updating) return;
    function _exit() {
        updating = false;
    }
    setTimeout(_exit, 5000);
    var i = 0;
    while (true) {
        i++;
        if (i > 1000000) break;
    };
}

function ligabomba() {
    
    var btnbmb = document.getElementById('btnbmb');
    var on = getONOFF(btnbmb.innerText == 'Ligar Bomba'); //btnbmb.innerText == 'Ligar Bomba'
    
    if (on == "ON") {
        var wait = newWaitWindow("Ligando a bomba...");
    } else {
        var wait = newWaitWindow("Desligando a bomba...");
    }
    
    sendCommand(`SET PUMP ${on}`, function (rtext) {
        wait.destroy();
        /*try {
            update();
        } catch {
            if (on == "ON") { newDialogMessage('Erro', 'Não foi possível ligar a bomba.'); }
            else { newDialogMessage('Erro', 'Não foi possível desligar a bomba.'); }
        }*/
    });

}

function setbtnmode (modemanual) {
    if (modemanual) {
        btntst.innerText = "Modo Automático";
        darea.itemconfig(autled, {fillstyle: "#cccccc"});
        darea.itemconfig(manled, {fillstyle: "red"});
    } else {
        btntst.innerText = "Modo Manual";
        darea.itemconfig(autled, {fillstyle: "red"});
        darea.itemconfig(manled, {fillstyle: "#cccccc"});
    }
}

function modoteste () {
    var btntst = document.getElementById('btntst');
    var on = getONOFF(btntst.innerText == 'Modo Manual');
    if (on == "ON") {
        var wait = newWaitWindow("Entrando no modo manual...");
    } else {
        var wait = newWaitWindow("Entrando do modo automático...");
    }
    sendCommand(`SET MANUAL ${on}`, function (result) {
        wait.destroy();
        /*var mode;
        try {mode = JSON.parse(result)['manual'];} catch {mode = 0;}
        setbtnmode(mode);*/
    });            
}

function resetstate() {
    var wait = newWaitWindow("Reiniciando estado...");
    sendCommand("RESET STATE", function (rtext) {
        wait.destroy();
        if (rtext === '') {
            newDialogMessage('Erro', 'Não foi possível reiniciar o estado.');
        } else {
        }
    });
}

function resetvolume() {
    var wait = newWaitWindow("Reiniciando volume...");
    sendCommand("RESET VOLUME", function (rtext) {
        wait.destroy();
        if (rtext === '') {
            newDialogMessage('Erro', 'Não foi possível reiniciar o volume.');
        } else {

        }
    });
}

function configurar() {
    //location.href = "configurar.py";
}

function showmenu() {
    menu.style.left = "0px";
}

function hiddemenu() {
    menu.style.left = "-500px";
}

window.onload = function () {
    menu = document.getElementById("left-menu");
    darea = new GCanvas(405, 320);
    document.getElementById("drawarea").appendChild(darea);
    //updatemode();
    draw();
}

