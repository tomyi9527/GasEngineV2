GASEngine.ShaderSourceProcessor = function()
{
    GASEngine.ShaderSourceProcessor.Instance = this;
};

GASEngine.ShaderSourceProcessor.prototype =
{
    _shadersText: {},
    _shadersList: {},
    _globalDefaultprecision: '#ifdef GL_FRAGMENT_PRECISION_HIGH\r\n precision highp float;\r\n #else\r\n precision mediump float;\r\n#endif',
    _debugLines: true,
    _includeR: /#pragma include "([^"]+)"/g,
    _includeCondR: /#pragma include (["^+"]?["\ "[a-zA-Z_0-9](.*)"]*?)/g,
    _defineR: /\#define\s+([a-zA-Z_0-9]+)/,
    _precisionR: /precision\s+(high|low|medium)p\s+float/,

    addShaders: function(file_, text_)
    {
        this._shadersList[file_] = file_;
        this._shadersText[file_] = text_;
    },

    isShaderSourceValid: function(file)
    {
        return (this._shadersList[file] !== undefined && this._shadersList[file] !== null);
    },

    loadShaderSourceText: function(file)
    {
        if(this._shadersList[file] !== undefined)
            return;

        this._shadersList[file] = null;

        GASEngine.Resources.Instance.loadGlsl(file, function(source)
        {
            if(this._shadersList[file] === null)
            {
                this._shadersList[file] = file;
                this._shadersText[file] = source;
            }
            else
            {
                console.error('GASEngine.ShaderSourceProcessor.loadShaderSourceText: The same glsl file \"' + file + '\" is already in the records.');
            }

        }.bind(this));
    },

    getShaderSourceFileCount: function()
    {
        return window.Object.keys(this._shadersList).length;
    },

    instrumentShaderlines: function(content, sourceID)
    {
        // TODO instrumentShaderlines
        // http://immersedcode.org/2012/1/12/random-notes-on-webgl/
        // one ID per "file"
        // Each file has its line number starting at 0
        //   handle include, the do that numbering also in preprocess...
        // Then on shader error using sourceID and line you can point the correct line...
        // has to attach that info to osg.shader object.
        /*
          var allLines = content.split('\n');
          var i = 0;
          for (var k = 0; k _< allLines.length; k++) {
          if (!this._includeR.test(allLines[k])) {
          allLines[k] = "#line " + (i++) + " " + sourceID + '\n' + allLines[k] ;
          }
          }
          content = allLines.join('\n');
        */

        // seems just  prefixing first line seems ok to help renumbering error mesg
        return '\n#line ' + 0 + ' ' + sourceID + '\n' + content;
    },

    getShaderTextPure: function(shaderName)
    {
        var preShader = this._shadersText[shaderName];
        if(!preShader)
        {
            console.error('GASEngine.ShaderSourceProcessor.getShaderTextPure: Shader file/text: ' + shaderName + ' not registered.');
            preShader = '';
        }
        return preShader;
    },

    getShader: function(shaderName, shaderKey, defines, extensions, type)
    {
        var shader = this.getShaderTextPure(shaderName);
        return this.processShader(shader, shaderKey, defines, extensions, type);
    },

    // recursively  handle #include external glsl
    // files (for now in the same folder.)
    preprocess: function(content, sourceID, includeList, inputsDefines)
    {
        var self = this;
        return content.replace(this._includeCondR, function(_, name)
        {
            var includeOpt = name.split(' ');
            var includeName = includeOpt[0].replace(/"/g, '');

            // pure include is
            // \#pragma include "name";

            // conditionnal include is name included if _PCF defined
            // \#pragma include "name" "_PCF";
            if ( includeOpt.length > 1 && inputsDefines ) {

                // some conditions here.
                // if not defined we do not include
                var found = false;
                var defines = inputsDefines.map(function(defineString)
                {
                    // find '#define', remove duplicate whitespace, split on space and return the define Text
                    return self._defineR.test(defineString) && defineString.replace(/\s+/g, ' ').split(' ')[1];
                });

                for(var i = 1; i < includeOpt.length && !found; i++)
                {
                    var key = includeOpt[ i ].replace( /"/g, '' );
                    for(var k = 0; k < defines.length && !found; k++)
                    {

                        if(defines[k] !== false && defines[k] === key)
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if(!found)
                {
                    return '';
                }
            }

            // already included
            if(includeList.indexOf(includeName) !== -1)
                return '';

            // avoid endless loop, not calling the impure
            var txt = this.getShaderTextPure( includeName );
            // make sure it's not included twice
            includeList.push(includeName);

            if(this._debugLines)
            {
                txt = this.instrumentShaderlines( txt, sourceID );
            }
            sourceID++;
            // to the infinite and beyond !
            txt = this.preprocess(txt, sourceID, includeList, inputsDefines);

            return txt;

        }.bind( this ) );
    },

    //  process a shader and define
    //  get a full expanded single shader source code
    //  resolving include dependencies
    //  adding defines
    //  adding line instrumenting.
    processShader: function(shader, shaderKey, defines, extensions, type)
    {
        var includeList = [];
        var preShader = shader;
        var sourceID = 0;
        if(this._debugLines)
        {
            preShader = this.instrumentShaderlines(preShader, sourceID);
            sourceID++;
        }

        // removes duplicates
        if(defines !== undefined)
        {
            defines = defines.sort().filter(function(item, pos)
            {
                return !pos || item !== defines[pos - 1];
            });
        }

        if(extensions !== undefined && extensions.length > 0)
        {
            extensions = extensions.sort().filter(function(item, pos)
            {
                return !pos || item !== extensions[pos - 1];
            });
        }

        var postShader = this.preprocess(preShader, sourceID, includeList, defines);

        var prePrend = '';
        prePrend += '#version 100\n\n'; // webgl1  (webgl2 #version 130 ?)

        // then
        // it's extensions first
        // See https://khronos.org/registry/gles/specs/2.0/GLSL_ES_Specification_1.0.17.pdf
        // p14-15: before any non-processor token
        // add them
        if(extensions !== undefined && extensions.length > 0)
        {
            // could add an extension check support warning there...
            prePrend += extensions.join('\n') + '\n' + '\n';
        }

        // vertex shader doesn't need precision, it's highp per default, enforced per spec
        // but then not giving precision on uniform/varying might make conflicts arise
        // between both FS and VS if FS default is mediump !
        // && type !== 'vertex'
        if(this._globalDefaultprecision)
        {
            if(!this._precisionR.test(postShader))
            {
                // use the shaderhighprecision flag at shaderloader start
                //var highp = gl.getShaderPrecisionFormat(gl.FRAGMENT_SHADER, gl.HIGH_FLOAT);
                //var highpSupported = highp.precision != 0;
                prePrend += this._globalDefaultprecision + '\n' + '\n';
            }
        }

        if(shaderKey !== undefined && shaderKey.length > 0)
        {
            prePrend += '#define SHADER_NAME ' + shaderKey + '\n' + '\n';
        }

        // if defines
        // add them
        if(defines !== undefined && defines.length > 0)
        {
            prePrend += defines.join( '\n' ) + '\n';
        }

        postShader = prePrend + postShader;

        return postShader;
    }
};
