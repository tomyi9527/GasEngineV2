//PBR pipeline
GASEngine.PBRPipeline = function(gl)
{
    GASEngine.RenderPipeline.call(this, gl);

    this.uniformValues = {};
    this.skinningFlag = true;
    this.isShowTopologyObjects = false;
    this.topologyLineColor = new Float32Array([1.0, 1.0, 1.0, 1.0]);
    this.isShowUVTopologyObjects = false;
    this.viewMode = 0;
    this.metalMaterial = null;

    // create a bone texture or an array of floats
    this.maxBoneCount = 1024;

    if(GASEngine.WebGLDevice.Instance.doesSupportVertexTexture())
    {
        this.boneTextureSize = new Int32Array([0]);

        if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
        {
            // layout (1 matrix = 4 pixels)
            //      RGBA RGBA RGBA RGBA (=> column1, column2, column3, column4)
            //  with  8x8  pixel texture max   16 bones * 4 pixels =  (8 * 8)
            //       16x16 pixel texture max   64 bones * 4 pixels = (16 * 16)
            //       32x32 pixel texture max  256 bones * 4 pixels = (32 * 32)
            //       64x64 pixel texture max 1024 bones * 4 pixels = (64 * 64)

            var size = Math.sqrt(this.maxBoneCount * 4); // 4 pixels needed for 1 matrix
            size = GASEngine.nextPOT(Math.ceil(size));
            size = Math.max(size, 4);

            this.boneTextureSize[0] = size;

            this.boneMatrices = new Float32Array(this.boneTextureSize[0] * this.boneTextureSize[0] * 4); // 4 floats per RGBA pixel
        }
        else
        {
            var size = Math.sqrt(this.maxBoneCount * 32); // 32 pixels needed for 1 matrix
            size = GASEngine.nextPOT(Math.ceil(size));
            size = Math.max(size, 32);

            this.boneTextureSize[0] = size;

            this.boneMatrices = new Uint8Array(this.boneTextureSize[0] * this.boneTextureSize[0] * 4);
        }

        this.boneTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.boneTexture);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    }
    else
    {
        this.boneMatrices = new Float32Array(16 * this.maxBoneCount.length);
    }

    this.activeMorphWeights = new Float32Array(4);
    this.activeMorphTargets = [null, null, null, null];

    GASEngine.PBRPipeline.Instance = this;
};

GASEngine.PBRPipeline.prototype = Object.create(GASEngine.RenderPipeline.prototype);
GASEngine.PBRPipeline.prototype.constructor = GASEngine.PBRPipeline;

GASEngine.PBRPipeline.prototype.init = function(w, h, options)
{
    GASEngine.RenderPipeline.prototype.init.call(this, w, h);

    options = options || {};
}

GASEngine.PBRPipeline.prototype.finl = function()
{
    GASEngine.RenderPipeline.prototype.finl.call(this);   
}

