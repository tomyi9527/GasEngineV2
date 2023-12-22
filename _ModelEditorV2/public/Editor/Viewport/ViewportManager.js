(function()
{
    let ViewportManager = function()
    {
        mgs.Object.call(this);

        this._viewportContainer_ = new mgs.ManagerContainer({objectClass: mgs.Viewport});
    };
    mgs.classInherit(ViewportManager, mgs.Object);

    ViewportManager.prototype.createViewport = function(canvas3D)
    {
        return this._viewportContainer_.addByObjectID(canvas3D);
    };

    ViewportManager.prototype.removeViewport = function(viewportID)
    {
        return this._viewportContainer_.delete(viewportID);
    };

    ViewportManager.prototype.getViewport = function(viewportID)
    {
        return this._viewportContainer_.get(viewportID);
    };

    ViewportManager.prototype.getViewportBySceneID = function(sceneID)
    {
        let viewportList = this._viewportContainer_.getList();
        for (let i = 0;i < viewportList.length;i ++)
        {
            let viewport = viewportList[i];
            if (viewport.getSceneID() === sceneID)
            {
                return viewport;
            }
        }
    };
}());