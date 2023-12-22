(function ()
{
    let DelegateTestManager = function(params) 
    {
        params = params ? params : {};
        params.delegateObjectClass = mgs.DelegateTest;
        mgs.DelegateObjectManagerBase.call(this, params);
    };
    mgs.classInherit(DelegateTestManager, mgs.DelegateObjectManagerBase);

}());