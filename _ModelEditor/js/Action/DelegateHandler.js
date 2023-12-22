//Author: saralu
//Date: 2019/7/9
(function ()
{
    let DelegateHandler = function(options)
    {
        mgs.ActionHandler.call(this);

        options = options ? options : {};

        this._assetManager = options.assetManager;

        this._dataOperationHandlers = {};

        this.binders = {};
    }
    mgs.classInherit(DelegateHandler, mgs.ActionHandler);
    
    DelegateHandler.prototype.processCmd = function(cmd)
    {
        let binders = this.getBinders(cmd.i);
        if(binders)
        {
            for(let key in binders)
            {
                binders[key].emit('onAction', cmd);
            }
        }

    }
}());
