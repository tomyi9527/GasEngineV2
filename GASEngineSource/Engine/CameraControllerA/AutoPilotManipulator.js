
GASEngine.AutoPilotManipulator = function(manipulator)
{
    GASEngine.Manipulator.call(this, GASEngine.Manipulator.AUTOPILOT);
    this.init();
};

GASEngine.AutoPilotManipulator.AvailableControllerList =
    [
        'StandardMouseKeyboard',
        'DeviceOrientation',
        'Hammer'
];

GASEngine.AutoPilotManipulator.ControllerList =
    [
        'StandardMouseKeyboard',
        'DeviceOrientation',
        'Hammer'
    ];

GASEngine.AutoPilotManipulator.prototype = Object.create(GASEngine.Manipulator.prototype);
GASEngine.AutoPilotManipulator.prototype.constructor = GASEngine.AutoPilotManipulator;

GASEngine.AutoPilotManipulator.prototype.init = function()
{
    this._duration = 1.5; //1.5
    this._manipulator = null;
    this._eyeEnd = new GASEngine.Vector3();
    this._eyeDeltaLen = 0;
    this._targetEnd = new GASEngine.Vector3();
    this._targetDeltaLen = 0;

    this._onFinished = null;
    this._lastRatio = 0;
    this._finished = true;
    this._startTime = 0;

    var self = this;
    this._controllerList = {};
    GASEngine.AutoPilotManipulator.ControllerList.forEach(function(value)
    {
        if(GASEngine.AutoPilotManipulator[value] !== undefined)
        {
            self._controllerList[value] = new GASEngine.AutoPilotManipulator[value](self);
        }
    });
};

GASEngine.AutoPilotManipulator.prototype.setManipulator = function(manipulator)
{
    this._manipulator = manipulator;
};

//GASEngine.AutoPilotManipulator.prototype.getDuration = function()
//{
//    return this._duration;
//};

//GASEngine.AutoPilotManipulator.prototype.setDuration = function(duration)
//{
//    this._duration = duration;
//};

GASEngine.AutoPilotManipulator.prototype.setTargetAndEye = function() 
{
    var tmp = new GASEngine.Vector3();

    return function(eyeStart, targetStart, eyeEnd, targetEnd, onFinished)
    {
        this._manipulator.setTarget(targetStart);
        this._manipulator.setEyePosition(eyeStart);

        this._eyeEnd.copy(eyeEnd);
        this._manipulator.getEyePosition(tmp);
        tmp.subVectors(eyeEnd, tmp);
        this._eyeDeltaLen = tmp.length();

        this._targetEnd.copy(targetEnd);
        this._manipulator.getTarget(tmp);
        tmp.subVectors(targetEnd, tmp);
        this._targetDeltaLen = tmp.length();

        this._onFinished = onFinished;
        
        this._startTime = 0;
        this._finished = false;
        this._lastRatio = 0;
    }
}();

GASEngine.AutoPilotManipulator.prototype.getEyePosition = function(eye)
{
    return this._manipulator.getEyePosition(eye);
};

GASEngine.AutoPilotManipulator.prototype.getTarget = function(target)
{
    return this._manipulator.getTarget(target);
};


GASEngine.AutoPilotManipulator.prototype.interpolate = function(delta)
{
    if(this._finished)
        return true;

    if(this._startTime < this._duration)
    {
        this._startTime += delta;
    }

    var r = Math.min(this._duration, this._startTime);
    var ratio = r / this._duration;

    return ratio;
};

GASEngine.AutoPilotManipulator.prototype.update = function()
{
    var eyeTmp = new GASEngine.Vector3();
    var targetTmp = new GASEngine.Vector3();
    var tmp0 = new GASEngine.Vector3();
    var tmp1 = new GASEngine.Vector3();

    var easeOutQuart = function(t)
    {
        t = t - 1;
        return -(t * t * t * t - 1);
    };

    return function(delta)
    {
        if(this._finished === true)
        {
            return false;
        }

        var ratio = this.interpolate(delta);
        ratio = easeOutQuart(ratio);
        var deltaRatio = ratio - this._lastRatio;
        this._lastRatio = ratio;

        this._manipulator.getEyePosition(eyeTmp);
        this._manipulator.getTarget(targetTmp);

        tmp0.subVectors(this._targetEnd, targetTmp);
        tmp0.normalize();
        tmp0.multiplyScalar(deltaRatio * this._targetDeltaLen);
        tmp1.addVectors(targetTmp, tmp0);
        this._manipulator.setTarget(tmp1);

        tmp0.subVectors(this._eyeEnd, eyeTmp);
        tmp0.normalize();
        tmp0.multiplyScalar(deltaRatio * this._eyeDeltaLen);
        tmp1.addVectors(eyeTmp, tmp0);
        this._manipulator.setEyePosition(tmp1);

        this._manipulator.update(0);

        if(ratio >= 1.0)
        {
            this._startTime = 0;
            this._finished = true;
            this._lastRatio = 0;

            if(this._onFinished)
            {
                this._onFinished();
                this._onFinished = null;
            }
        }

        return true;
    };
}();

//GASEngine.AutoPilotManipulator.DeviceOrientation = GASEngine.AutoPilotManipulatorDeviceOrientationController;
//GASEngine.AutoPilotManipulator.Hammer = GASEngine.AutoPilotManipulatorHammerController;
GASEngine.AutoPilotManipulator.StandardMouseKeyboard = GASEngine.AutoPilotManipulatorStandardMouseKeyboardController;