
/*
* Descrição: JPGUI é uma biblioteca gráfica que facilita a construção de janelas e
* controles em páginas web.
* Autor: João Paulo F da Silva
* Modificação: 12/2020
* Versão: 1.4.0
*/


ResponseResult = {NULL:0, OK:1, CANCEL:2, YES:3, NO:4};
MouseButtons = {NONE:0, LEFT:1, RIGHT:2};

function newElement(tag, args={}) {
    var element = document.createElement(tag);
    element.ondestroy = null;
    Object.defineProperty(element,'parent', {
        set: function (value) {
            element.setParent(value);
        },
        get: function () {return element.getParent();},
        configurable: true
    });
    Object.defineProperty(element,'text',{
        set: function (value) {
          if (element.innerText == value) return;
          element.innerText = value;
        },
        get: function () {return element.innerText;},
        configurable: true
    });
    element.destroy = function() {
        if(!this.parent) return;
        if(this.ondestroy&&this.ondestroy(this)) return;
        this.parent.removeChild(this);
    };
    element.setParent = function (value) {
      if (this.parentNode != null) {
        this.parentNode.removeChild(this);
        this.parentNode = null;
      }
      if (value !== null) {
          try {
              value.appendChild(this);
          } catch (ex) {
            console.log(ex.message);
          }
      }
    };
    element.getParent = function () { return this.parentNode; }
    element.setProperties = function (props={}) {
        //definindo as propriedades do elemento passados em args
        var self = this;
        Object.keys(props).forEach(function(key) {
            if (key===null) return;
            self[key] = props[key];
        });
    };
    element.setProperties(args);
    return element;
}

function newJPInput(args={}) {
    var element = newElement("input");
    element.className = "entry";
    element.__readonly = false;
    Object.defineProperty(element,'editable',{
      set(value){
        var ronly = !value;
        if (element.__readonly == ronly) return;
        element.__readonly = ronly;
        element.readOnly = ronly;
        if(ronly) {
          element.className = element.className + " readonly";
        }else{
          element.className = element.className.replace(/readonly\s/g,'');
        }
      },
      get(){return !element.__readonly;}
    });
    element.setProperties(args);
    return element;
}

function newJPLabel(args={}) {
  var element = newElement('div');
  element.style.display = 'inline-block';
  element.className = 'label';
  element.setProperties(args);
  return element;
}

function newJPButton(args={}) {
  var element = newElement('button');
  element.className = 'button';
  element.result = ResponseResult.NULL;
  element.setProperties(args);
  return element;
}

function newJPComboBox(args={}) {
    var element = newElement('select');
    element.className = 'combobox select';
    Object.defineProperty(element,'items',{
        set(value){
            element.innerHTML = "";
            element.appendItems(value);
        },
        get(){ return element.getElementsByTag('option'); }
    });
    element.getItem = function(index) {
        return element.items[index];
    };
    element.appendItems = function(aitems) {
        element = this;
        aitems.forEach(function(item, index) {
            var o = document.createElement("option");
            o.value = item.value;
            o.innerHTML = item.text;
            element.appendChild(o);
        });
    }
    element.items = [];
    element.setProperties(args);
    return element;
}

function newJPTextArea(args={parent:null,text:'',rows:5,cols:50,readonly:false}) {
  var element = newElement('textarea');
  element.className = 'textarea';
  Object.defineProperty(element,'readonly',{
    set(value){
      element.readOnly = value;
      if(value) {
        element.className += " readonly";
      }else{
        element.className = element.className.replace(/readonly/g,'');
      }
    },
    get(){return element.readOnly;}
  });
  element.setProperties(args);
  return element;
}

function newJPPanel(args={}) {
    var element = newElement('div');
    element.className = 'panel';
    Object.defineProperty(element,'inline',{
      set(value){
          if (element.inline === value) return;
          if(value===true) {
              element.style.display = 'inline-block';
          } else {
              element.style.display = 'block';
          }
      },
      get(){return element.style.display === 'inline-block'; }
    });
    element.inline = false;
    element.setProperties(args);
    return element;
}

function newJPGroupBox(args={}) {

  var self = newElement('fieldset');

  newElement('legend', {parent:self});
  newElement('div', {parent:self});

  Object.defineProperty(self, 'text', {
    set (value) {self.childNodes[0].text = value;},
    get () {return self.childNodes[0];}
  });

  self.appendChild = function(child) {
    this.childNodes[1].appendChild(child);
  }

  self.setProperties(args);

  return self;

}

