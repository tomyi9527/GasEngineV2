(function ()
{
    let ControllerBase = function (options) 
    {
        options = options ? options : {};

        mgs.Object.call(this, options);
   
        this._view_ = this.createView(options.viewOptions);
    };
    mgs.classInherit(ControllerBase, mgs.Object);

    ControllerBase.prototype.createView = function(viewOptions)
    {
        console.log(viewOptions);
        return null;
    };

    ControllerBase.prototype.getView = function()
    {
        return this._view_;
    };

    ControllerBase.prototype.update = function(delta)
    {
        return this._view_;
    };
}());