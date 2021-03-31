

var GObject = function(configs={}) {
    var self = {
        strokestyle: "black",
        fillstyle: "white",
        linewidth: 1,
        tags: "",
        draw: function(ctx) {},
        translate: function(dx, dy) {},
        bounds: function() {return {}},
        setConfigs: function(configs) {
            var self = this;
            Object.keys(configs).forEach(function(key) {
                self[key] = configs[key];
            });
        }       
    }
    self.setConfigs(configs);
    return self;
};


var GCanvas = function(width=480, height=320) {

    var self = document.createElement("canvas");

    self.width = width;
    self.height = height;
    
    self.gobjects = [];

    self.deleteAll = function() {
        this.gobjects = [];
        this.redraw();
    };
    
    self.redraw = function(){
        var ctx = this.getContext("2d");
        ctx.clearRect(0,0,this.offsetWidth, this.offsetHeight);
        this.gobjects.forEach(function(gitem){
            gitem.draw(ctx);
        });
    };

    self.deleteFromTags = function(tags) {
        var newarray = [];
        this.gobjects.forEach(function(gitem){
            if(gitem.tags != tags) newarray.push(gitem);
        });
        this.gobjects = newarray;
        this.redraw();
    };

    self.delete = function(item) {
        var newarray = [];
        this.gobjects.forEach(function(gitem){
            if(gitem !== item) newarray.push(gitem);
        });
        this.gobjects = newarray;
        this.redraw();
    };

    self.itemconfig = function(item, configs={}) {
        Object.keys(configs).forEach(function(key) {
            item[key] = configs[key];
        });
        this.redraw();
    };

    self.itemcget = function(item, config) {
        return item[config];
    };

    self.move = function (item, dx, dy) {
        item.translate(dx, dy);
        this.redraw();
    };

    self.appendGElement = function (gelement) {
        this.gobjects.push(gelement);
        gelement.draw(this.getContext("2d"));
    };
    
    self.createPolyline = function(points,configs={strokestyle:"black",linewidth:2,tags:""}) {
        var pl = GObject({
            points: points,
            draw: function(ctx) {
                if(!this.points) return;
                if(this.points.length===0) return;
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                ctx.strokeStyle = this.strokestyle;
                ctx.moveTo(this.points[0].x, this.points[0].y);
                for(var i=1;i<this.points.length;i++) {
                    ctx.lineTo(this.points[i].x, this.points[i].y);
                }
                for(i=0;i<2;i++) { ctx.stroke(); }
            },
            translate: function(dx, dy) {
                if(!this.points) return;
                for(var i=0;i<this.points.length;i++) {
                    this.points[i].x += dx;
                    this.points[i].y += dy;
                }
            },
            bounds: function() {
                if(!this.points) return {x1:0,x2:0,y1:0,y2:0};
                var p1 = this.points[0];
                if(this.points.length===0) {
                    return {
                        x1:p1.x,
                        x2:p1.x,
                        y1:p1.y,
                        y2:p1.y
                    };
                }
                var p2 = this.points[this.points.length-1];
                return {
                    x1: p1.x,
                    x2: p2.x,
                    y1: p1.y,
                    y2: p2.y
                };
            }
        });

        pl.setConfigs(configs);

        this.appendGElement(pl);
        
        return pl;
    };
    
    self.createLine = function(x1,y1,x2,y2,configs={strokestyle:"black",linewidth:2,tags:""}) {
        var gl = GObject({
            x1: x1,
            y1: y1,
            x2: x2,
            y2: y2,
            draw: function(ctx) {
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                ctx.strokeStyle = this.strokestyle;
                ctx.moveTo(this.x1, this.y1);
                ctx.lineTo(this.x2, this.y2);
                for(i=0;i<2;i++) { ctx.stroke(); }
            },
            translate: function(dx, dy) {
                this.x1 += dx;
                this.x2 += dx;
                this.y1 += dy;
                this.y2 += dy;
            },
            bounds: function() {
                return {
                    x1: this.x1,
                    x2: this.x2,
                    y1: this.y1,
                    y2: this.y2
                };
            }
        });

        gl.setConfigs(configs);

        this.appendGElement(gl);

        return gl;
    };

    self.createRectangle = function(x,y,w,h,configs={strokestyle:"black", fillstyle:"white",linewidth:1,tags:""}) {
        var gl = GObject({
            x: x,
            y: y,
            w: w,
            h: h,
            draw: function(ctx) {
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                if (this.fillstyle) ctx.fillStyle = this.fillstyle;
                if (this.strokestyle) ctx.strokeStyle = this.strokestyle;
                ctx.rect(this.x, this.y, this.w, this.h);
                if (this.fillstyle) ctx.fill();
                if (this.strokestyle) ctx.stroke();
            },
            translate: function(dx, dy) {
                this.x += dx;
                this.y += dy;
            },
            bounds: function() {
                return {
                    x: this.x,
                    y: this.y,
                    w: this.w,
                    h: this.h
                };
            }
        });

        gl.setConfigs(configs);

        this.appendGElement(gl);
        
        return gl;
    };

    self.createCircle = function (cx, cy, r, configs = { strokestyle: "black", fillstyle: "white", linewidth: 1, tags: "" }) {
        var gl = GObject({
            x: cx,
            y: cy,
            r: r,
            draw: function (ctx) {
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                if (this.fillstyle) ctx.fillStyle = this.fillstyle;
                if (this.strokestyle) ctx.strokeStyle = this.strokestyle;
                ctx.arc(this.x, this.y, this.r, 0, 6.28);
                if (this.fillstyle) ctx.fill();
                if (this.strokestyle) ctx.stroke();
            },
            translate: function (dx, dy) {
                this.x += dx;
                this.y += dy;
            },
            bounds: function () {
                var d = 2 * this.r;
                return {
                    x: this.x - r,
                    y: this.y - r,
                    cx: this.x,
                    cy: this.y,
                    r: this.r,
                    w: d,
                    h: d
                };
            }
        });

        gl.setConfigs(configs);

        this.appendGElement(gl);

        return gl;
    };

    self.createArc = function(cx, cy, r, angleStart, angleEnd, configs={strokestyle:"black", fillstyle:null, linewidth:1, tags:""}) {
        
        var gl = GObject ({
            x: cx,
            y: cy,
            r: r,
            start: angleStart,
            end: angleEnd,
            draw: function (ctx) {
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                if (this.fillstyle) ctx.fillStyle = this.fillstyle;
                if (this.strokestyle) ctx.strokeStyle = this.strokestyle;
                ctx.arc(this.x, this.y, this.r, this.start, this.end);
                if (this.fillstyle) ctx.fill();
                if (this.strokestyle) ctx.stroke();
            },
            translate: function (dx, dy) {
                this.x += dx;
                this.y += dy;
            },
            bounds: function () {
                return {
                    cx: this.x,
                    cy: this.y,
                };
            }
        });

        gl.setConfigs(configs);

        this.appendGElement(gl);

        return gl;

    };

    self.createText = function(x, y, configs={text: "", font: {name:"Arial", size:11}}) {
        var gl = GObject({
            x: x,
            y: y,
            font: {name: "Consolas", size: 11},
            text: "",
            fillstyle: "black",
            draw: function(ctx) {
                ctx.beginPath();
                ctx.lineWidth = this.linewidth;
                ctx.fillStyle = this.fillstyle;
                ctx.font = (this.font.size + 'px ' + this.font.name).replace('"','');
                for(i=0;i<2;i++) {
                    ctx.fillText(this.text,this.x,this.y);
                }
            },
            bounds: function() {
                return {
                    x: this.x,
                    y: this.y
                };
            }
        });

        gl.setConfigs(configs);

        this.appendGElement(gl);
        
        return gl;
    };
    
    return self;
}

