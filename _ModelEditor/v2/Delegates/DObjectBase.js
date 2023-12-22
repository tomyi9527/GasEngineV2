//Author: saralu
//Date: 2019/5/31
//Delegate of entity

SceneEditor.DObjectBase = function()
{
    this.properties = [];
}

SceneEditor.DObjectBase.prototype.constructor = SceneEditor.DObjectBase;


SceneEditor.DObjectBase.prototype.update = function()
{

}

SceneEditor.DObjectBase.prototype.getProperties = function()
{
    return this.properties;
}

SceneEditor.DObjectBase.prototype.getPropertyById = function(id)
{
    var prop;
    for(var i=0; i< this.properties.length; i++)
    {
        prop = this.properties[i];
        if(prop.id === id)
        {
            return prop;
        }
    }
    return undefined;
}

//to normalize the vector3 and euler to xyz
SceneEditor.DObjectBase.prototype.normalizeVector3AndEuler = function(vec3)
{
    var vec = new GASEngine.Vector3();
    vec.set(parseFloat(vec3.x.toFixed(2)), parseFloat(vec3.y.toFixed(2)), parseFloat(vec3.z.toFixed(2)));
    return vec3;
}