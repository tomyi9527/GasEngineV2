(function()
{
    let translateBase = new GASEngine.Vector3();
    let localTranslateVec = new GASEngine.Vector3();
    let localTranslateBase = new GASEngine.Vector3();
    let translateVec = new GASEngine.Vector3();

    let ViewportMode_Translate = function(controller)
    {
        mgs.ViewportMode_AxisOp.call(this, controller);

        this._mode = 'translate';
    };
    mgs.classInherit(ViewportMode_Translate, mgs.ViewportMode_AxisOp);

    // -------------------------------------------- axis op events --------------------------------------------
    ViewportMode_Translate.prototype.onPreOp = function(selectEntity)
    {
        translateBase.copy(selectEntity.getWorldTranslation());
        localTranslateBase.copy(selectEntity.getLocalTranslation());
    };

    ViewportMode_Translate.prototype.onVectorChange = function(selectEntity, deltaVector, isAddHistory)
    {
        translateVec.addVectors(translateBase, deltaVector);
        selectEntity.worldPositionToLocal(translateVec, localTranslateVec);
        
        // send cmd
        let viewport = this._controller.getViewport(); 
        viewport.onEntityTranslate(
            selectEntity.uniqueID, 
            [localTranslateVec.x, localTranslateVec.y, localTranslateVec.z],
            [localTranslateBase.x, localTranslateBase.y, localTranslateBase.z],
            isAddHistory
        );
    };
}());