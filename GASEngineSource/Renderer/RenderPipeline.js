GASEngine.RenderPipeline = function(gl)
{
    this.gl = gl;
    this.TEXTURE_SAMPLER_TYPE = { 'sampler2D': gl.TEXTURE_2D, 'samplerCube': gl.TEXTURE_CUBE_MAP };

    /*    
    uniform1f   'float'
    uniform1fv
    uniform1i   'int samplerCube sampler3D sampler2D sampler1D sampler1DShadow sampler2DShadow'
    uniform1iv
    uniform2f   'vec2'
    uniform2fv  
    uniform2i   'vec2i'
    uniform2iv
    uniform3f   'vec3'
    uniform3fv
    uniform3i   'vec3i'
    uniform3iv
    uniform4f   'vec4'
    uniform4fv
    uniform4i   'vec4i'
    uniform4iv
    uniformMatrix2fv  'mat2'
    uniformMatrix3fv  'mat3'
    uniformMatrix4fv  'mat4'
    */
    this.UNIFORM_SETTER_TYPE = 
    {
        'float': gl.uniform1fv,
        'vec2': gl.uniform2fv,
        'vec3': gl.uniform3fv,
        'vec4': gl.uniform4fv,

        'sampler2D': gl.uniform1i,
        'samplerCube': gl.uniform1i,

        'int': gl.uniform1iv,
        'vec2i': gl.uniform2iv,
        'vec3i': gl.uniform3iv,
        'vec4i': gl.uniform4iv,

        'mat2': gl.uniformMatrix2fv,
        'mat3': gl.uniformMatrix3fv,
        'mat4': gl.uniformMatrix4fv
    };    
};

GASEngine.RenderPipeline.prototype =
{
    constructor: GASEngine.RenderPipeline,
};

GASEngine.RenderPipeline.prototype.init = function(w, h)
{
    this.renderWidth = w;
    this.renderHeight = h;
    var HW = this.renderWidth / 2;

    this.scissorLeft = new GASEngine.Vector4(0, 0, HW, this.renderHeight);
    this.scissorRight = new GASEngine.Vector4(HW, 0, HW, this.renderHeight);

    this.viewportLeft = new GASEngine.Vector4(0, 0, HW, this.renderHeight);
    this.viewportRight = new GASEngine.Vector4(HW, 0, HW, this.renderHeight);

    this.scissorEntire = new GASEngine.Vector4(0, 0, this.renderWidth, this.renderHeight);
    this.viewportEntire = new GASEngine.Vector4(0, 0, this.renderWidth, this.renderHeight);

    this.createRenderTargets(w, h);
};

GASEngine.RenderPipeline.prototype.finl = function()
{

};

GASEngine.RenderPipeline.prototype.createRenderTargets = function (w, h)
{
    w = (w === undefined)? this.renderWidth: w;
    h = (h === undefined)? this.renderHeight : h;
    
    var gl = this.gl;
    //depthRT
    this.depthRT = {};
    this.depthRT.width = w;
    this.depthRT.height = h;
    this.depthRT.texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.depthRT.texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.depthRT.width, this.depthRT.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.bindTexture(gl.TEXTURE_2D, null);

    this.depthRT.depthStencil = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.depthRT.depthStencil);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL/*gl.DEPTH_COMPONENT16*/, w, h);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);

    this.depthRT.frameBuffer = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.depthRT.frameBuffer);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.depthRT.texture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.depthRT.depthStencil);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
};

