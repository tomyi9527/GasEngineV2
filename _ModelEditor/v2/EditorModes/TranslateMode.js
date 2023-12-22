//Author: saralu
//Date: 2019-5-28
//TranlateMode

SceneEditor.TranslateMode = function(manipulator)
{
    SceneEditor.AxisOpMode.call(this, manipulator);
    this.translateVec = new GASEngine.Vector3(0,0,0);
}

SceneEditor.TranslateMode.prototype = Object.create(SceneEditor.AxisOpMode.prototype);
SceneEditor.TranslateMode.prototype.constructor = SceneEditor.TranslateMode;

SceneEditor.TranslateMode.prototype.mousedown = function()
{
    SceneEditor.AxisOpMode.prototype.mousedown.call(this);
}

SceneEditor.TranslateMode.prototype.mousemove = function()
{
    SceneEditor.AxisOpMode.prototype.mousemove.call(this);
}

SceneEditor.TranslateMode.prototype.mouseup = function()
{
    SceneEditor.AxisOpMode.prototype.mouseup.call(this);
}

SceneEditor.TranslateMode.prototype.update = function()
{
    SceneEditor.AxisOpMode.prototype.update.call(this);
}

SceneEditor.TranslateMode.prototype.blur = function()
{
    SceneEditor.AxisOpMode.prototype.blur.call(this);
}

SceneEditor.TranslateMode.prototype.onPreOp = function()
{
    this.translateBase = new GASEngine.Vector3();
    this.translateBase.copy(this.dEntity.getWorldTranslation());

    this.localTranslateVec = new GASEngine.Vector3();
    this.localTranslateBase = new GASEngine.Vector3();
    this.localTranslateBase.copy(this.dEntity.getTranslation());
};

SceneEditor.TranslateMode.prototype.onVectorChange = function(vector)
{
    this.translateVec.addVectors(this.translateBase, vector);

    this.dEntity.worldPositionToLocal(this.translateVec, this.localTranslateVec);
    
    // this.dEntity.setTranslation(this.localTranslateVec);
    this.needLog = true;
    this.logInfo = {name: 'translation', value: this.localTranslateVec, old: this.localTranslateBase};

    mgsEditor.emit(mgsEditor.EVENTS.viewport.transformChanged, this.dEntity, this.logInfo.name, this.logInfo.value, this.logInfo.old, false); 
};
