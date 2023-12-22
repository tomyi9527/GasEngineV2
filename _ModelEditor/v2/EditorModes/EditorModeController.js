//Author: saralu
//Date: 2019-5-28
//EditorModeController

SceneEditor.EditorModeController = function(manipulator)
{
    this._manipulator = manipulator;
    this._mouseEventNode = undefined;
    this._wheelEventNode = undefined;
    this._keyboardEventNode = undefined;
    this._eventList = ['mousedown', 'mouseup', 'mousemove', 'click', 'dblclick', 'mousewheel', 'keydown', 'keyup', 'blur'];
    this._mousePosition = new GASEngine.Vector2();


    this.editorModeList = {};
    this.currentMode = null;
    this.currentSpace = null;
    // this.initMode = true;
    this.currentPickObject = null;

    this.currentModeTab = null;

    //used for global plane
    this.globalPlane = null;
    this.unitX = new GASEngine.Vector3( 1, 0, 0 );
	this.unitY = new GASEngine.Vector3( 0, 1, 0 );
    this.unitZ = new GASEngine.Vector3( 0, 0, 1 );
    
    this.tmpX = new GASEngine.Vector3();
    this.tmpY = new GASEngine.Vector3();
    this.tmpZ = new GASEngine.Vector3();
    this.applyQuaternion = new GASEngine.Quaternion();
    this.worldQuaternionInv = new GASEngine.Quaternion();
    this.pickName = null;

    this.identityQuaternion = new GASEngine.Quaternion();

    
    this.worldQuaternion = new GASEngine.Quaternion();
    this.worldPosition = new GASEngine.Vector3();
    this.worldScale = new GASEngine.Vector3();
    
    this.parentQuaternion = new GASEngine.Quaternion();
    this.parentPosition = new GASEngine.Vector3();
    this.parentScale = new GASEngine.Vector3();

    this.parentQuaternionInv = new GASEngine.Quaternion();
    
    this.tempQuaternion = new GASEngine.Quaternion();
    this.rotateQuaternionPre = new GASEngine.Quaternion();
    this.tempVector = new GASEngine.Vector3();
    this.tempVector2 = new GASEngine.Vector3();

    this.eye = new GASEngine.Vector3();

    this.cameraPosition = new GASEngine.Vector3();
    this.cameraQuaternion = new GASEngine.Quaternion();
    this.cameraScale = new GASEngine.Vector3(1, 1, 1);

    // this.canvas = document.getElementById('viewport-canvas');
    this.canvas = SceneEditor.Editor.Instance.canvas;

    var opacity_Hover = 1.0;
    var opacity_Disabled = 0.25;
    var opacity_Normal = 1.0;
    var opacity_Plane_Hover = 1.0;
    var opacity_Plane_Disabled = 0.1;
    var opacity_Plane_Normal = 0.25;
    var opacity_Sphere_Hover = 0.5;
    var opacity_Sphere_Disabled = 0.0;
    var opacity_Sphere_Normal = 0.0;
    // var opacity_Sphere_Hover = 0.5;//for debug
    // var opacity_Sphere_Disabled = 0.1;
    // var opacity_Sphere_Normal = 0.1;
    this.opacityMap = {
        plane: {
            hover: opacity_Plane_Hover,
            disabled: opacity_Plane_Disabled,
            normal: opacity_Plane_Normal
        },
        sphere: {
            hover: opacity_Sphere_Hover,
            disabled: opacity_Sphere_Disabled,
            normal: opacity_Sphere_Normal
        },
        other: {
            hover: opacity_Hover,
            disabled: opacity_Disabled,
            normal: opacity_Normal
        }
    };
    this.hoverColor = [1.0, 1.0, 0.0];
    this.tmpPosition = new GASEngine.Vector3();
    this.tmpScale = new GASEngine.Vector3();
    this.tmpRotation = new GASEngine.Euler();
    this.tmpQuaternion = new GASEngine.Quaternion();
}

