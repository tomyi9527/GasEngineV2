//Author: saralu
//Date: 2019-5-28
//SelectMode

SceneEditor.SelectMode = function(manipulator)
{
    SceneEditor.NavigateMode.call(this, manipulator);
    this.mode = 'select';

    this.hotspotDrag = false;
    this.drag = false;
    this.pickInfo = undefined;
    // this.gizmoPickers = null;
    this.gizmoPicker = null;
    // this.realEntityForMove = null;
    this.dEntity = null;
    // this.gizmos = null;
    this.needLog = false;
    this.logInfo = null;
}

SceneEditor.SelectMode.prototype = Object.create(SceneEditor.NavigateMode.prototype);
SceneEditor.SelectMode.prototype.constructor = SceneEditor.SelectMode;

SceneEditor.SelectMode.prototype.pickGizmo = function()
{
    var children = this.dEntity ? this.dEntity.picker ? this.dEntity.picker.children : [] : [];
    return SceneEditor.Editor.Instance.eventInput.getIntersectWrapper(children);
},

SceneEditor.SelectMode.prototype.pickObject = function()
{
    var pointerPos = GASEngine.Utilities.getPointerPosition(this.canvas);
    var pickInfo = SceneEditor.Editor.Instance.sceneInstance.pickPosition_V2(pointerPos.x, pointerPos.y);
    return pickInfo;
}

SceneEditor.SelectMode.prototype.mousedown = function()
{
    var hotspot = GASEngine.HotspotManager.Instance.pickHotspot();
    if(hotspot)
    {
        this.hotspotDrag = GASEngine.HotspotManager.Instance.startDrag();
        if(this.hotspotDrag)
        {
            return;
        }
    }

    if(this.dEntity)
    { 
        this.pickInfo = this.pickGizmo();
    }
    else
    {
        this.pickInfo = this.pickObject();
    }

    SceneEditor.NavigateMode.prototype.mousedown.call(this);
}

SceneEditor.SelectMode.prototype.mouseup = function()
{
    SceneEditor.NavigateMode.prototype.mouseup.call(this);

    if(this.hotspotDrag)
    {
        GASEngine.HotspotManager.Instance.endDrag();
        this.hotspotDrag = false;
        return;
    }
  
    if(event.button === 0 && event.cancelBubble === false && !this.drag)
    {
        var pickInfo = this.pickObject();
        if(pickInfo && pickInfo.object.type !== 'helper')
        {
            // this.dEntity = new SceneEditor.DEntity(pickInfo.object);  
            // if(!(this.dEntity && this.dEntity.uniqueID === pickInfo.object.uniqueID))
            // {
            //     SceneEditor.EditorModeController.prototype.removeGizmos(this.dEntity);
            //     this.dEntity = new SceneEditor.DEntity(pickInfo.object);  
            //     SceneEditor.EditorModeController.prototype.initGizmos(this.dEntity);
            // }
            // SceneEditor.EditorModeController.prototype.removeGizmos(this.dEntity);
            SceneEditor.Editor.Instance.eventInput.removeGizmos(this.dEntity);
            this.dEntity = new SceneEditor.DEntity(pickInfo.object);  
            // SceneEditor.EditorModeController.prototype.initGizmos(this.dEntity);
            SceneEditor.Editor.Instance.eventInput.initGizmos(this.dEntity);
        }
        else 
        {
            var pickInfo = this.pickGizmo();
            if(!pickInfo && this.dEntity)
            {
                if(!this.pickInfo)
                {
                    // SceneEditor.EditorModeController.prototype.removeGizmos(this.dEntity);
                    SceneEditor.Editor.Instance.eventInput.removeGizmos(this.dEntity);
                    this.dEntity = null;
                }
            }
        }

        // SceneEditor.EditorModeController.prototype.selectedObjectChangedCallback(this.dEntity); 
        SceneEditor.Editor.Instance.eventInput.selectedObjectChangedCallback(this.dEntity); 
    }

    
    if(this.needLog)
    {
        mgsEditor.emit(mgsEditor.EVENTS.viewport.transformChanged, this.dEntity, this.logInfo.name, this.logInfo.value, this.logInfo.old); 
        this.needLog = false;
    }
    
    this.pickInfo = undefined;
}

SceneEditor.SelectMode.prototype.update = function()
{
    SceneEditor.NavigateMode.prototype.update.call(this);    

    // mouse move codes
    {
        var mousePos = SceneEditor.Editor.Instance.mousePos;
        var rect = this.canvas.getBoundingClientRect();
        var offsetX = mousePos.x - rect.left;
        var offsetY = mousePos.y - rect.top;

        GASEngine.HotspotManager.Instance.setMousePosition(offsetX, offsetY);
        if(this.hotspotDrag)
        {
            GASEngine.HotspotManager.Instance.drag();
            return;
        }

        if(this.dEntity && !this.drag)
        {
            var pickInfo = this.pickGizmo();
            if(pickInfo)
            {
                this.pickInfo = pickInfo;
                this.gizmoPicker = this.pickInfo.object;
                // console.log(this.gizmoPicker);
                // console.log(this.pickInfo.point);
            }
            else
            {
                this.pickInfo = null;
                this.gizmoPicker = null;
            }
        }
    }
}