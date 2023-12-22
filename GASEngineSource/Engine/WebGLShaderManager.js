
GASEngine.WebGLShaderManager = function()
{
    if(GASEngine.ShaderSourceProcessor.Instance)
    {
        this.shaderSourceProcessor = GASEngine.ShaderSourceProcessor.Instance;
    }
    else
    {
        this.shaderSourceProcessor = new GASEngine.ShaderSourceProcessor();
    }

    this.cache = new Map();

    var gl = GASEngine.WebGLDevice.Instance.gl;
    this.uniformTypeTable = new Map();

    this.uniformTypeTable.set(gl.FLOAT, 'float');
    this.uniformTypeTable.set(gl.FLOAT_VEC2, 'vec2');
    this.uniformTypeTable.set(gl.FLOAT_VEC3, 'vec3');
    this.uniformTypeTable.set(gl.FLOAT_VEC4, 'vec4');

    this.uniformTypeTable.set(gl.SAMPLER_2D, 'sampler2D');
    this.uniformTypeTable.set(gl.SAMPLER_CUBE, 'samplerCube');

    this.uniformTypeTable.set(gl.INT, 'int');
    this.uniformTypeTable.set(gl.INT_VEC2, 'vec2i');
    this.uniformTypeTable.set(gl.INT_VEC3, 'vec3i');
    this.uniformTypeTable.set(gl.INT_VEC4, 'vec4i');

    this.uniformTypeTable.set(gl.FLOAT_MAT2, 'mat2');
    this.uniformTypeTable.set(gl.FLOAT_MAT3, 'mat3');
    this.uniformTypeTable.set(gl.FLOAT_MAT4, 'mat4');

    //this.uniformTypeTable.set(gl.BOOL, 'bool');
    //this.uniformTypeTable.set(gl.BOOL_VEC2, 'vec2b');
    //this.uniformTypeTable.set(gl.BOOL_VEC3, 'vec3b');
    //this.uniformTypeTable.set(gl.BOOL_VEC4, 'vec4b');

    //const GLenum BYTE                           = 0x1400;
    //const GLenum UNSIGNED_BYTE                  = 0x1401;
    //const GLenum SHORT                          = 0x1402;
    //const GLenum UNSIGNED_SHORT                 = 0x1403;
    //const GLenum INT                            = 0x1404;
    //const GLenum UNSIGNED_INT                   = 0x1405;
    //const GLenum FLOAT                          = 0x1406;
    //const GLenum FLOAT_VEC2                     = 0x8B50;
    //const GLenum FLOAT_VEC3                     = 0x8B51;
    //const GLenum FLOAT_VEC4                     = 0x8B52;
    //const GLenum INT_VEC2                       = 0x8B53;
    //const GLenum INT_VEC3                       = 0x8B54;
    //const GLenum INT_VEC4                       = 0x8B55;
    //const GLenum BOOL                           = 0x8B56;
    //const GLenum BOOL_VEC2                      = 0x8B57;
    //const GLenum BOOL_VEC3                      = 0x8B58;
    //const GLenum BOOL_VEC4                      = 0x8B59;
    //const GLenum FLOAT_MAT2                     = 0x8B5A;
    //const GLenum FLOAT_MAT3                     = 0x8B5B;
    //const GLenum FLOAT_MAT4                     = 0x8B5C;
    //const GLenum SAMPLER_2D                     = 0x8B5E;
    //const GLenum SAMPLER_CUBE                   = 0x8B60;

    this.structRe = /^([\w\d_]+)\.([\w\d_]+)$/;
    this.arrayStructRe = /^([\w\d_]+)\[(\d+)\]\.([\w\d_]+)$/;
    this.arrayRe = /^([\w\d_]+)\[0\]$/;

    GASEngine.WebGLShaderManager.Instance = this;
};