SceneEditor.EditorModeController.prototype.changePick = function(entity)
{
    this.removeGizmos(this.currentPickObject);
    this.currentPickObject = null;
    if(entity)
    {
        this.currentPickObject = new SceneEditor.DEntity(entity);
        this.initGizmos(this.currentPickObject);
    }
    // mgsEditor.emit(mgsEditor.EVENTS.onAssetSelectedEnd, this.currentPickObject); 
    this.setCurrentModeInner();
}

// SceneEditor.EditorModeController.prototype.deletePick = function(entity)
// {
//     this.removeGizmos(this.currentPickObject);
//     this.currentPickObject = null;
//     // mgsEditor.emit(mgsEditor.EVENTS.onAssetDeleteEnd, null); 
//     this.setCurrentModeInner();
// }

SceneEditor.EditorModeController.prototype.init = function(canvas)
{
    var navigateMode = new SceneEditor.NavigateMode(this._manipulator);
    this.editorModeList['navigate'] = navigateMode;

    var selectMode = new SceneEditor.SelectMode(this._manipulator);
    this.editorModeList['select'] = selectMode;

    var translateMode = new SceneEditor.TranslateMode(this._manipulator);
    this.editorModeList['translate'] = translateMode;

    var rotateMode = new SceneEditor.RotateMode(this._manipulator);
    this.editorModeList['rotate'] = rotateMode;

    var scaleMode = new SceneEditor.ScaleMode(this._manipulator);
    this.editorModeList['scale'] = scaleMode;

    this.currentMode = 'translate';
    this.currentModeTab = 'translate';
    this.currentSpace = 'world';

    this.matrix = new GASEngine.Matrix4();
    this.EGizmoQuaternion = new GASEngine.Quaternion();

    var node = canvas || document;
    this.addEventListeners(node);
}

SceneEditor.EditorModeController.prototype.onMouseDown = function(evt)
{
    this.getCurrentModeController().callbacks['mousedown'](evt);
};

SceneEditor.EditorModeController.prototype.onMouseUp = function(evt)
{
    var canvas3d = SceneEditor.Editor.Instance.editorLayout.sceneEditor.getCanvas3d();

    if (canvas3d.isFocused())
    {
        this.getCurrentModeController().callbacks['mouseup'](evt);
    }
};

SceneEditor.EditorModeController.prototype.onMouseMove = function(evt)
{
    this.getCurrentModeController().callbacks['mousemove'](evt);
};

SceneEditor.EditorModeController.prototype.onClick = function(evt)
{
    this.getCurrentModeController().callbacks['click'](evt);
};

SceneEditor.EditorModeController.prototype.onDBClick = function(evt)
{
    this.getCurrentModeController().callbacks['dblclick'](evt);
};

SceneEditor.EditorModeController.prototype.onMouseWheel = function(evt)
{
    this.getCurrentModeController().callbacks['mousewheel'](evt);
};

SceneEditor.EditorModeController.prototype.onKeyDown = function(evt)
{
    this.getCurrentModeController().callbacks['keydown'](evt);
};

SceneEditor.EditorModeController.prototype.onKeyUp = function(evt)
{
    this.getCurrentModeController().callbacks['keyup'](evt);
};

SceneEditor.EditorModeController.prototype.onBlur = function(evt)
{
    this.getCurrentModeController().blur(evt);
};

