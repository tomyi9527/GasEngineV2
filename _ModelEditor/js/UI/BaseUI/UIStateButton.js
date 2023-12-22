(function ()
{
    let UIStateButton = function(options) 
    {
        mgs.UIBase.call(this, options);
        
        let root = this.getRoot();

        // set options
        options = options ? options : {};
        root.innerHTML = options.text || '';
        this.setSelected(options.selected);

        // bind events
        root.addEventListener('click', this._onClick.bind(this), false);
        
    };
    mgs.classInherit(UIStateButton, mgs.UIBase);

    UIStateButton.prototype._onClick = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();
    
        this.selected = !this.selected;
        this.setSelected(this.selected);
    };

    UIStateButton.prototype.setSelected = function(selected, ignoreTrigger)
    {
        let root = this.getRoot();

        this.selected = selected;
        if (this.selected)
        {
            root.classList.add('selected');
        }
        else
        {
            root.classList.remove('selected');
        }

        if (!ignoreTrigger && this.onStateChange)
        {
            this.onStateChange(this, this.selected);
        }
    }
}());