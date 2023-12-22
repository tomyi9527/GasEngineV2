(function ()
{
    let AssetPanelController = function (options) 
    {
        mgs.ControllerBase.call(this, options);

        let view = this.getView();
        
    };
    mgs.classInherit(AssetPanelController, mgs.ControllerBase);

    AssetPanelController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UIPanel(viewOptions);
        return view;
    };
}());