GASEngine.WebGLShaderManager.prototype =
{
    constructor: GASEngine.WebGLShaderManager,

    init: function()
    {

    },

    finl: function()
    {
        this.cache = null;
        this.uniformTypeTable = null;
        this.shaderSourceProcessor = null;
        GASEngine.ShaderSourceProcessor.Instance = null;
    },

    getShaderProgram: function (item)
    {
        if(item.mesh === null || item.material === null)
            return null;

        var vsKey = item.material.generateVertexShaderKey(item);
        var fsKey = item.material.generateFragmentShaderKey(item);

        var firstLevel = this.cache.get(item.material.hashCode);
        var secondLevel;
        var shaderProgram;

        if(firstLevel)
        {
            secondLevel = firstLevel.get(vsKey);
            if(secondLevel)
            {
                shaderProgram = secondLevel.get(fsKey);
                if(shaderProgram)
                {
                    return shaderProgram;
                }
            }
            else
            {
                secondLevel = new Map();
                firstLevel.set(vsKey, secondLevel);
            }
        }
        else
        {
            firstLevel = new Map();
            this.cache.set(item.material.hashCode, firstLevel);

            secondLevel = new Map();
            firstLevel.set(vsKey, secondLevel);
        }

        this.shaderSourceProcessor.loadShaderSourceText(item.material.vertexShaderFile);
        this.shaderSourceProcessor.loadShaderSourceText(item.material.fragmentShaderFile);

        if(!this.shaderSourceProcessor.isShaderSourceValid(item.material.vertexShaderFile) ||
            !this.shaderSourceProcessor.isShaderSourceValid(item.material.fragmentShaderFile))
        {
            return null;
        }

        shaderProgram = this.createShaderProgram(
            item.material.vertexShaderFile,
            vsKey,
            item.material.fragmentShaderFile,
            fsKey,
            ['#define SHADERKEY ' + vsKey],
            ['#define SHADERKEY ' + fsKey],
            [],
            ['#extension GL_EXT_shader_texture_lod : require']);

        secondLevel.set(fsKey, shaderProgram);

        return shaderProgram;
    },

    _fetchUniformLocations: function(gl, program)
    {
        //DO NOT SUPPORT INNER STRUCT AND INNER ARRAY
        var uniforms = {};
        var n = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);

        for(var i = 0; i < n; i++)
        {
            var info = gl.getActiveUniform(program, i);
            var name = info.name;
            var size = info.size;
            var type = this.uniformTypeTable.get(info.type);
            var location = gl.getUniformLocation(program, name);

            var matches = this.structRe.exec(name);
            if(matches)
            {
                var structName = matches[1];
                var structProperty = matches[2];

                var uniformsStruct = uniforms[structName];
                if(!uniformsStruct)
                {
                    uniformsStruct = uniforms[structName] = {};
                }

                uniformsStruct[structProperty] = {};
                uniformsStruct[structProperty].location = location;
                uniformsStruct[structProperty].type = type;
                uniformsStruct[structProperty].size = size;
                continue;
            }

            matches = this.arrayStructRe.exec(name);

            if(matches)
            {
                var arrayName = matches[1];
                var arrayIndex = matches[2];
                var arrayProperty = matches[3];

                var uniformsArray = uniforms[arrayName];
                if(!uniformsArray)
                {
                    uniformsArray = uniforms[arrayName] = [];
                }

                var uniformsArrayIndex = uniformsArray[arrayIndex];
                if(!uniformsArrayIndex)
                {
                    uniformsArrayIndex = uniformsArray[arrayIndex] = {};
                }

                uniformsArrayIndex[arrayProperty] = {};
                uniformsArrayIndex[arrayProperty].location = location;
                uniformsArrayIndex[arrayProperty].type = type;
                uniformsArrayIndex[arrayProperty].size = size;
                continue;
            }

            matches = this.arrayRe.exec(name);
            if(matches)
            {
                var arrayName = matches[1];

                uniforms[arrayName] = {};
                uniforms[arrayName].location = location;
                uniforms[arrayName].type = type;
                uniforms[arrayName].size = size;
                continue;
            }

            uniforms[name] = {};
            uniforms[name].location = location;
            uniforms[name].type = type;
            uniforms[name].size = size;
        }

        return uniforms;
    },

    _fetchAttributeLocations: function(gl, program)
    {
        var attributes = {};
        var n = gl.getProgramParameter(program, gl.ACTIVE_ATTRIBUTES);
        for(var i = 0; i < n; i++)
        {
            var info = gl.getActiveAttrib(program, i);
            var name = info.name;

            attributes[name] = {};
            attributes[name].location = gl.getAttribLocation(program, name);
            attributes[name].type = this.uniformTypeTable.get(info.type);
            attributes[name].size = info.size;
        }

        return attributes;
    },

    _compileShaderProgram: function(vsSource, fsSource)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var vs = gl.createShader(gl.VERTEX_SHADER);
        var ps = gl.createShader(gl.FRAGMENT_SHADER);

        gl.shaderSource(vs, vsSource);
        gl.compileShader(vs);

        gl.shaderSource(ps, fsSource);
        gl.compileShader(ps);

        if(!gl.getShaderParameter(vs, gl.COMPILE_STATUS))
        {
            console.log(gl.getShaderInfoLog(vs));
            vs = undefined;
        }

        if(!gl.getShaderParameter(ps, gl.COMPILE_STATUS))
        {
            var str = gl.getShaderInfoLog(ps);
            console.log(str);
            ps = undefined;
        }

        var sp = gl.createProgram();
        gl.attachShader(sp, vs);
        gl.attachShader(sp, ps);
        gl.linkProgram(sp);

        var linkStatus = gl.getProgramParameter(sp, gl.LINK_STATUS);
        if(!linkStatus)
        {
            console.log(gl.getProgramInfoLog(sp));
            console.log("Could not initialise shaders");
        }

        return sp;
    },

    createShaderProgram: function(vertex, vertexShaderKey, fragment, fragmentShaderKey, vsDefines, fsDefines, vsExtensions, fsExtensions)
    {
        var vsSource = this.shaderSourceProcessor.getShader(vertex, vertexShaderKey, vsDefines, vsExtensions, undefined);
        var fsSource = this.shaderSourceProcessor.getShader(fragment, fragmentShaderKey, fsDefines, fsExtensions, undefined);

        var program = this._compileShaderProgram(vsSource, fsSource);
        var gl = GASEngine.WebGLDevice.Instance.gl;
        var uniforms = this._fetchUniformLocations(gl, program);
        var attributes = this._fetchAttributeLocations(gl, program);

        return { 'program': program, 'uniforms': uniforms, 'attributes': attributes, 'vsKey': vertexShaderKey, 'fsKey': fragmentShaderKey};
    }    
};