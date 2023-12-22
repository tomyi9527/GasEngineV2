
GASEngine.OrbitManipulator = function()
{
    GASEngine.Manipulator.call(this, GASEngine.Manipulator.ORBIT);
    this.init();
};

GASEngine.OrbitManipulator.AvailableControllerList =
[
    'StandardMouseKeyboard',
    'DeviceOrientation',
    'Hammer'
];

GASEngine.OrbitManipulator.ControllerList =
[
    'StandardMouseKeyboard',
    'DeviceOrientation',
    'Hammer'
];

GASEngine.TWO_PI = 2 * Math.PI;
GASEngine.lowerOrEqual = function(val, limit)
{
    return val < limit + 0.00001;
};

GASEngine.OrbitManipulator.prototype = Object.create(GASEngine.Manipulator.prototype);
GASEngine.OrbitManipulator.prototype.constructor = GASEngine.OrbitManipulator;

GASEngine.OrbitManipulator.prototype.computeHomePosition = function(center, radius)
{
    this._distance = this.getHomeDistance(radius);
    this.setTarget(center);

    this._rotation.identity();

    var eyePos = new GASEngine.Vector3();
    this.getEyePosition(eyePos);

    this.setEyePosition(eyePos);
};

GASEngine.OrbitManipulator.prototype.getHomePosition = function()
{
};

GASEngine.OrbitManipulator.prototype.init = function()
{
    this._distance = 25.0;
    this._target = new GASEngine.Vector3();

    this._upy = new GASEngine.Vector3(0.0, 1.0, 0.0);
   
    this._rotation = new GASEngine.Matrix4();
    this._time = 0.0;

    this._vrMatrix = new GASEngine.Matrix4();

    this._rotate = new GASEngine.DelayInterpolator(2);
    this._pan = new GASEngine.DelayInterpolator(2);
    this._zoom = new GASEngine.DelayInterpolator(1);

    this._minSpeed = 1e-4;
    this._scaleMouseMotion = 1.0;

    this._cameraWorldMatrix = new GASEngine.Matrix4();

    this._autoPushTarget = true;

    this._limitPitchUp = Math.PI * 0.5 * 0.9;
    this._limitPitchDown = -this._limitPitchUp;

    this._limitYawLeft = -Math.PI;
    this._limitYawRight = -this._limitYawLeft;

    this._limitZoomIn = 1e-4;
    this._limitZoomOut = Infinity;

    var self = this;

    GASEngine.OrbitManipulator.ControllerList.forEach(function(value)
    {
        if(GASEngine.OrbitManipulator[value] !== undefined)
        {
            self._controllerList[value] = new GASEngine.OrbitManipulator[value](self);
        }
    });
};

GASEngine.OrbitManipulator.prototype.setLimitPitchUp = function(up)
{
    this._limitPitchUp = up;
};

GASEngine.OrbitManipulator.prototype.setLimitPitchDown = function(down)
{
    this._limitPitchDown = down;
};

GASEngine.OrbitManipulator.prototype.setLimitYawLeft = function(left)
{
    this._limitYawLeft = left;
};

GASEngine.OrbitManipulator.prototype.setLimitYawRight = function(right)
{
    this._limitYawRight = right;
};

GASEngine.OrbitManipulator.prototype.setLimitZoomOut = function(zoomOut)
{
    this._limitZoomOut = zoomOut;
};

GASEngine.OrbitManipulator.prototype.setLimitZoomIn = function(zoomIn)
{
    this._limitZoomIn = zoomIn;
};

GASEngine.OrbitManipulator.prototype.setDelay = function(dt)
{
    this._rotate.setDelay(dt);
    this._pan.setDelay(dt);
    this._zoom.setDelay(dt);
};

GASEngine.OrbitManipulator.prototype.reset = function()
{
    this.init();
};

GASEngine.OrbitManipulator.prototype.setTarget = function(target)
{
    this._target.set(target.x, target.y, target.z);
    var eyePos = new GASEngine.Vector3();
    this.getEyePosition(eyePos);
    this._distance = target.distanceTo(eyePos);
};

