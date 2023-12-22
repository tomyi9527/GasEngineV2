GASEngine.OrbitManipulatorStandardMouseKeyboardController = function(manipulator)
{
    this._manipulator = manipulator;
    this.init();
};

GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE = 0;
GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN = 1;
GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM = 2;

GASEngine.OrbitManipulatorStandardMouseKeyboardController.prototype =
{
    init: function()
    {
        this.releaseButton();
        this._rotateKey = 65; // a
        this._zoomKey = 83; // s
        this._panKey = 68; // d

        this._mode = undefined;
        this._delay = 0.15;
    },

    reset: function()
    {
        this.releaseButton();
        this.setMode(undefined);
        this._manipulator.getRotateInterpolator().reset();
        this._manipulator.getPanInterpolator().reset();
        this._manipulator.getZoomInterpolator().reset();
    },

    getMode: function()
    {
        return this._mode;
    },

    setMode: function(mode)
    {
        this._mode = mode;
    },

    setEventProxy: function(proxy)
    {
        this._eventProxy = proxy;
    },

    setManipulator: function(manipulator)
    {
        this._manipulator = manipulator;
    },

    mousemove: function(ev)
    {
        if(this._buttonup === true)
        {
            return;
        }

        var pos = this._eventProxy.getPositionRelativeToCanvas(ev);
        if(isNaN(pos[0]) === false && isNaN(pos[1]) === false)
        {
            var mode = this.getMode();
            if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE)
            {
                this._manipulator.getRotateInterpolator().setDelay(this._delay);
                this._manipulator.getRotateInterpolator().setTarget(pos[0], pos[1]);

            }
            else if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN)
            {
                this._manipulator.getPanInterpolator().setTarget(pos[0], pos[1]);

            }
            else if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM)
            {
                var zoom = this._manipulator.getZoomInterpolator();
                if(zoom.isReset())
                {
                    zoom.setStart(pos[1]);
                    zoom.set(0.0);
                }
                var dy = pos[1] - zoom.getStart();
                zoom.setStart(pos[1]);
                var v = zoom.getTarget()[0];
                zoom.setTarget(v - dy / 20.0);
            }
        }

        ev.preventDefault();
    },

    mousedown: function(ev)
    {
        var manipulator = this._manipulator;
        var mode = this.getMode();
        if(mode === undefined)
        {
            if(ev.button === 0)
            {
                if(ev.shiftKey)
                {
                    this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN);
                }
                else if(ev.ctrlKey)
                {
                    this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM);
                }
                else
                {
                    this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE);
                }
            }
            else
            {
                // For users on Mac machines for who CTRL+LeftClick is naturally converted 
                // into a RightClick in Firefox.
                if(ev.button === 2 && ev.ctrlKey)
                {
                    this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM);
                }
                else
                {
                    this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN);
                }
            }
        }

        this.pushButton();

        var pos = this._eventProxy.getPositionRelativeToCanvas(ev);
        mode = this.getMode();
        if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE)
        {
            manipulator.getRotateInterpolator().reset();
            manipulator.getRotateInterpolator().set(pos[0], pos[1]);
        }
        else if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN)
        {
            manipulator.getPanInterpolator().reset();
            manipulator.getPanInterpolator().set(pos[0], pos[1]);
        }
        else if(mode === GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM)
        {
            manipulator.getZoomInterpolator().setStart(pos[1]);
            manipulator.getZoomInterpolator().set(0.0);
        }
    },

    mouseup: function()
    {
        this.releaseButton();
        this.setMode(undefined);
    },

    mouseout: function()
    {
        this.releaseButton();
        this.setMode(undefined);
    },

    mousewheel: function(ev, intDelta)
    {
        var manipulator = this._manipulator;
        var zoomTarget = manipulator.getZoomInterpolator().getTarget()[0] - intDelta;
        manipulator.getZoomInterpolator().setTarget(zoomTarget);
    },

    pushButton: function()
    {
        this._buttonup = false;
    },

    releaseButton: function()
    {
        this._buttonup = true;
    },

    keydown: function(ev)
    {
        if(ev.keyCode === 32)
        {
            let center = new GASEngine.Vector3();
            this._manipulator.getTarget(center);
            let radius = this._manipulator.getCamera().far / 10.0;
            this._manipulator.computeHomePosition(center, radius);
            ev.preventDefault();
        }
        else if(ev.keyCode === this._panKey && this.getMode() !== GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN)
        {
            this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.PAN);
            this._manipulator.getPanInterpolator().reset();
            this.pushButton();
            ev.preventDefault();
        }
        else if(ev.keyCode === this._zoomKey && this.getMode() !== GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM)
        {
            this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.ZOOM);
            this._manipulator.getZoomInterpolator().reset();
            this.pushButton();
            ev.preventDefault();
        }
        else if(ev.keyCode === this._rotateKey && this.getMode() !== GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE)
        {
            this.setMode(GASEngine.OrbitManipulatorStandardMouseKeyboardController.ROTATE);
            this._manipulator.getRotateInterpolator().reset();
            this.pushButton();
            ev.preventDefault();
        }
    },

    keyup: function(ev)
    {
        if(ev.keyCode === this._panKey)
        {
            this.mouseup(ev);
        }
        else if(ev.keyCode === this._rotateKey)
        {
            this.mouseup(ev);
        }
        else if(ev.keyCode === this._rotateKey)
        {
            this.mouseup(ev);
        }

        this.setMode(undefined);
    }
};