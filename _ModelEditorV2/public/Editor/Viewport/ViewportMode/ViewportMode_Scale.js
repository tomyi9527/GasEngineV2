(function()
{
    let scaleVec = new GASEngine.Vector3(1, 1, 1);
    let scaleBase = new GASEngine.Vector3(1, 1, 1);
    let deltaVectorTemp = new GASEngine.Vector3(1, 1, 1);

    let ViewportMode_Scale = function(controller)
    {
        mgs.ViewportMode_AxisOp.call(this, controller);

        this._mode = 'scale';
    };
    mgs.classInherit(ViewportMode_Scale, mgs.ViewportMode_AxisOp);

    // -------------------------------------------- axis op events --------------------------------------------
    ViewportMode_Scale.prototype.onPreOp = function(selectEntity)
    {
        scaleBase.copy(selectEntity.getLocalScale()); 
    };

    ViewportMode_Scale.prototype.onVectorChange = function(selectEntity, deltaVector, isAddHistory)
    {
        deltaVectorTemp.copy(deltaVector);
        deltaVectorTemp.applyMatrix4(this._getObjectRotInverse());
        scaleVec.copy(scaleBase).multiply(deltaVectorTemp).add(scaleBase);

        // send cmd
        let viewport = this._controller.getViewport();
        viewport.onEntityScale(selectEntity.uniqueID, [scaleVec.x, scaleVec.y, scaleVec.z], [scaleBase.x, scaleBase.y, scaleBase.z], isAddHistory);
    };
}());