SceneEditor.EditorModeController.prototype.addEventListeners = function(node)
{
    var canvas3d = SceneEditor.Editor.Instance.editorLayout.sceneEditor.getCanvas3d();
    var canvas = canvas3d.getRoot();

    var cbMouse = node && node['addEventListener'].bind(node);
    for(var i = 0; i < this._eventList.length; i++)
    {
        var ev = this._eventList[i];
        // window.addEventListener(ev, this.getCurrentModeController().callbacks[ev]);
        // cbMouse(ev, this.getCurrentModeController().callbacks[ev]);

        if (ev === 'mousedown')
        {
            canvas.addEventListener(ev, this.onMouseDown.bind(this));
        }
        else if (ev === 'mouseup')
        {
            window.addEventListener(ev, this.onMouseUp.bind(this), true);
        }
        else if (ev === 'mousemove')
        {
            canvas.addEventListener(ev, this.onMouseMove.bind(this));
        }
        else if (ev === 'click')
        {
            canvas.addEventListener(ev, this.onClick.bind(this));
        }
        else if (ev === 'dblclick')
        {
            canvas.addEventListener(ev, this.onDBClick.bind(this));
        }
        else if (ev === 'mousewheel')
        {
            canvas.addEventListener(ev, this.onMouseWheel.bind(this));
        }
        else if (ev === 'keydown')
        {
            canvas3d.on('onKeyDown', this.onKeyDown.bind(this));
        }
        else if (ev === 'keyup')
        {
            canvas3d.on('onKeyUp', this.onKeyUp.bind(this));
        }
        else if (ev === 'blur')
        {
            window.addEventListener('blur', this.onBlur.bind(this));
        }
    }
}

SceneEditor.EditorModeController.prototype.removeEventListeners = function(node)
{
    // var cbMouse = node && node['removeEventListener'].bind(node);
    // for(var i = 0; i < this._eventList.length; i++)
    // {
    //     var ev = this._eventList[i];
    //     window.removeEventListener(ev, this.getCurrentModeController().callbacks[ev]);
    //     // cbMouse(ev, this.getCurrentModeController().callbacks[ev]);
    // }
}


SceneEditor.EditorModeController.prototype.getCurrentMode = function()
{
    return this.currentMode;
}

SceneEditor.EditorModeController.prototype.setCurrentModeInner = function(mode)
{
    mode = mode || this.currentModeTab ;
    if(mode === this.currentMode) return;
    // var canvas = document.getElementById('viewport-canvas');
    // this.canvas = SceneEditor.Editor.Instance.canvas;
    // var node = this.canvas || document;
    // this.removeEventListeners(node);
    this.currentMode = mode;
    // this.addEventListeners(node);
    this.changeGizmo();
}


SceneEditor.EditorModeController.prototype.setCurrentMode = function(mode)
{
    if(mode === this.currentModeTab) return;
    this.currentModeTab = mode;
    if(this.currentPickObject)
    {
        this.setCurrentModeInner(this.currentModeTab);
    }
    else 
    {
        this.setCurrentModeInner('select');
    }
}

SceneEditor.EditorModeController.prototype.setCurrentSpace = function(space)
{
    if(space === this.currentSpace) return;
    this.currentSpace = space;
    if(!this.currentPickObject) return;
    this._updateRotationGizmo();
    //TODO: 旋转gizmo
    // this.updateGizmo();
}


SceneEditor.EditorModeController.prototype.changeGizmo = function()
{
    if(!this.currentPickObject || !this.currentPickObject.gizmo) return;
    this.removeGizmos(this.currentPickObject);
    this.initGizmos(this.currentPickObject);
    // this.removeHelper();
    // this.addHelper();
}

SceneEditor.EditorModeController.prototype.getHelperEntity = function(entity)
{
    while(entity !== null)
    {
        if(!entity.parent || entity.parent.type !== 'helper')
        {
            break;
        }
        else
        {
            entity = entity.parent;
        }
    }
    return entity;
};

SceneEditor.EditorModeController.prototype.getCurrentModeController = function()
{
    return this.editorModeList[this.currentMode];
}

SceneEditor.EditorModeController.prototype.getIntersectWrapper = function(entities)
{
    var mousePos = SceneEditor.Editor.Instance.mousePos;
    var pointerPos = GASEngine.Utilities.getPointerPosition1(mousePos, this.canvas);
    // var pointerPos = GASEngine.Utilities.getPointerPosition(this.canvas);
    var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    var raycaster = new GASEngine.Raycaster();
    raycaster.setFromCamera(pointerPos, cameras[0]);
    var intersects = raycaster.intersectObjects(entities);
    return intersects.length > 0 ? intersects[0] : null;
}