GASEngine.PBRPipeline.prototype.createRenderTargets = function(w, h)
{
    w = (w === undefined)? this.renderWidth: w;
    h = (h === undefined)? this.renderHeight : h;

    //call base
    GASEngine.RenderPipeline.prototype.createRenderTargets.call(this, w, h);

    // var gl = this.gl;

    // //RT0
    // this.RT0 = {};
    // this.RT0.width = w;
    // this.RT0.height = h;
    // this.RT0.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT0.width, this.RT0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT0.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT0.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT0.texture, 0);
    // gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.depthRT.depthStencil);    
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //outlineRT
    // this.outlineRT = {};
    // this.outlineRT.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.outlineRT.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT0.texture, 0);
    // gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.depthRT.depthStencil);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1
    // this.RT1 = {};
    // this.RT1.width = w;
    // this.RT1.height = h;
    // this.RT1.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1.width, this.RT1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1.depthStencil = gl.createRenderbuffer();
    // gl.bindRenderbuffer(gl.RENDERBUFFER, this.RT1.depthStencil);
    // gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL/*gl.DEPTH_COMPONENT16*/, w, h);
    // gl.bindRenderbuffer(gl.RENDERBUFFER, null);

    // this.RT1.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1.texture, 0);
    // gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.RT1.depthStencil);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);


    // //RT1_4_0
    // this.RT1_4_0 = {};
    // this.RT1_4_0.width = Math.ceil(w / 4);
    // this.RT1_4_0.height = Math.ceil(h / 4);
    // this.RT1_4_0.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_0.width, this.RT1_4_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_4_0.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_4_0.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_4_0.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1_4_1
    // this.RT1_4_1 = {};
    // this.RT1_4_1.width = Math.ceil(w / 4);
    // this.RT1_4_1.height = Math.ceil(h / 4);
    // this.RT1_4_1.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_1.width, this.RT1_4_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_4_1.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_4_1.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_4_1.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1_4_2
    // this.RT1_4_2 = {};
    // this.RT1_4_2.width = Math.ceil(w / 4);
    // this.RT1_4_2.height = Math.ceil(h / 4);
    // this.RT1_4_2.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_2.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_2.width, this.RT1_4_2.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_4_2.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_4_2.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_4_2.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //For BLUR 8 10
    // //RT1_8_0
    // this.RT1_8_0 = {};
    // this.RT1_8_0.width = Math.ceil(w / 8);
    // this.RT1_8_0.height = Math.ceil(h / 8);
    // this.RT1_8_0.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_0.width, this.RT1_8_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_8_0.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_8_0.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_8_0.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1_8_1
    // this.RT1_8_1 = {};
    // this.RT1_8_1.width = Math.ceil(w / 8);
    // this.RT1_8_1.height = Math.ceil(h / 8);
    // this.RT1_8_1.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_1.width, this.RT1_8_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_8_1.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_8_1.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_8_1.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1_8_2
    // this.RT1_8_2 = {};
    // this.RT1_8_2.width = Math.ceil(w / 8);
    // this.RT1_8_2.height = Math.ceil(h / 8);
    // this.RT1_8_2.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_2.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_2.width, this.RT1_8_2.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_8_2.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_8_2.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_8_2.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //For BLUR 12
    // //RT1_16_0
    // this.RT1_16_0 = {};
    // this.RT1_16_0.width = Math.ceil(w / 16);
    // this.RT1_16_0.height = Math.ceil(h / 16);
    // this.RT1_16_0.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_16_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_16_0.width, this.RT1_16_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_16_0.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_16_0.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_16_0.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // //RT1_16_1
    // this.RT1_16_1 = {};
    // this.RT1_16_1.width = Math.ceil(w / 16);
    // this.RT1_16_1.height = Math.ceil(h / 16);
    // this.RT1_16_1.texture = gl.createTexture();
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_16_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_16_1.width, this.RT1_16_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // this.RT1_16_1.frameBuffer = gl.createFramebuffer();
    // gl.bindFramebuffer(gl.FRAMEBUFFER, this.RT1_16_1.frameBuffer);
    // gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.RT1_16_1.texture, 0);
    // gl.bindFramebuffer(gl.FRAMEBUFFER, null);
};

GASEngine.PBRPipeline.prototype.renderItem_V1 = function(camera, item)
{
    if (item.material)
    {
        item.material.skinning = this.skinningFlag;
        item.material.viewMode = this.viewMode;
    }

    var SP = GASEngine.WebGLShaderManager.Instance.getShaderProgram(item);
    if(SP === null)
        return;

    item.material.updateUniforms(this.uniformValues, camera, item, SP);
    item.material.updateRenderStates(item);
    
    var subMeshes = item.mesh.getStream('subMesh');
    var subMesh = subMeshes[item.subMeshIndex];
    var webglStreamRecords = item.mesh.getWebglRecords();

    if(item.mesh.isMorphed() && this.skinningFlag)
    {
        item.mesh.sortMorphWeights(this.activeMorphWeights, this.activeMorphTargets);
        this.uniformValues['morphTargetInfluences'] = this.activeMorphWeights;

        for(var i = 0; i < this.activeMorphTargets.length; ++i)
        {
            if(this.activeMorphTargets[i] !== null)
            {
                var records = this.activeMorphTargets[i].getWebglRecords();
                webglStreamRecords.set('morphTarget' + i, records.get('position'));
                webglStreamRecords.set('morphNormal' + i, records.get('normal'));
            }
            else
            {
                webglStreamRecords.set('morphTarget' + i, null);
                webglStreamRecords.set('morphNormal' + i, null);
            }
        }
    }

    var drawMode = item.mesh._webglDrawMode;
    var isWireframe = item.material.wireframe;
    var isshowUVLayout = item.material.showUV;
    var drawStart = 0;
    var drawCount = 0;
    if (isWireframe) 
    {
        drawStart = 0;
        drawCount = item.mesh.getStream('topology').length;
        if(isshowUVLayout) drawCount = item.mesh.getStream('uvtopology').length;
    } else if (subMesh) {
        drawStart = subMesh.start;
        drawCount = subMesh.count;
    }
    //TODO: isWireframe变量，主要用于控制drawMode为LINES方式，而不覆盖原来的drawMode.使用时可以修改材质为WireframeMaterial。
    //也可以不修改材质，两者的显示效果是不一样的，rainyyuan
    this.renderMesh(SP, this.uniformValues, webglStreamRecords, drawStart, drawCount, drawMode, isWireframe, isshowUVLayout);
};

