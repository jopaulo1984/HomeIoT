
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

var REMOTE_IP = "192.168.2.106";

function RequestPOST(url, dados, response) {
    var xhttp = new XMLHttpRequest();
	
    xhttp.open("POST", url, true);
    xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            response(xhttp.responseText);
        }
    };
    xhttp.send(dados);
}

function sendCommand(cmd, response) {
    RequestPOST(`http://${REMOTE_IP}:30128/`, `cmd=${cmd}`, function (rtext) {
        response(rtext);
    });
}

function newWaitWindow(msg) {
    return newJPLoader({title: msg});
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

    sendCommand("GET INFO", response);

}

function showInfos() {

    var wait = newWaitWindow("Carregando informações...");

    updateInfos(function () {

        var panel = newJPPanel({className: "infos-panel"});

        Object.keys(infos).forEach(function (key) {
            newElement("div", {text: key, parent: newElement("tr", {parent: panel})});
            var tab = newElement("table", {parent: panel});
            tab.className = "table";
            Object.keys(infos[key]).forEach(function (skey) {
                var tr = newElement("tr", {parent: tab});
                var th = newElement("th", {parent: tr});
                var td = newElement("td", {parent: tr});
                th.innerText = skey;
                td.innerText = infos[key][skey];
            });
        });
        
        var win = newJPDialog({title: 'Informações', content: panel, buttons: [newJPButton({text: 'Ok', result: ResponseResult.OK})]});
        win.showModal(function (sender, result) {
            sender.destroy();
        });

        wait.destroy();

    });

}

function getONOFF(value) {
    return value ? 'ON' : 'OFF';
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
    if (value === 2) return 68;
    if (value === 1) return 30;
    return 18;
}

function update() {

    updating = true;

    updateInfos(function () {
		        
        var pmpst = infos['status']['pump'];
        var measu = infos['measures'];
        var hlevel = getCxHeight(measu['cxlevel']);
        
        darea.itemconfig(bomba, { fillstyle: getONOFFColor(pmpst) });
        darea.itemconfig(bmbestd, { text: 'Estado: ' + getONOFF(pmpst) });
        darea.itemconfig(vazao, { value: measu['flowr'] });
        darea.itemconfig(vcount, { text: 'Volume: ' + measu['vcount'] + ' L' });
        darea.itemconfig(tubo1, { strokestyle: getFlowColor(measu['flowr']) });
        darea.itemconfig(tubo2, { strokestyle: getFlowColor(measu['flowr']) });
        darea.itemconfig(cxnivel, { y: 89 - hlevel, h: hlevel });
        darea.itemconfig(lockled, { fillstyle: infos['status']['state'] == 2 ? 'red' : '#cccccc' });

        var btnbmb = document.getElementById('btnbmb');

        btnbmb.innerText = pmpst == 1 ? "Desligar Bomba" : "Ligar Bomba";
        btnbmb.style.backgroundColor = pmpst == 1 ? "#ffcccc" : "#ccffcc";

        setbtnmode(infos['status']['manual']);

        updating = false;

        setTimeout(update, 1000);

    });

    //updatemode();

}

function draw() {

    //cisterna
    darea.createRectangle(10, 210, 80, 80);
    darea.createText(10, 200, { text: "CISTERNA", font: bfont });
    darea.createRectangle(11, 218, 78, 70, { strokestyle: '#88ddff', fillstyle: '#88ddff' });

    //bomba
    bomba = darea.createCircle(180, 180, 20, { fillstyle: "#cccccc" });

    lockled = darea.createCircle(220 , 185, 5, { fillstyle: "#cccccc" });
    darea.createText(230, 190, { text: "Bloqueada", font: bfont });

    darea.createText(150, 220, { text: "BOMBA", font: bfont });
    bmbestd = darea.createText(150, 235, { text: "Estado: ?", font: bfont });
    vcount = darea.createText(150, 250, { text: "Volume: ?", font: bfont });

    vazao = new DashGauge({
        x: 110,
        y: 130,
        label: "Vazão",
        sufix: "L/min",
        value: 0,
        min: 0,
        max: 20,
        colorranges: [
            {min: 10, max: 12, color: "yellow"},
            {min: 0, max: 9.9, color: "red"},
            {min: 12, max: 20, color: "#33FF33"}
        ],        
        width: 40,
        thickness: 30,
        font: {name:"Consolas", size:32}
    });

    darea.appendGElement(vazao);

    //cx d'agua
    cxdagua = darea.createRectangle(280, 10, 79, 80);
    cxnivel = darea.createRectangle(282, 108, 76, 0, { strokestyle: '#88ddff', fillstyle: '#88ddff' });
    darea.createText(275, 105, { text: "CX. D'ÁGUA", font: bfont });

    //tubulacoes
    tubo1 = darea.createPolyline([{ x: 80, y: 285 }, { x: 80, y: 180 }, { x: 180, y: 180 }], { linewidth: 4, strokestyle: '#cccccc' });
    tubo2 = darea.createPolyline([{ x: 180, y: 160 }, { x: 220, y: 160 }, { x: 220, y: 20 }, { x: 290, y: 20 }], { linewidth: 4, strokestyle: '#cccccc' });

    //sinotico
    var x1 = 20;
    var x2 = 30;
    var y1 = 20;
    darea.createText(x1 - 5, y1, { text: "Modo", font: bfont });
    autled = darea.createCircle(x1 , y1 + 15, 5, { fillstyle: "#cccccc" });
    darea.createText(x2, y1 + 20, { text: "Automático", font: bfont });
    manled = darea.createCircle(x1 , y1 + 35, 5, { fillstyle: "#cccccc" });
    darea.createText(x2, y1 + 40, { text: "Manual", font: bfont });

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
    var on = btnbmb.innerText == 'Ligar Bomba' ? 1 : 0; //btnbmb.innerText == 'Ligar Bomba'
    
    if (on) {
        var wait = newWaitWindow("Ligando a bomba...");
    } else {
        var wait = newWaitWindow("Desligando a bomba...");
    }
    
    sendCommand(`SET PUMP ${on}`, function (rtext) {
        wait.destroy();
        update();
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
    var on = btntst.innerText == 'Modo Manual' ? 1 : 0;
    if (on) {
        var wait = newWaitWindow("Entrando no modo manual...");
    } else {
        var wait = newWaitWindow("Entrando do modo automático...");
    }
    sendCommand(`SET MANUAL ${on}`, function (result) {
        wait.destroy();
        update();
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

/*function showmenu() {
    menu.className = "menu-visible";
}

function hiddemenu() {
    menu.className = "menu-hidden";
}*/

function btnmenuclick() {
    if (menu.className == "menu-hidden" || menu.className == "menu-start") {
        menu.className = "menu-visible";
    } else {
        menu.className = "menu-hidden";
    }
}

window.onload = function () {
    menu = document.getElementById("left-menu");
    darea = new GCanvas(360, 320);
    document.getElementById("drawarea").appendChild(darea);
    //updatemode();
    draw();
}

