//Author: saralu
//Date: 2019-5-27
//NavigateMode

SceneEditor.NavigateMode = function(manipulator)
{
    SceneEditor.EditorModeBase.call(this);
    this.mode = 'navigate';
    this._manipulator = manipulator;
    this._delay = 10.15;
    this._stepFactor = 1.0;
    this.globalDrag = false;

    this.callbacks =
    {
        mousedown: this.mousedown.bind(this),
        mouseup: this.mouseup.bind(this),
        mousemove: this.mousemove.bind(this),
        mousewheel: this.mousewheel.bind(this),
        click: this.click.bind(this),
        dblclick: this.dblclick.bind(this),
        keydown: this.keydown.bind(this),
        keyup: this.keyup.bind(this)
    };

    var manipulator = this._manipulator.getCurrentManipulator();
    this._stepFactor = 4;
    manipulator.setStepFactor(this._stepFactor);

    
    mgsEditor.on(mgsEditor.EVENTS.toolBar.navigateUniformStepValue, function(value)
    {
        manipulator.setStepFactor(40 * value);
    });
}

SceneEditor.NavigateMode.prototype = Object.create(SceneEditor.EditorModeBase.prototype);
SceneEditor.NavigateMode.prototype.constructor = SceneEditor.NavigateMode;

SceneEditor.NavigateMode.prototype.mousedown = function()
{
    if (event.button !== 2) return;

    var manipulator = this._manipulator.getCurrentManipulator();
    manipulator.getLookPositionInterpolator().reset();
    this.globalDrag = true;
}

SceneEditor.NavigateMode.prototype.mouseup = function()
{
    if (event.button !== 2) return;
    
    this.globalDrag = false;

    var manipulator = this._manipulator.getCurrentManipulator();
    manipulator.getForwardInterpolator().setDelay(this._delay);
    manipulator.getForwardInterpolator().setTarget(0);

    manipulator.getSideInterpolator().setDelay(this._delay);
    manipulator.getSideInterpolator().setTarget(0);

    manipulator.getUpInterpolator().setDelay(this._delay);
    manipulator.getUpInterpolator().setTarget(0);
}

SceneEditor.NavigateMode.prototype.mousewheel = function()
{
    event.preventDefault();
    var delta = 0;
    // Old school scrollwheel delta
    if(event.wheelDelta)
    {
        delta = event.wheelDelta / 120;
    }

    if(event.detail)
    {
        delta = -event.detail / 3;
    }
    var manipulator = this._manipulator.getCurrentManipulator();
    // this._stepFactor = Math.min(Math.max(0.001, this._stepFactor + delta * 0.01), 4.0);
    // manipulator.setStepFactor(this._stepFactor);
    var zoomTarget = manipulator.getZoomInterpolator().getTarget()[0] - delta;
    manipulator.getZoomInterpolator().setTarget(zoomTarget);
}

SceneEditor.NavigateMode.prototype.keydown = function()
{   
    if (!this.globalDrag) return;
    
    var manipulator = this._manipulator.getCurrentManipulator();
    if(event.code === 'KeyW')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(target[0], ++target[1]);
        manipulator.getForwardInterpolator().setDelay( this._delay );
        manipulator.getForwardInterpolator().setTarget(1);
        event.preventDefault();
    }
    if(event.code === 'KeyS')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(target[0], --target[1]);
        manipulator.getForwardInterpolator().setDelay(this._delay);
        manipulator.getForwardInterpolator().setTarget(-1);
        event.preventDefault();
    }
    if(event.code === 'KeyA')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(++target[0], target[1]);
        manipulator.getSideInterpolator().setDelay( this._delay );
        manipulator.getSideInterpolator().setTarget(-1);
        event.preventDefault();
    }
    if(event.code === 'KeyD')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(--target[0], target[1]);
        manipulator.getSideInterpolator().setDelay(this._delay);
        manipulator.getSideInterpolator().setTarget(1);
        event.preventDefault();
    }
    if(event.code === 'KeyQ')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(--target[0], target[1]);
        manipulator.getUpInterpolator().setDelay(this._delay);
        manipulator.getUpInterpolator().setTarget(-1);
        event.preventDefault();
    }
    if(event.code === 'KeyE')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        // var target = manipulator.getLookPositionInterpolator().getTarget();
        // manipulator.getLookPositionInterpolator().setTarget(--target[0], target[1]);
        manipulator.getUpInterpolator().setDelay(this._delay);
        manipulator.getUpInterpolator().setTarget(1);
        event.preventDefault();
    }

    // console.log(event.code);
}

SceneEditor.NavigateMode.prototype.keyup = function()
{
    var manipulator = this._manipulator.getCurrentManipulator();
    if(event.code === 'KeyW' || event.code === 'KeyS')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        manipulator.getForwardInterpolator().setDelay(this._delay);
        manipulator.getForwardInterpolator().setTarget(0);
    }
    if(event.code === 'KeyA' || event.code === 'KeyD')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        manipulator.getSideInterpolator().setDelay(this._delay);
        manipulator.getSideInterpolator().setTarget(0);
    }
    if(event.code === 'KeyQ' || event.code === 'KeyE')
    {
        // manipulator.getLookPositionInterpolator().setDelay(this._delay);
        manipulator.getUpInterpolator().setDelay(this._delay);
        manipulator.getUpInterpolator().setTarget(0);
    }
}

SceneEditor.NavigateMode.prototype.dblclick = function()
{
    var manipulator = SceneEditor.Editor.Instance.cameraManipulator.getCurrentManipulator();
    GASEngine.PBRPipeline.Instance.showHotspot = true;
    var hotspot = GASEngine.HotspotManager.Instance.createHotspot();
    if(hotspot)
    {
        var eye = new GASEngine.Vector3();
        var target = new GASEngine.Vector3();
        manipulator.getEyePosition(eye),
        manipulator.getTarget(target);

        GASEngine.CameraManager.Instance._storeCameraStatusForTest(
            hotspot.id,
            eye,
            target,
            manipulator.getType());
    }
}

SceneEditor.NavigateMode.prototype.update = function()
{
    if(!this.globalDrag) return;

    var rect = this.canvas.getBoundingClientRect();
    var offsetX = SceneEditor.Editor.Instance.mousePos.x - rect.left;
    var offsetY = SceneEditor.Editor.Instance.mousePos.y - rect.top;

    var posX = this.canvas.clientWidth - offsetX;
    var posY = this.canvas.clientHeight - offsetY;
    var manipulator = this._manipulator.getCurrentManipulator();
    manipulator.getLookPositionInterpolator().setDelay(this._delay);
    manipulator.getLookPositionInterpolator().setTarget(-posX, posY);
}