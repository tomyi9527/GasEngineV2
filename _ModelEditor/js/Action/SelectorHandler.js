//Author: saralu
//Date: 2019/7/9
(function ()
{
    let SelectorHandler = function(options)
    {
        mgs.ActionHandler.call(this);

        options = options ? options : {};

        this._assetManager = options.assetManager;

        this._dataOperationHandlers = {};

        this.binders = {};
    }
    mgs.classInherit(SelectorHandler, mgs.ActionHandler);
    
    SelectorHandler.prototype.processCmd = function(cmd)
    {
        let binders = this.getBinders(cmd.i);
        if(binders)
        {
            for(let key in binders)
            {
                binders[key].emit('onSelect', cmd);
            }
        }

    }
}());