GASEngine.OrbitManipulator.prototype.setEyePosition = (function()
{
    var forward = new GASEngine.Vector3();
    var right = new GASEngine.Vector3();
    var up = new GASEngine.Vector3();

    return function(eye)
    {
        var result = this._rotation.elements;
        var center = this._target;

        forward.subVectors(eye, center); //DO REMEMBER OPENGL OR GASEngine PROJECT NEGATIVE Z DIRECTION!
        forward.normalize(); //forward_z

        right.crossVectors(this._upy, forward); //right_x
        right.normalize();

        up.crossVectors(forward, right); //up_z
        up.normalize();

        result[0] = right.x;
        result[1] = right.y;
        result[2] = right.z;
        result[3] = 0.0;
        result[4] = up.x;
        result[5] = up.y;
        result[6] = up.z;
        result[7] = 0.0;
        result[8] = forward.x;
        result[9] = forward.y;
        result[10] = forward.z;
        result[11] = 0.0;
        result[12] = 0;
        result[13] = 0;
        result[14] = 0;
        result[15] = 1.0;

        this._distance = center.distanceTo(eye);
    };
})();


GASEngine.OrbitManipulator.prototype.setMinSpeed = function(s)
{
    this._minSpeed = s;
};

GASEngine.OrbitManipulator.prototype.getMinSpeed = function()
{
    return this._minSpeed;
};

GASEngine.OrbitManipulator.prototype.setDistance = function(d)
{
    this._distance = d;
};

GASEngine.OrbitManipulator.prototype.getDistance = function()
{
    return this._distance;
};

GASEngine.OrbitManipulator.prototype.getSpeedFactor = function()
{
    return Math.max(this._distance, this._minSpeed);
};

GASEngine.OrbitManipulator.prototype.computePan = (function()
{
    var inv = new GASEngine.Matrix4();
    var x = new GASEngine.Vector3();
    var y = new GASEngine.Vector3();

    return function(dx, dy)
    {
        if(!this._camera) return;
        var proj = this._camera.projectionMatrix.elements;
        var vFov = proj[15] === 1 ? 1.0 : 2.00 / proj[5];
        var speed = this.getSpeedFactor() * vFov;
        dy *= speed;
        dx *= speed;

        x.x = this._rotation.elements[0];
        x.y = this._rotation.elements[1];
        x.z = this._rotation.elements[2];
        x.normalize();

        y.x = this._rotation.elements[4];
        y.y = this._rotation.elements[5];
        y.z = this._rotation.elements[6];
        y.normalize();

        x.multiplyScalar(dx);
        y.multiplyScalar(dy);
        this._target.add(x);
        this._target.add(y);
    };
})();

GASEngine.OrbitManipulator.prototype.computeRotation = (function()
{
    var rightDir = new GASEngine.Vector3(1.0, 0.0, 0.0);
    var tmpMatrix4 = new GASEngine.Matrix4();

    return function(dx, dy)
    {
        var pitch = Math.asin(this._rotation.elements[9]) + dy / 10.0;
        pitch = Math.min(Math.max(pitch, this._limitPitchDown), this._limitPitchUp);

        var deltaYaw = -dx / 10.0;
        var previousYaw = Math.atan2(this._rotation.elements[2], this._rotation.elements[0]);
        var yaw = this._computeYaw(previousYaw, deltaYaw, this._limitYawLeft, this._limitYawRight);

        this._rotation.makeRotationAxis(this._upy, -yaw);
        tmpMatrix4.makeRotationAxis(rightDir, -pitch);
        this._rotation.multiply(tmpMatrix4);
    };
})();

GASEngine.OrbitManipulator.prototype._computeYaw = function(previousYaw, deltaYaw, left, right)
{
    var yaw = previousYaw + deltaYaw;

    if(right !== Math.PI || left !== -Math.PI)
    {
        if(right < left)
        {
            if(yaw > Math.PI)
            {
                previousYaw -= TWO_PI;
                yaw -= TWO_PI;
            }

            if(yaw < -Math.PI)
            {
                previousYaw += TWO_PI;
                yaw += TWO_PI;
            }
        }

        if(deltaYaw === 0)
        {
            var isOutsideLimit = false;
            if(left > right)
            {
                isOutsideLimit = (yaw < left && yaw > right);
            }
            else
            {
                isOutsideLimit = yaw < left || yaw > right;
            }

            if(isOutsideLimit)
            {
                yaw = Math.abs(yaw - left) > Math.abs(yaw - right) ? right : left;
            }
        }

        if(deltaYaw > 0.0 && lowerOrEqual(previousYaw, right) && yaw > right)
        {
            yaw = right;
        }
        else if(deltaYaw < 0.0 && lowerOrEqual(left, previousYaw) && yaw < left)
        {
            yaw = left;
        }
    }
    return yaw;
};

GASEngine.OrbitManipulator.prototype.computeZoom = function(dz)
{
    this.zoom(dz);
};

