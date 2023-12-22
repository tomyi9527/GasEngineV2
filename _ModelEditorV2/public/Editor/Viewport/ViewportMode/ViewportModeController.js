(function()
{
    let ViewportModeController = function(viewport, application, canvas3D, viewportGizmo, viewportCamera)
    {
        mgs.Events.call(this);

        // input
        this._viewport = viewport;
        this._application = application;
        this._canvas3D = canvas3D;
        this._viewportGizmo = viewportGizmo;
        this._viewportCamera = viewportCamera;

        let canvas = canvas3D.getRoot();
        canvas.addEventListener('mousedown', this._onMouseDown.bind(this), false);
        window.addEventListener('mouseup', this._onMouseUpOutter.bind(this), true);
        window.addEventListener('mousemove', this._onMouseMove.bind(this), false);
        canvas.addEventListener('click', this._onClick.bind(this), false);
        canvas.addEventListener('dblclick', this._onDBClick.bind(this));
        canvas.addEventListener('mousewheel', this._onMouseWheel.bind(this));
        this._canvas3D.on('onKeyDown', this._onKeyDown.bind(this));
        this._canvas3D.on('onKeyUp', this._onKeyUp.bind(this));
        window.addEventListener('blur', this._onBlur.bind(this));

        // modes
        this._currentMode = null;
        this._modes = {};

        this._modes['translate'] = new mgs.ViewportMode_Translate(this);
        this._modes['rotate'] = new mgs.ViewportMode_Rotate(this);
        this._modes['scale'] = new mgs.ViewportMode_Scale(this);

        this._isLocalMode = false;

        // select
        this._selectEntity = null;
        this._selectGizmoInfo = null;
    };
    mgs.classInherit(ViewportModeController, mgs.Events);

    // -------------------------------------------- getter --------------------------------------------
    ViewportModeController.prototype.getViewportGizmo = function()
    {
        return this._viewportGizmo;
    };

    ViewportModeController.prototype.getViewportCamera = function()
    {
        return this._viewportCamera;
    };

    ViewportModeController.prototype.getCanvas3D = function()
    {
        return this._canvas3D;
    };

    ViewportModeController.prototype.getApplication = function()
    {
        return this._application;
    };

    ViewportModeController.prototype.getSceneEntities = function()
    {
        return this._application;
    };

    ViewportModeController.prototype.getViewport = function()
    {
        return this._viewport;
    };

    // -------------------------------------------- mode & select --------------------------------------------
    ViewportModeController.prototype.setCurrentMode = function(mode)
    {
        this._currentMode = this._modes[mode];
        this._currentMode.onEnterMode();
    };

    ViewportModeController.prototype.getCurrentMode = function()
    {
        return this._currentMode;
    };

    ViewportModeController.prototype.selectEntity = function(entity)
    {
        this._selectEntity = entity;
        this._selectGizmoInfo = null;

        if (this._currentMode)
        {
            this._currentMode.onSelectEntity(this._selectEntity);
        }
    };

    ViewportModeController.prototype.getSelectEntity = function()
    {
        return this._selectEntity;
    };

    ViewportModeController.prototype.selectGizmoInfo = function(gizmoInfo)
    {
        this._selectGizmoInfo = gizmoInfo;
    };

    ViewportModeController.prototype._updateGizmo = function(gizmoInfo)
    {
        if (this._selectEntity)
        {

        }
        this._selectGizmoInfo = gizmoInfo;
    };

    ViewportModeController.prototype.setLocalMode = function(isLocalMode)
    {
        this._isLocalMode = isLocalMode;
    };

    ViewportModeController.prototype.isLocalMode = function()
    {
        if (this._currentMode.getModeName() === 'scale')
        {
            return true;
        }
    
        return this._isLocalMode;
    };


    // -------------------------------------------- update --------------------------------------------
    ViewportModeController.prototype.destroy = function()
    {
        let canvas = this._canvas3D.getRoot();

        canvas.removeEventListener('mousedown', this._onMouseDown.bind(this), false);
        window.removeEventListener('mouseup', this.onMouseUpOutter.bind(this), true);
        window.removeEventListener('mousemove', this._onMouseMove.bind(this), false);
        canvas.removeEventListener('click', this._onClick.bind(this), false);
        canvas.removeEventListener('dblclick', this._onDBClick.bind(this));
        canvas.removeEventListener('mousewheel', this._onMouseWheel.bind(this));
        this._canvas3D.unbind('onKeyDown', this._onKeyDown.bind(this));
        this._canvas3D.unbind('onKeyUp', this._onKeyUp.bind(this));
        window.removeEventListener('blur', this._onBlur.bind(this));
    };

    ViewportModeController.prototype.update = function(delta)
    {
        this._updateGizmo();

        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.update(delta);
        }

        
    };

    // -------------------------------------------- events --------------------------------------------
    ViewportModeController.prototype._onMouseDown = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onMouseDown(evt);
        }
    };

    ViewportModeController.prototype._onMouseUpOutter = function(evt)
    {
        if (this._canvas3D.isFocused())
        {
            this._onMouseUp(evt);
        }
    };

    ViewportModeController.prototype._onMouseUp = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onMouseUp(evt);
        }
    };

    ViewportModeController.prototype._onMouseMove = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onMouseMove(evt);
        }
    };

    ViewportModeController.prototype._onClick = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onClick(evt);
        }
    };

    ViewportModeController.prototype._onDBClick = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onDBClick(evt);
        }
    };

    ViewportModeController.prototype._onMouseWheel = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onMouseWheel(evt);
        }
    };

    ViewportModeController.prototype._onKeyDown = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onKeyDown(evt);
        }
    };

    ViewportModeController.prototype._onKeyUp = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onKeyUp(evt);
        }
    };

    ViewportModeController.prototype._onBlur = function(evt)
    {
        let mode = this.getCurrentMode();
        if (mode !== null)
        {
            mode.onBlur(evt);
        }
    };
}());