GASEngine.PBRPipeline.prototype.renderDepth_V1 = function ()
{
    var depthMaterial = null;
    return function (cameras, cameraCount)
    {
        var opaqueList = cameras[0].getOpaqueList();
        var opaqueListLength = cameras[0].getOpaqueListLength();

        for (var i = 0; i < opaqueListLength; ++i)
        {
            var item = opaqueList[i];

            var oldMaterial = item.material;

            if (depthMaterial === null) {
                depthMaterial = GASEngine.MaterialFactory.Instance.create('depth');
            }
            item.material = depthMaterial;

            this.renderItem_V1(cameras[0], item);

            item.material = oldMaterial;
        }
    };
}();

GASEngine.PBRPipeline.prototype.renderMetal_V1 = function ()
{
    return function (cameras, cameraCount)
    {
        if (this.metalMaterial === null) {
            this.metalMaterial = GASEngine.MaterialFactory.Instance.create('matcap');
            this.metalMaterial.matCapEnable = true;
            this.metalMaterial.matCapMap = GASEngine.MaterialMapFactory.Instance.create();
            GASEngine.Resources.Instance.loadTexture
            (
                '/system/copper.jpg',
                false,
                function(webglTexture, width, height)
                {
                    GASEngine.PBRPipeline.Instance.metalMaterial.matCapMap.webglTexture = webglTexture;
                },
                null,
                null
            );
        }

        var opaqueList = cameras[0].getOpaqueList();
        var opaqueListLength = cameras[0].getOpaqueListLength();

        for (var i = 0; i < opaqueListLength; ++i)
        {
            var item = opaqueList[i];

            var oldMaterial = item.material;
            item.material = this.metalMaterial;

            this.renderItem_V1(cameras[0], item);

            item.material = oldMaterial;
        }

        var skybox = cameras[0].getSkybox();
        this.renderItem_V1(cameras[0], skybox);

        var transparentList = cameras[0].getTransparentList();
        var transparentListLength = cameras[0].getTransparentListLength();
        for(var i = 0; i < transparentListLength; ++i)
        {
            var item = transparentList[i];

            var oldMaterial = item.material;
            item.material = this.metalMaterial;
            
            this.renderItem_V1(cameras[0], item);

            item.material = oldMaterial;
        }
    };
}();

GASEngine.PBRPipeline.prototype.renderPBR_V1 = function(cameras, cameraCount)
{
    var opaqueList = cameras[0].getOpaqueList();
    var opaqueListLength = cameras[0].getOpaqueListLength();    

    for(var i = 0; i < opaqueListLength; ++i)
    {
        var item = opaqueList[i];
        this.renderItem_V1(cameras[0], item);
    }

    var skybox = cameras[0].getSkybox();
    this.renderItem_V1(cameras[0], skybox);

    var transparentList = cameras[0].getTransparentList();
    var transparentListLength = cameras[0].getTransparentListLength();
    for(var i = 0; i < transparentListLength; ++i)
    {
        var item = transparentList[i];
        this.renderItem_V1(cameras[0], item);
    }
};

GASEngine.PBRPipeline.prototype.renderHotspots = function (cameras, cameraCount)
{
    var hotspot = cameras[0].getHotspotItem();
    this.renderItem_V1(cameras[0], hotspot);
};

