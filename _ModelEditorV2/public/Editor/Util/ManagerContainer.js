(function ()
{
    // 1，带ID的某类对象集合
    // 2，需要统一获取、创建、删除、插入等操作
    // 3，需要按顺序获取列表

    let ManagerContainer = function(params) 
    {
        mgs.Object.call(this);

        this._objectClass_ = params.objectClass;

        this._containerList_ = [];
        this._containerMap_ = new Map();
    };
    mgs.classInherit(ManagerContainer, mgs.Object);

    ManagerContainer.prototype.add = function(id, params)
    {
        if (this._containerMap_.has(id))
        {
            console.error('id:' + id + ' already existed!');
            return null;
        }

        let obj = new this._objectClass_(params);
        this._containerList_.push(obj);
        this._containerMap_.set(id, obj);
        return obj;
    };

    ManagerContainer.prototype.addByObjectID = function(params)
    {
        let obj = new this._objectClass_(params);
        let id = obj.uuid;
        this._containerList_.push(obj);
        this._containerMap_.set(id, obj);
        return obj;
    };

    ManagerContainer.prototype.insert = function(index, id, params)
    {
        if (this._containerMap_.has(id))
        {
            console.error('id:' + id + ' already existed!');
            return null;
        }

        let obj = new this._objectClass_(params);
        mgs.Util.arrayInsert(this._containerList_, index, obj);
        this._containerMap_.set(id, obj);
        return obj;
    };

    ManagerContainer.prototype.delete = function(id)
    {
        if (!this._containerMap_.has(id))
        {
            console.error('id not existed! : ' + id);
            return null;
        }

        let obj = this._containerMap_.get(id);
        mgs.Util.arrayRemove(this._containerList_, obj);
        this._containerMap_.delete(id);
        return obj;
    };

    ManagerContainer.prototype.clear = function()
    {
        // let count = this._containerList_.length;
        // for (let i = 0;i < count;i ++)
        // {
        //     Object.destroyObject(this._containerList_[i]);
        // }
        this._containerList_.length = 0;
        this._containerMap_.clear();
    };

    ManagerContainer.prototype.get = function(id)
    {
        if (!this._containerMap_.has(id))
        {
            return null;
        }

        let obj = this._containerMap_.get(id);
        return obj;
    };

    ManagerContainer.prototype.getList = function()
    {
        return this._containerList_;
    };

    ManagerContainer.prototype.arrange = function(arrangeRule)
    {
        this._containerList_.sort(arrangeRule);
    };
}());