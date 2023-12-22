(function ()
{
    let UICanvas = function(options) 
    {
        options = options || {};
        options.elementTag = 'canvas';

        mgs.UIBase.call(this, options);
    };
    mgs.classInherit(UICanvas, mgs.UIBase);

    UICanvas.prototype.resize = function(width, height)
    {
        let root = this.getRoot();
        if (root.width === width && root.height === height)
        {
            return;
        }

        root.width = width;
        root.height = height;
    };
}());