//点光源，聚光灯，直射光
//position, color, intensity,
//直射光：方向
//点光源：前向距离衰减
//聚光灯：方向，张角，前向距离衰减，张角衰减

//culling
//将灯光放到mesh
//renderItem：lightlist rendering

//shader计算

//DirectionalLight
GASEngine.DirectionalLightComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.type = 'directional'; 
    //this.position = new Float32Array([0.0, 0.0, 0.0]);
    this.color = new Float32Array([1.0, 0.0, 0.0, 1.0]);
    //this.intensity = 0.1;
    this.ambientIntensity = 0.5;
    this.diffuseIntensity = 0.2;
    this.specularPower = 16.0;
    this.specularIntensity = 0.0;
    this.shininess = 32.0;
    this.direction = new Float32Array([0, 0, 2.0]);
};

GASEngine.DirectionalLightComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.DirectionalLightComponent.prototype.constructor = GASEngine.DirectionalLightComponent;
GASEngine.DirectionalLightComponent.prototype.typeName = 'directionalLight';

//PointLight
GASEngine.PointLightComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.type = 'point'; 
    this.position = new Float32Array([10.0, 0.0, 0.0]);
    this.color = new Float32Array([0.0, 1.0, 0.0, 1.0]);
    // this.intensity = 0;
    this.ambientIntensity = 0.5;
    this.diffuseIntensity = 0.2;
    this.specularPower = 16.0;
    this.specularIntensity = 0.0;
    this.constant = 1.0;
    this.linear = 0.4;
    this.exp = 0.2;
    this.shininess = 32.0;
    //this.decay = 0.1;
};

GASEngine.PointLightComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.PointLightComponent.prototype.constructor = GASEngine.PointLightComponent;
GASEngine.PointLightComponent.prototype.typeName = 'pointLight';

GASEngine.PointLightComponent.prototype.setPosition = function(x, y, z)
{
    this.position = new Float32Array([x, y, z]);
}


//SpotLight
GASEngine.SpotLightComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.type = 'spot'; 
    this.position = new Float32Array([100, 100, 100]);
    this.color = new Float32Array([0.0, 0.0, 1.0, 1.0]);
    // this.intensity = 0;
    this.ambientIntensity = 0.5;
    this.diffuseIntensity = 0.2;
    this.specularPower = 16.0;
    this.specularIntensity = 0.0;
    this.constant = 1.0;
    this.linear = 0.4;
    this.exp = 0.2;
    this.shininess = 1.0;
    
    this.direction = new Float32Array([0, 1, -2.0]);
    this.angle = Math.cos(30 * Math.PI / 180); 
};

GASEngine.SpotLightComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.SpotLightComponent.prototype.constructor = GASEngine.SpotLightComponent;
GASEngine.SpotLightComponent.prototype.typeName = 'spotLight';

GASEngine.SpotLightComponent.prototype.setPosition = function(x, y, z)
{
    this.position = new Float32Array([x, y, z]);
}












