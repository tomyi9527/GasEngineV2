//Author: saralu
//Date: 2019/7/8
//attribute
SceneEditor.AttributeManager = function(panel)
{
    SceneEditor.PanelManager.call(this, panel);
}

SceneEditor.AttributeManager.prototype = Object.create(SceneEditor.PanelManager.prototype);
SceneEditor.AttributeManager.prototype.constructor = SceneEditor.AttributeManager;


SceneEditor.AttributeManager.prototype.init = function()
{
    this.initBinder();
    this.initCmdHandler();
}

SceneEditor.AttributeManager.prototype.initBinder = function()
{
    let panel = this.panel;
    this._attributeBinder = new mgs.ActionBinder();
    this._attributeBinder.on('onSelect', function(cmd)
    {
        let entityID = cmd.n;
        if(!entityID)
        {
            panel.bindData(null);
            return;
        }
        let entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
        let dEntity = new SceneEditor.DEntity(entity);
        panel.bindData(dEntity);
    });

    let tmp = new GASEngine.Vector3();
    this._attributeBinder.on('onAction', function(cmd)
    {
        // console.log(cmd);
        if(cmd.t === 'set')//修改属性
        {  
            var obj = cmd.p;
            var path = obj.name;
            var pathArr = path.split('/');
            var ui = panel.getItemByPropertyID(path);
            if(Array.isArray(ui))
            // if(typeof ui === 'object')
            {
                tmp.set(cmd.n.x, cmd.n.y, cmd.n.z);
                if(pathArr[0] === 'rotation')
                {
                    tmp.x = GASEngine.radToDeg(tmp.x);
                    tmp.y = GASEngine.radToDeg(tmp.y);
                    tmp.z = GASEngine.radToDeg(tmp.z);
                }
                ui[0].setValue(tmp.x.toFixed(3));
                ui[1].setValue(tmp.y.toFixed(3));
                ui[2].setValue(tmp.z.toFixed(3));
                // ui.x.setValue(tmp.x.toFixed(3));
                // ui.y.setValue(tmp.y.toFixed(3));
                // ui.z.setValue(tmp.z.toFixed(3));
            }
            else
            {
                var tmpStr = cmd.n;
                if(pathArr[0] === 'rotation')
                {
                    tmpStr = GASEngine.radToDeg(tmpStr);
                }
                ui.setValue(tmpStr);
            }
            
        }
        else if(cmd.t === 'delete')//删除属性
        {

        }
    });

    this._attributeHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('s');
    this._attributeHandler.bind('e', this._attributeBinder);

    this._delegateHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('d');
    this._delegateHandler.bind('a', this._attributeBinder);

}

SceneEditor.AttributeManager.prototype.initCmdHandler = function()
{
    let panel = this.panel;
     //set
     mgsEditor.on(mgsEditor.EVENTS.attribute.changed, function(name, value)
     {
        var nameArr = name.split('/');
        if(nameArr[0] === 'rotation') 
        {
            value = GASEngine.degToRad(value);
        }
        let dEntity = panel.dEntity;
        let entityID = dEntity.engineEntity.uniqueID;
        let obj = {entity: entityID, name: name, value: value};
        var op = {
            h: 'd',
            t: 'set',
            i: 'a',
            p: obj
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd]);
     });
}