function newJPForm(args={}) {
  var element = newElement('form', args);
  element.className = 'form';
  return element;
}

function newJPGrid(args={}) {

  var element = newJPPanel(args);
  element.table = newElement('table', args={parent:element});
  element.className = "grid";
  element.table.className = "table";

  element.createColumn = function(row=null,colspan=1,rowspan=1,child=null) {
    var c = newElement('td', {parent:row});
    c.colSpan = colspan;
    c.rowSpan = rowspan;
    if(child) c.appendChild(child);
    return c;
  }

  element.createRow = function(columns=[],parent=null) {
    var tr = newElement('tr', {parent:parent});
    columns.forEach(function(item,index){
      tr.appendChild(item);
    });
    return tr;
  }

  element.insert = function(row,column,component,width=1,height=1) {
    var rows = this.table.getElementsByTagName('tr');
    if(row<rows.length) {
      var cols = rows[row].getElementsByTagName('td');
      if(column<cols.length) {
        cols[column].appendChild(component);
      }else{
        for(var i=cols.length;i<column;i++) {
          this.createColumn(rows[row],width,height);
        }
        this.createColumn(rows[row],width,height,component);
      }
    }else{
      for(var i=rows.length;i<row;i++) {
        this.createRow([],this.table);
      }
      var r = this.createRow([],this.table);
      for(var i=0;i<column;i++) {
        this.createColumn(r,width,height);
      }
      this.createColumn(r,width,height,component);
    }
  }

  return element;

}

function newJPTableView(args={}) {

  var element = newElement('div');

  element.thead = newElement('table', {parent:element});
  element.panelBody = newJPPanel({parent:element})
  element.tbody = newElement('table', {parent:element.panelBody});

  element.className = 'tableview';
  element.style.padding = '5px';
  element.thead.className = 'thead';
  element.tbody.className = 'tbody';

  element.thead.style.display = 'block';
  element.tbody.style.display = 'inline-block';

  element.__selection = [];
  element.__selected = null;

  Object.defineProperty(element,'header', {
      set (value) {
          element.setHeader(value);
      }
  });

  Object.defineProperty(element,'values', {
      set (value) {
          element.setValues(value);
      }
  });

  Object.defineProperty(element,'selection', {
    get() {return element.__selection;}
  });

  Object.defineProperty(element,'selected', {
    set(value){
      element.__selected = value;
      element.__selection = [];
      if (element.tbody.childNodes.length===0) return;
      element.tbody.childNodes[1].childNodes.forEach(function(item, index) {
        if(index===value){
          element.__selection = [index];
          item.className = 'selected';
        }else{
          item.className = 'no-selected';
        }
      });
    },
    get(){return element.__selected;}
  });

  element.setHeader = function(value) {
    this.thead.innerHTML = '';
    //this.setValues([]);
    var cg = newElement('colgroup', {parent:this.thead});
    var th = newElement('thead', {parent:this.thead});
    var tr = newElement('tr', {parent:th});
    value.forEach(function(item, index) {
      newElement('col', {parent:cg});
      newElement('th', {parent:tr}).text = item;
    });
  };

  element.setValues = function(value) {
    this.__selection = [];
    this.__selected = null;
    this.tbody.innerHTML = '';
    var cg = newElement('colgroup', {parent:this.tbody});
    var tb = newElement('tbody', {parent:this.tbody});
    value.forEach(function(linha,l) {
      var tr = newElement('tr', {parent:tb});
      var selalter = false;
      tr.className = 'no-selected';
      tr.onclick = function(e) {
        var tr = this;
        var self = tr.parentNode.parentNode.parentNode.parentNode;
        this.parentNode.childNodes.forEach(function(item, index) {
          if(item==tr){
            if(e.ctrlKey) {
              self.selection.push(index);
            }else{
              self.__selection = [index];
            }
            item.className = 'selected';
            selalter = self.__selected != index;
            self.__selected = index;
          }else{
            item.className = 'no-selected';
          }
        });
        self.onSelectionChanged && selalter && self.onSelectionChanged(self.selection);
      };
      tr.ondblclick = function(e) {
        var self = this.parentNode.parentNode.parentNode.parentNode;
        var clk = self.onSelectionChanged;
        self.onSelectionChanged = null;
        this.onclick(e);
        self.onSelectionDblClick && self.onSelectionDblClick(self.selection);
        self.onSelectionChanged = clk;
      };
      linha.forEach(function(column,c) {
        newElement('td', {parent:tr}).innerHTML = column;
      });
    });
    this.updateValues();
   };

  element.updateValues = function() {
    var tb = this.tbody.getElementsByTagName('tbody')[0];
    //var thead = this.tbody.getElementsByTagName('tbody')[0];
    if(this.thead.childNodes.length==0) return;
    var tr1 = this.thead.getElementsByTagName('tr')[0];
    var cg1 = this.thead.getElementsByTagName('colgroup')[0];
    var cg2 = this.tbody.getElementsByTagName('colgroup')[0];
    if(tb.childNodes.length > 0) {
        var tw = 0;
        tb.childNodes[0].childNodes.forEach(function(item,index){
            var thcol = cg1.childNodes[index];
            var tdcol = newElement('col', {parent:cg2});
            var th = tr1.childNodes[index];
            var td = item;
            var w = td.clientWidth;
            if (th.clientWidth > w) {
              w = th.clientWidth;
            }
            tw += w;
            thcol.style.width = w + 'px';
            tdcol.style.width = thcol.style.width;
        });
        this.style.width = (tw + 20) + 'px';
        this.tbody.style.width = this.style.width;
        this.thead.style.width = this.style.width;
    }
  }

  element.onSelectionChanged = null;
  element.onSelectionDblClick = null;

  element.setProperties(args);

  return element;

}

