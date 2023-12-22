//MaterialMap//
GASEngine.MaterialMap = function()
{
    this.texture = '';

    this.wrapModeU = "REPEAT";
    this.wrapModeV = "REPEAT";

    this.minFilter = "LINEAR_MIPMAP_NEAREST";
    this.maxFilter = "NEAREST_MIPMAP_NEAREST";

    this.translation = new Float32Array([0.0, 0.0, 0.0]);
    this.rotation = new Float32Array([0.0, 0.0, 0.0]);
    this.scaling = new Float32Array([1.0, 1.0, 1.0]);
    this.rotationPivot = new Float32Array([0.0, 0.0, 0.0]);
    this.scalingPivot = new Float32Array([0.0, 0.0, 0.0]);

    this.pixelChannels = new Float32Array([1.0, 1.0, 1.0, 1.0]);

    this.uvSwap = false;
};

GASEngine.MaterialMap.REPEAT = 0;
GASEngine.MaterialMap.CLAMP_TO_EDGE = 1;
GASEngine.MaterialMap.MIRRORED_REPEAT = 2;

GASEngine.MaterialMap.LINEAR = 0;
GASEngine.MaterialMap.NEAREST = 1;
GASEngine.MaterialMap.LINEAR_MIPMAP_LINEAR = 2;
GASEngine.MaterialMap.NEAREST_MIPMAP_NEAREST = 3;
GASEngine.MaterialMap.NEAREST_MIPMAP_LINEAR = 4;
GASEngine.MaterialMap.LINEAR_MIPMAP_NEAREST = 5;

GASEngine.MaterialMap.prototype =
{
    constructor: GASEngine.MaterialMap,

    getTexture: function()
    {
        return this.texture;
    }
};