GASEngine.PBRPipeline.prototype.renderHelpers = function(cameras, cameraCount)
{
    var helperList = cameras[0].getHelperList();
    var helperListLength = cameras[0].getHelperListLength();
    for(var i = 0; i < helperListLength; ++i)
    {
        var item = helperList[i];
        this.renderItem_V1(cameras[0], item);
    }
};

GASEngine.PBRPipeline.prototype.renderWireframeOverlay = (function ()
{
    var wireframeMaterial = null;
    return function(cameras, cameraCount) {
        if (!this.isShowTopologyObjects) {
            return;
        }
        // create wireframe material
        if (!wireframeMaterial) {
            wireframeMaterial = GASEngine.MaterialFactory.Instance.create('wireframe');   
        }
        // enable material alpha
        wireframeMaterial.blending = GASEngine.NormalBlending;
        // setting line rgb color
        wireframeMaterial.setLineColor(
            this.topologyLineColor[0], 
            this.topologyLineColor[1], 
            this.topologyLineColor[2])
        // setting line alpha
        wireframeMaterial.setLineAlpha(this.topologyLineColor[3]); 

        var drawList = cameras[0].getMeshList();
        var drawListLength = cameras[0].getMeshListLength();
        for(var i = 0; i < drawListLength; ++i)
        {
            var item = drawList[i];
            // swap and render
            const oldMaterial = item.material;
            item.material = wireframeMaterial;
            this.renderItem_V1(cameras[0], item);
            item.material = oldMaterial;
        }
    };
})();

GASEngine.PBRPipeline.prototype.renderUVLayout = function (cameras, cameraCount)
{
    var uvlayoutMaterial = null;
    var opaqueList = cameras[0].getOpaqueList();
    var opaqueListLength = cameras[0].getOpaqueListLength();
    for(var i = 0; i < opaqueListLength; ++i)
    {
        var item = opaqueList[i];
        if (this.isShowUVTopologyObjects) {
            if (uvlayoutMaterial === null) uvlayoutMaterial = GASEngine.MaterialFactory.Instance.create('uvlayout');
            const oldMaterial = item.material;
            item.material = uvlayoutMaterial;
            this.renderItem_V1(cameras[0], item);
            item.material = oldMaterial;
        }
    }
    // var uvlayoutMaterial = null;
    // return function (cameras, cameraCount) {
    //     var opaqueList = cameras[0].getOpaqueList();
    //     var opaqueListLength = cameras[0].getOpaqueListLength();
    //     //console.log(opaqueListLength);
    //     //多个mesh 分块显示
    //     var row = 1, col = 1, uvScale = 1.0;
    //     var materials = [];
    //     if (opaqueListLength > 1) {
    //         for (var i = 0; i < opaqueListLength; ++i){
    //             var idx = opaqueList[i].mesh.materialIndex;
    //             if (materials.indexOf(idx) === -1) materials.push(idx);
    //         }
    //     }
    //     if (materials.length > 1) {
    //         row = (materials.length > 2) ? 2 : 1;
    //         col = Math.ceil(materials.length / row);
    //         uvScale = 1.0 / col;
    //     }

    //     for(var i = 0; i < opaqueListLength; ++i)
    //     {
    //         var item = opaqueList[i];
    //         if(item.mesh.isshowUVLayout && !item.mesh.isWireframe) {
    //             if(uvlayoutMaterial === null) {
    //                 uvlayoutMaterial = GASEngine.MaterialFactory.Instance.create('uvlayout');
    //             }
    //             uvlayoutMaterial.uvScale = new Float32Array([uvScale]);
    //             if (col != 1) {
    //                 var index = materials.indexOf(item.mesh.materialIndex);
    //                 var colIndex = index % col;
    //                 if (colIndex === 0)
    //                     uvlayoutMaterial.offset[0] = 0;
    //                 else if (colIndex === col - 1)
    //                     uvlayoutMaterial.offset[0] = 1;
    //                 else
    //                     uvlayoutMaterial.offset[0] = colIndex;
    //                 if (row === 2) uvlayoutMaterial.offset[1] = parseInt(index / col);
    //                 console.log(uvlayoutMaterial.offset);
    //             }
    //             var oldMaterial = item.material;
    //             item.material = uvlayoutMaterial;
    //             item.mesh.isWireframe = true;
    //             this.renderItem_V1(cameras[0], item);
    //             item.mesh.isWireframe = false;
    //             item.material = oldMaterial;
    //         }
    //     }             
    // };
};