function newJPMask(args={}) {

  var element = newJPPanel(args);

  this.child = null;

  element.className = "mask";

  element.style.position = "absolute";
  element.style.top = "0px";
  element.style.left = "0px";
  element.style.width = "100%";
  element.style.height = "100%";
  element.style.background = 'rgba(0,0,0,0.5)';

  element.destroy = function () {
    var body = document.getElementsByTagName("body")[0];
    body.removeChild(this);
  }

  if(element.parent === null) {
    element.parent = document.getElementsByTagName("body")[0];
  }

  return element;

}

function newJPWindow(args={}) {

  if (!Object.keys(args).includes('parent')) {
      if (!Object.keys(args).includes('toplevel') || args['toplevel'] === false) {
          args.parent = document.getElementsByTagName("body")[0];
      }
  }

  var element = newJPPanel();

  element.style.visibility = "hidden";
  element.mask = null;
  element.ptop = newJPPanel({parent:element});
  element.ptitle = newJPLabel({parent:element.ptop})
  element.mainpanel = newJPPanel({parent:element});
  element.__defaultWidth = 100;
  element.__defaultHeight = 100;
  element.__resizable = true;

  element.__toplevel = false;
  element.__topbtns  = false;

  Object.defineProperty(element, 'toplevel', {
      set(value) {
          if (element.__toplevel === value) { return; }
          element.__toplevel = value;
          if (value === true) {
              element.mask = newJPMask({child:element});
              element.mask.style.visibility = 'hidden';
              element.ondestroy = function() {
                this.mask.destroy();
              };
              element.parent = element.mask;
          } else {
              element.parent = element.mask.parent;
              element.mask.destroy();
          }
      },
      get() { return element.__toplevel; }
  });

  Object.defineProperty(element, 'topbuttons', {
      set(value) {

          if (element.__topbtns === value) { return; }

          element.__topbtns = value;

          if (value === true) {

              element.botoes = newJPPanel({parent:element.ptop, inline:true});
              element.botoes.className = 'window-buttons';

              if(element.resizable) {
                element.btnmaxmin = newJPButton({parent:element.botoes, text:'+'});
                element.btnmaxmin.className = 'window-button minmax-button';
                element.btnmaxmin.window = element;
                element.btnmaxmin.onclick = function() {
                  if(this.window.maximinized()) {
                    this.window.minimize();
                    element.btnmaxmin.value = '+';
                  }else{
                    this.window.maximinize();
                    element.btnmaxmin.value = '-';
                  }
                }
              }

              element.btnclose = newJPButton({text:'x', parent:element.botoes});
              element.btnclose.className = 'window-button close-button';
              element.btnclose.window = element;
              element.btnclose.onclick = function() {
                if(this.window.onclose&&this.window.onclose()){
                  return;
                }
                this.window.destroy();
              }

          } else {

              element.ptop.removeChild(element.botoes);

          }
      },
      get() { return element.__topbtns; }
  });

  Object.defineProperty(element, 'title', {
      set(value) {
          element.ptitle.text = value;
      },
      get() { return element.ptitle.text; }
  });

  Object.defineProperty(element, 'resizable', {
      set(value) {
          if (element.__resizable === value) return;
          element.__resizable = value;
          if (element.__resizable) {
              element.style.width = element.__defaultWidth + 'px';
              element.style.height = element.__defaultHeight + 'px';
          } else {
              element.style.width = "";
              element.style.height = "";
              element.mainpanel.style.width = "";
              element.mainpanel.style.height = "";
          }
      },
      get() { return element.__resizable; }
  });

  element.topbuttons = true;

  element.toplevel = false;

  element.title = "Window";

  element.className = 'window';

  element.mainpanel.className = 'main-panel';

  with(element.mainpanel.style) {
    overflow = 'auto';
    margin = '0px 5px 5px 5px';
  };

  element.ptitle.className = 'title-panel';
  element.ptop.className = 'top-panel';
  element.ptitle.className = 'title';

  element.onresized = null;

  element.doresized = function() {
    this.updateSize();
    this.onresized && this.onresized(this.offsetWidth, this.offsetHeight);
  }

  element.maximinize = function() {
    if(this.maximinized()) return;
    this.style.width = '100%';
    this.style.height = '100%';
    this.style.left = '0px';
    this.style.top = '0px';
    this.doresized();
  }

  element.minimize = function() {
    if(!this.maximinized()) return;
    this.style.width = this.defaultWidth + 'px';
    this.style.height = this.defaultHeight + 'px';
    this.style.top = this.defaultTop + 'px';
    this.style.left = this.defaultLeft + 'px';
    this.doresized();
  }

  element.maximinized = function() {
    return this.style.width === '100%' && this.style.height === '100%';
  }

  element.geometry = function(x, y, width, height) {
    this.size(width, height);
    this.move(x, y);
  }

  element.updateSize = function() {
  }

  element.size = function(width, height) {
    if(this.maximinized()) return;
    if(this.offsetWidth===width && this.offsetHeight===height) return;
    this.__defaultHeight = height;
    this.__defaultWidth = width;
    this.style.width = width + 'px';
    this.style.height = height + 'px';
    this.doresized();
  }

  element.onmousedown = function(evt) {
    this.mdown = {offset:{x:evt.offsetX,y:evt.offsetY},
                  screen:{x:evt.screenX,y:evt.screenY},
                  left:this.offsetLeft,
                  top:this.offsetTop,
                  width:this.offsetWidth,
                  height:this.offsetHeight,
                  addWidth:function(x) {
                              var d = x - this.x;
                              return this.width + d;
                           },
                  addHeight:function(y) {
                              var d = y - this.y;
                              return this.height + d;
                           },
                  moving:this.ptop.style.cursor=='move'&&evt.buttons==MouseButtons.LEFT,
                  resizing:this.resizable&&(this.style.cursor=='se-resize'||this.style.cursor=='e-resize'||this.style.cursor=='s-resize')&&evt.buttons===MouseButtons.LEFT
                  };
    this.parentNode.selectedWindow = this;
  }

  element.onmouseup = function(){
    this.mdown = null;
    this.ptop.style.cursor = 'default';
  }

  element.onmousemove = function(evt) {
    if(this.mdown) return;
    intop = evt.y < this.offsetTop + this.ptop.offsetHeight;

    if(intop) {
      this.ptop.style.cursor = 'move';
      return;
    }else{
      if(this.ptop.style.cursor != 'default') this.ptop.style.cursor = 'default';
    }

    if(!this.resizable) {
      if(this.style.cursor != 'default') this.style.cursor = 'default';
      return;
    }

    var rx = this.offsetWidth - 10;
    var by = this.offsetHeight - 10;
    inborderx = rx < evt.offsetX && evt.offsetX < this.offsetWidth;
    inbordery = by < evt.offsetY && evt.offsetY < this.offsetHeight;
    if(!(inborderx||inbordery)) {
      this.style.cursor = 'default';
    }else if(inborderx&&inbordery) {
      this.style.cursor = 'se-resize';
    }else if(inborderx) {
      this.style.cursor = 'e-resize';
    }else if(inbordery) {
      this.style.cursor = 's-resize';
    }
  }

  element.ptop.onmouseup = function(e) {
      this.mdown = null;
  };
  /*=========================*/

  element.style.position = "absolute";

  //properties
  element.onclose = null;

  //methods and functions
  element.move = function(x, y) {
    if (y < 0) y = 0;
    if(this.maximinized()) return;
    this.defaultLeft = x;
    this.defaultTop = y;
    this.style.top = y + "px";
    this.style.left = x + "px";
  };

  element.show = function() {
    if(element.mask) element.mask.style.visibility = 'visible';
    this.style.visibility = 'visible';
  };

  element.alignCenter = function() {
    wmx = (this.parentNode.offsetWidth / 2).toFixed(0);
    wmy = (this.parentNode.offsetHeight / 2).toFixed(0);
    emx = (this.offsetWidth / 2).toFixed(0);
    emy = (this.offsetHeight / 2).toFixed(0);
    this.move(wmx - emx, wmy - emy);
  };

  element.appendChild = function(child) {
    element.mainpanel.appendChild(child);
  }

  element.setProperties(args);

  element.parentNode.onmousemove = function(evt) {
      if(!this.selectedWindow) return;
      var win = this.selectedWindow;
      if(!win.mdown) return;
      var dx = evt.screenX - win.mdown.screen.x;
      var dy = evt.screenY - win.mdown.screen.y;
      if(win.mdown.moving) {
        win.move(win.mdown.left+dx, win.mdown.top+dy);
      }else if(win.mdown.resizing) {
        if(win.style.cursor=='se-resize') {
          win.size(win.mdown.width + dx, win.mdown.height + dy);
        }else if(win.style.cursor=='e-resize') {
          win.size(win.mdown.width + dx, win.mdown.height);
        }else if(win.style.cursor=='s-resize') {
          win.size(win.mdown.width, win.mdown.height + dy);
        }
      }
  };

  element.parentNode.onmouseup = function(evt) {
      if(!this.selectedWindow) return;
      var win = this.selectedWindow;
      win.mdown = null;
      win.ptop.style.cursor = 'default';
  };

  return element;

}