GASEngine.OrbitManipulator.prototype.setAutoPushTarget = function(bool)
{
    this._autoPushTarget = bool;
};

GASEngine.OrbitManipulator.prototype.zoom = (function()
{
    var dir = new GASEngine.Vector3();
    return function(ratio)
    {
        var newValue = this._distance + this.getSpeedFactor() * (ratio - 1.0);

        if(this._autoPushTarget && newValue < this._limitZoomIn)
        {
            // push the target instead of zooming on it
            dir.subVectors(this._target, this.getEyePosition(dir));
            dir.normalize();
            dir.multiplyScalar(this._limitZoomIn - newValue);
            this._target.add(dir);
        }

        this._distance = Math.max(this._limitZoomIn, Math.min(this._limitZoomOut, newValue));
    };
})();

GASEngine.OrbitManipulator.prototype.getRotateInterpolator = function()
{
    return this._rotate;
};

GASEngine.OrbitManipulator.prototype.getPanInterpolator = function()
{
    return this._pan;
};

GASEngine.OrbitManipulator.prototype.getZoomInterpolator = function()
{
    return this._zoom;
};

GASEngine.OrbitManipulator.prototype.getTarget = function(target)
{
    return target.copy(this._target);
};

GASEngine.OrbitManipulator.prototype.getEyePosition = function(eye)
{
    this.computeEyePosition(this._target, this._distance, eye);
    return eye;
};

GASEngine.OrbitManipulator.prototype.computeEyePosition = (function()
{
    var tmpDist = new GASEngine.Vector3();
    var tmpInverse = new GASEngine.Matrix4();

    return function(target, distance, eye)
    {
        tmpDist.set(0.0, 0.0, distance);
        tmpDist.transformDirection_V1(this._rotation);
        eye.addVectors(target, tmpDist);
    };
})();

GASEngine.OrbitManipulator.prototype.update = (function()
{
    var eye = new GASEngine.Vector3();
    var tmpInverse = new GASEngine.Matrix4();
    return function(dt)
    {
        var delta;
        var mouseFactor = 0.1;
        delta = this._rotate.update(dt);
        this.computeRotation(-delta[0] * mouseFactor * this._scaleMouseMotion, -delta[1] * mouseFactor * this._scaleMouseMotion);

        var panFactor = 0.002;
        delta = this._pan.update(dt);
        this.computePan(-delta[0] * panFactor, -delta[1] * panFactor);

        delta = this._zoom.update(dt);
        this.computeZoom(1.0 + delta[0] / 10.0);

        this._cameraWorldMatrix.multiplyMatrices(this._rotation, this._vrMatrix);

        var target = this._target;
        var distance = this._distance;
        eye.set(0.0, 0.0, distance);
        eye.transformDirection_V1(this._cameraWorldMatrix);
        eye.add(target);

        //this._cameraWorldMatrix.elements[0] = this._rotation.elements[0];
        //this._cameraWorldMatrix.elements[1] = this._rotation.elements[1];
        //this._cameraWorldMatrix.elements[2] = this._rotation.elements[2];
        //this._cameraWorldMatrix.elements[3] = this._rotation.elements[3];

        //this._cameraWorldMatrix.elements[4] = this._rotation.elements[4];
        //this._cameraWorldMatrix.elements[5] = this._rotation.elements[5];
        //this._cameraWorldMatrix.elements[6] = this._rotation.elements[6];
        //this._cameraWorldMatrix.elements[7] = this._rotation.elements[7];

        //this._cameraWorldMatrix.elements[8] = this._rotation.elements[8];
        //this._cameraWorldMatrix.elements[9] = this._rotation.elements[9];
        //this._cameraWorldMatrix.elements[10] = this._rotation.elements[10];
        //this._cameraWorldMatrix.elements[11] = this._rotation.elements[11];

        this._cameraWorldMatrix.elements[12] = eye.x;
        this._cameraWorldMatrix.elements[13] = eye.y;
        this._cameraWorldMatrix.elements[14] = eye.z;
        this._cameraWorldMatrix.elements[15] = 1.0;
    };
})();

GASEngine.OrbitManipulator.prototype.setPoseVR = function(vrMatrix)
{
    this._vrMatrix = vrMatrix;
};

GASEngine.OrbitManipulator.StandardMouseKeyboard = GASEngine.OrbitManipulatorStandardMouseKeyboardController;
GASEngine.OrbitManipulator.DeviceOrientation = GASEngine.OrbitManipulatorDeviceOrientationController;
GASEngine.OrbitManipulator.Hammer = GASEngine.OrbitManipulatorHammerController;