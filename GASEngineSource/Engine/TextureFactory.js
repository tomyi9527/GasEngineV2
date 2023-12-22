//*******************************************************
//Texture Factory
GASEngine.TextureFactory = function()
{
    this.usedPool = [];
    //this.freePool = [];

    GASEngine.TextureFactory.Instance = this;
};

GASEngine.TextureFactory.prototype =
{
    constructor: GASEngine.TextureFactory,

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
        var newObject = new GASEngine.Texture();

        this.usedPool.push(newObject);

        return newObject;
    },

    destroy: function(object)
    {
        for(var i = 0; i < this.usedPool.length; ++i)
        {
            if(this.usedPool[i] === object)
            {
                this.usedPool.splice(i, 1);
                return;
            }
        }

        console.error('GASEngine.TextureFactory.destroy: Can not destroy a texture that is not existent in the pool.');
    }
};