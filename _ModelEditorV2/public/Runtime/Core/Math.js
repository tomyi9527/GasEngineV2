(function()
{
    let _Math = {};

    ///////////////////////////// 矩形相交 /////////////////////////////
    _Math.lineToRect = function(lineBegin, lineEnd) 
    {
        let minX = Math.min(lineBegin.x, lineEnd.x);
        let minY = Math.min(lineBegin.y, lineEnd.y);
        let maxX = Math.max(lineBegin.x, lineEnd.x);
        let maxY = Math.max(lineBegin.y, lineEnd.y);

        let rect = {};
        rect.left = minX;
        rect.width = maxX - minX;
        rect.top = minY;
        rect.height = maxY - minY;

        return rect;
    };
    
    _Math.intersectRect = function(r1, r2) 
    {
        return !(r2.left > r1.left + r1.width || 
                 r2.left + r2.width < r1.left || 
                 r2.top > r1.top + r1.height ||
                 r2.top + r2.height < r1.top);
    };

    _Math.pointInRect = function(p, r) 
    {
        return (p.x >= r.left && p.x <= r.left + r.width) &&
                (p.y >= r.top && p.y <= r.top + r.height);
    };

    ///////////////////////////// 差集、交集、并集 /////////////////////////////
    let defaultValueHandler = function(e)
    {
        return e;
    };

    // 两集合相减
    _Math.subtractSet = function(a1, a2, valueHandler)
    {
        valueHandler = valueHandler ? valueHandler : defaultValueHandler;
        let arr = a1.filter(function(e)
        {
            let len = a2.length;
            for (let i = 0;i < len;i ++)
            {
                if (valueHandler(e) === valueHandler(a2[i]))
                {
                    return false;
                }
            }

            return true;
        });

        return arr;
    };

    // 差集
    _Math.differenceSet = function(a1, a2, valueHandler)
    {
        valueHandler = valueHandler ? valueHandler : defaultValueHandler;
        let out1 = this.subtractSet(a1, a2, valueHandler);
        let out2 = this.subtractSet(a2, a1, valueHandler);
        return out1.concat(out2);
    };

    // 交集
    _Math.commonSet = function(a1, a2, valueHandler)
    {
        valueHandler = valueHandler ? valueHandler : defaultValueHandler;
        let out = a1.filter(function(e)
        {
            let len = a2.length;
            for (let i = 0;i < len;i ++)
            {
                if (valueHandler(e) === valueHandler(a2[i]))
                {
                    return true;
                }
            }

            return false;
        });

        return out;
    };

    // 并集
    _Math.intersectionSet  = function(a1, a2, valueHandler)
    {
        valueHandler = valueHandler ? valueHandler : defaultValueHandler;
        let out1 = this.subtractSet(a1, a2, valueHandler);
        let out2 = this.commonSet(a1, a2, valueHandler);
        let out3 = this.subtractSet(a2, a1, valueHandler);
        return out1.concat(out2).concat(out3);
    };

    // 相等
    _Math.isSetEqual  = function(a1, a2, valueHandler)
    {
        valueHandler = valueHandler ? valueHandler : defaultValueHandler;
        let diff = _Math.differenceSet(a1, a2, valueHandler);
        return diff.length === 0;
    };
    
    mgs.assign('Math', _Math);

}());