GASEngine.PBRPipeline.prototype.viewport = function(x, y, w, h)
{
    this.gl.viewport(x, y, w, h);
};

GASEngine.PBRPipeline.prototype.clearDepth = function(depth)
{
    this.gl.clearDepth(depth);
    this.gl.clear(this.gl.DEPTH_BUFFER_BIT);
};

GASEngine.PBRPipeline.prototype.clearStencil = function(stencil)
{
    this.gl.clearStencil(stencil);
    this.gl.stencilMask(0xff); //
    this.gl.clear(this.gl.STENCIL_BUFFER_BIT);
};

GASEngine.PBRPipeline.prototype.clearDepthAndStencil = function(depth, stencil)
{
    this.gl.clearDepth(depth);
    this.gl.clearStencil(stencil);
    this.gl.stencilMask(0xff); //
    this.gl.clear(this.gl.DEPTH_BUFFER_BIT | this.gl.STENCIL_BUFFER_BIT);
};

GASEngine.PBRPipeline.prototype.clearColor = function(r, g, b, a) //0.0 ~ 1.0
{
    this.gl.clearColor(r, g, b, a);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
};

GASEngine.PBRPipeline.prototype.clearColorDepthAndStencil = function(r, g, b, a, depth, stencil)
{
    this.gl.clearColor(r, g, b, a);
    this.gl.clearDepth(depth);
    this.gl.clearStencil(stencil);
    this.gl.stencilMask(0xff); //
    this.gl.depthMask(true);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT | this.gl.STENCIL_BUFFER_BIT);
};

GASEngine.PBRPipeline.prototype.bindFramebuffer = function(framebuffer) 
{
    this.gl.bindFramebuffer(this.gl.FRAMEBUFFER, framebuffer);
};

GASEngine.PBRPipeline.prototype.deleteFrameBuffer = function(fb)
{
    this.gl.deleteTexture(fb.texture);
    this.gl.deleteRenderbuffer(fb.depth_stencil);
    this.gl.deleteFramebuffer(fb.framebuffer);
    fb = {};
};

GASEngine.PBRPipeline.prototype.scissor = function(x, y, w, h) //x,y are the coordinates of lower left corner
{
    this.gl.enable(this.gl.SCISSOR_TEST);
    //gl.disable(this.gl.SCISSOR_TEST);
    this.gl.scissor(x, y, w, h);
};

GASEngine.PBRPipeline.prototype.setUniforms_r = function(uniforms, values)
{
    var gl = this.gl;
    var arrUniform = Object.keys(uniforms);
    for(var i = 0; i < arrUniform.length; ++i)
    {
        var uniformName = arrUniform[i];
        var uniformConfig = uniforms[uniformName];

        var uniformLocation = uniformConfig.location;
        var uniformType = uniformConfig.type;
        var uniformSize = uniformConfig.size;

        var uniformValue = values[uniformName];
        if(uniformValue)
        {
            if(uniformType === undefined)
            {
                if(uniformConfig instanceof Array)
                {
                    //Array
                    for(var j = 0; j < uniformConfig.length; ++j)
                    {
                        this.setUniforms_r(uniformConfig[j], uniformValue[j]);
                    }
                }
                else
                {
                    //structure
                    this.setUniforms_r(uniformConfig, uniformValue);
                }
            }
            else if(uniformType === 'samplerCube' || uniformType === 'sampler2D')
            {
                var __webglTexture = uniformValue;
                gl.uniform1i(uniformLocation, this.textureSlotIndex);
                gl.activeTexture(gl.TEXTURE0 + this.textureSlotIndex);
                gl.bindTexture(this.TEXTURE_SAMPLER_TYPE[uniformType], __webglTexture);
                ++this.textureSlotIndex;
            }
            else
            {
                if(uniformType === 'mat2' || uniformType === 'mat3' || uniformType === 'mat4')
                {
                    this.UNIFORM_SETTER_TYPE[uniformType].call(gl, uniformLocation, false, uniformValue);
                }
                else
                {
                    this.UNIFORM_SETTER_TYPE[uniformType].call(gl, uniformLocation, uniformValue);
                }
            }
        }
        else
        {
            console.log('Uniform "' + uniformName + '" has not been set correctly!');
            return false;
        }
    }

    return true;
};

