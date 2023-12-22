(function()
{
    let originQuaterion = new GASEngine.Quaternion();

    let entityWorldPosition = new GASEngine.Vector3();
    let entityWorldQuaternion = new GASEngine.Quaternion();

    let cameraPosition = new GASEngine.Vector3();
    let cameraQuaternion = new GASEngine.Quaternion();
    let cameraScale = new GASEngine.Vector3(1, 1, 1);

    let gizmoScale = new GASEngine.Vector3(1, 1, 1);

    let ViewportMode_Select = function(controller)
    {
        mgs.ViewportMode_Navigate.call(this, controller);

        this._pickerGizmoChildInfo = null;
    };
    mgs.classInherit(ViewportMode_Select, mgs.ViewportMode_Navigate);

    // -------------------------------------------- utils --------------------------------------------
    ViewportMode_Select.prototype._pickerSceneEntity = function()
    {
        let application = this._controller.getApplication(); 

        let canvas = application.getWebGLDevice().canvas;
        let camera = application.getCurrentCamera();

        var mousePos = mgs.editor.getMousePos();
        var pointerPos = GASEngine.Utilities.getPointerPosition1(mousePos, canvas);

        var raycaster = new GASEngine.Raycaster();
        raycaster.setFromCamera(pointerPos, camera);

        let entities = application.getSceneRootChildrenEntities();
        var intersects = raycaster.intersectObjects(entities);
        if (intersects.length > 0)
        {
            let entity = intersects[0].object;
            return entity;
        }

        return null;
    };

    // -------------------------------------------- utils --------------------------------------------
    ViewportMode_Select.prototype._cancelPick = function()
    {
        let viewportGizmo = this._controller.getViewportGizmo();
        let selectEntity = this._controller.getSelectEntity();

        if (this._pickerGizmoChildInfo !== null)
        {
            let modeName = this.getModeName();
            let gizmoItem = viewportGizmo.getGizmoItem(modeName);
            gizmoItem.setChildInfoHover(null);
    
            // end pick
            this.onSelectEvent_EndPick(this._pickerGizmoChildInfo, selectEntity);
            this._pickerGizmoChildInfo = null;
        }
    };

    ViewportMode_Select.prototype._updateGizmoTRS = function(delta, entity)
    {
        let application = this._controller.getApplication(); 
        let camera = application.getCurrentCamera();
        let matrix = camera.getWorldMatrix();
        matrix.decompose(cameraPosition, cameraQuaternion, cameraScale);
        
        let viewportGizmo = this._controller.getViewportGizmo();

        let isLocal = this._controller.isLocalMode();

        // position
        entityWorldPosition.copy(entity.getWorldTranslation());

        // rotation
        if (isLocal)
        {
            entityWorldQuaternion.copy(entity.getWorldQuaternion());
        }
        else
        {
            entityWorldQuaternion.copy(originQuaterion);
        }

        // scale
        var eyeDistance = entityWorldPosition.distanceTo(cameraPosition);
        var ratio = eyeDistance / 6;
        gizmoScale.set(ratio, ratio, ratio);

        viewportGizmo.setGizmoTRS(entityWorldPosition, entityWorldQuaternion, gizmoScale);

        // update 
        viewportGizmo.update(delta);
    };

    ViewportMode_Select.prototype.getNearPoint = function(mousePos, zOffset)
    {
        let application = this._controller.getApplication(); 

        let canvas = application.getWebGLDevice().canvas;
        let camera = application.getCurrentCamera();

        var pointerPos = GASEngine.Utilities.getPointerPosition1(mousePos, canvas);

        var fov = camera.fov;
        var near = camera.near;
        var far = camera.far;
        var aspect = camera.aspect;

        var halfH = near * Math.tan(GASEngine.degToRad(fov * 0.5) );
        var halfW = halfH * aspect;

        var nearPoint = new GASEngine.Vector3(pointerPos.x * halfW, pointerPos.y * halfH, near + (far - near) * (zOffset ? zOffset : 0));
        nearPoint.z = - nearPoint.z;

        // nearPoint.add(cameraPosition);
        nearPoint.applyMatrix4(camera.getWorldMatrix());

        return nearPoint;
    };


    ViewportMode_Select.prototype.getMousePositionRay = function()
    {
        let mousePos = mgs.editor.getMousePos();
        let application = this._controller.getApplication(); 
        let camera = application.getCurrentCamera();

        var ray = new GASEngine.Ray();
        var nearPoint = this.getNearPoint(mousePos);
        ray.origin.setFromMatrixPosition(camera.getWorldMatrix());
        ray.direction.copy(nearPoint).sub(ray.origin).normalize();

        return ray;
    };

    // -------------------------------------------- select events --------------------------------------------
    ViewportMode_Select.prototype.onSelectEvent_BeginPick = function(pickerGizmoChildInfo, selectEntity)
    {
    };

    ViewportMode_Select.prototype.onSelectEvent_EndPick = function(pickerGizmoChildInfo, selectEntity)
    {
    };

    ViewportMode_Select.prototype.onSelectEvent_UpdatePick = function(pickerGizmoChildInfo, selectEntity)
    {
    };


    // -------------------------------------------- events --------------------------------------------
    ViewportMode_Select.prototype.onEnterMode = function()
    {
        let selectEntity = this._controller.getSelectEntity();

        let modeName = this.getModeName();
        let viewportGizmo = this._controller.getViewportGizmo();

        if (selectEntity)
        {
            viewportGizmo.enableGizmoItem(modeName);
        }

        let gizmoItem = viewportGizmo.getGizmoItem(modeName);
        gizmoItem.setChildInfoHover(null);

        this._pickerGizmoChildInfo = null;
    };

    ViewportMode_Select.prototype.onSelectEntity = function(entity)
    {
        let modeName = this.getModeName();
        let viewportGizmo = this._controller.getViewportGizmo();

        if (entity)
        {
            viewportGizmo.enableGizmoItem(modeName);
            this._updateGizmoTRS(0, entity);
        }
        else
        {
            viewportGizmo.enableGizmoItem(null);
        }
        
        let gizmoItem = viewportGizmo.getGizmoItem(modeName);
        gizmoItem.setChildInfoHover(null);

        this._pickerGizmoChildInfo = null;  
    };

    ViewportMode_Select.prototype.onMouseDown = function(evt)
    {
        mgs.ViewportMode_Navigate.prototype.onMouseDown.call(this, evt);

        if (event.button !== 0) return;

        // gizmo pick
        let viewportGizmo = this._controller.getViewportGizmo();
        let selectEntity = this._controller.getSelectEntity();

        if (selectEntity)
        {
            let modeName = this.getModeName();
            this._pickerGizmoChildInfo = viewportGizmo.pickGizmoChildInfo(this._controller.getCurrentMode().getModeName());

            if (this._pickerGizmoChildInfo)
            {
                let gizmoItem = viewportGizmo.getGizmoItem(modeName);
                gizmoItem.setChildInfoHover(this._pickerGizmoChildInfo.name);

                // begin pick
                this.onSelectEvent_BeginPick(this._pickerGizmoChildInfo, selectEntity);
            }
        }
    };


    ViewportMode_Select.prototype.onMouseUp = function(evt)
    {
        mgs.ViewportMode_Navigate.prototype.onMouseUp.call(this, evt);

        if (event.button !== 0) return;

        let selectEntity = this._controller.getSelectEntity();
        
        if (this._pickerGizmoChildInfo !== null)
        {
            this._cancelPick();
        }
        else
        {
            // check pick entity
            let pickerEntity = this._pickerSceneEntity();
            if (pickerEntity !== selectEntity)
            {
                let viewport = this._controller.getViewport(); 
                viewport.onEntitySelect(pickerEntity ? pickerEntity.uniqueID : null);
            }
        }
    };

    ViewportMode_Select.prototype.onBlur = function(evt)
    {
        mgs.ViewportMode_Navigate.prototype.onBlur.call(this, evt);

        // cancel pick
        this._cancelPick();
    };

    ViewportMode_Select.prototype.onKeyDown = function(evt)
    {
        mgs.ViewportMode_Navigate.prototype.onKeyDown.call(this, evt);

        if(evt.code === 'Delete')
        {
            let selectEntity = this._controller.getSelectEntity();
            if (selectEntity)
            {
                let viewport = this._controller.getViewport();
                let preParentID = selectEntity.parent ? selectEntity.parent.uniqueID : null;
                let refID = null;
                if (selectEntity.parent)
                {
                    let index = mgs.Util.arrayElementIndex(selectEntity.parent.children, selectEntity) + 1;
                    let refEntity = selectEntity.parent.children[index];
                    refID = refEntity ? refEntity.uniqueID : null;
                }
                viewport.onEntityDelete(selectEntity.uniqueID, preParentID);
            }
        }
    }; 

    ViewportMode_Select.prototype.update = function(delta)
    {
        mgs.ViewportMode_Navigate.prototype.update.call(this, delta);

        let viewportGizmo = this._controller.getViewportGizmo();
        let selectEntity = this._controller.getSelectEntity();

        if (selectEntity)
        {
            // update gizmo position
            this._updateGizmoTRS(delta, selectEntity);

            if (this._pickerGizmoChildInfo)
            {
                // do pick update
                this.onSelectEvent_UpdatePick(this._pickerGizmoChildInfo, selectEntity);
            }
            else
            {
                // deal hover effect
                let modeName = this.getModeName();
                let pickerGizmoChildInfo = viewportGizmo.pickGizmoChildInfo(modeName);
                if (pickerGizmoChildInfo && (!this.isNavigateDrag()) )
                {
                    let gizmoItem = viewportGizmo.getGizmoItem(modeName);
                    gizmoItem.setChildInfoHover(pickerGizmoChildInfo.name);
                }
                else
                {
                    let gizmoItem = viewportGizmo.getGizmoItem(modeName);
                    gizmoItem.setChildInfoHover(null);
                }
            }
        }
    };
}());