(function ()
{
    let UIButton = function(options) 
    {
        mgs.UIBase.call(this, options);
        
        let root = this.getRoot();

        // set options
        options = options ? options : {};
        root.innerHTML = options.text || '';

        // bind events
        root.addEventListener('click', this._onClick.bind(this), false);  
    };
    mgs.classInherit(UIButton, mgs.UIBase);

    UIButton.prototype._onClick = function(evt)
    {
        evt.preventDefault();
        evt.stopPropagation();

        this.emit('onClick', evt);
    };
}());