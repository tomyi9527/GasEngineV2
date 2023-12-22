//Author: saralu
//Date: 2019/5/31
//Delegate of component

SceneEditor.DComponent = function(component)
{
    SceneEditor.DObjectBase.call(this);
    this.engineComponent = component;
    this.setProperty();
}

SceneEditor.DComponent.prototype = Object.create(SceneEditor.DObjectBase.prototype);
SceneEditor.DComponent.prototype.constructor = SceneEditor.DComponent;

SceneEditor.DComponent.prototype.setProperty = function()
{
    var propName = {
        id: 'ComponentName',
        displayName: '名称',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.typeName;
            }
            else 
            {
                return '';
            }
        }.bind(this),
        set: function(name)
        {
            if(this.engineComponent)
            {
                this.engineComponent.typeName = name;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };


    this.properties.push(propName);
}

//meshRenderer-material
SceneEditor.DMeshRendererComponent = function(component)
{
    SceneEditor.DComponent.call(this, component);
}

SceneEditor.DMeshRendererComponent.prototype = Object.create(SceneEditor.DComponent.prototype);
SceneEditor.DMeshRendererComponent.prototype.constructor = SceneEditor.DMeshRendererComponent;

SceneEditor.DMeshRendererComponent.prototype.setProperty = function()
{
    SceneEditor.DComponent.prototype.setProperty.call(this);
    var propMaterial = {
        id: 'material',
        displayName: '材质',
        type: 'array',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        getElementCount: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.materials.length;
            }
            else 
            {
                return 0;
            }
        }.bind(this),
        get: function(index)
        {
            if(this.engineComponent)
            {
                if(index < 0 || index >= this.engineComponent.materials.length) return false;
                var dMaterial = new SceneEditor.DMaterial(this.engineComponent.materials[index]);
                return dMaterial;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(index, material)
        {
            if(this.engineComponent)
            {
                if(index < 0 || index >= this.engineComponent.materials.length) return false;
                this.engineComponent.materials[index] = material;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    this.properties.push(propMaterial);
}

//material
SceneEditor.DMaterial = function(material)
{
    SceneEditor.DObjectBase.call(this);
    this.engineMaterial = material;
    this.setProperty();
}
SceneEditor.DMaterial.prototype = Object.create(SceneEditor.DObjectBase.prototype);
SceneEditor.DMaterial.prototype.constructor = SceneEditor.DMaterial;

SceneEditor.DMaterial.prototype.setProperty = function()
{
    var propName = {
        id: 'materialName',
        displayName: '名称',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineMaterial)
            {
                return this.engineMaterial.typeName;
            }
            else 
            {
                return '';
            }
        }.bind(this),
        set: function(name)
        {
            if(this.engineMaterial)
            {
                this.engineMaterial.typeName = name;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propPS = {
        id: 'PS',
        displayName: 'PS',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineMaterial)
            {
                return this.engineMaterial.fragmentShaderFile;
            }
            else 
            {
                return '';
            }
        }.bind(this),
        set: function(pshader)
        {
            if(this.engineMaterial)
            {
                this.engineMaterial.fragmentShaderFile = pshader;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propVS = {
        id: 'VS',
        displayName: 'VS',
        type: 'string',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineMaterial)
            {
                return this.engineMaterial.vertexShaderFile;
            }
            else 
            {
                return '';
            }
        }.bind(this),
        set: function(vshader)
        {
            if(this.engineMaterial)
            {
                this.engineMaterial.vertexShaderFile = vshader;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    this.properties.push(propName);
    this.properties.push(propPS);
    this.properties.push(propVS);
}




//meshFilter-mesh
SceneEditor.DMeshFilterComponent = function(component)
{
    SceneEditor.DComponent.call(this, component);
}

SceneEditor.DMeshFilterComponent.prototype = Object.create(SceneEditor.DComponent.prototype);
SceneEditor.DMeshFilterComponent.prototype.constructor = SceneEditor.DMeshFilterComponent;

SceneEditor.DMeshFilterComponent.prototype.setProperty = function()
{
    SceneEditor.DComponent.prototype.setProperty.call(this);
}



//camera
SceneEditor.DCameraComponent = function(component)
{
    SceneEditor.DComponent.call(this, component);
}
SceneEditor.DCameraComponent.prototype = Object.create(SceneEditor.DComponent.prototype);
SceneEditor.DCameraComponent.prototype.constructor = SceneEditor.DCameraComponent;
SceneEditor.DCameraComponent.prototype.setProperty = function()
{
    SceneEditor.DComponent.prototype.setProperty.call(this);
    var propType = {
        id: 'cameraType',
        displayName: '相机种类',
        type: 'string',
        range: [{perspective: 1}, {orthographic: 2}],
        editorType: 'Drapdown',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.type;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(type)
        {
            if(this.engineComponent)
            {
                this.engineComponent.type = type;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propNear = {
        id: 'near',
        displayName: '近平面',
        type: 'number',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.near;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(near)
        {
            if(this.engineComponent)
            {
                this.engineComponent.near = near;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };

    var propFar = {
        id: 'far',
        displayName: '远平面',
        type: 'number',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.far;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(far)
        {
            if(this.engineComponent)
            {
                this.engineComponent.far = far;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };
    
    var propFov = {
        id: 'fov',
        displayName: 'Fov',
        type: 'number',
        range: undefined,
        editorType: 'Input',
        viewType: 'Span',
        showMode: 'view',
        get: function()
        {
            if(this.engineComponent)
            {
                return this.engineComponent.fov;
            }
            else
            {
                return false;
            }
        }.bind(this),
        set: function(fov)
        {
            if(this.engineComponent)
            {
                this.engineComponent.fov = fov;
            }
            else
            {
                return false;
            }
        }.bind(this)
    };
    


    this.properties.push(propType);
    this.properties.push(propNear);
    this.properties.push(propFar);
    this.properties.push(propFov);

}

//light