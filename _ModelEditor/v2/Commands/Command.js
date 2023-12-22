//Author: saralu
//Date: 2019/7/8
//编辑时的命令

//params is a json object
/*
params =
{
    h: 'd'/'s', //handlerType,delegate和selector
    t: 'select'/'set'/'delete'/'insert', // 操作类型
    p: [entity, attribute, next],           // 数据资源的操作路径
    i: 'e', //binderId, e代表entity,a代表attribute
    o: oldValue,                  // 操作前的数值
    n: newValue,                  // 操作后的数值
}
*/
SceneEditor.Command = function(params)
{
    this.h = params.h;
    this.t = params.t;
    this.p = params.p;
    this.i = params.i;
    this.init();
    this.tmpVec3 = new GASEngine.Vector3();
}

SceneEditor.Command.prototype.init = function()
{
    if(this.i === 'e')
    {
        this.initEntityCmd();
    }
    else if(this.i === 'a')
    {
        this.initAttributeCmd();
    }
}

SceneEditor.Command.prototype.getInverse = function()
{
    if(this.i === 'e')
    {
        this.getInverseOfEntityCmd();
    }
    else if(this.i === 'a')
    {
        this.getInverseOfAttributeCmd();
    }
}

SceneEditor.Command.prototype.initEntityCmd = function()
{
    if(this.t === 'select')
    {
        var lastEntity = SceneEditor.Editor.Instance.cmdManager.getLastSelector();
        this.n = this.p;
        this.o = lastEntity;
    }
    else if(this.t === 'set')
    {
        // this.entity[this.attribute] = this.n;
        var entityID = this.p;
        var parent = SceneEditor.Editor.Instance.sceneDelegate.getParentOfEntity(entityID);
        var parentID = parent ? parent.uniqueID : null;
        var obj = {child: entityID, parentNode: parentID};
        this.n = obj;
    }
    else if(this.t === 'delete')
    {
        var entityID = this.p;
        var entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
        var parent = SceneEditor.Editor.Instance.sceneDelegate.getParentOfEntity(entityID);
        var parentId = parent ? parent.uniqueID : null;
        var idx = parent ? parent.findChild(entity) : -1;
        var obj = {child: entityID, parent: parentId, index: idx};
        this.n = obj;
    }
    else if(this.t === 'insert')
    {
        this.n = this.p;
        if(this.p.parent && this.p.child)
        {
            var pID = this.p.parent;
            var cID = this.p.child;
            var child = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(cID);
            var parent = SceneEditor.Editor.Instance.sceneDelegate.getParentOfEntity(cID);
            var next = SceneEditor.Editor.Instance.sceneDelegate.getNextEntity(child, parent);
            var obj;
            if(next)
            {
                obj = {first: cID, next: next.uniqueID};
            }
            else
            {
                obj = {parent: parent.uniqueID, child: cID};
            }

            this.o = obj;
        }
        else if(this.p.first && this.p.next)
        {
            var fID = this.p.first;
            var first = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(fID);
            var parent = SceneEditor.Editor.Instance.sceneDelegate.getParentOfEntity(fID);
            var next = SceneEditor.Editor.Instance.sceneDelegate.getNextEntity(first, parent);
            var obj;
            if(next)
            {
                obj = {first: fID, next: next.uniqueID};
            }
            else
            {
                obj = {parent: parent.uniqueID, child: fID};
            }
            this.o = obj;
        }
    }
}

SceneEditor.Command.prototype.getInverseOfEntityCmd = function()
{
    if(this.t === 'select')
    {
        var tmp = this.n;
        this.n = this.o;
        this.o = tmp;
    }
    else if(this.t === 'set')
    {   
        this.t = 'delete';
        this.o = this.n;
        var entityID = this.n.child;
        var entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
        var parent = SceneEditor.Editor.Instance.sceneDelegate.getParentOfEntity(entityID);
        var parentID = parent ? parent.uniqueID : null;
        var obj = {child: entityID, parent: parentID};
        this.n = obj;
    }
    else if(this.t === 'delete')
    {
        this.t = 'set';
        this.o = this.n;
        var entity = this.n.child;
        var index = this.n.index;
        var pId = this.n.parent;
        var obj = {child: entity, parentNode: pId, index: index};
        this.n = obj;
    }
    else if(this.t === 'insert')
    {
        var tmp = this.n;
        this.n = this.o;
        this.o = tmp;
    }
}

SceneEditor.Command.prototype.initAttributeCmd = function()
{
    if(this.t === 'set')
    {
        var obj = this.p;
        this.n = obj.value;
        if(obj.flag === 'transform')
        {
            this.o = obj.old;
        }
        else
        {
            var entityID = obj.entity;
            var entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
            var dEntity = new SceneEditor.DEntity(entity);
            if(!dEntity) return;
            var path = obj.name;
            var pathArr = path.split('/');
            var attributeId = pathArr[0];
            var attribute = dEntity.getPropertyById(attributeId);
            if(!attribute) return;
            if(pathArr.length === 1)
            {
                this.o = attribute.get();
            }
            else
            {
                tmpVec3 = attribute.get();
                var vecIndex = pathArr[1];
                var num = vecIndex === 'x' ? tmpVec3.x : vecIndex === 'y' ? tmpVec3.y : tmpVec3.z;
                this.o = num;
            }
        }
    }
}


SceneEditor.Command.prototype.getInverseOfAttributeCmd = function()
{
    if(this.t === 'set')
    {
        var tmp = this.n;
        this.n = this.o;
        this.o = tmp;
    }
}