(function ()
{
    let ActionHandler = function() 
    {
        mgs.Object.call(this);

        this._binderMaps = {};
    };
    mgs.classInherit(ActionHandler, mgs.Object);

    ActionHandler.prototype.getBinders = function(id)
    {
        let binderMap = this._binderMaps[id];
        return binderMap;
    };

    ActionHandler.prototype.bind = function(id, binder)
    {
        let binderMap = this._binderMaps[id];
        if (!binderMap)
        {
            binderMap = {};
            this._binderMaps[id] = binderMap;
        }

        if (!binderMap[binder.uuid])
        {
            binderMap[binder.uuid] = binder;
            binder.id = id;
            binder.handler = this;
            binder.emit('onBind');
        }
    };

    ActionHandler.prototype.unbind = function(binder)
    {
        let binderMap = this._binderMaps[binder.id];
        if (binderMap)
        {
            if (binderMap[binder.uuid])
            {
                binder.emit('PreUnbind');
                delete binderMap[binder.uuid];
                binder.id = null;
                binder.handler =  null;
            }
        }
    };
}());
