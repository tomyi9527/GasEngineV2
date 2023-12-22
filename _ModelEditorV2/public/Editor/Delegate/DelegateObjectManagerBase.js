(function ()
{
    let DelegateObjectManagerBase = function(params) 
    {
        mgs.Object.call(this);

        params = params ? params : {};

        this._id_ = params.id;
        this._sourceObject_ = params.sourceObject;
        this._selectIDs_ = [];
        this._delegateObjectContainer_ = new mgs.ManagerContainer({objectClass: params.delegateObjectClass});
    };
    mgs.classInherit(DelegateObjectManagerBase, mgs.Object);

    // ------ get / set ------ //
    DelegateObjectManagerBase.prototype.getID = function()
    {
        return this._id_;
    };

    DelegateObjectManagerBase.prototype.getSourceObject = function()
    {
        return this._sourceObject_;
    };

    // ------ data operation ------ //
    DelegateObjectManagerBase.prototype.init = function()
    {

    };

    DelegateObjectManagerBase.prototype.clear = function()
    {
        this._delegateObjectContainer_.clear();
    };

    DelegateObjectManagerBase.prototype.add = function(id, params)
    {
        params.id = id;
        params.managerID = this._id_;
        params.manager = this;
        return this._delegateObjectContainer_.add(id, params);
    };

    DelegateObjectManagerBase.prototype.delete = function(id)
    {
        return this._delegateObjectContainer_.delete(id);
    };

    DelegateObjectManagerBase.prototype.get = function(id)
    {
        return this._delegateObjectContainer_.get(id);
    };

    DelegateObjectManagerBase.prototype.move = function(fromID, toID)
    {

    };

    DelegateObjectManagerBase.prototype.select = function(ids)
    {
        mgs.Util.arrayCopy(ids, this._selectIDs_);
    };
}());