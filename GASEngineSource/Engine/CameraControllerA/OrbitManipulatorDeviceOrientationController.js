GASEngine.OrbitManipulatorDeviceOrientationController = function(manipulator)
{
    this._manipulator = manipulator;
    this.init();
};

GASEngine.OrbitManipulatorDeviceOrientationController.prototype =
{
    init: function()
    {
        this._stepFactor = 1.0;
        this._rotation = new GASEngine.Matrix4();
    },

    update: (function()
    {
        var _z = new GASEngine.Vector3(0, 0, 1);
        var _euler = new GASEngine.Euler();
        var _rotation = new GASEngine.Matrix4();
        var _quat0 = new GASEngine.Quaternion();
        var _quat1 = new GASEngine.Quaternion(-Math.sqrt(0.5), 0, 0, Math.sqrt(0.5));// - PI/2 around the x-axis
        var _quat3 = new GASEngine.Quaternion();

        return function(alpha, beta, gamma, orient)
        {
            _euler.set(beta, alpha, -gamma, 'YXZ');
            _quat3.setFromEuler(_euler);
            _quat3.multiply(_quat1);
            _quat3.multiply(_quat0.setFromAxisAngle(_z, -orient));
            this._rotation.makeRotationFromQuaternion(_quat3);

            this._manipulator.setPoseVR(this._rotation);
        };
    })()
};
