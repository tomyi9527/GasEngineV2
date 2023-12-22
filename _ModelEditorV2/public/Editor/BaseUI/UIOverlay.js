(function ()
{
    let UIOverlay = function (options) 
    {
        mgs.UIBase.call(this, options);
        
        options = options ? options : {};

        // bind events
        this.getRoot().addEventListener('mousedown', this._onMouseDown.bind(this), false);
        // window.addEventListener('mousedown', self._onMouseDown.bind(this), false);
    };
    mgs.classInherit(UIOverlay, mgs.UIBase);

    UIOverlay.prototype._onMouseDown = function(evt)
    {        
        
        // if (evt.button === 0)
        // {
            this.show(false);
        // }
        // evt.preventDefault();
        // evt.stopPropagation();   

        let starter = document.elementFromPoint(evt.clientX, evt.clientY);
        if (starter)
        {
            var newEvt = document.createEvent("MouseEvents");
            newEvt.initMouseEvent(
            "mousedown", 
            true, // evt.cancelBubble, 
            evt.cancelable, 
            evt.view, 
            evt.detail, 
            evt.screenX, 
            evt.screenY, 
            evt.clientX, 
            evt.clientY, 
            evt.ctrlKey, 
            evt.altKey, 
            evt.shiftKey,
            evt.metaKey, 
            evt.button, 
            evt.relatedTarget);
            starter.dispatchEvent(newEvt);
        }
    };

    UIOverlay.prototype.show = function(bShow)
    {
        if (bShow)
        {
            this.getRoot().style.display = 'block';
        }
        else
        {
            this.getRoot().style.display = 'none';
        }

        if (this.onShow)
        {
            this.onShow(bShow);
        }
    };
}());