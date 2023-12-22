//Author: saralu
//Date: 2019/7/8
//viewport 
//deal with the canvas and toolbar
SceneEditor.ViewportManager = function()
{
    SceneEditor.PanelManager.call(this);
}

SceneEditor.ViewportManager.prototype = Object.create(SceneEditor.PanelManager.prototype);
SceneEditor.ViewportManager.prototype.constructor = SceneEditor.ViewportManager;


SceneEditor.ViewportManager.prototype.init = function()
{
    this.initEventListener();
    this.initBinder();
    this.initCmdHandler();
}

SceneEditor.ViewportManager.prototype.initEventListener = function()
{
    mgsEditor.on(mgsEditor.EVENTS.toolBar.onTransformSelect, function(transformSelected)
    {
        // console.log(transformSelected);
        if(!transformSelected || !SceneEditor.Editor.Instance.eventInput) return;
        SceneEditor.Editor.Instance.eventInput.setCurrentMode(transformSelected);
    });


    mgsEditor.on(mgsEditor.EVENTS.toolBar.onCheckboxChanged, function(value)
    {
        var checked = value;
        var space = checked ? 'local' : 'world';
        SceneEditor.Editor.Instance.eventInput.setCurrentSpace(space);
    });
}

SceneEditor.ViewportManager.prototype.initBinder = function()
{
    this._viewportBinder = new mgs.ActionBinder();
    this._viewportBinder.on('onSelect', function(cmd)
    {
        let entityID = cmd.n;
        let entity = SceneEditor.Editor.Instance.sceneDelegate.getEntityByID(entityID);
        SceneEditor.Editor.Instance.eventInput.changePick(entity); 
    });


    this._viewportHandler = SceneEditor.Editor.Instance.cmdManager.getHandler('s');
    this._viewportHandler.bind('e', this._viewportBinder);
    
}

SceneEditor.ViewportManager.prototype.initCmdHandler = function()
{
    mgsEditor.on(mgsEditor.EVENTS.viewport.entityClicked, function(entity)
    {
        let id = entity ? entity.uniqueID : null;
        var op = {
            h: 's',
            t: 'select',
            i: 'e',
            p: id
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd]);
    });

    mgsEditor.on(mgsEditor.EVENTS.viewport.transformChanged, function(dEntity, name, value, old, needLog = true)
    {
        let entityID = dEntity ? dEntity.engineEntity.uniqueID : null;
        var obj = {entity: entityID, name: name, value: value, old: old, flag: 'transform'};
        var op = {
            h: 'd',
            t: 'set',
            i: 'a',
            p: obj
        }
        var cmd = new SceneEditor.Command(op);
        SceneEditor.Editor.Instance.cmdManager.processCmds([cmd], needLog);
    });
    

}