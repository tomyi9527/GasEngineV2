
GASEngine.WebGLTextureManager = function()
{
    this.webglBufferPool = new Map();

    GASEngine.WebGLTextureManager.Instance = this;
};

GASEngine.WebGLTextureManager._linear2Srgb = function(value, gamma)
{
    if(!gamma)
        gamma = 2.2;

    var result = 0.0;
    if(value < 0.0031308)
    {
        if(value > 0.0)
            result = value * 12.92;
    }
    else
    {
        result = 1.055 * Math.pow(value, 1.0 / gamma) - 0.055;
    }

    return result;
};

GASEngine.WebGLTextureManager._convert2FloatColor = function(color)
{
    var r, g, b;
    // rgb [255, 255, 255]
    if(color.length === 3)
    {
        r = color[0];
        g = color[1];
        b = color[2];
    }
    else if(color.length === 7)
    {
        // hex (24 bits style) '#ffaabb'
        var intVal = parseInt(color.slice(1), 16);
        r = intVal >> 16;
        g = intVal >> 8 & 0xff;
        b = intVal & 0xff;
    }

    var result = [0, 0, 0, 1];
    result[0] = r / 255.0;
    result[1] = g / 255.0;
    result[2] = b / 255.0;

    return result;
};

GASEngine.WebGLTextureManager._updateTextureFromRGBA = function(texture, color, srgb)
{
    var _c = new Uint8Array(4);
    for(var i = 0; i < 4; ++i)
    {
        if(srgb)
        {
            var srgbValue = GASEngine.WebGLTextureManager.linear2Srgb(color[i]);
            _c[index] = Math.floor(255 * srgbValue);
        }
        else
        {
            _c[index] = Math.floor(255 * color[i]);
        }
    }

    var gl = GASEngine.WebGLDevice.Instance.gl;

    var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
    var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, _c);

    gl.activeTexture(previousATU);
    gl.bindTexture(gl.TEXTURE_2D, previousAT);
};

GASEngine.WebGLTextureManager._isPOT = function(image)
{
    return (GASEngine.isPOT(image.naturalWidth) && GASEngine.isPOT(image.naturalHeight));
};

GASEngine.WebGLTextureManager._doesNeedPOT = function(wrapS, wrapT, minFilter)
{
    //If you want to use mipmap filter, the size of the texture must be power of two.
    //If you want to use repeat wrapping mode, the size of the texture must be power of two.
    //Webgl2 may not have these restrictions.
    var gl = GASEngine.WebGLDevice.Instance.gl;

    if(wrapS !== gl.CLAMP_TO_EDGE || wrapT !== gl.CLAMP_TO_EDGE)
        return true;

    if(minFilter !== gl.NEAREST && minFilter !== gl.LINEAR)
        return true;

    return false;
};

GASEngine.WebGLTextureManager._makePOT = function(image)
{
    if(image instanceof HTMLImageElement || image instanceof HTMLCanvasElement)
    {
        var canvas = document.createElement('canvas');
        canvas.width = GASEngine.nearestPOT(image.naturalWidth);
        canvas.height = GASEngine.nearestPOT(image.naturalWidth);

        var context = canvas.getContext('2d');
        context.drawImage( image, 0, 0, canvas.width, canvas.height );
        console.warn('GASEngine.WebGLTextureManager._makePOT: Image is not power of two (' + image.naturalWidth + 'x' + image.naturalWidth + '). Resized to ' + canvas.width + 'x' + canvas.height);
        return canvas;
    }
    else
    {
        console.error('GASEngine.WebGLTextureManager._makePOT: Only HTMLImageElement or HTMLCanvasElement can be resized.');
        return null;
    }
};

