(function()
{
    const PROPERTY_HEIGHT = '20px';
    const PROPERTY_FONT_SIZE = '14px';

    /*************************************** property控件基类 ***************************************/
    let UIAttributeGrid = function(options) 
    {
        options = options ? options : {};
        mgs.UIBase.call(this, options);
    
        // elements
        this.header = new mgs.UIBase({styleClass: 'UIAttributeGrid-header'});
        this.append(this.header);
    
        this.headerFoldButton = new mgs.UIBase({styleClass: 'UIAttributeGrid-header-foldButton'});
        this.header.append(this.headerFoldButton);
        this.headerFoldButton.getRoot().addEventListener('click', this._onFoldButtonClick.bind(this), this);
    
        this.headerTitle = new mgs.UIBase({styleClass: 'UIAttributeGrid-header-title'});
        this.headerTitle.getRoot().innerHTML = options.title ? options.title : '';
        this.header.append(this.headerTitle);
    
        this.content = new mgs.UIBase({styleClass: 'UIAttributeGrid-content'});
        this.append(this.content);

        this.setCanFolded(options.bFold);
        this.setFold(options.bFold);

        this._ID_ = null;
        this._fullPath_ = null;
        this._needUpdate = false;
    };
    mgs.classInherit(UIAttributeGrid, mgs.UIBase);

    UIAttributeGrid.prototype._onFoldButtonClick = function(evt)
    {
        let bFold = this.content.getRoot().style.display === 'block';
        this.setFold(bFold);
    };

    UIAttributeGrid.prototype.setFold = function(bFold)
    {
        if (this.headerFoldButton.getRoot().style.display === 'none')
        {
            return;
        }

        this.content.getRoot().style.display = bFold ? 'none' : 'block';
        this.headerFoldButton.getRoot().innerHTML = bFold ? mgs.ICONS.propertyContainer_FolderButtonRight : mgs.ICONS.propertyContainer_FolderButtonDown;
    };

    UIAttributeGrid.prototype.setCanFolded = function(bCanFolded)
    {
        this.headerFoldButton.getRoot().style.display = bCanFolded ? 'inline-block' : 'none';
    };

    UIAttributeGrid.prototype.setTitle = function(title)
    {
        this.headerTitle.getRoot().innerHTML = title;
    };


    UIAttributeGrid.prototype.setValue = function(value)
    {
    };

    UIAttributeGrid.prototype.setID = function(id)
    {
        this._ID_ = id;
    };

    UIAttributeGrid.prototype.getID = function()
    {
        return this._ID_;
    };

    UIAttributeGrid.prototype.getNeedUpdate = function()
    {
        return this._needUpdate;
    };

    UIAttributeGrid.prototype.setNeedUpdate = function(eedUpdate)
    {
        this._needUpdate = eedUpdate;
    };

    UIAttributeGrid.prototype.getFullPath = function()
    {
        return this._fullPath_;
    };

    UIAttributeGrid.prototype.setFullPath = function(fullPath)
    {
        this._fullPath_ = fullPath;
    };

    UIAttributeGrid.prototype.showGrid = function(isShow)
    {
        this.getRoot().style.display = isShow ? 'block' : 'none';
    };

    /*************************************** property控件基类 ***************************************/
    //////////////// Panel ////////////////
    let UIAttributeGrid_Panel = function(options) 
    {
        options = options ? options : {};
        mgs.UIAttributeGrid.call(this, options);

        this.setCanFolded(true);
        this.setFold(false);
    };
    mgs.classInherit(UIAttributeGrid_Panel, mgs.UIAttributeGrid);

    //////////////// Array ////////////////
    let UIAttributeGrid_Array = function(options) 
    {
        options = options ? options : {};
        mgs.UIAttributeGrid_Panel.call(this, options);

        // size field
        let sizeOptions = 
        {
            numberOnly: true,
            min: 0,
            max: 1000,
            styleOptions:
            {
                'width': '45px',
                'height': PROPERTY_HEIGHT,
                'margin-right': '3px',
            },
        };

        this.sizeField = new mgs.UITextField(sizeOptions);
        this.sizeField.elementInput.getRoot().style['line-height'] = PROPERTY_HEIGHT;
        this.sizeField.elementInput.getRoot().style['font-size'] = PROPERTY_FONT_SIZE;
        this.header.append(this.sizeField);
    };
    mgs.classInherit(UIAttributeGrid_Array, mgs.UIAttributeGrid_Panel);

    //////////////// Label ////////////////
    let UIAttributeGrid_Label = function(options) 
    {
        options = options ? options : {};
        mgs.UIAttributeGrid.call(this, options);

        this.label = document.createElement('div'); 
        this.label.classList.add('UIAttributeGrid-label');
        this.label.innerHTML = 'abc';
        this.header.appendElement(this.label);
    };
    mgs.classInherit(UIAttributeGrid_Label, mgs.UIAttributeGrid);

    UIAttributeGrid_Label.prototype.setValue = function(value)
    {
        this.label.innerHTML = value;
    };

    //////////////// TextField ////////////////
    let UIAttributeGrid_TextField = function(options) 
    {
        options = options ? options : {};
        mgs.UIAttributeGrid.call(this, options);

        // text field
        let textOptions = options.textOptions;
        this.textField = new mgs.UITextField(textOptions);
        this.textField.getRoot().classList.add('UIAttributeGrid_TextField-root');
        this.textField.elementInput.getRoot().classList.add('UIAttributeGrid_TextField-input');
        this.header.append(this.textField);

        let self = this;
        this.textField.onChange = function(value)
        {
            self.emit('onChange', self.getID(), value);
        };
    };
    mgs.classInherit(UIAttributeGrid_TextField, mgs.UIAttributeGrid);

    UIAttributeGrid_TextField.prototype.setValue = function(value)
    {
        this.textField.setValue(value);
    };

    //////////////// Slider ////////////////
    let UIAttributeGrid_Slider = function(options) 
    {
        options = options ? options : {};
        mgs.UIAttributeGrid.call(this, options);

        this.label = document.createElement('div'); 
        this.label.classList.add('UIAttributeGrid-label');
        this.label.style.width = '10px';
        this.label.style.overflow = 'hidden';
        this.label.style['padding-right'] = '30px';
        this.label.innerHTML = '0';
        this.header.appendElement(this.label);

        let sliderOptions = options.sliderOptions;
        this.UISlider = new mgs.UISlider(sliderOptions);
        this.UISlider.getRoot().style.width = '50%';
        this.header.append(this.UISlider);

        this.value = 0;
        this.preValue = 0;
        let self = this;
        this.UISlider.on('onValueChange', function(value, isStart, isEnd)
        {
            if (isStart)
            {
                this.preValue = this.value;
                console.log(this.preValue);
            }
            let oldValue = isEnd ? this.preValue : undefined;
            this.value = value;
            self.label.innerHTML = this.value.toFixed(2);
            self.emit('onChange', self.getID(), this.value, !isEnd, oldValue);
        });
    };
    mgs.classInherit(UIAttributeGrid_Slider, mgs.UIAttributeGrid);

    UIAttributeGrid_Slider.prototype.setValue = function(value)
    {
        this.value = parseFloat(value);

        this.label.innerHTML = this.value.toFixed(2);

        let self = this;
        setTimeout(() => {
            self.UISlider.setValue(this.value);
        }, 0);
    };

    //////////////// vector ////////////////
    let createVectorTitle_TextField = function(gridHeader, title, count)
    {
        this.label = document.createElement('div'); 
        this.label.classList.add('UIAttributeGrid_VectorBase-scalarLabel');
        this.label.innerHTML = title;
        gridHeader.appendChild(this.label);

        let percent = 60 / count;
        let uiTextField = new mgs.UITextField
        (
            {
                numberOnly: true,
                styleOptions:
                {
                    width: percent.toString() + '%',
                }
            }
        );
        gridHeader.appendChild(uiTextField.getRoot());
        uiTextField.getRoot().classList.add('UIAttributeGrid_VectorBase-scalarText');
        uiTextField.elementInput.getRoot().classList.add('UIAttributeGrid_VectorBase-scalarText-input');

        return uiTextField;
    };

    let UIAttributeGrid_VectorBase = function(options)
    {
        options = options ? options : {};
        mgs.UIAttributeGrid.call(this, options);

        let fieldNames = options.fieldNames;

        this.uiTextFields = [];
        for (let i = 0;i < fieldNames.length;i ++)
        {
            this.uiTextFields.push(createVectorTitle_TextField(this.header.getRoot(), fieldNames[i], fieldNames.length));
        }

        // value
        this.value = [];
        for (let i = 0;i < fieldNames.length;i ++)
        {
            this.value.push[0];
        }

        // bind
        let self = this;
        let bindField = function(index)
        {
            self.uiTextFields[index].onChange = function(value)
            {
                self.value[index] = value;
                self.value[index] = self.value[index].toFixed(3);

                let outValue = [];
                for (let i = 0;i < fieldNames.length;i ++)
                {
                    outValue.push[0];
                }
                mgs.Util.arrayCopy(self.value, outValue);
                self.outValueTranslate(outValue);

                self.emit('onChange', self.getID(), outValue);
            };
        };

        for (let i = 0;i < this.uiTextFields.length;i ++)
        {
            bindField(i);
        }
    }
    mgs.classInherit(UIAttributeGrid_VectorBase, mgs.UIAttributeGrid);

    UIAttributeGrid_VectorBase.prototype.setValue = function(value)
    {
        mgs.Util.arrayCopy(value, this.value);
        this.inValueTranslate(this.value);

        for (let i = 0;i < this.uiTextFields.length;i ++)
        {
            this.value[i] = this.value[i].toFixed(3);
            this.uiTextFields[i].setValue(this.value[i]);
        }
    };

    UIAttributeGrid_VectorBase.prototype.outValueTranslate = function(value)
    {
    };

    UIAttributeGrid_VectorBase.prototype.inValueTranslate = function(value)
    {
    };

    let UIAttributeGrid_Vector2 = function(options) 
    {
        options = options ? options : {};
        options.fieldNames = ['x', 'y'];
        mgs.UIAttributeGrid_VectorBase.call(this, options);

    };
    mgs.classInherit(UIAttributeGrid_Vector2, mgs.UIAttributeGrid_VectorBase);


    let UIAttributeGrid_Vector3 = function(options) 
    {
        options = options ? options : {};
        options.fieldNames = ['x', 'y', 'z'];
        mgs.UIAttributeGrid_VectorBase.call(this, options);
    };
    mgs.classInherit(UIAttributeGrid_Vector3, mgs.UIAttributeGrid_VectorBase);

    let UIAttributeGrid_DegreeVector = function(options) 
    {
        options = options ? options : {};
        options.fieldNames = ['x', 'y', 'z'];
        mgs.UIAttributeGrid_VectorBase.call(this, options);
    };
    mgs.classInherit(UIAttributeGrid_DegreeVector, mgs.UIAttributeGrid_VectorBase);

    UIAttributeGrid_DegreeVector.prototype.outValueTranslate = function(value)
    {
        for (let i = 0;i < value.length;i ++)
        {
            value[i] = GASEngine.degToRad(value[i]);
        }
    };

    UIAttributeGrid_DegreeVector.prototype.inValueTranslate = function(value)
    {
        for (let i = 0;i < value.length;i ++)
        {
            value[i] = GASEngine.radToDeg(value[i]);
        }
    };
}());