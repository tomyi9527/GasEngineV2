(function ()
{
    let UISlider = function(options) 
    {
        mgs.UIBase.call(this, options);

        // create element
        this.bar = new mgs.UIBase({styleClass:'UISlider-bar'});
        this.append(this.bar);

        this.handler = new mgs.UIBase({styleClass:'UISlider-handler'});
        this.append(this.handler);

        // set options
        options = options || {};
        this.precision = options.precision ? options.precision : 2;
        this._min = options.min ? options.min : 0;
        this._max = options.max ? options.max : 1;
    
        this.value = 0;

        // set custom style
        this.processStyleOptions(options);

        // bind events
        let self = this;
    
        let _onMouseMove = function(evt) 
        {
            evt.stopPropagation();
            evt.preventDefault();
    
            self._onHandlerMove(evt.pageX);
        };
    
        let _onMouseUp = function(evt) 
        {
            window.removeEventListener('mousemove', _onMouseMove);
            window.removeEventListener('mouseup', _onMouseUp);
    
            evt.stopPropagation();
            evt.preventDefault();
    
            self.handler.getRoot().classList.remove('active');
            self._onHandlerMove(evt.pageX, false, true);
        };

        let _onMouseDown = function(evt)
        {
            window.addEventListener('mousemove', _onMouseMove, false);
            window.addEventListener('mouseup', _onMouseUp, false);

            evt.stopPropagation();
            evt.preventDefault();
    
            self.handler.getRoot().classList.add('active');

            self.emit('onValueChange', self.value, true, false);
            self._onHandlerMove(evt.pageX);
        };

        let root = this.getRoot();
        root.addEventListener('mousedown', _onMouseDown, false);
    };
    mgs.classInherit(UISlider, mgs.UIBase);

    UISlider.prototype._onHandlerMove = function(pageX, isStart, isEnd)
    {
        let root = this.getRoot();

        let sliderRect = root.getBoundingClientRect();
        let handlerRect = this.handler.getRoot().getBoundingClientRect();

        let fullWidth = sliderRect.width - handlerRect.width;

        let x = Math.max(0, Math.min(1, (pageX - sliderRect.left) / fullWidth));
        let range = this._max - this._min;
        let value = x * range + this._min;
        value = parseFloat(value.toFixed(this.precision), 10);
        // this.value = value;

        // this.handler.getRoot().style.left = (x * fullWidth/sliderRect.width * 100) + '%';
        this.setValue(value);

        this.emit('onValueChange', this.value, isStart, isEnd);
    }

    UISlider.prototype.setValue = function(value)
    {
        let root = this.getRoot();
        let sliderRect = root.getBoundingClientRect();
        let handlerRect = this.handler.getRoot().getBoundingClientRect();
        let fullWidth = sliderRect.width - handlerRect.width;

        this.value = value;
        this.handler.getRoot().style.left = (value * fullWidth/sliderRect.width * 100) + '%';
    }
}());