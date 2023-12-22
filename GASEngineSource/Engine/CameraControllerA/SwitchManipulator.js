
GASEngine.SwitchManipulatorBase = function()
{
    this._manipulatorList = [];
    this._currentManipulator = undefined;
};

GASEngine.SwitchManipulatorBase.prototype =
{
    getCamera: function()
    {
        return this.getCurrentManipulator().getCamera();
    },

    setCamera: function(cam)
    {
        var cbList = this.getManipulatorList();
        for(var i = 0; i < cbList.length; ++i)
        {
            cbList[i].setCamera(cam);
        }
    },

    update: (function()
    {
        var eye = new GASEngine.Vector3();
        var target = new GASEngine.Vector3();
        return (function(delta)
        {
            var manipulator = this.getCurrentManipulator();
            if(manipulator !== undefined)
            {
                manipulator.update(delta);
            }

            var autoPilot = this._manipulatorList[GASEngine.Manipulator.AUTOPILOT];
            var result = autoPilot.update(delta);
        });
    })(),

    getCurrentController: function()
    {
        return this.getCurrentManipulator().getCurrentController();
    },

    getNumManipulator: function()
    {
        return this._manipulatorList.length;
    },

    addManipulator: function(manipulator)
    {
        this._manipulatorList.push(manipulator);
        if(this._currentManipulator === undefined)
        {
            this.setManipulatorType(0);
        }
    },

    getManipulatorList: function()
    {
        return this._manipulatorList;
    },

    setManipulatorType: function(index)
    {
        this._currentManipulator = index;
    },

    getCurrentManipulatorType: function()
    {
        return this._currentManipulator;
    },

    getCurrentManipulator: function()
    {
        return this._manipulatorList[this._currentManipulator];
    },

    reset: function()
    {
        this.getCurrentManipulator().reset();
    },

    computeHomePosition: function(center, radius)
    {
        var manipulator = this.getCurrentManipulator();
        if(manipulator !== undefined)
        {
            manipulator.computeHomePosition(center, radius);
        }
    },

    getCameraWorldMatrix: function()
    {
        var manipulator = this.getCurrentManipulator();
        if(manipulator !== undefined)
        {
            return manipulator.getCameraWorldMatrix();
        }
    },

    getHomeBound: function(boundStrategy)
    {
        return this.getCurrentManipulator().getHomeBound( boundStrategy );
    },

    getHomeDistance: function(bs)
    {
        return this.getCurrentManipulator().getHomeDistance( bs );
    }
};

GASEngine.SwitchManipulator = function() 
{
    GASEngine.SwitchManipulatorBase.call(this);
    this.init();
};

GASEngine.SwitchManipulator.prototype = Object.create(GASEngine.SwitchManipulatorBase.prototype);
GASEngine.SwitchManipulator.prototype.constructor = GASEngine.SwitchManipulator;

GASEngine.SwitchManipulator.prototype.init = function() 
{
    // GASEngine.SwitchManipulatorBase.prototype.setManipulatorType.call(this, GASEngine.Manipulator.ORBIT);
    GASEngine.SwitchManipulatorBase.prototype.setManipulatorType.call(this, GASEngine.Manipulator.FPS);

    var orbitManipulator = new GASEngine.OrbitManipulator();
    orbitManipulator.getRotateInterpolator()._epsilon = 0.1;
    orbitManipulator.getPanInterpolator()._epsilon = 0.1;
    orbitManipulator.getZoomInterpolator()._epsilon = 0.001;

    var fpsManipulator = new GASEngine.FirstPersonManipulator();
    fpsManipulator.getLookPositionInterpolator()._epsilon = 0.1;
    fpsManipulator.getSideInterpolator()._epsilon = 0.001;
    fpsManipulator.getForwardInterpolator()._epsilon = 0.001;
    fpsManipulator.getPanInterpolator()._epsilon = 0.001;
    fpsManipulator.getZoomInterpolator()._epsilon = 0.001;

    var autoPilotManipulator = new GASEngine.AutoPilotManipulator();
    autoPilotManipulator.setManipulator(orbitManipulator);
    // autoPilotManipulator.setManipulator(fpsManipulator);

    this.addManipulator(orbitManipulator);
    this.addManipulator(fpsManipulator);
    this.addManipulator(autoPilotManipulator);
};

GASEngine.SwitchManipulator.prototype.getOrbitManipulator = function() 
{
    return this._manipulatorList[GASEngine.Manipulator.ORBIT];
};

GASEngine.SwitchManipulator.prototype.getFirstPersonManipulator = function() 
{
    return this._manipulatorList[GASEngine.Manipulator.FPS];
};

