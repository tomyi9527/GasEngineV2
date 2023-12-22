
//sceneDelegate
SceneEditor.DEntityManager = function()
{
    this.init();
}

SceneEditor.DEntityManager.prototype.init = function()
{
    this._dEntityBinder = new mgs.ActionBinder();
    var tmpVec3 = new GASEngine.Vector3();
    this._dEntityBinder.on('onAction', function(cmd)
    {
       if(cmd.t === 'set')
       {
           var obj = cmd.p;
           var entityID = obj.entity;
           var entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
           var dEntity = new SceneEditor.DEntity(entity);
           var path = obj.name;
           var pathArr = path.split('/');
           var attributeId = pathArr[0];
           var attribute = dEntity.getPropertyById(attributeId); 
           if(!attribute) return;
           if(pathArr.length === 2) 
           {
                var vecIndex = pathArr[1];
                var num = parseFloat(cmd.n);
                var vec3 = attribute.get(attributeId);
                if(vecIndex === 'x')
                {
                    tmpVec3.set(num, vec3.y, vec3.z);
                }
                else if(vecIndex === 'y')
                {
                    tmpVec3.set(vec3.x, num, vec3.z);
                }
                else if(vecIndex === 'z')
                {
                    tmpVec3.set(vec3.x, vec3.y, num);
                }
                attribute.set(tmpVec3);
           }
           else
           {
                attribute.set(cmd.n);
           }
       }
       else if(cmd.t === 'delete')
       {
           
       }
    });

    this._delegateHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('d');
    this._delegateHandler.bind('a', this._dEntityBinder);
}

