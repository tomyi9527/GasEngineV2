(function ()
{
    let DelegateAsset = function(params) 
    {
        mgs.DelegateObjectBase.call(this);

        let sourceObject = this.getSourceObject();
    };
    mgs.classInherit(DelegateAsset, mgs.DelegateObjectBase);

}());