GASEngine.RenderPipeline.prototype.renderMesh = function(SP, uniformValues, streams, drawStart, drawCount, drawMode, isWireframe, isshowUVLayout)
{
    //renderBufferDirect is the core function of renderer
    if(!SP)
    {
        return;
    }

    var gl = this.gl;
    gl.useProgram(SP.program);

    var hasBoneMatrix = false;
    this.textureSlotIndex = 0;
    var result = this.setUniforms_r(SP.uniforms, uniformValues);
    if(!result)
        return;

    var arrAttributes = Object.keys(SP.attributes);
    for(var i = 0; i < arrAttributes.length; ++i)
    {
        var attrName = arrAttributes[i];
        var spAttribute = SP.attributes[attrName];
        var streamConf = streams.get(attrName);
        if(streamConf)
        {
            gl.enableVertexAttribArray(spAttribute.location);

            gl.bindBuffer(gl.ARRAY_BUFFER, streamConf.webglBuffer);
 
            gl.vertexAttribPointer(
                spAttribute.location,
                streamConf.componentCount,
                streamConf.componentType,
                streamConf.componentNormalized,
                streamConf.attributeStride,
                streamConf.attributeOffset);
        }
        else
        {
            gl.disableVertexAttribArray(spAttribute.location);
            //if(spAttribute.type === 'vec2')
            //{
            //    gl.vertexAttrib2fv(spAttribute.location, new Float32Array(2));
            //}
            //else if(spAttribute.type === 'vec3')
            //{
            //    gl.vertexAttrib3fv(spAttribute.location, new Float32Array(3));
            //}
            //else if(spAttribute.type === 'vec4')
            //{
            //    gl.vertexAttrib4fv(spAttribute.location, new Float32Array(4));
            //}
            //else
            //{
            //    gl.vertexAttrib1fv(spAttribute.location, new Float32Array(1));
            //}
        }
    }

    var donotHaveIndices = false;
    var indexType;
    var indexOffset;

    var record;
    if (isWireframe) {
        if(isshowUVLayout) record = streams.get('uvtopology');
        else record = streams.get('topology');
        if(record === undefined) { //TODO: 变量用于控制drawMode为LINES.如果没有topology数据，则还用原来的drawMode绘制,rainyyuan
            isWireframe = false;
        }
    }
    else {
        record = streams.get('index');
    }
    
    if(record !== undefined)
    {
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, record.webglBuffer);
        indexType = record.componentType;
        var size = (indexType === gl.UNSIGNED_INT) ? 4 : 2;
        indexOffset = drawStart * size;
    }
    else
    {
        donotHaveIndices = true;
    }
    
    GASEngine.WebGLRenderStates.Instance.applyRenderStates();

    if(isWireframe) {
        drawMode = gl.LINES;
    }

    if(donotHaveIndices)
    {
        gl.drawArrays(drawMode, drawStart, drawCount);
    }
    else
    {
        gl.drawElements(drawMode, drawCount, indexType, indexOffset); //TODO: UNSIGNED_SHORT
    }
   
    //TODO: restore render states
    for(var i = 0; i < this.textureSlotIndex; ++i)
    {
        gl.activeTexture(gl.TEXTURE0 + i);
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, null);
    }
};

GASEngine.RenderPipeline.prototype.onWindowResize = function(width, height)
{
    if(width <= 0 || height <= 0)
    {
        console.error('frame buffer size error');
        return;
    }
    
    this.renderWidth = width;
    this.renderHeight = height;

    /////////////////////////////////////
    var gl = this.gl;

    this.depthRT.width = this.renderWidth;
    this.depthRT.height = this.renderHeight;
    gl.bindTexture(gl.TEXTURE_2D, this.depthRT.texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.depthRT.width, this.depthRT.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    //Depth Stencil Buffers
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.depthRT.depthStencil);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL, this.renderWidth, this.renderHeight);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);

    var HW = this.renderWidth / 2;
    this.scissorLeft.set(0, 0, HW, this.renderHeight);
    this.viewportLeft.set(0, 0, HW, this.renderHeight);

    this.scissorRight.set(HW, 0, HW, this.renderHeight);
    this.viewportRight.set(HW, 0, HW, this.renderHeight);

    this.scissorEntire.set(0, 0, this.renderWidth, this.renderHeight);
    this.viewportEntire.set(0, 0, this.renderWidth, this.renderHeight);
    
    gl.viewport(0,0, this.renderWidth, this.renderHeight);
};