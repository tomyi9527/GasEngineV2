(function()
{
    // temp patch
    GASEngine.Entity.prototype.setWorldRotation = function(worldRotation)
    {
        this.worldRotationToLocal(worldRotation, this._rotation_);
        this.setLocalRotation(this._rotation_);
    };
}());