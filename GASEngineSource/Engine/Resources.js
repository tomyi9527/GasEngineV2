//Author: tomyi
//Date: 2017-06-19

GASEngine.Resources = function()
{
    GASEngine.Events.attach(this);

    this.gas2MeshDecoder                  = new GASEngine.GAS2MeshDecoder();
    this.gas2KeyframeAnimationDecoder     = new GASEngine.GAS2KeyframeAnimationDecoder();
    this.gas2MaterialDecoder              = new GASEngine.GAS2MaterialDecoder();
    this.ddsDecoder                         = new GASEngine.DDSDecoder();
    this.tgaDecoder                         = new GASEngine.TGADecoder();
    this.pvrDecoder                         = new GASEngine.PVRDecoder();

    this.imageDecodingTable = 
    {
        'tga':     this._decodeTGA,
        'dds':     this._decodeDDS
    };

    this.meshCache = new Map();
    this.materialCache = new Map();
    this.imageCache = new Map();
    this.textureCache = new Map();

    GASEngine.Resources.Instance = this;
};

GASEngine.Resources.prototype =
{
    constructor: GASEngine.Resources,

    //_decodePNG_JPG: function(binary, callback, type)
    //{
    //    var blob = new window.Blob([binary], { 'type': 'image/' + type });

    //    var domUrl = self.URL || self.webkitURL || self;
    //    var internalDataUrl = domUrl.createObjectURL(blob);

    //    var image = new Image();
    //    image.onload = function()
    //    {
    //        domUrl.revokeObjectURL(internalDataUrl);
    //        callback(image);
    //    };
    //    image.src = internalDataUrl;
    //},
    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.meshCache = null;
        this.materialCache = null;
        this.imageCache = null;
        this.textureCache = null;

        this.gas2MeshDecoder = null;
        this.gas2KeyframeAnimationDecoder = null;
        this.gas2MaterialDecoder = null;
        this.ddsDecoder = null;
        this.tgaDecoder = null;
        this.pvrDecoder = null;
    },

    _decodeTGA: function(binary)
    {
        var canvas = this.tgaDecoder.decode(binary);
        return canvas;
    },

    _decodeDDS: function(binary)
    {
        var mipmapData = this.ddsDecoder.decode(binary, true);
        return mipmapData;
    },

    _decodePVR: function(binary)
    {
        var mipmapData = this.pvrDecoder.decode(binary, true);
        return mipmapData;
    },

    _decodeETC: function(binary)
    {
        return null;
    },

    _decodeCube: (function()
    {
        deinterleaveImage4_CubeMap = function(size, src, dst)
        {
            var npixel = size * size;
            var npixel2 = 2 * size * size;
            var npixel3 = 3 * size * size;
            var idx = 0;
            for(var i = 0; i < npixel; i++)
            {
                dst[idx++] = src[i];
                dst[idx++] = src[i + npixel];
                dst[idx++] = src[i + npixel2];
                dst[idx++] = src[i + npixel3];
            }
        };

        deinterleaveImage3_CubeMap = function(size, src, dst)
        {
            var npixel = size * size;
            var idx = 0;
            for(var i = 0; i < npixel; i++)
            {
                dst[idx++] = src[i];
                dst[idx++] = src[i + npixel];
                dst[idx++] = src[i + 2 * npixel];
            }
        };

        pixelType = 'RGBA';

        return function(data, imageSize)
        {
            var maxLevel = Math.log(imageSize) / Math.LN2;
            var offset = 0;
            var images = {};
            for(var i = 0; i <= maxLevel; i++)
            {
                var size = Math.pow(2, maxLevel - i);
                var byteSize;
                if(offset >= data.byteLength)
                {
                    break;
                }

                for(var face = 0; face < 6; face++)
                {
                    if(!images[face])
                        images[face] = [];

                    var imageData;
                    var deinterleave;
                    if(pixelType === 'FLOAT')
                    {
                        byteSize = size * size * 4 * 3;
                        imageData = new Float32Array(data, offset, byteSize / 4);
                        deinterleave = new Float32Array(byteSize / 4);
                        deinterleaveImage3_CubeMap(size, imageData, deinterleave);
                    }
                    else
                    {
                        byteSize = size * size * 4;
                        imageData = new Uint8Array(data, offset, byteSize);
                        deinterleave = new Uint8Array(byteSize);
                        deinterleaveImage4_CubeMap(size, imageData, deinterleave);
                    }

                    var image = {};
                    image.data = deinterleave;
                    image.width = size;
                    image.height = size;
                    image.type = pixelType;
                    images[face].push(image);
                    offset += byteSize;
                }
            }

            return images;
        }
    })(),

    _decodePanorama: (function()
    {
        deinterleaveImage4_Panorama = function(size, src, dst)
        {
            var npixel = size * size;
            var npixel2 = 2 * npixel;
            var npixel3 = 3 * npixel;
            var idx = 0;
            for(var i = 0; i < npixel; i++)
            {
                dst[idx++] = src[i];
                dst[idx++] = src[i + npixel];
                dst[idx++] = src[i + npixel2];
                dst[idx++] = src[i + npixel3];
            }
        };

        deinterleaveImage3_Panorama = function(size, src, dst)
        {
            var npixel = size * size;
            var idx = 0;
            for(var i = 0; i < npixel; i++)
            {
                dst[idx++] = src[i];
                dst[idx++] = src[i + npixel];
                dst[idx++] = src[i + 2 * npixel];
            }
        };

        pixelType = 'RGBA';

        return function(data)
        {
            var imageSize = Math.sqrt(data.byteLength / 4);

            var imageData, deinterleave;
            if(pixelType === 'FLOAT')
            {
                imageData = new Float32Array(data);
                deinterleave = new Float32Array(data.byteLength / 4);
                deinterleaveImage3_Panorama(imageSize, imageData, deinterleave);
            }
            else
            {
                imageData = new Uint8Array(data);
                deinterleave = new Uint8Array(data.byteLength);
                deinterleaveImage4_Panorama(imageSize, imageData, deinterleave);
            }

            imageData = deinterleave;

            var image = {};
            image.data = imageData;
            image.width = imageSize;
            image.height = imageSize / 2;
            image.type = pixelType;

            return image;
        };
    })(),

    _decodeSPHCoefficients: function(sphCoef)
    {
        // use spherical harmonics with 9 coef
        var _sphCoef = sphCoef.slice(0, 9 * 3);

        var coef0 = 1.0 / (2.0 * Math.sqrt(Math.PI));
        var coef1 = -(Math.sqrt(3.0 / Math.PI) * 0.5);
        var coef2 = -coef1;
        var coef3 = coef1;
        var coef4 = Math.sqrt(15.0 / Math.PI) * 0.5;
        var coef5 = -coef4;
        var coef6 = Math.sqrt(5.0 / Math.PI) * 0.25;
        var coef7 = coef5;
        var coef8 = Math.sqrt(15.0 / Math.PI) * 0.25;

        var coef = [
            coef0, coef0, coef0,
            coef1, coef1, coef1,
            coef2, coef2, coef2,
            coef3, coef3, coef3,
            coef4, coef4, coef4,
            coef5, coef5, coef5,
            coef6, coef6, coef6,
            coef7, coef7, coef7,
            coef8, coef8, coef8,
        ];

        var _sphCoef = coef.map(function(value, index)
        {
            return value * _sphCoef[index];
        });
        //[0.21971432418454515, 0.2758703701933875, 0.35613621177076793,
        //0.10225326788350737, 0.16367988707743056, 0.2544617451864812,
        //-0.24532096939380132, -0.3019754078539687, -0.3627145633136445,
        //0.0036598282551576315, 0.00382456547807082, 0.004935862575243297,
        //-0.002195880314194102, -0.000995828395677054, 0.0001875217349815327,
        //-0.1695460356530012, -0.21178614307655216, -0.26029747849170165,
        //0.05574167445959989, 0.06481832831600216, 0.07105645808513175,
        //-0.0013283531583667676, -0.0015598204688720054, 0.0007777786724479177,
        //-0.010051609443711718, -0.01443906543128339, -0.023319244447714278]

        return _sphCoef;
    },

    _decodedIntegratedBRDF: function(data)
    {
        var imageSize = Math.sqrt(data.byteLength / 4);
        var integrateBRDFLUV = true;
        var imageData = new Uint8Array(data);

        var image = {};
        image.data = imageData;
        image.width = imageSize;
        image.height = imageSize;
        image.type = 'RGBA';
    },    

    loadAnimationClip: function(file, onSuccess)
    {
        GASEngine.FileSystem.Instance.read(file, function(binaryStream)
        {
            var dataView = new DataView(binaryStream);

            var clip = this.gas2KeyframeAnimationDecoder.parse(dataView);

            onSuccess(clip);

        }.bind(this));
    },

    loadMesh: function(file, onSuccess)
    {
        var mesh = this.meshCache.get(file);
        if(mesh !== undefined)
        {
            onSuccess(mesh);
            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(binaryStream)
        {
            var mesh = this.meshCache.get(file);
            if(mesh === undefined)
            {
                var dataView = new DataView(binaryStream);
                mesh = this.gas2MeshDecoder.parse(dataView);
                this.meshCache.set(file, mesh);
            }

            onSuccess(mesh);

        }.bind(this));
    },

    loadMaterial: function(file, onSuccess)
    {
        var material = this.materialCache.get(file);
        if(material !== undefined)
        {
            onSuccess(material);
            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(materialJSON)
        {
            var material = this.materialCache.get(file);
            if(material === undefined)
            {
                material = this.gas2MaterialDecoder.parse(materialJSON);
                this.materialCache.set(file, material);
            }

            onSuccess(material);

        }.bind(this));
    },

    loadImage: function(file, onSuccess, onProgress, onError)
    {
        var cacheItem = this.imageCache.get(file);
        if(cacheItem !== undefined)
        {
            onSuccess(cacheItem.image, cacheItem.ext);
            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(binary, type)
        {
            var cacheItem = this.imageCache.get(file);

            if(cacheItem === undefined)
            {
                var image;
                var decodingFunction = this.imageDecodingTable[type];
                if(decodingFunction !== undefined)
                {                
                    image = decodingFunction.call(this, binary);
                }
                else
                {
                    image = binary;
                }
                this.imageCache.set(file, {image: image, ext: type});
                onSuccess(image, type);
            } else {
                onSuccess(cacheItem.image, cacheItem.type);
            }

        }.bind(this), onProgress, onError);
    },

    //TODO: Maybe it is better to abstract a new high level texture manager.
    loadTextureOnMaterial: function(materialMap, onSuccess, onError)
    {
        let texturePath = materialMap.texture;
        materialMap.image = undefined;
        var promise = new Promise((rs, rj)=>{
            this.loadTexture
            (
                texturePath, 
                false, 
                function(webglTexture, width, height, image)
                {
                    materialMap.webglTexture = webglTexture;
                    materialMap.image = image;
                    rs(image);
                    if (onSuccess) onSuccess();
                }, 
                null, 
                ()=>{
                    rj();
                    materialMap.texture = '';
                    materialMap.image = null;
                    if (onError) onError();
                }
            );
        });
        if (!materialMap.image) {
            materialMap.image = promise;
        }
    },
    unloadTextureOnMaterial: function(materialMap)
    {
        // 为防止公用某个item，先取消清理步骤。
        // if (this.textureCache.has(materialMap.texture)) {
        //     this.textureCache.delete(materialMap.texture);
        // }
        materialMap.texture = '';
        materialMap.image = undefined;
        // if(materialMap.webglTexture)
        //    this.unloadTexture(materialMap.webglTexture);
        materialMap.webglTexture = undefined;
    },

    // TODO(beanpliu): 修改image的存储方式为 重载Resource类，此处后续需要删除。
    loadTexture: function(file, dontNeedPOT, onSuccess, onProgress, onError)
    {
        //get from texture cache.
        //id -> signed
        //cache no token url
        var record = this.textureCache.get(file);
        if(record !== undefined)
        {
            if(record.webglTexture != null)
            {
                onSuccess(record.webglTexture, record.width, record.height, record.image);
            }
            else
            {
                onError(null, 0, 0, null);
            }

            return;
        }

        this.loadImage(file, function(image, type)
        {
            var webglTextureRecord = this.textureCache.get(file); //Check again

            if(webglTextureRecord === undefined)
            {
                if(type === 'png' || type === 'jpg' || type === 'jpeg' || type === 'tga' || type === 'blob')
                {
                    webglTexture = GASEngine.WebGLTextureManager.Instance.createTextureFromImage(image, dontNeedPOT);
                    this.textureCache.set(file, 
                    {
                        'webglTexture': webglTexture, 
                        'width': image.naturalWidth, 
                        'height': image.naturalHeight,
                        'image': image
                    });

                    onSuccess(webglTexture, image.naturalWidth, image.naturalHeight, image);
                }
                else if(type === 'dds')
                {
                    webglTexture = GASEngine.WebGLTextureManager.Instance.createCompressedDXTTexture(image, dontNeedPOT);
                    this.textureCache.set(file, 
                    {
                        'webglTexture': webglTexture, 
                        'width': image.mipmaps[0].width, 
                        'height': image.mipmaps[0].height,
                        'image': image
                    });

                    onSuccess(webglTexture, image.mipmaps[0].width, image.mipmaps[0].height, image);
                }
                else
                {
                    webglTexture = null;
                    this.textureCache.set(file, 
                     {
                         'webglTexture': null, 
                         'width': 0, 
                         'height': 0,
                         'image': null
                     });

                    console.error('GASEngine.Resource.loadTextureOnMaterial: Function is not supported yet.');
                    onError(null, 0, 0, null);
                }
            }
            else
            {
                onSuccess(webglTextureRecord.webglTexture, webglTextureRecord.width, webglTextureRecord.height, webglTextureRecord.image);
            }

        }.bind(this), onProgress, onError);
    },
    //END TODO:

    loadCubeTexture: function(file, imageSize, onSuccess)
    {
        var record = this.textureCache.get(file);
        if(record !== undefined)
        {
            onSuccess(record.webglTexture, record.width, record.height);
            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(binary, type)
        {
            var webglCubeTextureRecord = this.textureCache.get(file);
            var webglCubeTexture;
            if(webglCubeTextureRecord === undefined)
            {
                var images = this._decodeCube(binary, imageSize);
                webglCubeTexture = GASEngine.WebGLTextureManager.Instance.createCubeTexture(images);
                this.textureCache.set(file, 
                    {
                        'webglTexture': webglCubeTexture, 
                        'width': imageSize, 
                        'height': imageSize
                    });
            }
            else
            {
                webglCubeTexture = webglCubeTextureRecord.webglTexture;
            }

            onSuccess(webglCubeTexture, imageSize, imageSize);

        }.bind(this));
    },

    loadPanorama: function(file, onSuccess, onProgress, onError)
    {
        var record = this.textureCache.get(file);
        if(record !== undefined)
        {
            if(record.webglTexture != null)
            {
                onSuccess(record.webglTexture, record.width, record.height);
            }
            else
            {
                onError(null, 0, 0);
            }

            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(binary, type)
        {
            var webglTexture = this.textureCache.get(file);
            if(webglTexture === undefined)
            {
                var image = this._decodePanorama(binary);
                webglTexture = GASEngine.WebGLTextureManager.Instance.createPanoramaTexture(image);
                this.textureCache.set(file, 
                    {
                        'webglTexture': webglTexture,
                        'width': image.width,
                        'height': image.height
                    });
            }

            onSuccess(webglTexture, image.width, image.height);

        }.bind(this));
    },

    loadSPH: function(file, onSuccess)
    {
        GASEngine.FileSystem.Instance.read(file, function(conf, type)
        {
            var decodedSPH = this._decodeSPHCoefficients(conf.diffuseSPH);

            var presetBrightness = conf.brightness;

            var sph = new Float32Array(decodedSPH);

            onSuccess(presetBrightness, sph);

        }.bind(this));
    },

    loadIntegratedBRDF: function(file, onSuccess)
    {
        var record = this.textureCache.get(file);
        if(record !== undefined)
        {
            if(record.webglTexture != null)
            {
                onSuccess(record.webglTexture, record.width, record.height);
            }
            else
            {
                onError(null, 0, 0);
            }

            return;
        }

        GASEngine.FileSystem.Instance.read(file, function(data, type)
        {
            var webglTexture = this.textureCache.get(file);

            if(webglTexture === undefined)
            {
                var imageSize = Math.sqrt(data.byteLength / 4);
                var imageData = new Uint8Array(data);

                var image = {};
                image.data = imageData;
                image.width = imageSize;
                image.height = imageSize;
                image.type = 'RGBA';

                webglTexture = GASEngine.WebGLTextureManager.Instance.createIntegratedBRDF(image);

                this.textureCache.set(file, 
                    {
                        'webglTexture': webglTexture, 
                        'width': imageSize, 
                        'height': imageSize
                    });
            }

            onSuccess(webglTexture, imageSize, imageSize);

        }.bind(this));
    },

    loadGlsl: function(file, onSuccess)
    {
        GASEngine.FileSystem.Instance.read(file, function(text, type)
        {
            onSuccess(text);

        }.bind(this));
    },

    unloadTexture: function(webglTexture) {
        GASEngine.WebGLTextureManager.Instance.destroy(webglTexture);
    },

    getMaterialPath: function(material) {
        for(var key of this.materialCache.keys()) {
            const value = this.materialCache.get(key);
            if(value === material) {
                return key;
            }

            if(value.typeName === 'compound') {
                const materials = value.materials;
                for(var matKey of materials.keys()) {
                    const mat = materials.get(matKey);
                    if(mat === material) {
                        return key;
                    }
                }
            }
        }
        return '';
    }

}