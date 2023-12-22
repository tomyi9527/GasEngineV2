//Author: saralu
//Date: 2019-5-28
//ScaleMode

SceneEditor.ScaleMode = function(manipulator)
{
    SceneEditor.AxisOpMode.call(this, manipulator);
    this.mode = 'scale';
    this.forceLocal = true;
    this.scaleVec = new GASEngine.Vector3(1, 1, 1);
}

SceneEditor.ScaleMode.prototype = Object.create(SceneEditor.AxisOpMode.prototype);
SceneEditor.ScaleMode.prototype.constructor = SceneEditor.ScaleMode;

SceneEditor.ScaleMode.prototype.mousedown = function()
{
    SceneEditor.AxisOpMode.prototype.mousedown.call(this);
}

SceneEditor.ScaleMode.prototype.mousemove = function()
{
    SceneEditor.AxisOpMode.prototype.mousemove.call(this);
}

SceneEditor.ScaleMode.prototype.mouseup = function()
{
    SceneEditor.AxisOpMode.prototype.mouseup.call(this);
}

SceneEditor.ScaleMode.prototype.blur = function()
{
    SceneEditor.AxisOpMode.prototype.blur.call(this);
}

SceneEditor.ScaleMode.prototype.update = function()
{
    SceneEditor.AxisOpMode.prototype.update.call(this);
}

SceneEditor.ScaleMode.prototype.onPreOp = function()
{
    this.scaleBase = this.dEntity.getScale(); // this.realEntityForMove ? this.realEntityForMove.scale.clone() : new GASEngine.Vector3(1 , 1, 1);
};

SceneEditor.ScaleMode.prototype.onVectorChange = function(vector)
{
    vector.applyMatrix4(this.rotObjectMatrixInverse);
    // this.realEntityForMove.scale.addVectors(this.scaleBase, vector);
    // this.dEntity.scale.addVectors(this.scaleBase, vector);
    // this.dEntity.update();

    // this.scaleVec.copy(this.scaleBase).multiply(vector).add(this.scaleBase);
    // this.dEntity.setScale(this.scaleVec);

    this.scaleVec.copy(this.scaleBase).multiply(vector).add(this.scaleBase);
    // this.dEntity.setScale(this.scaleVec);

    this.needLog = true;
    this.logInfo = {name: 'scale', value: this.scaleVec, old: this.scaleBase};

    mgsEditor.emit(mgsEditor.EVENTS.viewport.transformChanged, this.dEntity, this.logInfo.name, this.logInfo.value, this.logInfo.old, false); 
};
