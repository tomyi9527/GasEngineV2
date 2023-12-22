
//sceneDelegate
SceneEditor.DSceneManager = function()
{
    this.init();
}

SceneEditor.DSceneManager.prototype.init = function()
{
    this._dSceneBinder = new mgs.ActionBinder();
    this._dSceneBinder.on('onAction', function(cmd)
    {
       if(cmd.t === 'set')
       {
            let entityID = cmd.n.child;
            let entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
            let rootEntityID = cmd.n.parentNode;
            let rootEntity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(rootEntityID);
            let index = cmd.n.index;
            let next = rootEntity.getChildAt(index);
            rootEntity.addChild(entity, next);
       }
       else if(cmd.t === 'delete')
       {
            var childID = cmd.n.child;
            var child = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(childID);
            SceneEditor.Editor.Instance.deleteEntity(child);
       }
       else if(cmd.t === 'insert')
       {
            var obj = cmd.n;
            if(obj.parent && obj.child)
            {
                var pID = obj.parent;
                var cID = obj.child;
                var parent = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(pID);
                var child = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(cID);
                SceneEditor.Editor.Instance.onDropHeader(parent, child);
            }
            else if(obj.first && obj.next)
            {
                var fID = obj.first;
                var nID = obj.next;
                var first = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(fID);
                var next = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(nID);
                SceneEditor.Editor.Instance.onDropInsertArea(first, next);
            }
       }
    });

    this._delegateHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('d');
    this._delegateHandler.bind('e', this._dSceneBinder);
}

