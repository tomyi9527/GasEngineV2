(function ()
{
    const PROPERTY_HEIGHT = '15px';
    const PROPERTY_FONT_SIZE = '12px';

    let UIVector3 = function(options) 
    {
        let tmpVec3 = new GASEngine.Vector3(0,0,0);
        let baseOptions = {
            styleOptions:
            {
                display: 'flex'
            },
        }
        mgs.UIBase.call(this, baseOptions);
        
        this.propertyArr = [];
        // this.propertyMap = {};

        let labelOptions = 
        {
            elementTag: 'span',
            innerHTML: options.innerHTML,
            styleOptions:
            {
                width: '10%',
                display: 'inline-block',
                // 'min-width': '60px'
            },
        };
        var typeSpan = new mgs.UIBase(labelOptions);
        this.append(typeSpan);


        let labelStyleObj = 
        {
            width: '10%',
            display: 'inline-block',
            // 'min-width': '60px'
        };
    
        let textFieldSyleObj = 
        {
            width: '60%',
            overflow: 'hidden',
            // 'min-width': '120px'
        };


        var vector = options.vector;
        var optionsX = {
            innerHTML: 'x',
            id: options.id + '/x',
            text: vector.x.toFixed(3),
            // setFun: function(xStr)
            // {
            //     var x = parseFloat(xStr);
            //     tmpVec3.set(x, vector.y, vector.z);
            //     options.setFun(tmpVec3);
            // },
            lableStyle: labelStyleObj,
            textFieldSyle: textFieldSyleObj
        }
        var itemX = new mgs.UIPropertyItem(optionsX);
        this.propertyArr.push(itemX.getTextField());
        // this.propertyMap['x'] = itemX.getTextField();
        var optionsY = {
            innerHTML: 'y',
            id: options.id + '/y',
            text: vector.y.toFixed(3),
            // setFun: function(yStr)
            // {
            //     var y = parseFloat(yStr);
            //     tmpVec3.set(vector.x, y, vector.z);
            //     options.setFun(tmpVec3);
            // },
            lableStyle: labelStyleObj,
            textFieldSyle: textFieldSyleObj
        }
        var itemY = new mgs.UIPropertyItem(optionsY);
        this.propertyArr.push(itemY.getTextField());
        // this.propertyMap['y'] = itemY.getTextField();
        var optionsZ = {
            innerHTML: 'z',
            text: vector.z.toFixed(3),
            id: options.id + '/z',
            // setFun: function(zStr)
            // {
            //     var z = parseFloat(zStr);
            //     tmpVec3.set(vector.x, vector.y, z);
            //     options.setFun(tmpVec3);
            // },
            lableStyle: labelStyleObj,
            textFieldSyle: textFieldSyleObj
        }
        var itemZ = new mgs.UIPropertyItem(optionsZ);
        this.propertyArr.push(itemZ.getTextField());
        // this.propertyMap['z'] = itemZ.getTextField();
        this.append(itemX);
        this.append(itemY);
        this.append(itemZ);
    };
    mgs.classInherit(UIVector3,  mgs.UIBase);

    UIVector3.prototype.getTextField = function()
    {
        // return this.propertyMap;
        return this.propertyArr;
    }

}());