GASEngine.PBRPipeline.prototype.render_V1 = function(cameras, cameraCount)
{
    var gl = this.gl;
    
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.depthRT.frameBuffer);

    this.clearColorDepthAndStencil(0.9, 0.9, 0.9, 1.0, 1.0, 0);
    //clear color
    this.renderDepth_V1(cameras, cameraCount);

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    this.clearColorDepthAndStencil(0.8, 0.8, 0.8, 1.0, 1.0, 0);

    if (this.viewMode == ViewMode.Metal) {
        this.renderMetal_V1(cameras, cameraCount);
    } else {
        this.renderPBR_V1(cameras, cameraCount);
    }

    //TODO: 附加线框模式的绘制，内部会临时修改使用WireframeMaterial材质，如果在前面正常绘制的时候已经开启了isWirefame，则跳过，不会绘制两次
    this.renderWireframeOverlay(cameras, cameraCount);

    this.renderUVLayout(cameras, cameraCount);

    this.clearDepth(1.0);
    this.renderHelpers(cameras, cameraCount);
    if(GASEngine.PBRPipeline.Instance.showHotspot)
    {
        this.renderHotspots(cameras, cameraCount);
    }
};

GASEngine.PBRPipeline.prototype.onWindowResize = function(width, height)
{
    this.renderWidth = width;
    this.renderHeight = height;

    GASEngine.RenderPipeline.prototype.onWindowResize.call(this, width, height);

    //this.calcBackgroundImageRatio();

    // var gl = this.gl;
    
    // this.RT0.width = this.renderWidth;
    // this.RT0.height = this.renderHeight;
    // gl.bindTexture(gl.TEXTURE_2D, this.RT0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT0.width, this.RT0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // this.RT1.width = this.renderWidth;
    // this.RT1.height = this.renderHeight;
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1.width, this.RT1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_4_0
    // this.RT1_4_0.width = Math.ceil(this.renderWidth/4);
    // this.RT1_4_0.height = Math.ceil(this.renderHeight / 4);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_0.width, this.RT1_4_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_4_1
    // this.RT1_4_1.width = Math.ceil(this.renderWidth / 4);
    // this.RT1_4_1.height = Math.ceil(this.renderHeight / 4);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_1.width, this.RT1_4_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_4_2
    // this.RT1_4_2.width = Math.ceil(this.renderWidth / 4);
    // this.RT1_4_2.height = Math.ceil(this.renderHeight / 4);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_4_2.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_4_2.width, this.RT1_4_2.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);    

    // //RT1_8_0
    // this.RT1_8_0.width = Math.ceil(this.renderWidth / 8);
    // this.RT1_8_0.height = Math.ceil(this.renderHeight / 8);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_0.width, this.RT1_8_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_8_1
    // this.RT1_8_1.width = Math.ceil(this.renderWidth / 8);
    // this.RT1_8_1.height = Math.ceil(this.renderHeight / 8);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_1.width, this.RT1_8_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_8_2
    // this.RT1_8_2.width = Math.ceil(this.renderWidth / 8);
    // this.RT1_8_2.height = Math.ceil(this.renderHeight / 8);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_8_2.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_8_2.width, this.RT1_8_2.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_16_0
    // this.RT1_16_0.width = Math.ceil(this.renderWidth / 16);
    // this.RT1_16_0.height = Math.ceil(this.renderHeight / 16);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_16_0.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_16_0.width, this.RT1_16_0.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // //RT1_16_1
    // this.RT1_16_1.width = Math.ceil(this.renderWidth / 16);
    // this.RT1_16_1.height = Math.ceil(this.renderHeight / 16);
    // gl.bindTexture(gl.TEXTURE_2D, this.RT1_16_1.texture);
    // gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.RT1_16_1.width, this.RT1_16_1.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    // gl.bindTexture(gl.TEXTURE_2D, null);

    // gl.bindRenderbuffer(gl.RENDERBUFFER, this.RT1.depthStencil);
    // gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL, this.renderWidth, this.renderHeight);
    // gl.bindRenderbuffer(gl.RENDERBUFFER, null);
};