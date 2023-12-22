//Author: saralu
//Date: 2019-5-28
//RotateMode

SceneEditor.RotateMode = function(manipulator)
{
    SceneEditor.SelectMode.call(this, manipulator);
    this.mode = 'rotate';

    this.beginMousePos = new GASEngine.Vector2(0, 0); 

    this.AxisX = new GASEngine.Vector3(1, 0, 0);
    this.AxisY = new GASEngine.Vector3(0, 1, 0);
    this.AxisZ = new GASEngine.Vector3(0, 0, 1);

    this.axis = new GASEngine.Vector3();
    this.tempAxis = new GASEngine.Vector3();

    this.movDir = new GASEngine.Vector3();
    this.tangent = new GASEngine.Vector3();
    this.tangentVec = new GASEngine.Vector3();

    this.beginVec = new GASEngine.Vector3();
    this.endVec = new GASEngine.Vector3();
    this.crossVec = new GASEngine.Vector3();

    this.targetVec = new GASEngine.Vector3();

    this.speed = 15;

    this.rotateEuler = new GASEngine.Euler();
    this.baseEuler = new GASEngine.Euler();

    this.rotateVec = new GASEngine.Vector3();
    this.rotateBaseVec = new GASEngine.Vector3();
}

SceneEditor.RotateMode.prototype = Object.create(SceneEditor.SelectMode.prototype);
SceneEditor.RotateMode.prototype.constructor = SceneEditor.RotateMode;

SceneEditor.RotateMode.prototype.mousedown = function()
{
    SceneEditor.SelectMode.prototype.mousedown.call(this);
    
    if (event.button !== 0) return;

    if(this.pickInfo && this.dEntity)
    {   
        // this.entity = this.realEntityForMove;
        this.entity = this.dEntity.realEntity;

        this.entityQuaternion = new GASEngine.Quaternion();
        this.entityQuaternion.copy(this.entity.getWorldQuaternion());
        this.entityQuaternionInv = new GASEngine.Quaternion();
        this.entityQuaternionInv.copy(this.entityQuaternion).inverse();
        this.worldPosition = new GASEngine.Vector3();
        this.worldPosition.copy(this.entity.getWorldTranslation());

        this.isLocal = (SceneEditor.Editor.Instance.eventInput.currentSpace === 'local') || this.forceLocal;

        this.onPreOp();

        this.drag = true; 

        this.beginMousePos.copy(SceneEditor.Editor.Instance.mousePos);

        // this.worldPosition = SceneEditor.Editor.Instance.eventInput.worldPosition;
        var cameraPosition = SceneEditor.Editor.Instance.eventInput.cameraPosition;

        var name = this.gizmoPicker ? this.gizmoPicker.name ? this.gizmoPicker.name :  'X' : null;
        name = name ? name : 'X';

        this.isPlaneDrag = false;
        this.isAxisRot = false;

        if(name === 'X')
        {  
            this.axis.copy(this.AxisX);
            this.isAxisRot = true;
        }
        else if(name === 'Y')
        {
            this.axis.copy(this.AxisY);
            this.isAxisRot = true;
        }
        else if(name === 'Z')
        {
            this.axis.copy(this.AxisZ);
            this.isAxisRot = true;
        }
        else if(name === 'E')
        {
            this.axis.copy(cameraPosition).sub(this.worldPosition).normalize();
            this.isAxisRot = true;
        }
        else if(name === 'XYZE')
        {
            // this.axis.copy(cameraPosition).sub(this.worldPosition).normalize();
            // this.isAxisRot = ture;
        }

        if (this.isLocal)
        {
            if (name !== 'E' && name !== 'XYZE')
            {
                this.axis.applyQuaternion(this.entityQuaternion);
            }
        }


        var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
        var cameraComponent = cameras[0];

        var nearPoint = new GASEngine.Vector3(0, 0, -cameraComponent.near);
        nearPoint.applyMatrix4(cameraComponent.getWorldMatrix());
        this.mapPlane = GASEngine.Utilities.getFaceToPlane(nearPoint, cameraPosition);
        
        // this.mapPlane = GASEngine.Utilities.getFaceToPlane(this.worldPosition, cameraPosition);
        var beginRay = SceneEditor.Editor.Instance.eventInput.getMousePositionRay(this.beginMousePos);
        this.beginPointInMapPlane = GASEngine.Utilities.getRayIntersectPlanePoint(beginRay.origin, beginRay.direction, this.mapPlane); 

        if (this.isAxisRot)
        {
            // 轴转动处理
            var radius = SceneEditor.Editor.Instance.eventInput.getRotateRadius();
            var point = GASEngine.Utilities.getRayIntersectSpherePoint(beginRay.origin, beginRay.direction, this.worldPosition, radius);
            this.pointDir = point.sub(this.worldPosition);

            this.rotateDir = new GASEngine.Vector3();
            this.rotateDir.copy(this.axis).cross(this.pointDir).normalize();
        }
        else
        {
            // 自由转动处理
            this.lookAt = new GASEngine.Vector3();
            this.lookAt.copy(this.mapPlane.normal).multiplyScalar(-1); 
        }
    }
}

