
GASEngine.FirstPersonManipulator = function()
{
    GASEngine.Manipulator.call(this, GASEngine.Manipulator.FPS);
    this.init();
};

GASEngine.FirstPersonManipulator.AvailableControllerList =
[
    'StandardMouseKeyboard',
    'DeviceOrientation',
    'Hammer'
];

GASEngine.FirstPersonManipulator.ControllerList =
[
    'StandardMouseKeyboard',
    'DeviceOrientation',
    'Hammer'
];

GASEngine.FirstPersonManipulator.prototype = Object.create(GASEngine.Manipulator.prototype);
GASEngine.FirstPersonManipulator.prototype.constructor = GASEngine.FirstPersonManipulator;

GASEngine.FirstPersonManipulator.prototype.init = function () 
{
    this._rotation = new GASEngine.Matrix4();

    this._direction = new GASEngine.Vector3(0.0, 0.0, 1.0);
    this._eye = new GASEngine.Vector3(0.0, 0.0, 1.0);
    this._up = new GASEngine.Vector3(0.0, 1.0, 0.0);
    this._distance = 1.0;
    this._forward = new GASEngine.DelayInterpolator(1);
    this._side = new GASEngine.DelayInterpolator(1);
    this._up_ = new GASEngine.DelayInterpolator(1);
    this._lookPosition = new GASEngine.DelayInterpolator(2);

    this._pan = new GASEngine.DelayInterpolator(2);
    this._zoom = new GASEngine.DelayInterpolator(1);

    this._stepFactor = 1.0;
    this._angleVertical = 0.0;
    this._angleHorizontal = 0.0;

    this._tmpGetTargetDir = new GASEngine.Vector3();

    var self = this;

    this._controllerList = {};
    GASEngine.FirstPersonManipulator.ControllerList.forEach(function(value)
    {
        if(GASEngine.FirstPersonManipulator[value] !== undefined)
        {
            self._controllerList[value] = new GASEngine.FirstPersonManipulator[value](self);
        }
    });
};

GASEngine.FirstPersonManipulator.prototype.computeHomePosition = function(center, radius)
{
    this._distance = this.getHomeDistance(radius);

    this._direction.set(0.0, 0.0, 1.0);

    this._eye.copy(this._direction);
    this._eye.multiplyScalar(this._distance);
    this._eye.add(center);

    this.setTarget(center);
};

GASEngine.FirstPersonManipulator.prototype.setDelay = function(dt)
{
    this._forward.setDelay(dt);
    this._side.setDelay(dt);
    this._up_.setDelay(dt);
    this._lookPosition.setDelay(dt);
    this._pan.setDelay(dt);
    this._zoom.setDelay(dt);
};

GASEngine.FirstPersonManipulator.prototype.getEyePosition = function(eye)
{
    return eye.copy(this._eye);
};

GASEngine.FirstPersonManipulator.prototype.setEyePosition = function(eye)
{
    this._eye.copy(eye);
};

GASEngine.FirstPersonManipulator.prototype.getTarget = function(pos)
{
    this._tmpGetTargetDir.set(this._direction.x, this._direction.y, this._direction.z);
    var dir = this._tmpGetTargetDir.multiplyScalar(-this._distance);
    pos.addVectors(this._eye, dir);
    return pos;
};

GASEngine.FirstPersonManipulator.prototype.setTarget = function(pos)
{
    var dir = this._tmpGetTargetDir;
    dir.subVectors(pos, this._eye);
    dir.y = 0.0;
    dir.normalize();
    this._angleHorizontal = Math.acos(-dir.z);
    if(dir.x < 0.0)
    {
        this._angleHorizontal = -this._angleHorizontal;
    }

    dir.subVectors(pos, this._eye);
    dir.normalize();
    this._angleVertical = -Math.asin(dir.y);
    this._direction.copy(dir);
};

GASEngine.FirstPersonManipulator.prototype.getLookPositionInterpolator = function()
{
    return this._lookPosition;
};

GASEngine.FirstPersonManipulator.prototype.getSideInterpolator = function()
{
    return this._side;
};

GASEngine.FirstPersonManipulator.prototype.getForwardInterpolator = function()
{
    return this._forward;
};

GASEngine.FirstPersonManipulator.prototype.getUpInterpolator = function()
{
    return this._up_;
};

GASEngine.FirstPersonManipulator.prototype.getPanInterpolator = function()
{
    return this._pan;
};

GASEngine.FirstPersonManipulator.prototype.getZoomInterpolator = function()
{
    return this._zoom;
};

GASEngine.FirstPersonManipulator.prototype.getRotateInterpolator = function()
{
    // for compatibility with orbit hammer controllers
    return this._lookPosition;
};

GASEngine.FirstPersonManipulator.prototype.computeRotation = (function()
{
    var first = new GASEngine.Matrix4();
    var right = new GASEngine.Vector3(1.0, 0.0, 0.0);
    var upy = new GASEngine.Vector3(0.0, 0.0, 1.0);
    var upz = new GASEngine.Vector3(0.0, 1.0, 0.0);
    var LIMIT = Math.PI * 0.5;

    return function(dx, dy)
    {
        this._angleVertical += dy * 0.01;
        this._angleHorizontal -= dx * 0.01;
        if(this._angleVertical > LIMIT)
        {
            this._angleVertical = LIMIT;
        }
        else if(this._angleVertical < -LIMIT)
        {
            this._angleVertical = -LIMIT;
        }

        first.makeRotationAxis(right, -this._angleVertical);
        this._rotation.makeRotationAxis(upz, -this._angleHorizontal);
        this._rotation.multiply(first);

        this._direction.copy(upy);
        this._direction.applyMatrix4(this._rotation);
        this._direction.normalize();

        this._up.copy(upz);
        this._up.applyMatrix4(this._rotation);
        this._up.normalize();
    };
})();

