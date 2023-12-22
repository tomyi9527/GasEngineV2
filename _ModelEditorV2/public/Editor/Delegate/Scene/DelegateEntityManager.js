(function ()
{
    let DelegateEntityManager = function(params) 
    {
        params = params ? params : {};
        params.delegateObjectClass = mgs.DelegateEntity;
        mgs.DelegateObjectManagerBase.call(this, params);

        this._viewport_ = mgs.editor.viewportManager.getViewportBySceneID(this.getID());
    };
    mgs.classInherit(DelegateEntityManager, mgs.DelegateObjectManagerBase);

    DelegateEntityManager.prototype.createEntityDelegates = function(entity)
    {
        let application = this._viewport_.getApplication();
        if (application.isEditorEntity(entity))
        {
            return;
        }

        this.add(entity.uniqueID, {sourceObject: entity});
        for(var i = 0; i < entity.children.length; i++)
        {
            this.createEntityDelegates(entity.children[i]);
        }
    };

    DelegateEntityManager.prototype.init = function()
    {
        let scene = this.getSourceObject();
        
        this.createEntityDelegates(scene.root);
    };

    DelegateEntityManager.prototype.getRootDelegate = function()
    {
        let scene = this.getSourceObject();
        return this.get(scene.root.uniqueID);
    };

    DelegateEntityManager.prototype.deleteEntityDelegates = function(entity)
    {
        let application = this._viewport_.getApplication();
        if (application.isEditorEntity(entity))
        {
            return;
        }

        for(var i = 0; i < entity.children.length; i++)
        {
            this.deleteEntityDelegates(entity.children[i]);
        }
        this.delete(entity.uniqueID);
    };
}());