function newJPDialog(args={}) {

    var element = newJPWindow({resizable:false, toplevel:true, topbuttons:false});

    element.className += " dialog";
    element.__content = newElement('div', {parent:element});
    element.__content.className = "content";
    element.__content.style.padding = '10px';
    element.__dbuttons = newJPPanel({parent:element, className:"buttons"});
    element.__exitinclick = false;

    Object.defineProperty(element, 'exitinclick', {
        set(value) {
            if (!element.toplevel) return;
            if (element.__exitinclick === value) { return; }
            element.__exitinclick = value;
            if (value === true) {
                element.mask.onclick = function (e) {
                    var child = this.child;
                    if(child.offsetLeft<e.clientX&&e.clientX<(child.offsetLeft+child.clientWidth)&&
                        child.offsetTop<e.clientY&&e.clientY<(child.offsetTop+child.clientHeight)){
                        return;
                    }
                    this.destroy();
                }
            } else {
              element.mask.onclick = null;
            }
        },
        get() { return element.__exitinclick; }
    });

    Object.defineProperty(element, 'buttons', {
      set(value) {
          element.__dbuttons.innerHTML = "";
          value.forEach(function (item, index) {
            element.__dbuttons.appendChild(item);
            item.window = element;
            if(item.result!==null){
                item.onclick = function() {
                    this.window.onresult && this.window.onresult(this.window, this.result);
                };
            }
          });
      },
      get() { return element.__dbuttons.childNodes; }
    });

    Object.defineProperty(element, 'content', {
      set(value) {
          element.setContent(value);
      },
      get() { return element.__content.childNodes.length > 0 ? element.__content.childNodes[0] : null; }
    });

    element.setContent = function(obj) {
        this.__content.innerHTML = '';
        if(obj) this.__content.appendChild(obj);
        this.alignCenter();
    }

    element.showModal = function(onresult){
        this.onresult = onresult;
        this.show();
    };

    element.onresized = function() {
        this.__content.style.width = (this.offsetWidth - 40) + 'px';
        this.__content.style.height = (this.offsetHeight - this.__dbuttons.offsetHeight - this.ptop.offsetHeight - 40) + 'px';
    }

    element.onresult = null;

    element.setProperties(args);

    element.alignCenter();

    return element;

}

function newDialogMessage(title, msg, onclose=null) {
  var bok = newJPButton({text:'Ok'});
  bok.result = ResponseResult.OK;
  var dlg001 =  newJPDialog({
      title:title,
      content:newJPLabel({text:msg}),
      buttons:[bok],
      exitinclick:true
  });
  dlg001.ondestroy = onclose;
  dlg001.showModal(function (sender, result) {sender.destroy();});
  return dlg001;
}

function newInput(width, props={}) {
  var self = newJPInput(props);
  self.style.width = width + "px";
  return self;
}
