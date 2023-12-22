
export function color2RGB(color) {
    var sColor = color.toLowerCase();
    var reg = /^#([0-9a-fA-f]{3}|[0-9a-fA-f]{6})$/;
    if (sColor && reg.test(sColor)) {
        if (sColor.length === 4) {
            var sColorNew = "#";
            for (var i=1; i<4; i+=1) {
                sColorNew += sColor.slice(i, i+1).concat(sColor.slice(i, i+1));    
            }
            sColor = sColorNew;
        }
        var sColorChange = [];
        for (var i=1; i<7; i+=2) {
            sColorChange.push(parseInt("0x"+sColor.slice(i, i+2))/255);    
        }
        return sColorChange;
    }
    return sColor;
}

export function RGB2Color(colorObj) {
    if(!colorObj) return '';
    const {r, g, b} = colorObj;
    let color = [r, g, b];
    let hex = "#";

    for (var i = 0; i < 3; i++) {
        hex += ("0" + Math.round(Number(color[i]) * 255).toString(16)).slice(-2);
    }
    return hex;
}

export function animate({timing, draw, duration}) {
    let start = performance.now();
    requestAnimationFrame(function animate(time) {
      // timeFraction 从 0 增加到 1
      let timeFraction = (time - start) / duration;
      if (timeFraction > 1) timeFraction = 1;
      // 计算当前动画状态
      let progress = timing(timeFraction);
      draw(progress); // 绘制
      if (timeFraction < 1) {
        requestAnimationFrame(animate);
      }
    });
  }