GASEngine.SwitchManipulator.prototype.getAutoPilotManipulator = function()
{
    return this._manipulatorList[GASEngine.Manipulator.AUTOPILOT];
};

GASEngine.SwitchManipulator.prototype.setControllerType = function(type)
{
    this.getOrbitManipulator().setControllerType(type);
    this.getFirstPersonManipulator().setControllerType(type);
    this.getAutoPilotManipulator().setControllerType(type);
};

GASEngine.SwitchManipulator.prototype._setManipulatorDelay = function(manipulator, delay) 
{
    manipulator.setDelay(delay);
    for(var name in manipulator._controllerList)
    {
        var inputController = manipulator._controllerList[name];
        if(inputController._delay !== undefined)
        {
            inputController._delay = delay;
        }
    }
};

GASEngine.SwitchManipulator.prototype.setDelay = function(duration) 
{
    this._setManipulatorDelay(this._manipulatorList[GASEngine.Manipulator.ORBIT], duration),
    this._setManipulatorDelay(this._manipulatorList[GASEngine.Manipulator.FPS], duration)
};

GASEngine.SwitchManipulator.prototype.addManipulator = function(manipulator)
{
    GASEngine.SwitchManipulatorBase.prototype.addManipulator.call(this, manipulator);
};

GASEngine.SwitchManipulator.prototype.setManipulatorType = function()
{
    var target = new GASEngine.Vector3();
    var eye = new GASEngine.Vector3();
    return function(index) 
    {
        if(this.getCurrentManipulatorType() !== index && index !== GASEngine.Manipulator.AUTOPILOT)
        {
            var currentManipulator = this.getCurrentManipulator();

            var currentController = this.getCurrentController();
            currentController.reset();

            GASEngine.SwitchManipulatorBase.prototype.setManipulatorType.call(this, index),
            this.setTargetAndEye(currentManipulator.getTarget(target), currentManipulator.getEyePosition(eye))
        }
    };
}();

GASEngine.SwitchManipulator.prototype.setTargetAndEye = function(endTarget, endEye)
{
    var currentManipulator = this.getCurrentManipulator();
    if(this.getCurrentManipulatorType() === GASEngine.Manipulator.ORBIT)
    {
        if(endTarget)
        {
            currentManipulator.setTarget(endTarget);
        }

        if(endEye)
        {
            currentManipulator.setEyePosition(endEye);
        }
    }
    else if(this.getCurrentManipulatorType() === GASEngine.Manipulator.FPS)
    {
        if(endEye)
        {
            currentManipulator.setEyePosition(endEye);
        }

        if(endTarget)
        {
            currentManipulator.setTarget(endTarget);
        }
    }
    else if(this.getCurrentManipulatorType() === GASEngine.Manipulator.AUTOPILOT)
    {

    }
};

GASEngine.SwitchManipulator.prototype.setEyePosition = function(eye)
{
    this.getCurrentManipulator().setEyePosition(eye);
};

GASEngine.SwitchManipulator.prototype.getEyePosition = function(eye)
{
    return this.getCurrentManipulator().getEyePosition(eye);
};

GASEngine.SwitchManipulator.prototype.setTarget = function(target)
{
    this.getCurrentManipulator().setTarget(target);
};

GASEngine.SwitchManipulator.prototype.getTarget = function(target)
{
    return this.getCurrentManipulator().getTarget(target);
};

GASEngine.SwitchManipulator.prototype.setDistance = function(distance) 
{
    if(this.getCurrentManipulatorType() === GASEngine.Manipulator.ORBIT)
    {
        this.getCurrentManipulator().setDistance(distance);
    }
};

GASEngine.SwitchManipulator.prototype.getDistance = function()
{
    return this.getCurrentManipulator().getDistance();
};

GASEngine.SwitchManipulator.prototype.computeHomePosition = function(center, radius)
{
    GASEngine.SwitchManipulatorBase.prototype.computeHomePosition.call(this, center, radius);
};

GASEngine.SwitchManipulator.prototype.cameraGoto = (function()
{
    var eyeStart = new GASEngine.Vector3();
    var targetStart = new GASEngine.Vector3();
    return function(eyeEnd, targetEnd)
    {
        if(this.getCurrentManipulatorType() !== GASEngine.Manipulator.ORBIT)
        {
            console.error('GASEngine.SwitchManipulator.cameraGoto: camera controller must be orbit.');
            return;
        }
        this.getEyePosition(eyeStart);
        this.getTarget(targetStart);
        var autoPilot = this._manipulatorList[GASEngine.Manipulator.AUTOPILOT];
        autoPilot.setTargetAndEye(eyeStart, targetStart, eyeEnd, targetEnd);
    };
})();