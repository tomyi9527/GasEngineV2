GASEngine.Manipulator = function(type)
{
    this._controllerList = {};
    this._cameraWorldMatrix = new GASEngine.Matrix4();
    this._camera = undefined;
    this._type = type;
    this._controllerType = null;
};

GASEngine.Manipulator.ORBIT = 0;
GASEngine.Manipulator.FPS = 1;
GASEngine.Manipulator.AUTOPILOT = 2;

GASEngine.Manipulator.prototype =
{
    setCamera: function(c)
    {
        this._camera = c;
    },

    getCamera: function()
    {
        return this._camera;
    },

    setControllerType: function(type)
    {
        this._controllerType = type;
    },

    getControllerType: function()
    {
        return this._controllerType;
    },

    getType: function()
    {
        return this._type;
    },

    getHomeDistance: function(sceneRadius)
    {
        var dist = sceneRadius;
        if(this._camera)
        {
            var top = Math.tan(GASEngine.degToRad(this._camera.fov * 0.5)) * this._camera.near;
            var bottom = -top;
            var left = this._camera.aspect * bottom;
            var right = this._camera.aspect * top;

            var near = this._camera.near;

            var vertical2 = Math.abs(right - left) / near / 2;
            var horizontal2 = Math.abs(top - bottom) / near / 2;
            dist /= Math.sin(Math.atan2(horizontal2 < vertical2 ? horizontal2 : vertical2, 1));
        }
        else
        {
            dist *= 1.5;
        }

        return dist;
    },

    update: function()
    {
    },

    getCameraWorldMatrix: function()
    {
        return this._cameraWorldMatrix;
    },

    getCurrentController: function()
    {
        if(this._controllerType === null)
            return null;

        return this._controllerList[this._controllerType];
    }
};
