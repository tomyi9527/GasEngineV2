(function ()
{
    // --------------------- DelegateObjectBase --------------------- //
    let DelegateProperty = function(params)
    {
        mgs.Object.call(this);

        this._delegateObject_ = params.delegate;
        this._path_ = params.path;
        this._getter_ = params.getter;
        this._setter_ = params.setter;
        this._editParam_ = params.editParam ? params.editParam : {};

        this._fullPath_ = [];
        mgs.Util.arrayCopy(this._delegateObject_.getRootPath(), this._fullPath_);
        this._fullPath_.push(this._path_);

        // nest delegate
        this._nestDelegateObject_ = null;
        if (this._editParam_.nestDelegateClass)
        {
            this._nestDelegateObject_ = new this._editParam_.nestDelegateClass
            (
                {
                    sourceObject: this._delegateObject_.getSourceObject(),
                    rootPath: this._fullPath_,
                }
            );
        }
    }
    mgs.classInherit(DelegateProperty, mgs.Object);

    DelegateProperty.prototype.getValue = function()
    {
        if (this._getter_)
        {
            return this._getter_();
        }
        else
        {
            // default getter(多数原则)
            let rootObject = this._delegateObject_.getRootObject();
            return rootObject[this._path_];
        }
    };

    DelegateProperty.prototype.setValue = function(value)
    {
        if (this._setter_)
        {
            this._setter_(value);
        }
        else
        {
            // default setter(多数原则)
            let rootObject = this._delegateObject_.getRootObject();
            rootObject[this._path_] = value;
        }
    };

    DelegateProperty.prototype.getPath = function()
    {
        return this._path_;
    };

    DelegateProperty.prototype.getFullPath = function()
    {
        return this._fullPath_;
    };

    DelegateProperty.prototype.getEditorParam = function()
    {
        return this._editParam_;
    };

    DelegateProperty.prototype.getDelegateObject = function()
    {
        return this._delegateObject_;
    };

    DelegateProperty.prototype.getSourceObject = function()
    {
        return this._delegateObject_.getSourceObject();
    };

    DelegateProperty.prototype.getNestDelegateObject = function()
    {
        return this._nestDelegateObject_;
    };

    // --------------------- DelegateObjectBase --------------------- //
    let DelegateObjectBase = function(params) 
    {
        mgs.Object.call(this);

        params = params ? params : {};

        this._id_ = params.id;
        this._managerID_ = params.managerID;
        this._manager_ = params.manager;
        this._sourceObject = params.sourceObject;
        this._rootPath = params.rootPath ? params.rootPath : [];

        this._propertyContainer_ = new mgs.ManagerContainer({objectClass: mgs.DelegateProperty});
    };
    mgs.classInherit(DelegateObjectBase, mgs.Object);

    DelegateObjectBase.prototype.getID = function()
    {
        return this._id_;
    };

    DelegateObjectBase.prototype.getManagerID = function()
    {
        return this._managerID_;
    };

    DelegateObjectBase.prototype.getManager = function()
    {
        return this._manager_;
    };

    DelegateObjectBase.prototype.getSourceObject = function()
    {
        return this._sourceObject;
    };

    DelegateObjectBase.prototype.getRootPath = function()
    {
        return this._rootPath;
    };

    DelegateObjectBase.prototype.getRootObject = function()
    {
        let rootObject = this._sourceObject;
        for (let i = 0;i < this._rootPath.length;i ++)
        {
            rootObject = rootObject[this._rootPath[i]];
        }

        return rootObject;
    };

    // ------ inherit ------ //
    DelegateObjectBase.prototype.onDestroy = function()
    {
    };

    // ------ property ------ //
    DelegateObjectBase.prototype.addProperty = function(propertyParams)
    {
        propertyParams.delegate = this;
        return this._propertyContainer_.add(propertyParams.path, propertyParams);
    };

    DelegateObjectBase.prototype.insertProperty = function(index, propertyParams)
    {
        propertyParams._delegate_ = this;
        return this._propertyContainer_.insertProperty(index, propertyParams.path, propertyParams);
    };

    DelegateObjectBase.prototype.removeProperty = function(path)
    {
        return this._propertyContainer_.delete(path);
    };

    DelegateObjectBase.prototype.getProperty = function(path)
    {
        return this._propertyContainer_.get(path);
    };

    DelegateObjectBase.prototype.getPropertyByFullPath = function(fullPath)
    {
        let property = null;
        let currentDelegateObject = this;

        for (let i = 0;i < fullPath.length;i ++)
        {
            property = currentDelegateObject.getProperty(fullPath[i]);

            let nestDelegateObject = property.getNestDelegateObject();
            if (nestDelegateObject)
            {
                currentDelegateObject = nestDelegateObject;
            }
            else
            {
                break;
            }
        }
        return property;
    };

    DelegateObjectBase.prototype.getPropertyList = function()
    {
        return this._propertyContainer_.getList();
    };

    // --------------------- delegate hirarchy --------------------- //
    DelegateObjectBase.prototype.getChildDelegates = function()
    {
        return [];
    };

    DelegateObjectBase.prototype.getParentDelegate = function()
    { 
        return null;
    };

    // --------------------- map info --------------------- //
    // 路径的属性和ID关联

}());