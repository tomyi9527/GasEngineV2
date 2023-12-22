GASEngine.HammerController = function(manipulator)
{
    this._enable = true;
    this._manipulator = manipulator;
    this._manipulator.setControllerType('Hammer');

    this._eventNode = undefined;
};

GASEngine.HammerController.prototype =
{
    setEnable: function(bool)
    {
        this._enable = bool;
        this._manipulator.setControllerType('Hammer');
        var controller = this.getManipulatorController();
        controller.reset();
    },

    getEnable: function()
    {
        return this._enable;
    },

    init: function(options)
    {
        var deviceOptions =
        {
            prevent_default: true,
            drag_max_touches: 2,
            transform_min_scale: 0.08,
            transform_min_rotation: 180,
            transform_always_block: true,
            hold: false,
            release: false,
            swipe: false,
            tap: false
        };

        this._eventNode = options.eventNode;

        if(this._eventNode)
        {
            this._hammer = new Hammer(this._eventNode, deviceOptions);

            //if(options.getBoolean('scrollwheel') === false)
            //{
            //    this._hammer.get('pinch').set({ enable: false });
            //}
            //else
            //{
            //    this._hammer.get('pinch').set({enable: true});
            //}
            this._hammer.get('pinch').set({ enable: true });
        }
    },

    isValid: function()
    {
        if(this._enable && this._manipulator && this.getManipulatorController())
            return true;

        return false;
    },

    getManipulatorController: function()
    {
        return this._manipulator.getCurrentController();
    },

    update: function()
    {
        if(!this.isValid())
            return;

        this.getManipulatorController().setEventProxy(this._hammer);
    },

    remove: function()
    {
        if (!this.isValid())
            return;

        this.getManipulatorController().removeEventProxy(this._hammer);
    }
};