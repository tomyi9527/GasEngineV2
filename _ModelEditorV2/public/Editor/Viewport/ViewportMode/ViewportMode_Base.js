(function()
{
    let ViewportMode_Base = function(controller)
    {
        mgs.Object.call(this);

        this._controller = controller;
        this._mode = 'base';
    };
    mgs.classInherit(ViewportMode_Base, mgs.Object);

    ViewportMode_Base.prototype.getModeName = function()
    {
        return this._mode;
    };

    ViewportMode_Base.prototype.update = function(delta)
    {

    };

    ViewportMode_Base.prototype.onEnterMode = function()
    {
    };

    ViewportMode_Base.prototype.onSelectEntity = function()
    {
    };

    ViewportMode_Base.prototype.onMouseDown = function(evt)
    {

    };

    ViewportMode_Base.prototype.onMouseUp = function(evt)
    {
        
    };

    ViewportMode_Base.prototype.onMouseMove = function(evt)
    {

    };

    ViewportMode_Base.prototype.onClick = function(evt)
    {

    };

    ViewportMode_Base.prototype.onDBClick = function(evt)
    {

    };

    ViewportMode_Base.prototype.onMouseWheel = function(evt)
    {

    };

    ViewportMode_Base.prototype.onKeyDown = function(evt)
    {

    };

    ViewportMode_Base.prototype.onKeyUp = function(evt)
    {

    };

    ViewportMode_Base.prototype.onBlur = function(evt)
    {

    };
}());