SceneEditor.EditorModeController.prototype.getRotateRadius = function()
{
    if(this.currentPickObject.gizmo)
    {
        // var eyeDistance = this.worldPosition.distanceTo(this.cameraPosition);
        // var ratio = eyeDistance;

        return this.currentPickObject.gizmo.scale.x;
    }
}

SceneEditor.EditorModeController.prototype.getNearPoint = function(mouseCoordinates, zOffset)
{
    var pointerPos = GASEngine.Utilities.getPointerPosition1(mouseCoordinates, this.canvas);

    var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    var cameraComponent = cameras[0];

    var fov = cameraComponent.fov;
    var near = cameraComponent.near;
    var far = cameraComponent.far;
    var aspect = cameraComponent.aspect;

    var halfH = near * Math.tan(GASEngine.degToRad(fov * 0.5) );
    var halfW = halfH * aspect;

    var nearPoint = new GASEngine.Vector3(pointerPos.x * halfW, pointerPos.y * halfH, near + (far - near) * (zOffset ? zOffset : 0));
    nearPoint.z = - nearPoint.z;

    // nearPoint.add(cameraPosition);
    nearPoint.applyMatrix4(cameraComponent.getWorldMatrix());

    return nearPoint;
};

SceneEditor.EditorModeController.prototype.getMousePositionRay = function(mouseCoordinates)
{
    // {
    //     var pointerPos = GASEngine.Utilities.getPointerPosition1(mouseCoordinates, this.canvas);
    //     var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    //     var raycaster = new GASEngine.Raycaster();
    //     raycaster.setFromCamera(pointerPos, cameras[0]);
    //     // return raycaster.ray;
    //     console.log(raycaster.ray);
    // }

    var ray = new GASEngine.Ray();
    var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    var cameraComponent = cameras[0];

    var nearPoint = this.getNearPoint(mouseCoordinates);
    ray.origin.setFromMatrixPosition(cameraComponent.getWorldMatrix());
    ray.direction.copy(nearPoint).sub(ray.origin).normalize();

    return ray;
};

SceneEditor.EditorModeController.prototype.getPointerRay = function()
{
    var pointerPos = GASEngine.Utilities.getPointerPosition(this.canvas);
    var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    var raycaster = new GASEngine.Raycaster();
    raycaster.setFromCamera2(pointerPos, cameras[0]);
    return raycaster;
}

SceneEditor.EditorModeController.prototype.getNearPlanePoint = function(mouseCoordinates, cameraComponent) 
{
    if((cameraComponent && cameraComponent.type === 'perspective')) 
    {
        var matrix4 = new GASEngine.Matrix4();
        matrix4.getInverse(cameraComponent.getProjectionMatrix());

        var nearPlanePoint = new GASEngine.Vector3();
        nearPlanePoint.set(mouseCoordinates.x, mouseCoordinates.y, 0.0);
        nearPlanePoint.applyMatrix4(matrix4);
        nearPlanePoint.applyMatrix4(cameraComponent.getWorldMatrix());
        return nearPlanePoint
    } 
    else if((cameraComponent && cameraComponent.type === 'orthographic')) 
    {
        var nearPlanePoint = new GASEngine.Vector3();

        // set origin in plane of camera
        nearPlanePoint.set(mouseCoordinates.x, mouseCoordinates.y, (cameraComponent.near + cameraComponent.far) / (cameraComponent.near - cameraComponent.far));
        nearPlanePoint.unproject(cameraComponent); 
        return nearPlanePoint
    } 
    else 
    {
        console.error('GASEngine.Raycaster: Unsupported camera type.');
    }
}

