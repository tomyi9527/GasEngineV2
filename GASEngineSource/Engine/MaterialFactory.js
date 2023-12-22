//*******************************************************
//Material Factory
GASEngine.MaterialFactory = function()
{
    this.usedPool = [];
    //this.freePool = [];

    this.materialConstructors = {};

    this.materialConstructors[GASEngine.BlinnPhongMaterial.prototype.typeName] = GASEngine.BlinnPhongMaterial;
    this.materialConstructors[GASEngine.DielectricMaterial.prototype.typeName] = GASEngine.DielectricMaterial;
    this.materialConstructors[GASEngine.ElectricMaterial.prototype.typeName] = GASEngine.ElectricMaterial;
    this.materialConstructors[GASEngine.MatCapMaterial.prototype.typeName] = GASEngine.MatCapMaterial;
    this.materialConstructors[GASEngine.CompoundMaterial.prototype.typeName] = GASEngine.CompoundMaterial;
    this.materialConstructors[GASEngine.SkyboxMaterial.prototype.typeName] = GASEngine.SkyboxMaterial;
    this.materialConstructors[GASEngine.WireframeMaterial.prototype.typeName] = GASEngine.WireframeMaterial;
    this.materialConstructors[GASEngine.PureColorMaterial.prototype.typeName] = GASEngine.PureColorMaterial;
    this.materialConstructors[GASEngine.DepthMaterial.prototype.typeName] = GASEngine.DepthMaterial;
    this.materialConstructors[GASEngine.HotspotMaterial.prototype.typeName] = GASEngine.HotspotMaterial;
    this.materialConstructors[GASEngine.UVLayoutMaterial.prototype.typeName] = GASEngine.UVLayoutMaterial;
    this.materialConstructors[GASEngine.LambertMaterial.prototype.typeName] = GASEngine.LambertMaterial;

    GASEngine.MaterialFactory.Instance = this;
};

GASEngine.MaterialFactory.prototype =
{
    constructor: GASEngine.MaterialFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.usedPool.length = 0;
        this.materialConstructors = {};
    },

    create: function(typeName)
    {
        if(!this.materialConstructors.hasOwnProperty(typeName))
        {
            console.error('GASEngine.MaterialFactory.create: unknown material type!');
            return null;
        }

        var newMaterial = new this.materialConstructors[typeName]();
        newMaterial.typeName = typeName;
        newMaterial.uniqueID = -1;
        this.usedPool.push(newMaterial);

        return newMaterial;
    },

    destroy: function(e)
    {
        for(var i = 0; i < this.usedPool.length; ++i)
        {
            if(this.usedPool[i] === e)
            {
                this.usedPool.splice(i, 1);
                return;
            }
        }

        console.error('GASEngine.MaterialFactory.destroy: cannot destroy a material that is not existent in the pool.');
    }
};




//MaterialMap Factory//
GASEngine.MaterialMapFactory = function()
{
    this.usedPool = [];

    GASEngine.MaterialMapFactory.Instance = this;
};

GASEngine.MaterialMapFactory.prototype =
{
    constructor: GASEngine.MaterialMapFactory,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.usedPool.length = 0;
    },

    create: function()
    {
        var newObject = new GASEngine.MaterialMap();

        this.usedPool.push(newObject);

        return newObject;
    },

    destroy: function(e)
    {
        for(var i = 0; i < this.usedPool.length; ++i)
        {
            if(this.usedPool[i] === object)
            {
                this.usedPool.splice(i, 1);
                return;
            }
        }

        console.error('GASEngine.MaterialMapFactory.destroy: Can not destroy a material map that is not existent in the pool.');
    }
};