SceneEditor.RotateMode.prototype.mousemove = function()
{
    SceneEditor.SelectMode.prototype.mousemove.call(this);
}

SceneEditor.RotateMode.prototype.mouseup = function()
{
    SceneEditor.SelectMode.prototype.mouseup.call(this);

    if (event.button !== 0) return;

    this.beginMousePos.set(0, 0);
    this.drag = false;
    this.gizmoPicker = null;
}

SceneEditor.RotateMode.prototype.blur = function()
{
    this.beginMousePos.set(0, 0);
    this.drag = false;
    this.gizmoPicker = null;
}

SceneEditor.RotateMode.prototype.update = function()
{
    SceneEditor.SelectMode.prototype.update.call(this);

    if(!this.drag) return;

    if(this.gizmoPicker)
    {
        var endRay = SceneEditor.Editor.Instance.eventInput.getMousePositionRay(SceneEditor.Editor.Instance.mousePos);
        var endPointInMapPlane = GASEngine.Utilities.getRayIntersectPlanePoint(endRay.origin, endRay.direction, this.mapPlane); 

        this.movDir.copy(endPointInMapPlane).sub(this.beginPointInMapPlane);
        
        if (this.isAxisRot)
        {
            // 轴转动处理
            var radian = this.rotateDir.dot(this.movDir);
            this.tempAxis.copy(this.axis);
            this.tempAxis.applyQuaternion(this.entityQuaternionInv);
            this.onRotateChange(this.tempAxis, radian * this.speed);
        }
        else
        {
            // 自由转动处理
            var radian = this.movDir.length();
            this.crossVec.copy(this.movDir).cross(this.lookAt).normalize();
            var axis = this.crossVec;
            axis.applyQuaternion(this.entityQuaternionInv);
            this.onRotateChange(axis, radian * this.speed);
        }
    }
}

SceneEditor.RotateMode.prototype.onPreOp = function()
{  
    this.rotateTarget = new GASEngine.Quaternion();
    this.tempRotate = new GASEngine.Quaternion();

    this.rotateBase = new GASEngine.Quaternion();
    this.baseEuler = this.dEntity.getRotation();
    this.rotateBase.setFromEuler(this.baseEuler);

    // this.matrix = new GASEngine.Matrix4();
    // this.matrix.getInverse(this.entity.matrixWorld);
    
};

SceneEditor.RotateMode.prototype.onRotateChange = function(axis, radian)
{
    this.tempRotate.copy(this.rotateBase);
    this.rotateTarget.setFromAxisAngle(axis, radian);
    this.tempRotate.multiply(this.rotateTarget);
    
    this.rotateEuler.setFromQuaternion(this.tempRotate, 'XYZ');
    // this.dEntity.setRotation(this.rotateEuler);

    this.rotateVec.set(this.rotateEuler.x, this.rotateEuler.y, this.rotateEuler.z);
    this.rotateBaseVec.set(this.baseEuler.x, this.baseEuler.y, this.baseEuler.z);

    this.needLog = true;
    this.logInfo = {name: 'rotation', value: this.rotateVec, old: this.rotateBaseVec};
    mgsEditor.emit(mgsEditor.EVENTS.viewport.transformChanged, this.dEntity, this.logInfo.name, this.logInfo.value, this.logInfo.old, false); 
};