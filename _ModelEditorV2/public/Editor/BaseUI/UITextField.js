(function ()
{
    let UITextField = function(options) 
    {
        options = options || {};
        mgs.UIBase.call(this, options);

        let root = this.getRoot();

        // create element
        this.elementInput = new mgs.UIBase({elementTag: 'input', styleClass: 'UITextField-input'});
        
        this.append(this.elementInput);
        this._elementInputRoot = this.elementInput.getRoot();

        if (options.title)
        {
            this.elementTitle = new mgs.UIBase({elementTag: 'div', styleClass: 'UITextField-title', innerHTML: options.title});
            this.append(this.elementTitle);

            root.addEventListener('mouseenter', this._onMouseEnter.bind(this), false);
            root.addEventListener('mouseleave', this._onMouseLeave.bind(this), false);
        }

        // set options
        this._elementInputRoot.placeholder = options.placeholder ? options.placeholder : "";
        this.numberOnly = options.numberOnly;
        this._min = options.min;
        this._max = options.max;

        let value = options.text ? options.text : "";
        this.setValue(value);

        // bind events
        this._elementInputRoot.addEventListener('change', this._onChange.bind(this), false);
    };
    mgs.classInherit(UITextField, mgs.UIBase);

    UITextField.prototype._onKeyDown = function(evt)
    {
        // evt.stopPropagation();
        let isCtrlDown = evt.ctrlKey || evt.metaKey;
        if ((isCtrlDown && evt.keyCode == 90) || (isCtrlDown && evt.keyCode == 89)) 
        {
            evt.preventDefault();
            return false;
        }
    };

    UITextField.prototype._getValidValue = function(value)
    {
        let validValue = value;
        if (this.numberOnly)
        {
            validValue = parseFloat(validValue);
            if (isNaN(validValue))
            {
                validValue = 0;
            }

            if (this._min !== undefined)
            {
                validValue = Math.max(this._min, validValue);
            }

            if (this._max !== undefined)
            {
                validValue = Math.min(this._max, validValue);
            }
        }
        return validValue;
    };

    UITextField.prototype.setValue = function(value)
    {
        this.value = this._getValidValue(value);
        if (this._elementInputRoot.value !== this.value.toString())
        {
            this._elementInputRoot.value = this.value;
        }
    }

    UITextField.prototype._onMouseEnter = function(evt)
    {
        this.elementTitle.getRoot().style.display = 'none';
    };

    UITextField.prototype._onMouseLeave = function(evt)
    {
        this.elementTitle.getRoot().style.display = 'inline-block';
    };

    UITextField.prototype._onChange = function(evt)
    {
        
        let validValue = this._getValidValue(this._elementInputRoot.value);
        if (this._elementInputRoot.value !== validValue.toString())
        {
            this._elementInputRoot.value = validValue;
        }

        if (this.value !== validValue)
        {
            // this.value = validValue;
            
            if (this.onChange)
            {   
                let value = this.numberOnly ? parseFloat(this._elementInputRoot.value) : this._elementInputRoot.value;
                this.onChange(value);
            }
        }
    };
}());