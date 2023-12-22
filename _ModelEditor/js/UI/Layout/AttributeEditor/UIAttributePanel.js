(function ()
{
    let UIAttributePanel = function(options) 
    {
        mgs.UIPanel.call(this, options);  
        this.propertyMap = {};
    };
    mgs.classInherit(UIAttributePanel, mgs.UIPanel);

    UIAttributePanel.prototype.bindData = function(dEntity)
    {
        this.dEntity = dEntity;
        this.contentClear();
        if(!this.dEntity) return;
        var content = this.setProperty_r(this.dEntity);
        this.contentAppend(content);

        let addBtn = new mgs.UIButton
        (
            {
                styleOptions:
                {
                    width:'40%',
                    margin: '15px 30%',
                },
                text: '添加组件'
            }
        );
        this.contentAppend(addBtn);
        
    };
    
    let panelOptions = 
    {
        childStyleOptions:
        {
            content:
            {
                position: 'relative',
                top: '0px',
                margin: '5px 0',
                display: 'flex',
                'flex-direction':'column',
            },
            header:
            {
                height:'25px',
            },
        },
        foldOption:
        {
            horizontal:true
        }
    };

    UIAttributePanel.prototype.getItemByPropertyID = function(path)
    {
        let pathArr = path.split('/');
        let dom;
        if(pathArr.length === 1)
        {
            dom = this.propertyMap[pathArr[0]];
        }
        else if(pathArr.length === 2)
        {
            doms = this.propertyMap[pathArr[0]];
            dom = pathArr[1] === 'x' ? doms[0] :  pathArr[1] === 'y' ? doms[1] : doms[2];
        }
        return dom;
    };

    UIAttributePanel.prototype.setProperty_r = function(dEntity, panelFlag = false, title)
    {
        if(!dEntity) return null;
        var panelOpt = panelOptions;
        panelOpt.text = title;
        var content = panelFlag ? new mgs.UIPanel(panelOpt) : new mgs.UIBase();
        var props = dEntity.getProperties();
        var propCount = props.length;
        for(var i = 0; i < propCount; i++)
        {
            var prop = props[i];
            var line = this.printPropertyValue(prop);
            if(!line) continue;
            if(panelFlag)
            {
                content.contentAppend(line);
            }
            else 
            {
                content.append(line);
            }
        }
        return content;
    };

    UIAttributePanel.prototype.printPropertyValue = function(property)
    {
        var name = property.displayName;
        var type = property.type;
        var uiType = property.showMode === 'view' ? property.viewType : property.editorType;
        uiType = uiType || 'Span';
        var value, valueContainer;
        switch(type)
        {
            case 'string':
                value = property.get();
                var options = {
                    innerHTML: name,
                    id: property.id,
                    text: value,
                    setFun: property.set
                }
                var item = new mgs.UIPropertyItem(options);
                this.propertyMap[property.id] = item.getTextField();
                return item;
            case 'number':
                value = property.get().toFixed(3);
                var options = {
                    innerHTML: name,
                    id: property.id,
                    text: value,
                    setFun: property.set
                }
                var item = new mgs.UIPropertyItem(options);
                this.propertyMap[property.id] = item.getTextField();
                return item;
            case 'vector3':
                var vec = property.get();
                if(property.id === 'rotation')
                {
                    vec.x = GASEngine.radToDeg(vec.x);
                    vec.y = GASEngine.radToDeg(vec.y);
                    vec.z = GASEngine.radToDeg(vec.z);
                }
                var options = {
                    vector: vec,
                    isVector3: true,
                    id: property.id,
                    innerHTML: name,
                    setFun: property.set
                }
                var item = new mgs.UIVector3(options);
                this.propertyMap[property.id] = item.getTextField();
                return item;
            case 'array':
                var panelOpt = panelOptions;
                panelOpt.id = property.id;
                panelOpt.text = name;
                valueContainer = new mgs.UIPanel(panelOpt);
                var length = property.getElementCount();
                if(length === 0) return false;
                for(var i =0; i< length;i++)
                {
                    var child = property.get(i);
                    var container = this.setProperty_r(child, true, '子' + name + (i + 1));
                    if(container)
                    {
                        valueContainer.contentAppend(container);
                    }
                }
                var line = new mgs.UIBase();
                line.append(valueContainer);
                return line;
            case 'map':
                var panelOpt = panelOptions;
                panelOpt.id = property.id;
                panelOpt.text = name;
                valueContainer = new mgs.UIPanel(panelOpt);
                var types = property.getComponentTypes();
                for(var t = 0; t < types.length; t ++)
                {
                    var component = property.getComponent(types[t]);
                    var container = this.setProperty_r(component, true, types[t]);
                    if(container)
                    {
                        valueContainer.contentAppend(container);
                    }
                }
                var line = new mgs.UIBase();
                line.append(valueContainer);
                return line;
            default:
                value = '';
                return false;
        }
        
        // if(!isFolder)
        // {
        //     var labelOpt = labelOptions;
        //     labelOpt.innerHTML = name;
        //     var typeSpan = new mgs.UIBase(labelOpt);
        //     line.append(typeSpan);
        //     var textFiledOpt = textFieldOptions;
        //     textFiledOpt.id = 'assetbar-' + property.id;
        //     textFiledOpt.text = value;
        //     textFiledOpt.elementTag = uiType.toLowerCase();
        //     valueContainer = new mgs.UITextField(textFiledOpt);
        //     valueContainer.onChange = function(value)
        //     {
        //         property.set(value);
        //     }
        // }
        // line.append(valueContainer);
        // return line;
    };
}());