GASEngine.WebGLTextureManager.prototype = {

    constructor: GASEngine.WebGLTextureManager,

    init: function()
    {

    },

    finl: function()
    {
        this.webglBufferPool = null;
    },

    createEmptyTexture: function(width, height, format, channelType, mipmap)
    {
    },

    createTextureFromRGBA: function(color, srgb) //The range of color value is [0.0, 1.0]
    {
        var _c = new Uint8Array(4);
        for(var i = 0; i < 4; ++i)
        {
            if(srgb)
            {
                var srgbValue = GASEngine.WebGLTextureManager.linear2Srgb(color[i]);
                _c[i] = Math.floor(255 * srgbValue);
            }
            else
            {
                _c[i] = Math.floor(255 * color[i]);
            }
        }

        var gl = GASEngine.WebGLDevice.Instance.gl;

        var texture = gl.createTexture();

        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
    
        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, _c);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return texture;
    },

    createCubeTexture: function(images)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var cubeTexture = gl.createTexture();
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

        //gl.pixelStorei(gl.UNPACK_COLORSPACE_CONVERSION_WEBGL, false); //uncompatible in ie11 and edge
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeTexture);

        for(var face = 0; face < 6; ++face)
        {
            var img = images[face];
            var mipmapLevelCount = img.length;
            for(var level = 0; level < mipmapLevelCount; ++level)
            {
                gl.texImage2D(
                    gl.TEXTURE_CUBE_MAP_POSITIVE_X + face,
                    level,
                    gl.RGBA,
                    img[level].width,
                    img[level].height,
                    0,
                    gl.RGBA,
                    gl.UNSIGNED_BYTE,
                    img[level].data);
            }
        }

        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR); //for background cubemap
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

        //gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR); //for specular lighting
        ////Without this mipmap setting, textureCubeLodEXT won't work!
        //gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

        //only for webgl 2 context
        //gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_BASE_LEVEL, 0);
        //gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAX_LEVEL, mipmapLevelCount - 1);
        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return cubeTexture;
    },

    createPanoramaTexture: function(image)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var panoramaTexture = gl.createTexture();
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);

        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, panoramaTexture);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, image.width, image.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, image.data);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
       
        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return panoramaTexture;
    },

    createIntegratedBRDF: function(image)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var integratedBRDF = gl.createTexture();

        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

        gl.bindTexture(gl.TEXTURE_2D, integratedBRDF);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, image.width, image.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, image.data);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return integratedBRDF;
    },    

    createCompressedDXTTexture: function(pixelData, dontNeedPOT)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var texture = gl.createTexture();

        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture);

        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
        gl.pixelStorei(gl.UNPACK_ALIGNMENT, 4);

        for(var i = 0; i < pixelData.mipmapCount; ++i)
        {
            var mipmap = pixelData.mipmaps[i];
            gl.compressedTexImage2D(gl.TEXTURE_2D, i, pixelData.format, mipmap.width, mipmap.height, 0, mipmap.data);
        }

        if(dontNeedPOT)
        {
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }
        else
        {
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        }

        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return texture;
    },

    createTextureFromImage: function(image, dontNeedPOT)
    {
        if(image.naturalWidth === 0 || image.naturalHeight === 0)
        {
            console.error('WebGLTextureManager.createTextureFromImage: Can not create a texture of 0 width or height.');
            return null;
        }

        var gl = GASEngine.WebGLDevice.Instance.gl;

        var texture = gl.createTexture();

        var previousATU = gl.getParameter(gl.ACTIVE_TEXTURE);
        var previousAT = gl.getParameter(gl.TEXTURE_BINDING_2D);
        var previousFlipY = gl.getParameter(gl.UNPACK_FLIP_Y_WEBGL);
        var previousPremultiplyAlpha = gl.getParameter(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL);
        var previousAlignment = gl.getParameter(gl.UNPACK_ALIGNMENT);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
        gl.pixelStorei(gl.UNPACK_ALIGNMENT, 4);

        if(dontNeedPOT)
        {
            if(image)
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
            }

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }
        else
        {
            if(GASEngine.WebGLTextureManager._isPOT(image) === false)
            {
                image = GASEngine.WebGLTextureManager._makePOT(image);
            }

            if(image)
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
                gl.generateMipmap(gl.TEXTURE_2D);
            }

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        }

        gl.pixelStorei(gl.UNPACK_ALIGNMENT, previousAlignment);
        gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, previousPremultiplyAlpha);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, previousFlipY);
        gl.activeTexture(previousATU);
        gl.bindTexture(gl.TEXTURE_2D, previousAT);

        return texture;
    },

    destroy : function(texture) {
        GASEngine.WebGLDevice.Instance.gl.deleteTexture(texture);
        // 似乎这里的pool也没用上，也就不清理pool了。
    }
};