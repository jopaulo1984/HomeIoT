
var DashWidget = function(args={}) {
    var self = new GObject();
    self.setConfigs(args);
    return self;
};

var DashGauge = function(args={}) {
    var self = DashWidget({
        x: 50,
        y: 50,
        width: 100,
        height: 100,
        thickness: 20,
        background: "gray",
        valuecolor: "blue",
        value: 0,
        max: 100,
        min: 0,
        colorranges: [
            {min: -5, max: 5, color: "yellow"},
            {min: -10, max: -5, color: "red"},
            {min: 5, max: 10, color: "green"}
        ],
        label: "",
        font: {name:"Arial", size:11},
        prefix: "",
        sufix: "",
        draw: function (ctx) {
            //get bounds
            this.height = this.width;
            var r = this.width / 2;
            var cx = this.x + r;
            var cy = this.y + this.width;
            var a = (6.28 - 3.14) / (this.max - this.min);
            var b = this.width / 100;
            var thickness = this.thickness * b;
            var fsize = this.font.size * b;
            var fx = (a * this.value) - (a * this.min) + 3.14;
            var texty = this.y;

            this.height = this.width;

            //draw label
            ctx.beginPath();
            ctx.lineWidth = 1;
            ctx.fillStyle = "black";
            ctx.font = (fsize + 'px ' + this.font.name).replace('"','');
            for(i=0;i<2;i++) {
                ctx.fillText(this.label, this.x, texty);
            }

            //draw background arc
            ctx.beginPath();
            ctx.lineWidth = thickness;
            ctx.strokeStyle = "gray";
            ctx.arc(cx, cy, r, 3.14, 6.28);
            ctx.stroke();

            //draw arc value
            var arccolor = "blue";

            if (Array.isArray(this.colorranges)) {
                for (let index = 0; index < this.colorranges.length; index++) {
                    const element = this.colorranges[index];
                    if (element.min!=undefined && element.max!=undefined && element.color!=undefined) {
                        if (element.min <= this.value && this.value <= element.max) {
                            arccolor = element.color;
                            break;
                        }
                    }
                }
            }

            ctx.beginPath();
            ctx.lineWidth = thickness;
            ctx.strokeStyle = arccolor;
            ctx.arc(cx, cy, r, 3.14, fx);
            ctx.stroke();

            //draw label value
            ctx.beginPath();
            ctx.lineWidth = 1;
            ctx.fillStyle = "black";
            ctx.font = (fsize + 'px ' + this.font.name).replace('"','');
            for(i=0;i<2;i++) {
                ctx.fillText(this.prefix + this.value + this.sufix, this.x, texty + 30 * b);
            }

        }
    });
    self.setConfigs(args);
    return self;
};




