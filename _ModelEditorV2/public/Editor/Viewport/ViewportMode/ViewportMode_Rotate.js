(function()
{
    let AxisX = new GASEngine.Vector3(1, 0, 0);
    let AxisY = new GASEngine.Vector3(0, 1, 0);
    let AxisZ = new GASEngine.Vector3(0, 0, 1);
    let axis = new GASEngine.Vector3();
    let movDir = new GASEngine.Vector3();
    let tempAxis = new GASEngine.Vector3();

    let entityWorldQuaternion = new GASEngine.Quaternion();
    let entityWorldQuaternionInv = new GASEngine.Quaternion();
    let entityWorldPosition = new GASEngine.Vector3();

    let cameraPosition = new GASEngine.Vector3();
    let cameraQuaternion = new GASEngine.Quaternion();
    let cameraScale = new GASEngine.Vector3(1, 1, 1);

    let rotateTarget = new GASEngine.Quaternion();
    let tempRotate = new GASEngine.Quaternion();
    let rotateBase = new GASEngine.Quaternion();
    let baseEuler = new GASEngine.Euler();
    let rotateEuler = new GASEngine.Euler();

    let rotateDir = new GASEngine.Vector3();
    let lookAt = new GASEngine.Vector3();
    let crossVec = new GASEngine.Vector3();

    let isAxisRot = false;

    let mapPlane = null;
    let beginPointInMapPlane = null;

    let speed = 15;

    let ViewportMode_Rotate = function(controller)
    {
        mgs.ViewportMode_Select.call(this, controller);

        this._mode = 'rotate';
    };
    mgs.classInherit(ViewportMode_Rotate, mgs.ViewportMode_Select);

    // -------------------------------------------- utils --------------------------------------------
    ViewportMode_Rotate.prototype._updateRotateOp = function(selectEntity, isAddHistory)
    {
        let endRay = this.getMousePositionRay();
        let endPointInMapPlane = GASEngine.Utilities.getRayIntersectPlanePoint(endRay.origin, endRay.direction, mapPlane); 

        movDir.copy(endPointInMapPlane).sub(beginPointInMapPlane);
        
        if (isAxisRot)
        {
            // 轴转动处理
            var radian = rotateDir.dot(movDir);
            tempAxis.copy(axis);
            tempAxis.applyQuaternion(entityWorldQuaternionInv);
            this.onRotateChange(selectEntity, tempAxis, radian * speed, isAddHistory);
        }
        else
        {
            // 自由转动处理
            var radian = movDir.length();
            crossVec.copy(movDir).cross(lookAt).normalize();
            tempAxis.copy(crossVec);
            tempAxis.applyQuaternion(entityWorldQuaternionInv);
            this.onRotateChange(selectEntity, tempAxis, radian * speed, isAddHistory);
        }
    };

    // -------------------------------------------- axis op events --------------------------------------------
    ViewportMode_Rotate.prototype.onPreOp = function(selectEntity)
    {
        baseEuler.copy(selectEntity.getLocalRotation());
        rotateBase.setFromEuler(baseEuler);
    };

    ViewportMode_Rotate.prototype.onRotateChange = function(selectEntity, axis, radian, isAddHistory)
    {
        tempRotate.copy(rotateBase);
        rotateTarget.setFromAxisAngle(axis, radian);
        tempRotate.multiply(rotateTarget);

        rotateEuler.setFromQuaternion(tempRotate, 'XYZ');

        // send cmd
        let viewport = this._controller.getViewport();
        viewport.onEntityRotate(selectEntity.uniqueID, [rotateEuler.x, rotateEuler.y, rotateEuler.z], [baseEuler.x, baseEuler.y, baseEuler.z], isAddHistory);
    };

    // -------------------------------------------- select events --------------------------------------------
    ViewportMode_Rotate.prototype.onSelectEvent_BeginPick = function(pickerGizmoChildInfo, selectEntity)
    {
        this.onPreOp(selectEntity);

        entityWorldPosition.copy(selectEntity.getWorldTranslation());
        entityWorldQuaternion.copy(selectEntity.getWorldQuaternion());
        entityWorldQuaternionInv.copy(entityWorldQuaternion).inverse();
        
        // rotObjectMatrix.makeRotationFromQuaternion(entityWorldQuaternion);   
        // rotObjectMatrixInverse.getInverse(rotObjectMatrix);

        let camera = this._controller.getViewportCamera().getCurrentCanmera();
        nearToFar = camera.far - camera.near;

        let matrix = camera.getWorldMatrix();
        matrix.decompose(cameraPosition, cameraQuaternion, cameraScale);

        var pickName = pickerGizmoChildInfo.name;
        isAxisRot = false;

        if(pickName === 'X')
        {  
            axis.copy(AxisX);
            isAxisRot = true;
        }
        else if(pickName === 'Y')
        {
            axis.copy(AxisY);
            isAxisRot = true;
        }
        else if(pickName === 'Z')
        {
            axis.copy(AxisZ);
            isAxisRot = true;
        }
        else if(pickName === 'E')
        {
            axis.copy(cameraPosition).sub(entityWorldPosition).normalize();
            isAxisRot = true;
        }
        else if(pickName === 'XYZE')
        {

        }

        let isLocal = this._controller.isLocalMode();
        if (isLocal)
        {
            if (pickName !== 'E' && pickName !== 'XYZE')
            {
                axis.applyQuaternion(entityWorldQuaternion);
            }
        }

        let nearPoint = new GASEngine.Vector3(0, 0, -camera.near);
        nearPoint.applyMatrix4(camera.getWorldMatrix());
        mapPlane = GASEngine.Utilities.getFaceToPlane(nearPoint, cameraPosition);

        let beginRay = this.getMousePositionRay();
        beginPointInMapPlane = GASEngine.Utilities.getRayIntersectPlanePoint(beginRay.origin, beginRay.direction, mapPlane); 

        if (isAxisRot)
        {
            // 轴转动处理
            let viewportGizmo = this._controller.getViewportGizmo();
            let radius = viewportGizmo.getRotateRadius();

            let point = GASEngine.Utilities.getRayIntersectSpherePoint(beginRay.origin, beginRay.direction, entityWorldPosition, radius);
            let pointDir = point.sub(entityWorldPosition);

            rotateDir.copy(axis).cross(pointDir).normalize();
        }
        else
        {
            // 自由转动处理
            // lookAt.copy(mapPlane.normal).multiplyScalar(-1); 

            lookAt.copy(entityWorldPosition).sub(cameraPosition).normal(); 
        }
    };

    ViewportMode_Rotate.prototype.onSelectEvent_EndPick = function(pickerGizmoChildInfo, selectEntity)
    {
        this._updateRotateOp(selectEntity, true);
    };

    ViewportMode_Rotate.prototype.onSelectEvent_UpdatePick = function(pickerGizmoChildInfo, selectEntity)
    {
        this._updateRotateOp(selectEntity);
    };
}());