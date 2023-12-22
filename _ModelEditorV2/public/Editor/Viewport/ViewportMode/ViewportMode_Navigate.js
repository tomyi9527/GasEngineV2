(function()
{
    let ViewportMode_Navigate = function(controller)
    {
        mgs.ViewportMode_Base.call(this, controller);
        this._mode = 'navigate';

        this._isDrag = false;
        this._delay = 10.15;

        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();
        manipulator.getCurrentManipulator().getLookPositionInterpolator().reset();
    };
    mgs.classInherit(ViewportMode_Navigate, mgs.ViewportMode_Base);

    ViewportMode_Navigate.prototype.onMouseDown = function(evt)
    {
        if (event.button !== 2) return;

        this._isDrag = true;

        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();
        manipulator.getCurrentManipulator().getLookPositionInterpolator().reset();
    };

    ViewportMode_Navigate.prototype.onMouseUp = function(evt)
    {
        if (event.button !== 2) return;
    
        this._isDrag = false;
    
        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();

        let currentManipulator = manipulator.getCurrentManipulator();
        currentManipulator.getForwardInterpolator().setDelay(this._delay);
        currentManipulator.getForwardInterpolator().setTarget(0);
    
        currentManipulator.getSideInterpolator().setDelay(this._delay);
        currentManipulator.getSideInterpolator().setTarget(0);
    
        currentManipulator.getUpInterpolator().setDelay(this._delay);
        currentManipulator.getUpInterpolator().setTarget(0);
    };

    ViewportMode_Navigate.prototype.onClick = function(evt)
    {

    };

    ViewportMode_Navigate.prototype.onDBClick = function(evt)
    {

    };

    ViewportMode_Navigate.prototype.onMouseWheel = function(evt)
    {
        let delta = 0;

        if(evt.wheelDelta)
        {
            delta = evt.wheelDelta / 120;
        }
    
        if(evt.detail)
        {
            delta = - evt.detail / 3;
        }

        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();

        let currentManipulator = manipulator.getCurrentManipulator();
        let zoomTarget = currentManipulator.getZoomInterpolator().getTarget()[0] - delta;
        currentManipulator.getZoomInterpolator().setTarget(zoomTarget);
    };

    ViewportMode_Navigate.prototype.onKeyDown = function(evt)
    {
        if (!this._isDrag) return;
    
        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();

        let currentManipulator = manipulator.getCurrentManipulator();
        if(evt.code === 'KeyW')
        {
            currentManipulator.getForwardInterpolator().setDelay( this._delay );
            currentManipulator.getForwardInterpolator().setTarget(1);
        }
        if(evt.code === 'KeyS')
        {
            currentManipulator.getForwardInterpolator().setDelay(this._delay);
            currentManipulator.getForwardInterpolator().setTarget(-1);
        }
        if(evt.code === 'KeyA')
        {
            currentManipulator.getSideInterpolator().setDelay( this._delay );
            currentManipulator.getSideInterpolator().setTarget(-1);
        }
        if(evt.code === 'KeyD')
        {
            currentManipulator.getSideInterpolator().setDelay(this._delay);
            currentManipulator.getSideInterpolator().setTarget(1);
        }
        if(evt.code === 'KeyQ')
        {
            currentManipulator.getUpInterpolator().setDelay(this._delay);
            currentManipulator.getUpInterpolator().setTarget(-1);
        }
        if(evt.code === 'KeyE')
        {
            currentManipulator.getUpInterpolator().setDelay(this._delay);
            currentManipulator.getUpInterpolator().setTarget(1);
        }
    };

    ViewportMode_Navigate.prototype.onKeyUp = function(evt)
    {
        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();

        var currentManipulator = manipulator.getCurrentManipulator();
        if(evt.code === 'KeyW' || evt.code === 'KeyS')
        {
            currentManipulator.getForwardInterpolator().setDelay(this._delay);
            currentManipulator.getForwardInterpolator().setTarget(0);
        }
        if(evt.code === 'KeyA' || evt.code === 'KeyD')
        {
            currentManipulator.getSideInterpolator().setDelay(this._delay);
            currentManipulator.getSideInterpolator().setTarget(0);
        }
        if(evt.code === 'KeyQ' || evt.code === 'KeyE')
        {
            currentManipulator.getUpInterpolator().setDelay(this._delay);
            currentManipulator.getUpInterpolator().setTarget(0);
        }
    };

    ViewportMode_Navigate.prototype.onBlur = function(evt)
    {

    };
    
    ViewportMode_Navigate.prototype.update = function(delta)
    {
        if(!this._isDrag) return;

        let canvas = this._controller.getCanvas3D().getRoot();
        let mousePos = mgs.editor.getMousePos();

        let rect = canvas.getBoundingClientRect();
        let offsetX = mousePos.x - rect.left;
        let offsetY = mousePos.y - rect.top;
    
        let posX = canvas.clientWidth - offsetX;
        let posY = canvas.clientHeight - offsetY;
        
        let viewportCamera = this._controller.getViewportCamera();
        let manipulator = viewportCamera.getCurrentManipulator();

        let currentManipulator = manipulator.getCurrentManipulator();
        currentManipulator.getLookPositionInterpolator().setDelay(this._delay);
        currentManipulator.getLookPositionInterpolator().setTarget(-posX, posY);
    };

    ViewportMode_Navigate.prototype.isNavigateDrag = function(evt)
    {
        return this._isDrag;
    };
}());