SceneEditor.EditorModeController.prototype.getPointMapInPlane = function(point, planeNormal, planeOffset)
{
    // mapPoint = point + (planeOffset - point * planeNormal) * planeNormal;
    var mapPoint = new GASEngine.Vector3(); 
    mapPoint.copy(point);
    var dot = point.dot(planeNormal);
    var scaleV = new GASEngine.Vector3();
    scaleV.multiplyScalar(planeOffset - dot);
    mapPoint.add(scaleV);
    return mapPoint;
}

SceneEditor.EditorModeController.prototype.pointerIntersectPlane = function()
{
    var intersect = this.getIntersectWrapper([this.globalPlane]);
    var emptyVec = new GASEngine.Vector3();
    return intersect ? intersect.point: emptyVec;
}

SceneEditor.EditorModeController.prototype.update = function()
{
    var currentController = this.getCurrentModeController();
    if(this.currentPickObject)
    // if(currentController.dEntity)
    {
        // set the scale of helper to get same gizmo size 
        // var entity = currentController.dEntity.realEntity;
        var entity = this.currentPickObject.realEntity;
        if(!entity) return;
        this.worldPosition.copy(entity.getWorldTranslation());
        this.worldQuaternion.copy(entity.getWorldQuaternion());

        this.parentQuaternionInv.copy( this.parentQuaternion ).inverse();
        this.worldQuaternionInv.copy( this.worldQuaternion ).inverse();
        this.worldQuaternionInv.multiply(this.parentQuaternionInv);
       
        currentController.dEntity = this.currentPickObject;
        // currentController.dEntity = new SceneEditor.DEntity(this.currentPickObject);
        // currentController.realEntityForMove = entity;
        // currentController.realEntityForMove = this.dEntity;
        
        var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
        var matrix = cameras[0].getWorldMatrix();
        matrix.decompose(this.cameraPosition, this.cameraQuaternion, this.cameraScale);


        this.eye.copy(this.cameraPosition).sub(this.worldPosition).normalize();

        // var eyeTmp = new GASEngine.Vector3();
        // var targetTmp = new GASEngine.Vector3();
        // SceneEditor.Editor.Instance.cameraManipulator.getEyePosition(eyeTmp);
        // SceneEditor.Editor.Instance.cameraManipulator.getTarget(targetTmp);
        // var eyeDistance = eyeTmp.distanceTo(targetTmp);
        // var distance = eyeTmp.distanceTo(this.worldPosition);
        // var ratio = distance === 0 ? 1 : distance / eyeDistance;


        var eyeDistance = this.worldPosition.distanceTo(this.cameraPosition);
        var ratio = eyeDistance / 6;

        // end of set the scale of helper to get same gizmo size 
        
        this.pickName = currentController.gizmoPicker ? currentController.gizmoPicker.name : this.pickName;
        if(currentController.dEntity.gizmo)
        {
            this._updateRenderGizmo(currentController);
            this._updateRotationGizmo();   
            // this.currentPickObject.picker.scale.set( 1, 1, 1 ).multiplyScalar( ratio );
            // this.currentPickObject.gizmo.scale.set( 1, 1, 1 ).multiplyScalar( ratio );
            // this.currentPickObject.picker.translation.copy(this.worldPosition);
            // this.currentPickObject.gizmo.translation.copy(this.worldPosition);

            this.tmpScale.set( 1, 1, 1 ).multiplyScalar(ratio);
            this.currentPickObject.picker.setLocalScale(this.tmpScale);
            this.currentPickObject.gizmo.setLocalScale(this.tmpScale);
            this.currentPickObject.picker.setLocalTranslation(this.worldPosition);
            this.currentPickObject.gizmo.setLocalTranslation(this.worldPosition);

            this.currentPickObject.picker.update();
            this.currentPickObject.gizmo.update();

            // currentController.dEntity.picker.scale.set( 1, 1, 1 ).multiplyScalar( ratio );
            // currentController.dEntity.gizmo.scale.set( 1, 1, 1 ).multiplyScalar( ratio );
            // currentController.dEntity.picker.translation.copy(entity.translation);
            // currentController.dEntity.gizmo.translation.copy(entity.translation);

            // currentController.dEntity.picker.update();
            // currentController.dEntity.gizmo.update();
        }
    }

    currentController.update();
}


