GASEngine.StandardMouseKeyboard = function(manipulator)
{
    this._enable = true;
    this._manipulator = manipulator;
    this._manipulator.setControllerType('StandardMouseKeyboard');

    this._mouseEventNode = undefined;
    this._wheelEventNode = undefined;
    this._keyboardEventNode = undefined;
    this._eventList = ['mousedown', 'mouseup', 'mouseout', 'mousemove', 'dblclick'];
    this._mousePosition = new GASEngine.Vector2();
};

GASEngine.StandardMouseKeyboard.prototype =
{
    setEnable: function(bool)
    {
        this._enable = bool;
        this._manipulator.setControllerType('StandardMouseKeyboard');
        var controller = this.getManipulatorController();
        controller.reset();
    },

    getEnable: function()
    {
        return this._enable;
    },

    init: function(options)
    {
        var mouse = options.mouseEventNode;

        var mousewheel = options.wheelEventNode || mouse;
        var keyboard = options.keyboardEventNode || mouse;

        //if(options.getBoolean('scrollwheel') === false)
        //    mousewheel = null;

        this._mouseEventNode = mouse;
        this._wheelEventNode = mousewheel;
        this._keyboardEventNode = keyboard;

        // to remove listeners, you need the exact same callback
        this._callbacks =
        {
            mousedown: this.mousedown.bind(this),
            mouseup: this.mouseup.bind(this),
            mouseout: this.mouseout.bind(this),
            mousemove: this.mousemove.bind(this),
            dblclick: this.dblclick.bind(this),
            mousewheel: this.mousewheel.bind(this),
            preventDefault: this.preventDefault.bind(this),
            keydown: this.keydown.bind(this),
            keyup: this.keyup.bind(this)
        };

        this.addEventListeners();
    },

    _addOrRemoveEventListeners: function(remove)
    {
        var callbacks = this._callbacks;
        var cb = remove ? 'removeEventListener' : 'addEventListener';
        var cbMouse = this._mouseEventNode && this._mouseEventNode[cb].bind(this._mouseEventNode);
        var cbMousewheel = this._wheelEventNode && this._wheelEventNode[cb].bind(this._wheelEventNode);
        var cbKeyboard = this._keyboardEventNode && this._keyboardEventNode[cb].bind(this._keyboardEventNode);

        if(cbMouse)
        {
            for(var i = 0, l = this._eventList.length; i < l; i++)
            {
                var ev = this._eventList[i];
                cbMouse(ev, callbacks[ev], false);
            }
        }

        if(cbMousewheel)
        {
            cbMousewheel('DOMMouseScroll', callbacks.mousewheel, false);
            cbMousewheel('mousewheel', callbacks.mousewheel, false);
            cbMousewheel('MozMousePixelScroll', callbacks.preventDefault, false);
        }

        if(cbKeyboard)
        {
            cbKeyboard('keydown', callbacks.keydown, true);
            cbKeyboard('keyup', callbacks.keyup, true);
        }
    },

    addEventListeners: function()
    {
        return this._addOrRemoveEventListeners(false);
    },

    removeEventListeners: function()
    {
        return this._addOrRemoveEventListeners(true);
    },

    isValid: function()
    {
        if(this._enable && this._manipulator && this._manipulator.getCurrentController())
        {
            return true;
        }

        return false;
    },

    getManipulatorController: function()
    {
        return this._manipulator.getCurrentController();
    },

    keyup: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().keyup)
        {
            return this.getManipulatorController().keyup(ev);
        }

        return undefined;
    },

    keydown: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().keydown)
        {
            return this.getManipulatorController().keydown(ev);
        }

        return undefined;
    },

    mousedown: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().mousedown)
        {
            // prevent browser to enter in scroll mode
            if(ev.button === 1)
            {
                ev.preventDefault();
            }

            return this.getManipulatorController().mousedown(ev);
        }

        return undefined;
    },

    mouseup: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().mouseup)
        {
            return this.getManipulatorController().mouseup(ev);
        }        

        return undefined;
    },

    mouseout: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().mouseout)
        {
            return this.getManipulatorController().mouseout(ev);
        }

        return undefined;
    },

    mousemove: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().mousemove)
        {
            return this.getManipulatorController().mousemove(ev);
        }

        return undefined;
    },

    dblclick: function(ev)
    {
        if(this.isValid() && this.getManipulatorController().dblclick)
        {
            return this.getManipulatorController().dblclick(ev);
        }

        return undefined;
    },

    mousewheel: function(event)
    {
        if(!this.isValid())
            return undefined;

        var manipulatorAdapter = this.getManipulatorController();

        if(!manipulatorAdapter.mousewheel)
            return undefined;

        event.preventDefault();

        // from jquery
        var orgEvent = event || window.event;
        var args = [].slice.call(arguments, 1);
        var delta = 0;
        var deltaX = 0;
        var deltaY = 0;

        // Old school scrollwheel delta
        if(event.wheelDelta)
        {
            delta = event.wheelDelta / 120;
        }

        if(event.detail)
        {
            delta = -event.detail / 3;
        }

        // New school multidimensional scroll (touchpads) deltas
        deltaY = delta;

        // Gecko
        if(orgEvent.axis !== undefined && orgEvent.axis === orgEvent.HORIZONTAL_AXIS)
        {
            deltaY = 0;
            deltaX = -1 * delta;
        }

        // Webkit
        if(orgEvent.wheelDeltaY !== undefined)
        {
            deltaY = orgEvent.wheelDeltaY / 120;
        }

        if(orgEvent.wheelDeltaX !== undefined)
        {
            deltaX = -1 * orgEvent.wheelDeltaX / 120;
        }
        // Add event and delta to the front of the arguments
        args.unshift(event, delta, deltaX, deltaY);

        return this.getManipulatorController().mousewheel.apply(manipulatorAdapter, args);
    },

    preventDefault: function(event)
    {
        event.preventDefault();
    },

    divGlobalOffset: function(obj)
    {
        var x = 0;
        var y = 0;

        x = obj.offsetLeft;
        y = obj.offsetTop;

        var body = document.getElementsByTagName('body')[0];
        while(obj.offsetParent && obj !== body)
        {
            x += obj.offsetParent.offsetLeft;
            y += obj.offsetParent.offsetTop;
            obj = obj.offsetParent;
        }

        this._mousePosition[0] = x;
        this._mousePosition[1] = y;
        return this._mousePosition;
    },

    getPositionRelativeToCanvas: function(e, result)
    {
        return result = result || this._mousePosition,
        result[0] = void 0 === e.offsetX ? e.layerX : e.offsetX,
        result[1] = e.target.clientHeight - (void 0 === e.offsetY ? e.layerY : e.offsetY),
        result
    },

    getPositionRelativeToCanvas_backup: function(e, result)
    {
        var myObject = e.target;
        var posx, posy;
        if(e.pageX || e.pageY)
        {
            posx = e.pageX;
            posy = e.pageY;
        }
        else if(e.clientX || e.clientY)
        {
            posx = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
            posy = e.clientY + document.body.scrollTop + document.documentElement.scrollTop;
        }

        // posx and posy contain the mouse position relative to the document
        // Do something with this information
        var globalOffset = this.divGlobalOffset(myObject);
        posx = posx - globalOffset[0];
        posy = myObject.height - (posy - globalOffset[1]);

        // copy data to result if need to keep result
        // else we use a tmp variable inside manipulator
        // that we override at each call
        if(result === undefined)
        {
            result = this._mousePosition;
        }

        result[0] = posx;
        result[1] = posy;
        return result;
    },

    update: function()
    {
        if(!this.isValid())
        {
            return;
        }

        this.getManipulatorController().setEventProxy(this);
    }
};