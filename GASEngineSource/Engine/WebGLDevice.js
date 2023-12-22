GASEngine.WebGLDevice = function()
{
    this.webglExtensions = {};
    this.gl = null;
    this.canvas = null;
    this.maxVertexUniformVectorCount = 0;
    this.maxVertexTextureCount = 0;
    this.maxTextureSize = 0;

    GASEngine.WebGLDevice.Instance = this;
};

GASEngine.WebGLDevice.prototype =
{
    constructor: GASEngine.WebGLDevice,

    getWebglExtensions: function(name)
    {
        if(this.gl === null)
        {
            return null;
        }

        var gl = this.gl;

        if(this.webglExtensions[name] !== undefined)
        {
            return this.webglExtensions[name];
        }

        var extension;
        switch(name)
        {
            case 'EXT_texture_filter_anisotropic':
                extension = gl.getExtension('EXT_texture_filter_anisotropic') || gl.getExtension('MOZ_EXT_texture_filter_anisotropic') || gl.getExtension('WEBKIT_EXT_texture_filter_anisotropic');
                break;
            case 'WEBGL_compressed_texture_s3tc':
                extension = gl.getExtension('WEBGL_compressed_texture_s3tc') || gl.getExtension('MOZ_WEBGL_compressed_texture_s3tc') || gl.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');
                break;
            case 'WEBGL_compressed_texture_pvrtc':
                extension = gl.getExtension('WEBGL_compressed_texture_pvrtc') || gl.getExtension('WEBKIT_WEBGL_compressed_texture_pvrtc');
                break;
            case 'WEBGL_compressed_texture_etc1':
                extension = gl.getExtension('WEBGL_compressed_texture_etc1');
                break;
            default:
                extension = gl.getExtension(name);
        }

        if(extension === null)
        {
            console.warn('GASEngine.WebGLDevice: ' + name + ' extension not supported.');
        }

        this.webglExtensions[name] = extension;

        return extension;
    },

    init: function(canvas, width, height)
    {
        //TODO
        //add renderstate init function

        "use strict"

        this.canvas = canvas;

        this.canvas.width = width;
        this.canvas.height = height;

        this.canvas.addEventListener('webglcontextlost', this.onContextLost, false);
        this.canvas.addEventListener('webglcontextrestored', this.onContextRestored, false);

         var attributes =
        {
            alpha: false,
            depth: true,
            stencil: true,
            antialias: true,
            premultipliedAlpha: true,
            preserveDrawingBuffer: true,//解决截图时toDataURL返回黑色的问题
            failIfMajorPerformanceCaveat: true
        };

        this.gl = this.canvas.getContext('webgl', attributes) || this.canvas.getContext('experimental-webgl', attributes);

        if(this.gl === null)
        {
            if(this.canvas.getContext('webgl') !== null)
            {
                console.error('GASEngine.WebGLDevice.init: init webgl context failed with specified parameters.');
            }
            else
            {
                console.error('GASEngine.WebGLDevice.init: init webgl context failed.');
            }
           
            this.finl();

            return false;
        }

        this.maxVertexUniformVectorCount = this.gl.getParameter(this.gl.MAX_VERTEX_UNIFORM_VECTORS);
        this.maxTextureSize = this.gl.getParameter(this.gl.MAX_TEXTURE_SIZE);
        this.maxVertexTextureCount = this.gl.getParameter(this.gl.MAX_VERTEX_TEXTURE_IMAGE_UNITS);

        this.getWebglExtensions('WEBGL_compressed_texture_s3tc');
        this.getWebglExtensions('WEBGL_compressed_texture_pvrtc');
        this.getWebglExtensions('WEBGL_compressed_texture_etc1');
        this.getWebglExtensions('WEBGL_draw_buffers');
        this.getWebglExtensions('OES_texture_float');
        this.getWebglExtensions('OES_standard_derivatives');//for fwidth function
        this.getWebglExtensions('OES_texture_float_linear');
        this.getWebglExtensions('ANGLE_instanced_arrays'); //INSTANCE DRAWING
        this.getWebglExtensions('WEBGL_depth_texture');
        this.getWebglExtensions('EXT_shader_texture_lod');
        this.getWebglExtensions('OES_element_index_uint');

        console.log(this.gl.getSupportedExtensions());
        console.log(this.gl.getContextAttributes());


        new GASEngine.WebGLRenderStates();
        GASEngine.WebGLRenderStates.Instance.reset();

        return true;
    },

    setSize: function(width, height)
    {
        this.canvas.width = width;
        this.canvas.height = height;
    },

    getCanvasWidth: function()
    {
        return this.canvas.width;
    },

    getCanvasHeight: function()
    {
        return this.canvas.height;
    },

    finl: function()
    {
        this.gl = null;
        this.webglExtensions = {};

        this.canvas.removeEventListener('webglcontextlost', this.onContextLost, false);
        this.canvas.removeEventListener('webglcontextrestored', this.onContextRestored, false);

        this.canvas = null;
    },

    doesSupportFloatTexture: function()
    {
        return (this.getWebglExtensions('OES_texture_float') !== null);
    },

    doesSupportVertexTexture: function()
    {
        return (this.maxVertexTextureCount > 0);
    },

    doesSupportTextureLOD: function()
    {
        return (this.getWebglExtensions('EXT_shader_texture_lod') !== null);
    },

    onContextLost: function(event)
    {
        event.preventDefault();
        console.error('GASEngine.WebGLDevice.onContextLost: Webgl context has lost.');

        //Cancel render loop when device lost occur.
        //var requestId = window.requestAnimationFrame(draw);
        //window.cancelAnimationFrame(requestId);
    },

    onContextRestored: function(event)
    {
        event.preventDefault();
        console.error('GASEngine.WebGLDevice.onContextLost: Webgl context has restored.');
    },

    forceContextLost: function()
    {
        this.getWebglExtensions.get('WEBGL_lose_context').loseContext();
        //this.getWebglExtensions.get('WEBGL_lose_context').restoreContext();
    }
};