SceneEditor.EditorModeController.prototype._updateRotationGizmo = function()
{
    if(!this.currentPickObject.picker || !this.currentPickObject.gizmo)
    {
        return;
    }
    var space = this.currentSpace;
    if(this.currentMode === 'scale') space = 'local';
    if(space === 'local')
    {
        this._updateRotationGizmoInner(this.currentPickObject.picker, 'local');
        this._updateRotationGizmoInner(this.currentPickObject.gizmo, 'local');
    }
    else 
    {
        if(this.currentSpace === 'world')
        {
            this._updateRotationGizmoInner(this.currentPickObject.picker, 'world');
            this._updateRotationGizmoInner(this.currentPickObject.gizmo, 'world');
        }
    }
}


SceneEditor.EditorModeController.prototype._updateRotationGizmoInner = function(gizmos, space)
{
    for(var i = 0; i< gizmos.children.length; i++)
    {
        var gizmo = gizmos.children[i];
        var name = gizmo.name;
        if ( name === 'E' || name === 'E1' ) 
        {
            // gizmo.rotation.setFromQuaternion(this.cameraQuaternion, 'XYZ');
            
            this.matrix.lookAt(this.worldPosition, this.cameraPosition, this.unitY);
            this.EGizmoQuaternion.setFromRotationMatrix(this.matrix);
            // gizmo.rotation.setFromQuaternion(this.EGizmoQuaternion, 'XYZ');
            gizmo.setLocalQuaternion(this.EGizmoQuaternion);
        } 
        else 
        {
            if(space === 'local')
            {
                // gizmo.rotation.setFromQuaternion(this.worldQuaternion, 'XYZ');
                gizmo.setLocalQuaternion(this.worldQuaternion);
            }
            else
            {
                gizmo.matrixWorld.decompose(this.tempVector, this.rotateQuaternionPre, this.tempVector2 );
                this.tempQuaternion.copy( this.rotateQuaternionPre ).inverse();
                this.rotateQuaternionPre.multiply(this.tempQuaternion);
                // gizmo.rotation.setFromQuaternion(this.rotateQuaternionPre, 'XYZ');
                gizmo.setLocalQuaternion(this.rotateQuaternionPre);
            }

        }
    }
    return;
}

SceneEditor.EditorModeController.prototype._updateRenderGizmo = function(controller)
{
    var name = controller.gizmoPicker ? controller.gizmoPicker.name : null;

    var gizmos = controller.dEntity.gizmo;
    var children = gizmos ? gizmos.children : [];
    this._updateRenderGizmo_r(children, name);
}


SceneEditor.EditorModeController.prototype._updateRenderGizmo_r = function(gizmos, name)
{
    for(var i = 0; i< gizmos.length; i++)
    {
        var gizmo = gizmos[i];
        var state = name === null ? 'normal' : name === gizmo.name ? 'hover' : 'normal';
        var meshFilterComponent = gizmo.getComponent('meshFilter');
        var meshRenderer = gizmo.getComponent('meshRenderer');
        if(meshFilterComponent && meshRenderer)
        {
            var mesh = meshFilterComponent.getMesh();
            var materials = meshRenderer.getMaterials();
            var type = mesh.type === 'plane' ? 'plane' :  mesh.type === 'sphere' ? 'sphere' : 'other';
            this._updateGizmoColor(materials, state, type);
        }

        var children = gizmo ? gizmo.children : [];
        this._updateRenderGizmo_r(children, name);
    }

}

