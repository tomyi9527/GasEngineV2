(function ()
{
    /************************************************************
    UILayout：
        1，内容默认为display:flex
        2，width/height默认为auto
        3，position默认为relative
        4，flex-grow默认为1

    custom：
        1，宽高固定的layout可以单独设置width/height
    ************************************************************/

    let UILayout = function (options) 
    {
        mgs.UIBase.call(this, options);
        
        options = options ? options : {};
        let styleOptions = options.styleOptions;
        if (styleOptions)
        {
            if (!styleOptions['flex-grow'])
            {
                if (styleOptions.width || styleOptions.height)
                {
                    this.getRoot().style['flex-grow'] = 0;
                }
            }
        }

        this.bEnableHandler = true;
        this._processHandlerOption(options.handlerOption, options.handlerStyleOption);
    };
    mgs.classInherit(UILayout, mgs.UIBase);

    UILayout.prototype._processHandlerOption = function(handlerOption, handlerStyleOption)
    {
        if (handlerOption || handlerStyleOption)
        {
            this.handler =  new mgs.UIBase({styleClass:'UILayout-handler'});
            this.append(this.handler);
        }

        if (handlerOption)
        {
            let handler = this.handler.getRoot();
            let panel = this.getRoot();
            let handlerDir = handlerOption.direction;

            // auto change style
            if (handlerDir === 'left' || handlerDir === 'right')
            {
                // auto change style
                handler.style.height = 'auto';
                if (handlerDir === 'right')
                {
                    handler.style.left = 'auto';
                }
                else
                {
                    handler.style.right = 'auto';
                }

                handler.style.cursor = 'ew-resize';
            }
            else if (handlerDir === 'top' || handlerDir === 'bottom')
            {
                handler.style.width = 'auto';
                if (handlerDir === 'bottom')
                {
                    handler.style.top = 'auto';
                }
                else
                {
                    handler.style.bottom = 'auto';
                }

                handler.style.cursor = 'ns-resize';
            }

            let _resizeMove = function(evt)
            {
                if (! handler._resizeData) 
                {
                    handler._resizeData = 
                    {
                        x: evt.clientX,
                        y: evt.clientY,
                        width: panel.clientWidth,
                        height: panel.clientHeight
                    };
                } 
                else if (handlerDir === 'left' || handlerDir === 'right')
                {
                    var offsetX = handler._resizeData.x - evt.clientX;

                    if (handlerDir === 'right')
                    {
                        offsetX = -offsetX;
                    }

                    var width = handler._resizeData.width + offsetX; // Math.max(this._resizeLimits.min, Math.min(this._resizeLimits.max, (this._resizeData.width + offsetX)));

                    if (handlerOption.max)
                    {
                        width = Math.min(handlerOption.max, width);
                    }
                    if (handlerOption.min)
                    {
                        width = Math.max(handlerOption.min, width);
                    }
                    
                    panel.style.width = width + 'px';
                }
                else if (handlerDir === 'top' || handlerDir === 'bottom')
                {
                    var offsetY = handler._resizeData.y - evt.clientY;

                    if (handlerDir === 'bottom')
                    {
                        offsetY = -offsetY;
                    }

                    var height = handler._resizeData.height + offsetY; // Math.max(this._resizeLimits.min, Math.min(this._resizeLimits.max, (this._resizeData.width + offsetX)));
                    if (handlerOption.max)
                    {
                        height = Math.min(handlerOption.max, height);
                    }
                    if (handlerOption.min)
                    {
                        height = Math.max(handlerOption.min, height);
                    }

                    panel.style.height = height + 'px';
                }
            };

            let _resizeEnd = function(evt)
            {
                window.removeEventListener('mousemove', _resizeMove, false);
                window.removeEventListener('mouseup', _resizeEnd, true);
                // window.removeEventListener('touchmove', _resizeMove, false);
                // window.removeEventListener('touchend', _resizeEnd, false);

                handler.classList.remove('UILayout-handler-active');
                handler._resizeData = null;
                panel.style.transition = handler._panelOriginTransition;
            };

            let _resizeStart = function(evt)
            {
                window.addEventListener('mousemove', _resizeMove, false);
                window.addEventListener('mouseup', _resizeEnd, true);
                // window.addEventListener('touchmove', _resizeMove, false);
                // window.addEventListener('touchend', _resizeEnd, false);

                handler.classList.add('UILayout-handler-active');
                handler._resizeData = null;
                handler._panelOriginTransition = panel.style.transition;
                panel.style.transition = 'width 0s, height 0s';

                evt.preventDefault();
            };

            handler.addEventListener('mousedown', _resizeStart, false);
            // handler.addEventListener('touchstart', _resizeStart, false);
        }

        if (handlerStyleOption)
        {
            for (let key in handlerStyleOption)
            {
                this.handler.style[key] = handlerStyleOption[key];
            }
        }
    };
}());