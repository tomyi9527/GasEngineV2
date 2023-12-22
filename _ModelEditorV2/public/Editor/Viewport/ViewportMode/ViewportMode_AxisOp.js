(function()
{
    let AxisX = new GASEngine.Vector3(1, 0, 0);
    let AxisY = new GASEngine.Vector3(0, 1, 0);
    let AxisZ = new GASEngine.Vector3(0, 0, 1);
    let axis = new GASEngine.Vector3();
    let movDir = new GASEngine.Vector3();

    let entityWorldQuaternion = new GASEngine.Quaternion();
    let entityWorldPosition = new GASEngine.Vector3();

    let cameraPosition = new GASEngine.Vector3();
    let cameraQuaternion = new GASEngine.Quaternion();
    let cameraScale = new GASEngine.Vector3(1, 1, 1);

    let rotObjectMatrix = new GASEngine.Matrix4();
    let rotObjectMatrixInverse = new GASEngine.Matrix4();

    let rayFacePlane = null;
    let beginPointInRayFacePlane;

    let isPlaneDrag = false;
    let nearToFar = 0;
    let farPlane = null;

    let ViewportMode_AxisOp = function(controller)
    {
        mgs.ViewportMode_Select.call(this, controller);

        this._mode = 'axisOp';
    };
    mgs.classInherit(ViewportMode_AxisOp, mgs.ViewportMode_Select);

    // -------------------------------------------- utils --------------------------------------------
    ViewportMode_AxisOp.prototype._updateAxisOp = function(selectEntity, isAddHistory)
    {
        let ray = this.getMousePositionRay();
        let endPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(ray.origin, ray.direction, rayFacePlane);
        if (!endPointInRayFacePlane)
        {
            endPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(ray.origin, ray.direction, farPlane);
            if (rayFacePlane)
            {
                endPointInRayFacePlane = GASEngine.Utilities.getPointMapInPlane(endPointInRayFacePlane, rayFacePlane);
            }
        }

        if (isPlaneDrag)
        {
            movDir.copy(endPointInRayFacePlane).sub(beginPointInRayFacePlane);
        }
        else
        {
            movDir.copy(endPointInRayFacePlane).sub(beginPointInRayFacePlane);
            let dot = movDir.dot(axis);
            movDir.copy(axis).multiplyScalar(dot);
        }

        let len = movDir.length();
        if (nearToFar < len)
        {
            movDir.normalize().multiplyScalar(nearToFar);
        }

        this.onVectorChange(selectEntity, movDir, isAddHistory);
    };

    ViewportMode_AxisOp.prototype._getObjectRotInverse = function()
    {
        return rotObjectMatrixInverse;
    };

    // -------------------------------------------- axis op events --------------------------------------------
    ViewportMode_AxisOp.prototype.onPreOp = function(selectEntity)
    {
    };

    ViewportMode_AxisOp.prototype.onVectorChange = function(selectEntity, deltaVector, isAddHistory)
    {
    };

    // -------------------------------------------- select events --------------------------------------------
    ViewportMode_AxisOp.prototype.onSelectEvent_BeginPick = function(pickerGizmoChildInfo, selectEntity)
    {
        this.onPreOp(selectEntity);

        var pickName = pickerGizmoChildInfo.name;
        isPlaneDrag = false;
        
        if(pickName === 'X')
        {  
            axis.copy(AxisX);
        }
        else if(pickName === 'Y')
        {
            axis.copy(AxisY);
        }
        else if(pickName === 'Z')
        {
            axis.copy(AxisZ);
        }
        else if(pickName === 'XY')
        {
            axis.copy(AxisZ);
            isPlaneDrag = true;
        }
        else if(pickName === 'YZ')
        {
            axis.copy(AxisX);
            isPlaneDrag = true;
        }
        else if(pickName === 'ZX')
        {
            axis.copy(AxisY);
            isPlaneDrag = true;
        }

        entityWorldQuaternion.copy(selectEntity.getWorldQuaternion());
        entityWorldPosition.copy(selectEntity.getWorldTranslation());
        rotObjectMatrix.makeRotationFromQuaternion(entityWorldQuaternion);   
        rotObjectMatrixInverse.getInverse(rotObjectMatrix);

        let camera = this._controller.getViewportCamera().getCurrentCanmera();
        nearToFar = camera.far - camera.near;

        let matrix = camera.getWorldMatrix();
        matrix.decompose(cameraPosition, cameraQuaternion, cameraScale);

        let farPoint = new GASEngine.Vector3(0, 0, -camera.far);
        farPoint.applyMatrix4(camera.getWorldMatrix());

        farPlane = GASEngine.Utilities.getFaceToPlane(farPoint, cameraPosition);

        let isLocal = this._controller.isLocalMode();
        if (isLocal)
        {
            axis.applyMatrix4(rotObjectMatrix);
        }

        if (isPlaneDrag)
        {
            rayFacePlane = GASEngine.Utilities.getRayPlane(entityWorldPosition, axis);
        }
        else
        {
            rayFacePlane = GASEngine.Utilities.getRayFacePointPlane(entityWorldPosition, axis, cameraPosition);
        }

        let beginRay = this.getMousePositionRay();
        beginPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(beginRay.origin, beginRay.direction, rayFacePlane);
    };

    ViewportMode_AxisOp.prototype.onSelectEvent_EndPick = function(pickerGizmoChildInfo, selectEntity)
    {
        if (!beginPointInRayFacePlane)
        {
            return;
        }

        this._updateAxisOp(selectEntity, true);
    };

    ViewportMode_AxisOp.prototype.onSelectEvent_UpdatePick = function(pickerGizmoChildInfo, selectEntity)
    {
        if (!beginPointInRayFacePlane)
        {
            return;
        }

        this._updateAxisOp(selectEntity);
    };
}());