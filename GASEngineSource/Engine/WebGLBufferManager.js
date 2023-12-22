
GASEngine.WebGLBufferManager = function()
{
    this.webglBufferPool = new Map();

    GASEngine.WebGLBufferManager.Instance = this;
};

GASEngine.WebGLBufferManager.prototype = 
{
    constructor: GASEngine.WebGLBufferManager,

    init: function()
    {
        return true;
    },

    finl: function()
    {
        this.webglBufferPool = null;
    },

    createSharedArrayBuffer: function(stream, streamTypes, dynamic) // streamTypes:['position', 'color']
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;
        
        var records = [];

        var bufferType = gl.ARRAY_BUFFER;
        var webglBuffer = gl.createBuffer();
        gl.bindBuffer(bufferType, webglBuffer); //gl.ELEMENT_ARRAY_BUFFER
        if(dynamic === 'dynamic')
        {
            gl.bufferData(bufferType, stream, gl.DYNAMIC_DRAW); 
        }
        else
        {
            gl.bufferData(bufferType, stream, gl.STATIC_DRAW); //gl.DYNAMIC_DRAW, gl.STREAM_DRAW
        }

        var errorCode = gl.getError();
        if(errorCode === gl.OUT_OF_MEMORY)
        {
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            console.error('GASEngine.WebGLBufferManager.createBuffer: unable to create webgl buffer for not sufficient memory.');
            return null;
        }
        else
        {

            var attributeOffset = 0;
            var attributeStride = 0;
            for(var i = 0; i < streamTypes.length; ++i)
            {
                var componentCount;
                var componentType;
                var componentNormalized;
                var streamType = streamTypes[i];
                if(streamType === 'position' || streamType === 'normal' || streamType === 'normal1')
                {
                    componentCount = 3;
                    componentType = gl.FLOAT;
                    componentNormalized = false;
                    attributeOffset = attributeStride;
                    attributeStride += 12;                    
                }
                else if(streamType === 'color' || streamType === 'color1')
                {
                    componentCount = 4;
                    componentType = gl.UNSIGNED_BYTE;
                    componentNormalized = true;
                    attributeOffset = attributeStride;
                    attributeStride += 4;                    
                }

                var record = 
                {
                    'dynamic': dynamic,
                    'webglBuffer': webglBuffer,
                    'componentCount': componentCount,
                    'componentType': componentType,
                    'componentNormalized': componentNormalized,
                    'attributeStride': 0,
                    'attributeOffset': attributeOffset
                };

                records.push(record);
            }
            
            for(var i = 0; i < records.length; ++i)
            {
                records[i].attributeStride = attributeStride;
            }

            this.webglBufferPool.set(stream, records[0]);
        }

        gl.bindBuffer(bufferType, null);

        return records;
    },

    createBuffer: function(stream, streamType, dynamic)
    {
        var record = this.webglBufferPool.get(stream);
        if(record !== undefined)
        {
            console.error('GASEngine.WebGLBufferManager.createBuffer: the specified data is already in the pool.');
            return false;
        }

        var gl = GASEngine.WebGLDevice.Instance.gl;

        var bufferType, componentCount, componentType, componentNormalized, attributeStride, attributeOffset;
        if(streamType === 'position' || streamType === 'normal' || streamType === 'normal1')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 3;
            componentType = gl.FLOAT;
            componentNormalized = false;
            attributeStride = 12;
            attributeOffset = 0;
        }
        else if(streamType === 'tangent' || streamType === 'tangent1')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 4;
            componentType = gl.FLOAT;
            componentNormalized = false;
            attributeStride = 16;
            attributeOffset = 0;
        }
        else if(streamType === 'uv' || streamType === 'uv1')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 2;
            componentType = gl.FLOAT;
            componentNormalized = false;
            attributeStride = 8;
            attributeOffset = 0;
        }
        else if(streamType === 'color' || streamType === 'color1')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 4;
            componentType = gl.UNSIGNED_BYTE;
            //componentType = gl.FLOAT;
            componentNormalized = true;
            attributeStride = 4;
            //sattributeStride = 16;
            attributeOffset = 0;
        }
        else if(streamType === 'skinWeight')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 4;
            componentType = gl.FLOAT;
            componentNormalized = false;
            attributeStride = 16;
            attributeOffset = 0;
        }
        else if(streamType === 'skinIndex')
        {
            bufferType = gl.ARRAY_BUFFER;
            componentCount = 4;
            componentType = gl.UNSIGNED_SHORT;
            componentNormalized = false;
            attributeStride = 8;
            attributeOffset = 0;
        }
        else if(streamType === 'index')
        {
            if (stream instanceof Uint16Array) {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 3;
                componentType = gl.UNSIGNED_SHORT;
                componentNormalized = false;
                attributeStride = 6;
                attributeOffset = 0;
            }
            else {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 3;
                componentType = gl.UNSIGNED_INT;
                componentNormalized = false;
                attributeStride = 12;
                attributeOffset = 0;
            }
        }
        else if(streamType === 'topology')
        {
            if(stream instanceof Uint16Array) {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 2;
                componentType = gl.UNSIGNED_SHORT;
                componentNormalized = false;
                attributeStride = 4;
                attributeOffset = 0;
            }
            else {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 2;
                componentType = gl.UNSIGNED_INT;
                componentNormalized = false;
                attributeStride = 8;
                attributeOffset = 0;
            }
        }
        else if(streamType === 'uvtopology')
        {
            if (stream instanceof Uint16Array) {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 2;
                componentType = gl.UNSIGNED_SHORT;
                componentNormalized = false;
                attributeStride = 4;
                attributeOffset = 0;
            }
            else {
                bufferType = gl.ELEMENT_ARRAY_BUFFER;
                componentCount = 2;
                componentType = gl.UNSIGNED_INT;
                componentNormalized = false;
                attributeStride = 8;
                attributeOffset = 0;
            }
        }

        var webglBuffer = gl.createBuffer();
        gl.bindBuffer(bufferType, webglBuffer); //gl.ELEMENT_ARRAY_BUFFER
        if(dynamic === 'dynamic')
        {
            gl.bufferData(bufferType, stream, gl.DYNAMIC_DRAW); 
        }
        else
        {
            gl.bufferData(bufferType, stream, gl.STATIC_DRAW); //gl.DYNAMIC_DRAW, gl.STREAM_DRAW
        }
        var errorCode = gl.getError();
        if(errorCode === gl.OUT_OF_MEMORY)
        {
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            console.error('GASEngine.WebGLBufferManager.createBuffer: unable to create webgl buffer for not sufficient memory.');
            return null;
        }
        else
        {
            var record = {
                'dynamic': dynamic,
                'webglBuffer': webglBuffer,
                'componentCount': componentCount,
                'componentType': componentType,
                'componentNormalized': componentNormalized,
                'attributeStride': attributeStride,
                'attributeOffset': attributeOffset
            };

            this.webglBufferPool.set(stream, record);

            gl.bindBuffer(bufferType, null);

            return record;
        }
    },

    updateWebglBuffer: function(stream, streamType)
    {
        var record = this.webglBufferPool.get(stream);
        if(record !== undefined)
        {
            if(record.webglBuffer !== null && record.dynamic === 'dynamic')
            {
                var gl = GASEngine.WebGLDevice.Instance.gl;
                if(streamType !== 'index' && streamType !== 'topology' && streamType !== 'uvtopology')
                {
                    gl.bindBuffer(gl.ARRAY_BUFFER, record.webglBuffer);
                    gl.bufferSubData(gl.ARRAY_BUFFER, 0, stream);
                }
                else
                {
                    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, record.webglBuffer);
                    gl.bufferSubData(gl.ELEMENT_ARRAY_BUFFER, 0, stream);
                }
                //<
            }
        }
    },

    getWebglBuffer: function(stream)
    {
        var record = this.webglBufferPool.get(stream);
        return (record === undefined) ? null : record;
    },

    removeWebglBuffer: function(stream) {
        var record = this.webglBufferPool.get(stream);
        if (record !== undefined) {
            this.webglBufferPool.delete(stream);
        } else {
            console.log('GASEngine.WebGLBufferManager.removeWebglBuffer: cannot remove a buffer that is not existent in the pool.');
        }
    }
};