SceneEditor.EditorModeController.prototype._updateGizmoColor = function(materials, state, type)
{
    if(!materials) return;
    var opacity = this.opacityMap[type][state];
    for(var i = 0; i < materials.length; i++)
    {
        var material = materials[i];

        var colorBase = material.getPureColorBase();
        if(!colorBase) continue;
        var color = [];
        color[3] = opacity;
        if(state === 'hover' && type !== 'sphere')
        {
            // color[0] = 0.5 * (1 + colorBase[0]);
            // color[1] = 0.5 * (1 + colorBase[1]);
            // color[2] = 0.5 * (1 + colorBase[2]);
            color[0] = this.hoverColor[0];
            color[1] = this.hoverColor[1];
            color[2] = this.hoverColor[2];
        }
        else
        {
            color[0] = colorBase[0];
            color[1] = colorBase[1];
            color[2] = colorBase[2];
        }
        material.setPureColor(color);
    }
}


SceneEditor.EditorModeController.prototype.selectedObjectChangedCallback = function(dEntity)
{
    // if(!dEntity) 
    // {
    //     mgsEditor.emit(mgsEditor.EVENTS.onAssetDeleteEnd); 
    // }
    var entity =  dEntity ? dEntity.engineEntity : null;
    mgsEditor.emit(mgsEditor.EVENTS.viewport.entityClicked, entity); 
}


SceneEditor.EditorModeController.prototype.initGizmos = function(dEntity)
{
    if(!dEntity) return;
    dEntity.gizmo = GASEngine.WebglCommon.Instance.getTransformControls();
    dEntity.picker = GASEngine.WebglCommon.Instance.getTransformPickers();

    this.tmpPosition.copy(dEntity.getWorldTranslation());
    this.tmpRotation.set(0, 0, 0, 'XYZ');
    this.tmpScale.set(1, 1, 1);

    if(dEntity.gizmo) {
        dEntity.gizmo.setLocalTranslation(this.tmpPosition);
        dEntity.gizmo.setLocalRotation(this.tmpRotation);
        dEntity.gizmo.setLocalScale(this.tmpScale);

        let index = SceneEditor.Editor.Instance.sceneInstance.root.findChild(dEntity.gizmo);
        if(index === -1)
        {
            SceneEditor.Editor.Instance.sceneInstance.appendEntityOnRoot(dEntity.gizmo);
        }
    }

    if(dEntity.picker) {
        dEntity.picker.setLocalTranslation(this.tmpPosition);
        dEntity.picker.setLocalRotation(this.tmpRotation);
        dEntity.picker.setLocalScale(this.tmpScale);
    }

}

SceneEditor.EditorModeController.prototype.removeGizmos = function(dEntity)
{
    if(!dEntity || !dEntity.gizmo) return;
    var parent = dEntity.gizmo.parent;
    if(!parent) return;
    parent.removeChild(dEntity.gizmo);

    // reset
    // dEntity.gizmo.translation.set(0, 0, 0);
    // dEntity.gizmo.rotation.set(0, 0, 0, 'XYZ');
    // dEntity.gizmo.scale.set(1, 1, 1);
    
    // dEntity.picker.translation.set(0, 0, 0);
    // dEntity.picker.rotation.set(0, 0, 0, 'XYZ');
    // dEntity.picker.scale.set(1, 1, 1);

    this.tmpPosition.set(0, 0, 0);
    this.tmpRotation.set(0, 0, 0, 'XYZ');
    this.tmpScale.set(1, 1, 1);

    dEntity.gizmo.setLocalTranslation(this.tmpPosition);
    dEntity.gizmo.setLocalRotation(this.tmpRotation);
    dEntity.gizmo.setLocalScale(this.tmpScale);
    
    dEntity.picker.setLocalTranslation(this.tmpPosition);
    dEntity.picker.setLocalRotation(this.tmpRotation);
    dEntity.picker.setLocalScale(this.tmpScale);


    dEntity.gizmo = null;
    dEntity.picker = null;
}