GASEngine.FirstPersonManipulator.prototype.reset = function () 
{
    this.init();
};

GASEngine.FirstPersonManipulator.prototype.setDistance = function(d)
{
    this._distance = d;
};

GASEngine.FirstPersonManipulator.prototype.getDistance = function()
{
    return this._distance;
};

GASEngine.FirstPersonManipulator.prototype.setStepFactor = function(t)
{
    this._stepFactor = t;
};

GASEngine.FirstPersonManipulator.prototype.computePosition = (function()
{
    //var vec = vec2.create();
    var vec = new GASEngine.Vector3();
    return function(dt)
    {
        this._forward.update(dt);
        this._side.update(dt);
        this._up_.update(dt);

        var factor = this._distance < 1e-3 ? 1e-3 : this._distance;

        var proj = this._camera.projectionMatrix.elements;
        var vFov = proj[15] === 1 ? 1.0 : 2.00 / proj[5];

        vec.x = this._forward.getCurrent()[0];
        vec.y = this._side.getCurrent()[0];
        vec.z = this._up_.getCurrent()[0];
        var len2 = vec.lengthSq();
        if(len2 > 1.0)
        {
            vec.multiplyScalar(1.0 / Math.sqrt(len2));
        }

        var pan = this._pan.update(dt);
        var zoom = this._zoom.update(dt);

        var timeFactor = this._stepFactor * factor * vFov * dt;
        var directFactor = this._stepFactor * factor * vFov * 0.005;

        this.moveForward(vec.x * timeFactor - zoom[0] * directFactor * 30.0);
        this.strafe(vec.y * timeFactor - pan[0] * directFactor);
        this.strafeVertical(-pan[1] * directFactor);
        this.strafeVertical(vec.z * directFactor * 5.0);
    };
})();

GASEngine.FirstPersonManipulator.prototype.update = (function()
{
    var tmpTarget = new GASEngine.Vector3();
    return function (dt)
    {
        var delta = this._lookPosition.update(dt);
        this.computeRotation(-delta[0] * 0.5, -delta[1] * 0.5);
        this.computePosition(dt);

        this._cameraWorldMatrix.elements[0] = this._rotation.elements[0];
        this._cameraWorldMatrix.elements[1] = this._rotation.elements[1];
        this._cameraWorldMatrix.elements[2] = this._rotation.elements[2];
        this._cameraWorldMatrix.elements[3] = this._rotation.elements[3];

        this._cameraWorldMatrix.elements[4] = this._rotation.elements[4];
        this._cameraWorldMatrix.elements[5] = this._rotation.elements[5];
        this._cameraWorldMatrix.elements[6] = this._rotation.elements[6];
        this._cameraWorldMatrix.elements[7] = this._rotation.elements[7];

        this._cameraWorldMatrix.elements[8] = this._rotation.elements[8];
        this._cameraWorldMatrix.elements[9] = this._rotation.elements[9];
        this._cameraWorldMatrix.elements[10] = this._rotation.elements[10];
        this._cameraWorldMatrix.elements[11] = this._rotation.elements[11];

        this._cameraWorldMatrix.elements[12] = this._eye.x;
        this._cameraWorldMatrix.elements[13] = this._eye.y;
        this._cameraWorldMatrix.elements[14] = this._eye.z;
        this._cameraWorldMatrix.elements[15] = 1.0;

        this._vrEnable = false; // setPoseVR is called on each frame
    };
})();

GASEngine.FirstPersonManipulator.prototype.setPoseVR = function(q, pos)
{
    //this._vrEnable = true;
    //quat.copy( this._vrRot, q );
    //vec3.sub( this._vrTrans, pos, this._vrPos );
    //vec3.copy( this._vrPos, pos );
};

GASEngine.FirstPersonManipulator.prototype.moveForward = (function()
{
    var tmp = new GASEngine.Vector3();
    return function(distance)
    {
        this._direction.normalize();
        tmp.copy(this._direction);
        tmp.multiplyScalar(-distance);
        this._eye.add(tmp);
    };
})();

GASEngine.FirstPersonManipulator.prototype.strafe = (function() //move left
{
    var tmp = new GASEngine.Vector3();

    return function(distance)
    {
        tmp.crossVectors(this._up, this._direction); //right_x
        tmp.normalize();
        tmp.multiplyScalar(distance);
        this._eye.add(tmp);
    };
})();

GASEngine.FirstPersonManipulator.prototype.strafeVertical = (function() //move up
{
    var tmp = new GASEngine.Vector3();

    return function(distance)
    {
        tmp.copy(this._up);
        tmp.normalize();
        tmp.multiplyScalar(distance);
        this._eye.add(tmp);
    };
})();

//GASEngine.FirstPersonManipulator.DeviceOrientation = GASEngine.FirstPersonManipulatorDeviceOrientationController;
//GASEngine.FirstPersonManipulator.Hammer = GASEngine.FirstPersonManipulatorHammerController;
GASEngine.FirstPersonManipulator.StandardMouseKeyboard = GASEngine.FirstPersonManipulatorStandardMouseKeyboardController;
