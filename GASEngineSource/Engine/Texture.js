GASEngine.Texture = function()
{
    this.fileName = null;
    this.srgb = null;

    this._webglTexture = null;
    this._status = GASEngine.Texture.STATUS_INVALID;
};

GASEngine.Texture.STATUS_INVALID = 0;
GASEngine.Texture.STATUS_QUEUING = 1;
GASEngine.Texture.STATUS_LOADING = 2;
GASEngine.Texture.STATUS_CREATING = 3;
GASEngine.Texture.STATUS_VALID = 4;
GASEngine.Texture.STATUS_ERROR = 5;

GASEngine.Texture.prototype =
{
    constructor: GASEngine.Texture,

    isValid: function()
    {
        return (this._status === GASEngine.Texture.STATUS_VALID);
    },

    isError: function()
    {
        return (this._status === GASEngine.Texture.STATUS_ERROR);
    },

    setStatus: function(status)
    {
        this._status = status;
    },

    getStatus: function()
    {
        return this._status;
    },

    getWebglResource: function()
    {
        return this._webglTexture;
    },

    update: function(force)
    {
        if(force || this._status === GASEngine.Texture.STATUS_INVALID)
        {
            this._status = GASEngine.Texture.STATUS_LOADING;

            GASEngine.Resources.Instance.loadImage(

                this.fileName,

                function onSuccess(type, image)
                {

                }.bind(this),
            
                function onProgress(type, image)
                {

                }.bind(this),

                function onError(type, image)
                {

                }.bind(this)
            );
        }
    }
};
