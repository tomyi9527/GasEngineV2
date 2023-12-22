//Author: saralu
//Date: 2019-5-28
//TranlateMode

SceneEditor.AxisOpMode = function(manipulator)
{
    SceneEditor.SelectMode.call(this, manipulator);
    this.mode = 'translate';

    this.beginMousePos = new GASEngine.Vector2(0, 0); 

    this.AxisX = new GASEngine.Vector3(1, 0, 0);
    this.AxisY = new GASEngine.Vector3(0, 1, 0);
    this.AxisZ = new GASEngine.Vector3(0, 0, 1);

    this.axis = new GASEngine.Vector3();

    this.movDir = new GASEngine.Vector3();

    this.speed = 1;
}

SceneEditor.AxisOpMode.prototype = Object.create(SceneEditor.SelectMode.prototype);
SceneEditor.AxisOpMode.prototype.constructor = SceneEditor.AxisOpMode;

SceneEditor.AxisOpMode.prototype.mousedown = function()
{
    SceneEditor.SelectMode.prototype.mousedown.call(this);
    
    if (event.button !== 0) return;

    // if(this.dEntity)
    if(this.pickInfo && this.dEntity)
    {    
        // var worldPosition = SceneEditor.Editor.Instance.eventInput.worldPosition;
        var cameraPosition = SceneEditor.Editor.Instance.eventInput.cameraPosition;

        var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
        var cameraComponent = cameras[0];
        this.nearToFar = cameraComponent.far - cameraComponent.near;

        var farPoint = new GASEngine.Vector3(0, 0, -cameraComponent.far);
        farPoint.applyMatrix4(cameraComponent.getWorldMatrix());
        this.farPlane = GASEngine.Utilities.getFaceToPlane(farPoint, cameraPosition);

        this.entity = this.dEntity.realEntity;
        var entityQuaternion = new GASEngine.Quaternion();
        entityQuaternion.copy(this.entity.getWorldQuaternion());
        var worldPosition = new GASEngine.Vector3();
        worldPosition.copy(this.entity.getWorldTranslation());

        this.rotObjectMatrix = new GASEngine.Matrix4();
        this.rotObjectMatrix.makeRotationFromQuaternion(entityQuaternion);   

        
        this.rotObjectMatrixInverse = new GASEngine.Matrix4();
        this.rotObjectMatrixInverse.getInverse(this.rotObjectMatrix);

        var isLocal = (SceneEditor.Editor.Instance.eventInput.currentSpace === 'local') || this.forceLocal;

        this.onPreOp();

        this.beginMousePos.copy(SceneEditor.Editor.Instance.mousePos);

        var name = this.gizmoPicker ? this.gizmoPicker.name ? this.gizmoPicker.name :  'X' : null;
        name = name ? name : 'X';

        this.isPlaneDrag = false;
 
        if(name === 'X')
        {  
            this.axis.copy(this.AxisX);
        }
        else if(name === 'Y')
        {
            this.axis.copy(this.AxisY);
        }
        else if(name === 'Z')
        {
            this.axis.copy(this.AxisZ);
        }
        else if(name === 'XY')
        {
            this.axis.copy(this.AxisZ);
            this.isPlaneDrag = true;
        }
        else if(name === 'YZ')
        {
            this.axis.copy(this.AxisX);
            this.isPlaneDrag = true;
        }
        else if(name === 'XZ')
        {
            this.axis.copy(this.AxisY);
            this.isPlaneDrag = true;
        }

        if (isLocal)
        {
            this.axis.applyMatrix4(this.rotObjectMatrix);
        }

        if (this.isPlaneDrag)
        {
            this.rayFacePlane = GASEngine.Utilities.getRayPlane(worldPosition, this.axis);
        }
        else
        {
            this.rayFacePlane = GASEngine.Utilities.getRayFacePointPlane(worldPosition, this.axis, cameraPosition);
        }

        var beginRay = SceneEditor.Editor.Instance.eventInput.getMousePositionRay(this.beginMousePos);
        this.beginPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(beginRay.origin, beginRay.direction, this.rayFacePlane); 

        if (!this.beginPointInRayFacePlane)
        {
            return;
        }
        
        this.drag = true; 
    }
}

SceneEditor.AxisOpMode.prototype.mouseup = function()
{
    SceneEditor.SelectMode.prototype.mouseup.call(this);

    if (event.button !== 0) return;

    this.beginMousePos.set(0, 0);
    this.drag = false;
    this.gizmoPicker = null;
}

SceneEditor.AxisOpMode.prototype.blur = function()
{
    SceneEditor.SelectMode.prototype.blur.call(this);

    this.beginMousePos.set(0, 0);
    this.drag = false;
    this.gizmoPicker = null;
}

SceneEditor.AxisOpMode.prototype.update = function()
{
    SceneEditor.SelectMode.prototype.update.call(this);

    if(!this.drag) return;

    if(this.gizmoPicker)
    {
        var ray = SceneEditor.Editor.Instance.eventInput.getMousePositionRay(SceneEditor.Editor.Instance.mousePos);
        var endPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(ray.origin, ray.direction, this.rayFacePlane);
        if (!endPointInRayFacePlane)
        {
            endPointInRayFacePlane = GASEngine.Utilities.getRayIntersectPlanePoint(ray.origin, ray.direction, this.farPlane);
            if (this.rayFacePlane)
            {
                endPointInRayFacePlane = GASEngine.Utilities.getPointMapInPlane(endPointInRayFacePlane, this.rayFacePlane);
            }
        }

        if (this.isPlaneDrag)
        {
            this.movDir.copy(endPointInRayFacePlane).sub(this.beginPointInRayFacePlane);
        }
        else
        {
            this.movDir.copy(endPointInRayFacePlane).sub(this.beginPointInRayFacePlane);
            var dot = this.movDir.dot(this.axis);
            this.movDir.copy(this.axis).multiplyScalar(dot);
        }

        var len = this.movDir.length();
        if (this.nearToFar < len)
        {
            this.movDir.normalize().multiplyScalar(this.nearToFar);
        }

        this.onVectorChange(this.movDir);
    }
}

SceneEditor.AxisOpMode.prototype.onPreOp = function()
{

};

SceneEditor.AxisOpMode.prototype.onVectorChange = function(vector)
{

};