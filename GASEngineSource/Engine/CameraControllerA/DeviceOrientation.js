GASEngine.DeviceOrientation = function(manipulator)
{
    this._manipulator = manipulator;
    this._manipulator.setControllerType('DeviceOrientation');
    this._enable = false;

    // Landscape mobile orientation testing defaults
    this._deviceOrientation = {};
    this._screenOrientation = 0;
};

GASEngine.DeviceOrientation.prototype =
{
    setEnable: function(bool)
    {
        this._enable = bool;
        this._manipulator.setControllerType('DeviceOrientation');
        var controller = this.getManipulatorController();
        controller.reset();
    },

    getEnable: function()
    {
        return this._enable;
    },

    init: function()
    {
        var self = this;

        // Check because Chrome send _one_ event with all angles to null
        window.addEventListener('deviceorientation', function(rawEvtData)
        {
            if(rawEvtData.alpha !== null && rawEvtData.alpha !== undefined)
            {
                self._deviceOrientation = rawEvtData;
            }
        }, false );

        window.addEventListener('orientationchange', function()
        {
            if(screen.orientation !== null)
            {
                //self._screenOrientation = screen.orientation.angle;
                self._screenOrientation = window.orientation || 0;
            }
        }, false );

    },

    getManipulatorController: function()
    {
        return this._manipulator.getCurrentController();
    },

    isValid: function()
    {
        if(!this._enable)
            return false;

        if(!this._deviceOrientation)
            return false;

        if(!this._manipulator)
            return false;

        if(!this.getManipulatorController())
            return false;

        return true;
    },

    update: function()
    {
        if (!this.isValid())
            return;

        // update the manipulator with the rotation of the device
        var manipulatorAdapter = this.getManipulatorController();
        if(manipulatorAdapter.update)
        {
            var alpha = this._deviceOrientation.alpha ? GASEngine.degToRad(this._deviceOrientation.alpha) : 0; // Z
            var beta = this._deviceOrientation.beta ? GASEngine.degToRad(this._deviceOrientation.beta) : 0; // X'
            var gamma = this._deviceOrientation.gamma ? GASEngine.degToRad(this._deviceOrientation.gamma) : 0; // Y''
            var orient = this._screenOrientation ? GASEngine.degToRad(this._screenOrientation) : 0; // O

            manipulatorAdapter.update(alpha, beta, gamma, orient);
        }
    }
};
