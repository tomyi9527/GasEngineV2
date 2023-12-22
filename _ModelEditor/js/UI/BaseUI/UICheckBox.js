(function ()
{
    let UICheckbox = function(options) 
    {
        mgs.UIBase.call(this, options);
        
        let root = this.getRoot();

        // set options
        options = options ? options : {};

        this.disabled = false;

        // bind events
        root.addEventListener('click', this._onClick.bind(this), false);  
    };
    mgs.classInherit(UICheckbox, mgs.UIBase);

    UICheckbox.prototype._onClick = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();
        
        if(this.disabled) return;

        this.setValue(!this.checked);
        if (this.onChange)
        {
            this.onChange(this.checked);
        }  
    };

    UICheckbox.prototype.setValue = function(value)
    {
        let root = this.getRoot();

        this.checked = value;

        if (this.checked)
        {
            root.classList.add('checked');
            root.innerHTML = mgs.ICONS.checkbox;
        }
        else
        {
            root.classList.remove('checked');
            root.innerHTML = '';
        }
    };

    UICheckbox.prototype.setDisabled = function(disabled)
    {
        let root = this.getRoot();

        this.disabled = disabled;

        if (this.disabled)
        {
            root.classList.add('disabled');
        }
        else
        {
            root.classList.remove('disabled');
        }
    }

}());