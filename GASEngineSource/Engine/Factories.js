//*******************************************************
//Component Factory
GASEngine.ComponentFactory = function()
{
    this.components = [];

    this.componentConstructors = {};
    
    this.componentConstructors[GASEngine.AnimatorComponent.prototype.typeName] = GASEngine.AnimatorComponent;
    this.componentConstructors[GASEngine.MeshFilterComponent.prototype.typeName] = GASEngine.MeshFilterComponent;
    this.componentConstructors[GASEngine.MeshRendererComponent.prototype.typeName] = GASEngine.MeshRendererComponent;
    this.componentConstructors[GASEngine.CameraComponent.prototype.typeName] = GASEngine.CameraComponent;
    this.componentConstructors[GASEngine.PunctualLightComponent.prototype.typeName] = GASEngine.PunctualLightComponent;
    this.componentConstructors[GASEngine.EnvironmentalLightComponent.prototype.typeName] = GASEngine.EnvironmentalLightComponent;
    this.componentConstructors[GASEngine.ResourceComponent.prototype.typeName] = GASEngine.ResourceComponent;

    this.componentConstructors[GASEngine.DirectionalLightComponent.prototype.typeName] = GASEngine.DirectionalLightComponent;
    this.componentConstructors[GASEngine.PointLightComponent.prototype.typeName] = GASEngine.PointLightComponent;
    this.componentConstructors[GASEngine.SpotLightComponent.prototype.typeName] = GASEngine.SpotLightComponent;

    GASEngine.ComponentFactory.Instance = this;
}

GASEngine.ComponentFactory.prototype =
{
    constructor: GASEngine.ComponentFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.components.length = 0;
        this.componentConstructors = {};
    },

    create: function(typeName, uniqueID)
    {
        if(!this.componentConstructors.hasOwnProperty(typeName))
        {
            console.error('GASEngine.ComponentFactory.create: unknown component type!');
            return null;
        }

        var newComponent = new this.componentConstructors[typeName](uniqueID);
        this.components.push(newComponent);

        return newComponent;
    },

    destroy: function(component)
    {
        for(var i = 0; i < this.components.length; ++i)
        {
            if(this.components[i] === component)
            {
                this.components.splice(i, 1);
                component.destroy();
                return;
            }
        }

        console.error('GASEngine.ComponentFactory.destroyComponent: cannot destroy an component that is not existent in the pool.');
    }
};


//*******************************************************
//Entity Factory
GASEngine.EntityFactory = function()
{
    this.usedEntityPool = [];
    //this.freeEntityPool = [];

    GASEngine.EntityFactory.Instance = this;
};

GASEngine.EntityFactory.prototype =
{
    constructor: GASEngine.EntityFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.usedEntityPool.length = 0;
    },

    create: function()
    {
        var newEntity = new GASEngine.Entity();

        this.usedEntityPool.push(newEntity);

        return newEntity;
    },

    destroy: function(entity)
    {
        for(var i = 0; i < this.usedEntityPool.length; ++i)
        {
            if(this.usedEntityPool[i] === entity)
            {
                this.usedEntityPool.splice(i, 1);
                entity.destroy();
                return;
            }
        }

        console.error('GASEngine.EntityFactory.destroy: cannot destroy an entity that is not existent in the pool.');
    }
};


//*******************************************************
//Mesh Factory
GASEngine.MeshFactory = function()
{
    this.usedMeshPool = [];
    //this.freeMeshPool = [];

    GASEngine.MeshFactory.Instance = this;
};

GASEngine.MeshFactory.prototype =
{
    constructor: GASEngine.MeshFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.usedMeshPool.length = 0;
    },

    create: function()
    {
        var newMesh = new GASEngine.Mesh();

        this.usedMeshPool.push(newMesh);

        return newMesh;
    },

    destroy: function(mesh)
    {
        for(var i = 0; i < this.usedMeshPool.length; ++i)
        {
            if(this.usedMeshPool[i] === mesh)
            {
                this.usedMeshPool.splice(i, 1);
                return;
            }
        }

        console.error('GASEngine.MeshFactory.destroy: Can not destroy a mesh that is not existent in the pool.');
    }
};


//*******************************************************
//KeyframeAnimation Factory
GASEngine.KeyframeAnimationFactory = function()
{
    this.usedKeyframeAnimationPool = [];
    //this.freeKeyframeAnimationPool = [];

    GASEngine.KeyframeAnimationFactory.Instance = this;
};

GASEngine.KeyframeAnimationFactory.prototype =
{
    constructor: GASEngine.KeyframeAnimationFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.usedKeyframeAnimationPool.length = 0;
    },

    createKeyframeAnimation: function()
    {
        var newKeyframeAnimation = new GASEngine.KeyframeAnimation();

        this.usedKeyframeAnimationPool.push(newKeyframeAnimation);

        return newKeyframeAnimation;
    },

    destroyKeyframeAnimation: function(keyframeAnimation)
    {
        for(var i = 0; i < this.usedKeyframeAnimationPool.length; ++i)
        {
            if(this.usedKeyframeAnimationPool[i] === keyframeAnimation)
            {
                this.usedKeyframeAnimationPool.splice(i, 1);
                keyframeAnimation.destroy();
                return;
            }
        }

        console.error('GASEngine.KeyframeAnimationFactory.destroyKeyframeAnimation: cannot destroy a keyframe animation that is not existent in the pool.');
    }
};