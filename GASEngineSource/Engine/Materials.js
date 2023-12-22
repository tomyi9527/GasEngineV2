//Author: tomyi
//Date: 2017-06-21

GASEngine.Material = function()
{
    this.name = null;
    this.uniqueID = -1;
    this.visible = true;
    this.mutable = false;
    //0:eCullingOff	Renders both the inner and outer polygons for the selected model.
    //1:eCullingOnCCW (Counter-Clockwise)	Renders only the polygons that compose the outside of the model.
    //2:eCullingOnCW (Clockwise)	Renders only the polygons that compose the inside of the selected model.
    this.culling = 0;

    this.skinning = true;
    this.highlight = false;
    this.depthBias = false;
    this.vertexColor = true;

    this.highlightMask = false;
    this.highlightMaskColor = new Float32Array([0.8, 0.8, 0.5, 0.0]);
    // GASEngine.NoBlending = 0;
    // GASEngine.NormalBlending = 1;
    // GASEngine.AdditiveBlending = 2;
    // GASEngine.SubtractiveBlending = 3;
    // GASEngine.MultiplyBlending = 4;
    // GASEngine.CustomBlending = 5;
    this.blending = 0;
    this.depthTest = true;

    this.wireframe = false;
    this.showUV = false;

    this.viewMode = 0;

    this.vertexShaderElements = [];
    this.fragmentShaderElements = [];
};

GASEngine.Material.CullingOff = 0;//CullNone
GASEngine.Material.CullingOnCCW = 1; //CullBack
GASEngine.Material.CullingOnCW = 2; //CullFront
GASEngine.Material.CullingOnBoth = 3; //CullAll


GASEngine.Material.prototype =
{
    constructor: GASEngine.Material,

    getCulling: function() {
        return this.culling;
    },
    setCulling: function(value) {
        this.culling = value;
    },

    generateVertexShaderKey: function (item)
    {
        var shaderKey = 0;

        if(!item.mesh.getStream('position'))
        {
            console.error('GASEngine.DielectricMaterial.generateVertexShaderKey: A mesh must contain a position stream for rendering.');
            return -1;
        }

        if(item.mesh.getStream('normal'))
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.NORMAL);

        if(item.mesh.getStream('tangent'))
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.TANGENT);

        if(item.mesh.getStream('uv'))
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.UV0);

        if(item.mesh.getStream('uv2'))
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.UV1);

        if(item.mesh.getStream('color') && this.vertexColor)
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.COLOR0);

        if(this.skinning)
        {
            var boneList = item.mesh.getStream('bone');
            if(boneList)
            {
                if(boneList.length === item.mesh.bones.length)
                {
                    if(item.mesh.getStream('skinWeight') && item.mesh.getStream('skinIndex'))
                    {        
                        shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.SKINNING);
                    }
                }
            }
        }

        if(item.mesh.getMorphTargetCount() > 0)
        {
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.MORPHPOSITION);

            var target0 = item.mesh.getMorphTarget(0);
            if (item.mesh.getStream('normal') && target0.getStream('normal'))
            {
                shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.MORPHNORMAL);
            }
        }

        if(this.depthBias)
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.DEPTHBIAS);

        if(GASEngine.WebGLDevice.Instance.doesSupportVertexTexture())
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.VERTEXTEXTURE);

        if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.FLOATTEXTURE);

        if(this.viewMode == 2)
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.WORLDNORMAL);

        //displacement
        if(this.displacementEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.DISPLACEMENT);

        if(this.displacementMap != null && this.displacementMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.VSMacros.DISPLACEMENTMAP);

        return shaderKey;
    },

    updateRenderStates: function ()
    {
        GASEngine.WebGLRenderStates.Instance.setFrontFace(0); //CCW is front face
        switch(this.culling)
        {
            case GASEngine.Material.CullingOff:
            {
                GASEngine.WebGLRenderStates.Instance.setCullingFaceEnable(0);
                break;
            }
            case GASEngine.Material.CullingOnCCW:
            {
                GASEngine.WebGLRenderStates.Instance.setCullingFaceEnable(1);
                GASEngine.WebGLRenderStates.Instance.setCullingFace(0); //Cull back
                break;
            }
            case GASEngine.Material.CullingOnCW:
            {
                GASEngine.WebGLRenderStates.Instance.setCullingFaceEnable(1);
                GASEngine.WebGLRenderStates.Instance.setCullingFace(1); //Cull front
                break;
            }
            case GASEngine.Material.CullingOnBoth:
            {
                GASEngine.WebGLRenderStates.Instance.setCullingFaceEnable(1);
                GASEngine.WebGLRenderStates.Instance.setCullingFace(3); //Cull both
                break;
            }
            default:
                break;
        }

        //blend mode
        if(this.blending === GASEngine.NoBlending) 
        {
            GASEngine.WebGLRenderStates.Instance.setAlphaBlendEnable(0);
            GASEngine.WebGLRenderStates.Instance.setAlphaBlendMode(0);
        }
        else 
        {
            GASEngine.WebGLRenderStates.Instance.setAlphaBlendEnable(1);
            GASEngine.WebGLRenderStates.Instance.setAlphaBlendMode(1);
        }
        GASEngine.WebGLRenderStates.Instance.setColorWriteMask(0xF);

        //depth test
        GASEngine.WebGLRenderStates.Instance.setDepthTestEnable(this.depthTest? 1 : 0);
        GASEngine.WebGLRenderStates.Instance.setDepthTestMode(0);
        GASEngine.WebGLRenderStates.Instance.setDepthWriteEnable(1);

        //stencil test
        GASEngine.WebGLRenderStates.Instance.setStencilTestEnable(1);
        GASEngine.WebGLRenderStates.Instance.setStencilOp(0);

        var stencilRef = 1;
        if(this.highlight) 
        {
            stencilRef += 2;
            GASEngine.WebGLRenderStates.Instance.setStencilOp(1);
        }
        GASEngine.WebGLRenderStates.Instance.setStencilMode(stencilRef);
        GASEngine.WebGLRenderStates.Instance.setStencilMask(0xff);
    },

    getLinkedTextureMaps: function(list)
    {
        if (this.albedoMap !== null && this.albedoMap.texture !== '')
        {
            list.push(this.albedoMap);
        }

        if (this.displacementMap !== null && this.displacementMap.texture !== '')
        {
            list.push(this.displacementMap);
        }

        if (this.normalMap !== null && this.normalMap.texture !== '')
        {
            list.push(this.normalMap);
        }

        if (this.transparencyMap !== null && this.transparencyMap.texture !== '')
        {
            list.push(this.transparencyMap);
        }

        if (this.emissiveMap !== null && this.emissiveMap.texture !== '')
        {
            list.push(this.emissiveMap);
        }
    }
};

// additional attribute
GASEngine.Material.prototype.getHighlightMaskEnable = function() { return this.highlightMask; }
GASEngine.Material.prototype.setHighlightMaskEnable = function(value) { this.highlightMask = value; }
GASEngine.Material.prototype.getHighlightMaskColor = function() { return [this.highlightMaskColor[0], this.highlightMaskColor[1], this.highlightMaskColor[2]]; }
GASEngine.Material.prototype.setHighlightMaskColor = function(r, g, b) {
    this.highlightMaskColor[0] = r;
    this.highlightMaskColor[1] = g;
    this.highlightMaskColor[2] = b;
}
GASEngine.Material.prototype.getHighlightMaskAlpha = function() { 
    return this.highlightMaskColor[3];
}
GASEngine.Material.prototype.setHighlightMaskAlpha = function(value) { 
    this.highlightMaskColor[3] = value;
}


//BlinnPhongMaterial
GASEngine.BlinnPhongMaterial = function()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/BlinnPhongFragment.glsl';

    this.reflectiveRatio = 0.5;

    //Albedo
    this.albedoEnable = true;
    this.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
    this.albedoMap = null;
    this.albedoColor = new Float32Array([1.0, 1.0, 1.0]);
    this.albedoFactor = new Float32Array([1.0]);

    //Specular
    this.specularEnable = true;
    this.specularDefault = new Float32Array([1.0, 1.0, 1.0]);
    this.specularMap = null;
    this.specularColor = new Float32Array([1.0, 1.0, 1.0]);
    this.specularFactor = new Float32Array([0.0]);

    //Glossiness
    this.glossinessEnable = true;
    this.glossinessDefault = new Float32Array([1.0]);
    this.glossinessMap = null;
    this.glossinessChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.glossinessFactor = new Float32Array([0.1]);

    //Displacement
    this.displacementEnable = false;
    this.displacementDefault = new Float32Array([0.5]);
    this.displacementMap = null;
    this.displacementChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.displacementFactor = new Float32Array([0.5]);

    //Normal
    this.normalEnable = false;
    this.normalDefault = new Float32Array([0.500000,0.500000,1.000000]);
    this.normalMap = null;
    this.normalFactor = new Float32Array([1.0]);
    this.normalFlipY = new Float32Array([0.0]);

    //Transparency
    this.transparencyEnable = false;
    this.transparencyDefault = new Float32Array([1.0,1.0,1.0,1.0]);
    this.transparencyMap = null;
    this.transparencyChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.transparencyFactor = new Float32Array([1.0]);
    this.transparencyAlphaInvert = new Int32Array([0]);
    this.transparencyBlendMode = 0;

    //Emissive
    this.emissiveEnable = false;
    this.emissiveDefault = new Float32Array([1.0, 1.0, 1.0]);
    this.emissiveMap = null;
    this.emissiveColor = new Float32Array([0.0, 0.0, 0.0]);
    this.emissiveFactor = new Float32Array([0.0]);
    this.emissiveMultiplicative = new Float32Array([0.0]);

     //Light
     this.lightEnable = false;
     this.lightDefault = new Float32Array([1.0,1.0,1.0]);
     this.lightMap = null;
     this.lightColor = new Float32Array([1.0, 1.0, 1.0]);
     this.lightFactor = new Float32Array([1.0]);
};

GASEngine.BlinnPhongMaterial.FSMacros = {
    'ALBEDO': 0,
    'ALBEDOMAP': 1,
    'SPECULAR': 2,
    'SPECULARMAP': 3,
    'GLOSSINES': 4,
    'GLOSSINESMAP': 5,
    'DISPLACEMENT': 6,
    'DISPLACEMENTMAP': 7, 
    'NORMAL': 8,
    'NORMALMAP': 9,
    
    'TRANSPARENCY': 10,
    'TRANSPARENCYMAP': 11,
    'EMISSIVE': 12,
    'EMISSIVEMAP': 13,

    'DIRECTIONALLIGHTING': 14,
    'SPOTLIGHTING': 15,
    'POINTLIGHTING': 16,
    'ENVIRONMENTALLIGHTING': 17,
    'CUBEMAP': 18,
    'PANORAMA': 19
};

GASEngine.BlinnPhongMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.BlinnPhongMaterial.prototype.constructor = GASEngine.BlinnPhongMaterial;
GASEngine.BlinnPhongMaterial.prototype.typeName = 'blinnPhong';
GASEngine.BlinnPhongMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.BlinnPhongMaterial.prototype.typeName);

// TODO:saralu
GASEngine.BlinnPhongMaterial.prototype.generateFragmentShaderKey = function(item)
{
    var shaderKey = 0;
    //albedo
    if(this.albedoEnable)
    shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.ALBEDO);

    if(this.albedoMap != null && this.albedoMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.ALBEDOMAP);

    //specular
    if(this.specularEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.SPECULAR);

    if(this.specularMap != null && this.specularMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.SPECULARMAP);

    //glossiness
    if(this.glossinessEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.GLOSSINES);

    if(this.glossinessMap != null && this.glossinessMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.GLOSSINESMAP);

    //displacement
    if(this.displacementEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.DISPLACEMENT);

    if(this.displacementMap != null && this.displacementMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.DISPLACEMENTMAP);

    //normal
    if(this.normalEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.NORMAL);

    if(this.normalMap != null && this.normalMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.NORMALMAP);

    //transparency
    if(this.transparencyEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.TRANSPARENCY);

    if(this.transparencyMap != null && this.transparencyMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.TRANSPARENCYMAP);

    //emissvie
    if(this.emissiveEnable)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.EMISSIVE);

    if(this.emissiveMap != null && this.emissiveMap.webglTexture)
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.EMISSIVEMAP);

    //Directional lighting
    if(item.directionalLight)
    {
        if(item.directionalLight.position !== null &&
            item.directionalLight.color !== null &&
            item.directionalLight.intensity !== null &&
            item.directionalLight.direction !== null)
        {
            shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.DIRECTIONALLIGHTING);
        }
    }

    if(item.pointLight)
    {
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.POINTLIGHTING);
    }
    if(item.spotLight)
    {
        shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.SPOTLIGHTING);
    }

      //environmental lighting
      if(GASEngine.WebGLDevice.Instance.doesSupportTextureLOD())
      {
          if(item.envirnonmentalLight.specularCubeMap !== null &&
              item.envirnonmentalLight.sph !== null &&
              item.envirnonmentalLight.specularIntegratedBRDF !== null)
          {
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.ENVIRONMENTALLIGHTING);
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.CUBEMAP);
          }
          else if(item.envirnonmentalLight.specularPanorama !== null &&
              item.envirnonmentalLight.sph !== null &&
              item.envirnonmentalLight.specularIntegratedBRDF !== null)
          {
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.ENVIRONMENTALLIGHTING);
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.PANORAMA);
          }
      }
      else
      {
          if(item.envirnonmentalLight.specularPanorama !== null &&
              item.envirnonmentalLight.sph !== null &&
              item.envirnonmentalLight.specularIntegratedBRDF !== null)
          {
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.ENVIRONMENTALLIGHTING);
              shaderKey |= (1 << GASEngine.BlinnPhongMaterial.FSMacros.PANORAMA);
          }
      }

    return shaderKey;
}

GASEngine.BlinnPhongMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();
    var environmentMatrix = new GASEngine.Matrix4();
    var rotationMatrix = new GASEngine.Matrix4();

    return function(uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        //<
        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.ALBEDO))
        {
            uniformValues['albedoColor'] = item.material.albedoColor;
            uniformValues['albedoFactor'] = item.material.albedoFactor;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.ALBEDOMAP))
            {
                uniformValues['albedoMap'] = item.material.albedoMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.SPECULAR))
        {

            uniformValues['specularFactor'] = item.material.specularFactor;
            uniformValues['specularColor'] = item.material.specularColor;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.SPECULARMAP))
            {
                uniformValues['specularMap'] = item.material.specularMap.webglTexture;
                uniformValues['specularChannel'] = item.material.specularMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.GLOSSINES))
        {
            uniformValues['glossinessFactor'] = item.material.glossinessFactor;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.GLOSSINESMAP))
            {
                uniformValues['glossinessMap'] = item.material.glossinessMap.webglTexture;
                uniformValues['glossinessChannel'] = item.material.glossinessMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.DISPLACEMENT))
        {
            uniformValues['displacementMapFactor'] = item.material.displacementMapFactor;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.DISPLACEMENTMAP))
            {
                uniformValues['displacementMap'] = item.material.displacementMap.webglTexture;
                uniformValues['displacementChannel'] = item.material.displacementMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.NORMAL))
        {
            uniformValues['normalFactor'] = item.material.normalFactor;
            uniformValues['normalFlipY'] = item.material.normalFlipY;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.NORMALMAP))
            {
                uniformValues['normalMap'] = item.material.normalMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.TRANSPARENCY))
        {
            //TODO:
            this.blending = GASEngine.NormalBlending;

            uniformValues['transparencyFactor'] = item.material.transparencyFactor;
            uniformValues['transparencyAlphaInvert'] = item.material.transparencyAlphaInvert;
 
            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.TRANSPARENCYMAP))
            {
                uniformValues['transparencyMap'] = item.material.transparencyMap.webglTexture;
                uniformValues['transparencyChannel'] = item.material.transparencyMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.EMISSIVE))
        {
            uniformValues['emissiveColor'] = item.material.emissiveColor;
            uniformValues['emissiveFactor'] = item.material.emissiveFactor;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.EMISSIVEMAP))
            {
                uniformValues['emissiveMap'] = item.material.emissiveMap.webglTexture;
            }
        }
        //<

        if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.ENVIRONMENTALLIGHTING))
        {
            uniformValues['sph'] = item.envirnonmentalLight.sph;
            uniformValues['specularIntegratedBRDF'] = item.envirnonmentalLight.specularIntegratedBRDF;
            uniformValues['environmentExposure'] = item.envirnonmentalLight.environmentExposure;

            rotationMatrix.makeRotationY(-item.envirnonmentalLight.orientation * Math.PI / 180.0);
            //rotationMatrix.makeRotationY(-threeJSScene.__skybox.backgroundConf.orientation * Math.PI / 180.0);
            environmentMatrix.multiplyMatrices(matrixView, rotationMatrix);
            uniformValues['environmentMatrix'] = environmentMatrix.elements;

            if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.CUBEMAP))
            {
                uniformValues['specularCubeMap'] = item.envirnonmentalLight.specularCubeMap;
                uniformValues['specularCubeMapLODRange'] = item.envirnonmentalLight.specularCubeMapLODRange;
                uniformValues['specularCubeMapSize'] = item.envirnonmentalLight.specularCubeMapSize;
            }
            else if(SP.fsKey & (1 << GASEngine.BlinnPhongMaterial.FSMacros.PANORAMA))
            {
                uniformValues['specularPanorama'] = item.envirnonmentalLight.specularPanorama;
                uniformValues['specularPanoramaLODRange'] = item.envirnonmentalLight.specularPanoramaLODRange;
                uniformValues['specularPanoramaSize'] = item.envirnonmentalLight.specularPanoramaSize;
            }
        }

        //Directional lighting
        if(item.directionalLight)
        {
            if(item.directionalLight.position !== null &&
                item.directionalLight.color !== null &&
                item.directionalLight.intensity !== null &&
                item.directionalLight.direction !== null)
            {
                var directionalLight = {};
                //directionalLight.position = new Float32Array(item.directionalLight.position);
                directionalLight.color = new Float32Array(item.directionalLight.color);
                //directionalLight.intensity = new Float32Array([item.directionalLight.intensity]);
                directionalLight.ambientIntensity = new Float32Array([item.directionalLight.ambientIntensity]);
                directionalLight.diffuseIntensity = new Float32Array([item.directionalLight.diffuseIntensity]);
                directionalLight.specularPower = new Float32Array([item.directionalLight.specularPower]);
                directionalLight.specularIntensity = new Float32Array([item.directionalLight.specularIntensity]);
                directionalLight.direction = new Float32Array(item.directionalLight.direction);
                directionalLight.shininess = new Float32Array([item.directionalLight.shininess]);
                uniformValues['directionalLight'] = directionalLight;
                //uniform['length'] =
                //.isOn =
                //性能
                //max light count
            }
        }

        if(item.pointLight)
        {
            if(item.pointLight.position !== null && 
                item.pointLight.color !== null &&
                item.pointLight.intensity !== null &&
                item.pointLight.decay !== null)
            {
                var pointLight = {};
                pointLight.position = new Float32Array(item.pointLight.position);
                pointLight.color = new Float32Array(item.pointLight.color);
                // pointLight.intensity = new Float32Array([item.pointLight.intensity]);
                pointLight.ambientIntensity = new Float32Array([item.pointLight.ambientIntensity]);
                pointLight.diffuseIntensity = new Float32Array([item.pointLight.diffuseIntensity]);
                pointLight.specularPower = new Float32Array([item.pointLight.specularPower]);
                pointLight.specularIntensity = new Float32Array([item.pointLight.specularIntensity]);
                // pointLight.decay = new Float32Array([item.pointLight.decay]);
                pointLight.constant = new Float32Array([item.pointLight.constant]);
                pointLight.linear = new Float32Array([item.pointLight.linear]);
                pointLight.exp = new Float32Array([item.pointLight.exp]);
                pointLight.shininess = new Float32Array([item.pointLight.shininess]);
                uniformValues['pointLight'] = pointLight;
            } 
        }

        if(item.spotLight)
        {
            if(item.spotLight.position !== null && 
                item.spotLight.color !== null &&
                item.spotLight.intensity !== null &&
                item.spotLight.direction !== null &&
                item.spotLight.angle !== null &&
                item.spotLight.distanceDecay !== null &&
                item.spotLight.angleDecay !== null)
            {
                var spotLight = {};
                spotLight.position = new Float32Array(item.spotLight.position);
                spotLight.color = new Float32Array(item.spotLight.color);
                spotLight.ambientIntensity = new Float32Array([item.spotLight.ambientIntensity]);
                spotLight.diffuseIntensity = new Float32Array([item.spotLight.diffuseIntensity]);
                spotLight.specularPower = new Float32Array([item.spotLight.specularPower]);
                spotLight.specularIntensity = new Float32Array([item.spotLight.specularIntensity]);  
                // spotLight.intensity = new Float32Array([item.spotLight.intensity]);
                spotLight.direction = new Float32Array(item.spotLight.direction);
                spotLight.angle = new Float32Array([item.spotLight.angle]);
                spotLight.constant = new Float32Array([item.spotLight.constant]);
                spotLight.linear = new Float32Array([item.spotLight.linear]);
                spotLight.exp = new Float32Array([item.spotLight.exp]);
                spotLight.shininess = new Float32Array([item.spotLight.shininess]);
                uniformValues['spotLight'] = spotLight;
            } 
        }

    }
})();

GASEngine.BlinnPhongMaterial.prototype.updateRenderStates = function()
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};


//WireframeMaterial
GASEngine.WireframeMaterial = function ()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/WireframeFragment.glsl';

    this.wireframe = true;
    this.depthBias = true;
    this.lineColor = new Float32Array([1.0, 1.0, 1.0, 1.0]);
};

GASEngine.WireframeMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.WireframeMaterial.prototype.constructor = GASEngine.WireframeMaterial;
GASEngine.WireframeMaterial.prototype.typeName = 'wireframe';
GASEngine.WireframeMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.WireframeMaterial.prototype.typeName);

GASEngine.WireframeMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.WireframeMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];

        uniformValues['lineColor'] = this.lineColor;
        if(item.mesh.isSkinned() &&
        GASEngine.PBRPipeline.Instance &&
        GASEngine.PBRPipeline.Instance.boneTexture &&
        GASEngine.PBRPipeline.Instance.boneMatrices)
        {
            var boneTexture = GASEngine.PBRPipeline.Instance.boneTexture;
            var boneTextureSize = GASEngine.PBRPipeline.Instance.boneTextureSize;
            var boneMatrices = GASEngine.PBRPipeline.Instance.boneMatrices;            

            item.mesh.computeSkinningMatrices(boneMatrices);

            gl.bindTexture(gl.TEXTURE_2D, boneTexture);
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

            if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.FLOAT, boneMatrices);
            }
            else
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.UNSIGNED_BYTE, boneMatrices);
            }

            uniformValues['boneTexture'] = boneTexture;
            uniformValues['boneTextureSize'] = boneTextureSize;
        }
    }
})();

GASEngine.WireframeMaterial.prototype.updateRenderStates = function(item)
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

GASEngine.WireframeMaterial.prototype.setLineColor = function(r, g, b)
{
    this.lineColor[0] = r;
    this.lineColor[1] = g;
    this.lineColor[2] = b;
};

GASEngine.WireframeMaterial.prototype.setLineAlpha = function(alpha)
{
    this.lineColor[3] = alpha;
};

//DielectricMaterial//
GASEngine.DielectricMaterial = function()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/PBRFragment.glsl';

    //Albedo
    this.albedoEnable = true;
    this.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
    this.albedoMap = null;
    this.albedoColor = new Float32Array([1.0, 1.0, 1.0]);
    this.albedoFactor = new Float32Array([1.0]);

    //***************************************************************************
    //For dielectric
    //Metalness
    this.metalnessEnable = false;
    this.metalnessDefault = new Float32Array([1.0]);
    this.metalnessMap = null;
    this.metalnessChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.metalnessFactor = new Float32Array([1.0]);

    //SpecularF0
    this.specularF0Enable = false;
    this.specularF0Default = new Float32Array([1.0]);
    this.specularF0Map = null;
    this.specularF0Channel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.specularF0Factor = new Float32Array([1.0]);

    //For electric
    //Specular
    this.specularEnable = false;
    this.specularDefault = new Float32Array([1.0, 1.0, 1.0]);
    this.specularMap = null;
    this.specularColor = new Float32Array([1.0, 1.0, 1.0]);
    this.specularFactor = new Float32Array([1.0]);
    //***************************************************************************

    //Roughness
    this.roughnessEnable = false;
    this.roughnessDefault = new Float32Array([1.0]);
    this.roughnessMap = null;
    this.roughnessChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.roughnessFactor = new Float32Array([1.0]);
    this.roughnessInvert = new Int32Array([0]);

    //Displacement
    this.displacementEnable = false;
    this.displacementDefault = new Float32Array([0.5]);
    this.displacementMap = null;
    this.displacementChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.displacementFactor = new Float32Array([0.5]);

    //Normal
    this.normalEnable = false;
    this.normalDefault = new Float32Array([0.5,0.5,1.0]);
    this.normalMap = null;
    this.normalFactor = new Float32Array([1.0]);
    this.normalFlipY = new Int32Array([1]);

    //AO
    this.aoEnable = false;
    this.aoDefault = new Float32Array([1.0]);
    this.aoMap = null;
    this.aoChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.aoFactor = new Float32Array([1.0]);
    this.aoOccludeSpecular = new Int32Array([1]);

    //Cavity
    this.cavityEnable = false;
    this.cavityDefault = new Float32Array([1.0]);
    this.cavityMap = null;
    this.cavityChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.cavityFactor = new Float32Array([1.0]);

    //Transparency
    this.transparencyEnable = false;
    this.transparencyDefault = new Float32Array([1.0]);
    this.transparencyMap = null;
    this.transparencyChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.transparencyFactor = new Float32Array([1.0]);
    this.transparencyAlphaInvert = new Int32Array([0]);
    this.transparencyBlendMode = 0;

    //Emissive
    this.emissiveEnable = false;
    this.emissiveDefault = new Float32Array([0.0, 0.0, 0.0],);
    this.emissiveMap = null;
    this.emissiveColor = new Float32Array([0.0, 0.0, 0.0]);
    this.emissiveFactor = new Float32Array([0.0]);
};

GASEngine.DielectricMaterial.VSMacros = {
    'POSITION': 0,
    'NORMAL': 1,
    'TANGENT': 2,
    'UV0': 3,
    'UV1': 4,
    'COLOR0': 5,
    'COLOR1': 6,
    'SKINNING': 7,
    'MORPHPOSITION': 8,
    'MORPHNORMAL': 9,
    'DEPTHBIAS': 10,
    'VERTEXTEXTURE': 11,
    'FLOATTEXTURE': 12,
    'WORLDNORMAL': 13,
    'DISPLACEMENT': 14,
    'DISPLACEMENTMAP': 15
};

GASEngine.DielectricMaterial.FSMacros = {
    'ALBEDO': 0,
    'ALBEDOMAP': 1,
    'METALNESS': 2,
    'METALNESSMAP': 3,
    'SPECULARF0': 4,
    'SPECULARF0MAP': 5,
    'SPECULAR': 6,
    'SPECULARMAP': 7,
    'ROUGHNESS': 8,
    'ROUGHNESSMAP': 9,
    'NORMAL': 10,
    'NORMALMAP': 11,
    'AO': 12,
    'AOMAP': 13,
    'CAVITY': 14,
    'CAVITYMAP': 15,
    'TRANSPARENCY': 16,
    'TRANSPARENCYMAP': 17,
    'EMISSIVE': 18,
    'EMISSIVEMAP': 19,
    'DIELECTRIC': 20,
    'ELECTRIC': 21,
    'ENVIRONMENTALLIGHTING': 22,
    'PUNCTUALLIGHTING': 23,
    'CUBEMAP': 24,
    'PANORAMA': 25,
    'MOBILE': 26,
    'OUTPUTALBEDO': 27,
    'OUTPUTNORMALS': 28,
    'OUTPUTLIT': 29,
    'HIGHLIGHTMASK': 30
};

GASEngine.DielectricMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.DielectricMaterial.prototype.constructor = GASEngine.DielectricMaterial;
GASEngine.DielectricMaterial.prototype.typeName = 'dielectric';
GASEngine.DielectricMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.DielectricMaterial.prototype.typeName);

GASEngine.DielectricMaterial.prototype.getLinkedTextureMaps = function(list)
{
    if(this.albedoMap !== null && this.albedoMap.texture !== '')
    {
        list.push(this.albedoMap);
    }

    if(this instanceof GASEngine.ElectricMaterial)
    {
        if(this.specularMap !== null && this.specularMap.texture !== '')
        {
            list.push(this.specularMap);
        }
    }
    else
    {
        if(this.metalnessMap !== null && this.metalnessMap.texture !== '')
        {
            list.push(this.metalnessMap);
        }

        if(this.specularF0Map !== null && this.specularF0Map.texture !== '')
        {
            list.push(this.specularF0Map);
        }
    }    

    if(this.roughnessMap !== null && this.roughnessMap.texture !== '')
    {
        list.push(this.roughnessMap);
    }

    if(this.displacementMap !== null && this.displacementMap.texture !== '')
    {
        list.push(this.displacementMap);
    }

    if(this.normalMap !== null && this.normalMap.texture !== '')
    {
        list.push(this.normalMap);
    }

    if(this.aoMap !== null && this.aoMap.texture !== '')
    {
        list.push(this.aoMap);
    }

    if(this.cavityMap !== null && this.cavityMap.texture !== '')
    {
        list.push(this.cavityMap);
    }

    if(this.transparencyMap !== null && this.transparencyMap.texture !== '')
    {
        list.push(this.transparencyMap);
    }

    if(this.emissiveMap !== null && this.emissiveMap.texture !== '')
    {
        list.push(this.emissiveMap);
    }
};

GASEngine.DielectricMaterial.prototype.generateFragmentShaderKey = (function ()
{
    return function (item)
    {
        var shaderKey = 0;

        if(item.material instanceof GASEngine.ElectricMaterial)
        {
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ELECTRIC);
        }
        else
        {
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.DIELECTRIC);
        }

        //environmental lighting
        if(item.envirnonmentalLight) {
            if(GASEngine.WebGLDevice.Instance.doesSupportTextureLOD())
            {
                if(item.envirnonmentalLight.specularCubeMap !== null &&
                    item.envirnonmentalLight.sph !== null &&
                    item.envirnonmentalLight.specularIntegratedBRDF !== null)
                {
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ENVIRONMENTALLIGHTING);
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.CUBEMAP);
                }
                else if(item.envirnonmentalLight.specularPanorama !== null &&
                    item.envirnonmentalLight.sph !== null &&
                    item.envirnonmentalLight.specularIntegratedBRDF !== null)
                {
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ENVIRONMENTALLIGHTING);
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.PANORAMA);
                }
            }
            else
            {
                if(item.envirnonmentalLight.specularPanorama !== null &&
                    item.envirnonmentalLight.sph !== null &&
                    item.envirnonmentalLight.specularIntegratedBRDF !== null)
                {
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ENVIRONMENTALLIGHTING);
                    shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.PANORAMA);
                }
            }
        }

        //albedo
        if(this.albedoEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ALBEDO);

        if(this.albedoMap != null && this.albedoMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ALBEDOMAP);

        //metalness
        if(this.metalnessEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.METALNESS);

        if(this.metalnessMap != null && this.metalnessMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.METALNESSMAP);

        //specularF0
        if(this.specularF0Enable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARF0);

        if(this.specularF0Map != null && this.specularF0Map.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARF0MAP);

        //specular for electric
        if(this.specularEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.SPECULAR);

        if(this.specularMap != null && this.specularMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARMAP);

        //roughness
        if(this.roughnessEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ROUGHNESS);

        if(this.roughnessMap != null && this.roughnessMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.ROUGHNESSMAP);

        //normal
        if(this.normalEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.NORMAL);

        if(this.normalMap != null && this.normalMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.NORMALMAP);

        //ao
        if(this.aoEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.AO);

        if(this.aoMap != null && this.aoMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.AOMAP);

        //cavity
        if(this.cavityEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.CAVITY);

        if(this.cavityMap != null && this.cavityMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.CAVITYMAP);

        //transparency
        if(this.transparencyEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.TRANSPARENCY);

        if(this.transparencyMap != null && this.transparencyMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.TRANSPARENCYMAP);

        //emissvie
        if(this.emissiveEnable)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.EMISSIVE);

        if(this.emissiveMap != null && this.emissiveMap.webglTexture)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.EMISSIVEMAP);

        //viewmode
        if (this.viewMode == 1)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.OUTPUTALBEDO);  
        
        if (this.viewMode == 2)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.OUTPUTNORMALS);  
        
        if (this.viewMode == 3) 
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.OUTPUTLIT);
            
        if (this.highlightMask)
            shaderKey |= (1 << GASEngine.DielectricMaterial.FSMacros.HIGHLIGHTMASK);
        
        return shaderKey;
    };
})();

GASEngine.DielectricMaterial.prototype.updateRenderStates = function()
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

GASEngine.DielectricMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();
    var environmentMatrix = new GASEngine.Matrix4();
    var rotationMatrix = new GASEngine.Matrix4();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();
        var matrixViewProjection = camera.getViewProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];

        //
        if(item.mesh.isSkinned() &&
            GASEngine.PBRPipeline.Instance &&
            GASEngine.PBRPipeline.Instance.boneTexture &&
            GASEngine.PBRPipeline.Instance.boneMatrices)
        {
            var boneTexture = GASEngine.PBRPipeline.Instance.boneTexture;
            var boneTextureSize = GASEngine.PBRPipeline.Instance.boneTextureSize;
            var boneMatrices = GASEngine.PBRPipeline.Instance.boneMatrices;            

            item.mesh.computeSkinningMatrices(boneMatrices);

            gl.bindTexture(gl.TEXTURE_2D, boneTexture);
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

            if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.FLOAT, boneMatrices);
            }
            else
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.UNSIGNED_BYTE, boneMatrices);
            }

            uniformValues['boneTexture'] = boneTexture;
            uniformValues['boneTextureSize'] = boneTextureSize;
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.ENVIRONMENTALLIGHTING))
        {
            uniformValues['sph'] = item.envirnonmentalLight.sph;
            uniformValues['specularIntegratedBRDF'] = item.envirnonmentalLight.specularIntegratedBRDF;
            uniformValues['environmentExposure'] = item.envirnonmentalLight.environmentExposure;

            rotationMatrix.makeRotationY(-item.envirnonmentalLight.orientation * Math.PI / 180.0);
            //rotationMatrix.makeRotationY(-threeJSScene.__skybox.backgroundConf.orientation * Math.PI / 180.0);
            environmentMatrix.multiplyMatrices(matrixView, rotationMatrix);
            uniformValues['environmentMatrix'] = environmentMatrix.elements;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.CUBEMAP))
            {
                uniformValues['specularCubeMap'] = item.envirnonmentalLight.specularCubeMap;
                uniformValues['specularCubeMapLODRange'] = item.envirnonmentalLight.specularCubeMapLODRange;
                uniformValues['specularCubeMapSize'] = item.envirnonmentalLight.specularCubeMapSize;
            }
            else if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.PANORAMA))
            {
                uniformValues['specularPanorama'] = item.envirnonmentalLight.specularPanorama;
                uniformValues['specularPanoramaLODRange'] = item.envirnonmentalLight.specularPanoramaLODRange;
                uniformValues['specularPanoramaSize'] = item.envirnonmentalLight.specularPanoramaSize;
            }
        }

        //<
        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.ALBEDO))
        {
            uniformValues['albedoColor'] = item.material.albedoColor;
            uniformValues['albedoFactor'] = item.material.albedoFactor;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.ALBEDOMAP))
            {
                uniformValues['albedoMap'] = item.material.albedoMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.SPECULAR))
        {
            uniformValues['specularFactor'] = item.material.specularFactor;
            uniformValues['specularColor'] = item.material.specularColor;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARMAP))
            {
                uniformValues['specularMap'] = item.material.specularMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.METALNESS))
        {
            uniformValues['metalnessFactor'] = item.material.metalnessFactor;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.METALNESSMAP))
            {
                uniformValues['metalnessMap'] = item.material.metalnessMap.webglTexture;
                uniformValues['metalnessChannel'] = item.material.metalnessMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARF0))
        {
            uniformValues['specularF0Factor'] = item.material.specularF0Factor;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.SPECULARF0MAP))
            {
                uniformValues['specularF0Map'] = item.material.specularF0Map.webglTexture;
                uniformValues['specularF0Channel'] = item.material.specularF0Map.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.ROUGHNESS))
        {
            uniformValues['roughnessFactor'] = item.material.roughnessFactor;
            uniformValues['roughnessInvert'] = item.material.roughnessInvert;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.ROUGHNESSMAP))
            {
                uniformValues['roughnessMap'] = item.material.roughnessMap.webglTexture;
                uniformValues['roughnessChannel'] = item.material.roughnessMap.pixelChannels;
            }
        }

        if(SP.vsKey & (1 << GASEngine.DielectricMaterial.VSMacros.DISPLACEMENT))
        {
            uniformValues['displacementMapFactor'] = item.material.displacementMapFactor;

            if(SP.vsKey & (1 << GASEngine.DielectricMaterial.VSMacros.DISPLACEMENTMAP))
            {
                uniformValues['displacementMap'] = item.material.displacementMap.webglTexture;
                uniformValues['displacementChannel'] = item.material.displacementMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.NORMAL))
        {
            uniformValues['normalFactor'] = item.material.normalFactor;
            uniformValues['normalFlipY'] = item.material.normalFlipY;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.NORMALMAP))
            {
                uniformValues['normalMap'] = item.material.normalMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.AO))
        {
            uniformValues['aoFactor'] = item.material.aoFactor;
            uniformValues['aoOccludeSpecular'] = item.material.aoOccludeSpecular;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.AOMAP))
            {
                uniformValues['aoMap'] = item.material.aoMap.webglTexture;
                uniformValues['aoChannel'] = item.material.aoMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.CAVITY))
        {
            uniformValues['cavityFactor'] = item.material.cavityFactor;
 
            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.CAVITYMAP))
            {
                uniformValues['cavityMap'] = item.material.cavityMap.webglTexture;
                uniformValues['cavityChannel'] = item.material.cavityMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.TRANSPARENCY))
        {
            //TODO:
            this.blending = GASEngine.NormalBlending;
            
            uniformValues['transparencyFactor'] = item.material.transparencyFactor;
            uniformValues['transparencyAlphaInvert'] = item.material.transparencyAlphaInvert;
 
            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.TRANSPARENCYMAP))
            {
                uniformValues['transparencyMap'] = item.material.transparencyMap.webglTexture;
                uniformValues['transparencyChannel'] = item.material.transparencyMap.pixelChannels;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.EMISSIVE))
        {
            uniformValues['emissiveColor'] = item.material.emissiveColor;
            uniformValues['emissiveFactor'] = item.material.emissiveFactor;

            if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.EMISSIVEMAP))
            {
                uniformValues['emissiveMap'] = item.material.emissiveMap.webglTexture;
            }
        }

        if(SP.fsKey & (1 << GASEngine.DielectricMaterial.FSMacros.HIGHLIGHTMASK))
        {
            uniformValues['highlightMaskColor'] = item.material.highlightMaskColor;
        }
        //<
    };
})();

// TODO(beanpliu): getxxxMapImage 接口需要弃用
GASEngine.DielectricMaterial.prototype.getAlbedoEnable = function() { return this.albedoEnable; }
GASEngine.DielectricMaterial.prototype.getAlbedoMapPath = function() { if (this.albedoMap) return this.albedoMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getAlbedoMapImage = function() { if (this.albedoMap) return this.albedoMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getAlbedoColor = function() { return [this.albedoColor[0], this.albedoColor[1], this.albedoColor[2]]; }
GASEngine.DielectricMaterial.prototype.getAlbedoFactor = function() { return this.albedoFactor[0]; }
GASEngine.DielectricMaterial.prototype.getSpecularEnable = function() { return null; } // see electric
GASEngine.DielectricMaterial.prototype.getSpecularColor = function() { return null; }
GASEngine.DielectricMaterial.prototype.getSpecularFactor = function() { return null; }
GASEngine.DielectricMaterial.prototype.getSpecularMapPath = function() { return null; }
GASEngine.DielectricMaterial.prototype.getSpecularMapImage = function() { return null; }
GASEngine.DielectricMaterial.prototype.getMetalnessEnable = function() { return this.metalnessEnable; }
GASEngine.DielectricMaterial.prototype.getMetalnessFactor = function() { return this.metalnessFactor[0]; }
GASEngine.DielectricMaterial.prototype.getMetalnessMapPath = function() { if (this.metalnessMap) return this.metalnessMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getMetalnessMapImage = function() { if (this.metalnessMap) return this.metalnessMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getSpecularF0Enable = function() { return this.specularF0Enable; }
GASEngine.DielectricMaterial.prototype.getSpecularF0Factor = function() { return this.specularF0Factor[0]; }
GASEngine.DielectricMaterial.prototype.getSpecularF0MapPath = function() { if (this.specularF0Map) return this.specularF0Map.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getSpecularF0MapImage = function() { if (this.specularF0Map) return this.specularF0Map.image; else return null; }
GASEngine.DielectricMaterial.prototype.getRoughnessEnable = function() { return this.roughnessEnable && this.roughnessInvert[0] === 0; }
GASEngine.DielectricMaterial.prototype.getRoughnessFactor = function() { return this.roughnessFactor[0]; }
GASEngine.DielectricMaterial.prototype.getRoughnessMapPath = function() { if (this.roughnessMap) return this.roughnessMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getRoughnessMapImage = function() { if (this.roughnessMap) return this.roughnessMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getGlossinessEnable = function() { return this.roughnessEnable && this.roughnessInvert[0] === 1; }
GASEngine.DielectricMaterial.prototype.getGlossinessFactor = function() { return this.roughnessFactor[0]; }
GASEngine.DielectricMaterial.prototype.getGlossinessMapPath = function() { if (this.roughnessMap) return this.roughnessMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getGlossinessMapImage = function() { if (this.roughnessMap) return this.roughnessMap.image; else return null; }

GASEngine.DielectricMaterial.prototype.getNormalEnable = function() { return this.normalEnable; }
GASEngine.DielectricMaterial.prototype.getNormalFactor = function() { return this.normalFactor[0]; }
GASEngine.DielectricMaterial.prototype.getNormalFlipY = function() { return !!this.normalFlipY[0]; }
GASEngine.DielectricMaterial.prototype.getNormalMapPath = function() { if (this.normalMap) return this.normalMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getNormalMapImage = function() { if (this.normalMap) return this.normalMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getAoEnable = function() { return this.aoEnable; }
GASEngine.DielectricMaterial.prototype.getAoFactor = function() { return this.aoFactor[0]; }
GASEngine.DielectricMaterial.prototype.getAoMapPath = function() { if (this.aoMap) return this.aoMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getAoMapImage = function() { if (this.aoMap) return this.aoMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getAoOccludeSpecular = function() { return !!this.aoOccludeSpecular[0]; }
GASEngine.DielectricMaterial.prototype.getCavityEnable = function() { return this.cavityEnable; }
GASEngine.DielectricMaterial.prototype.getCavityFactor = function() { return this.cavityFactor[0]; }
GASEngine.DielectricMaterial.prototype.getCavityMapPath = function() { if (this.cavityMap) return this.cavityMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getCavityMapImage = function() { if (this.cavityMap) return this.cavityMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getOpacityEnable = function() { return this.transparencyEnable; }
GASEngine.DielectricMaterial.prototype.getOpacityFactor = function() { return this.transparencyFactor[0]; }
GASEngine.DielectricMaterial.prototype.getOpacityAlphaInvert = function() { return !this.transparencyAlphaInvert[0]; }
GASEngine.DielectricMaterial.prototype.getOpacityMapPath = function() { if (this.transparencyMap) return this.transparencyMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getOpacityMapImage = function() { if (this.transparencyMap) return this.transparencyMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.getEmissiveEnable = function() { return this.emissiveEnable; }
GASEngine.DielectricMaterial.prototype.getEmissiveColor = function() { return [this.emissiveColor[0], this.emissiveColor[1], this.emissiveColor[2]]; }
GASEngine.DielectricMaterial.prototype.getEmissiveFactor = function() { return this.emissiveFactor[0]; }
GASEngine.DielectricMaterial.prototype.getEmissiveMapPath = function() { if (this.emissiveMap) return this.emissiveMap.texture; else return null; }
GASEngine.DielectricMaterial.prototype.getEmissiveMapImage = function() { if (this.emissiveMap) return this.emissiveMap.image; else return null; }
GASEngine.DielectricMaterial.prototype.setAlbedoEnable = function(value) {
    this.albedoEnable = value;
}
GASEngine.DielectricMaterial.prototype.setAlbedoColor = function(r, g, b) {
    this.albedoColor[0] = r;
    this.albedoColor[1] = g;
    this.albedoColor[2] = b;
}
GASEngine.DielectricMaterial.prototype.setAlbedoFactor = function(value) {
    this.albedoFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setSpecularEnable = function(value) {
    console.log('dielectric not supported field: specularEnable');
}
GASEngine.DielectricMaterial.prototype.setSpecularColor = function(r, g, b) {
    console.log('dielectric not supported field: specularColor');
}
GASEngine.DielectricMaterial.prototype.setSpecularFactor = function(value) {
    console.log('dielectric not supported field: specularFactor');
}
GASEngine.DielectricMaterial.prototype.setMetalnessEnable = function(value) {
    this.metalnessEnable = value;
}
GASEngine.DielectricMaterial.prototype.setMetalnessFactor = function(value) {
    this.metalnessFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setSpecularF0Enable = function(value) {
    this.specularF0Enable = value;
}
GASEngine.DielectricMaterial.prototype.setSpecularF0Factor = function(value) {
    this.specularF0Factor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setRoughnessEnable = function(value) {
    this.roughnessEnable = value;
    if (value) {
        this.roughnessInvert[0] = 0;
    }
}
GASEngine.DielectricMaterial.prototype.setRoughnessFactor = function(value) {
    this.roughnessFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setGlossinessEnable = function(value) {
    this.roughnessEnable = value;
    if (value) {
        this.roughnessInvert[0] = 1;
    }
}
GASEngine.DielectricMaterial.prototype.setGlossinessFactor = function(value) {
    this.roughnessFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setNormalEnable = function(value) {
    this.normalEnable = value;
}
GASEngine.DielectricMaterial.prototype.setNormalFactor = function(value) {
    this.normalFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setNormalFlipY = function(value) {
    this.normalFlipY[0] = value;
}
GASEngine.DielectricMaterial.prototype.setAoEnable = function(value) {
    this.aoEnable = value;
}
GASEngine.DielectricMaterial.prototype.setAoFactor = function(value) {
    this.aoFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setAoOccludeSpecular = function(value) {
    this.aoOccludeSpecular[0] = value;
}
GASEngine.DielectricMaterial.prototype.setCavityEnable = function(value) {
    this.cavityEnable = value;
}
GASEngine.DielectricMaterial.prototype.setCavityFactor = function(value) {
    this.cavityFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setOpacityEnable = function(value) {
    this.transparencyEnable = value;
}
GASEngine.DielectricMaterial.prototype.setOpacityFactor = function(value) {
    this.transparencyFactor[0] = value;
}
GASEngine.DielectricMaterial.prototype.setOpacityAlphaInvert = function(value) {
    this.transparencyAlphaInvert[0] = !value;
}
GASEngine.DielectricMaterial.prototype.setEmissiveEnable = function(value) {
    this.emissiveEnable = value;
}
GASEngine.DielectricMaterial.prototype.setEmissiveColor = function(r, g, b) {
    this.emissiveColor[0] = r;
    this.emissiveColor[1] = g;
    this.emissiveColor[2] = b;
}
GASEngine.DielectricMaterial.prototype.setEmissiveFactor = function(value) {
    this.emissiveFactor[0] = value;
}


//ElectricMaterial
GASEngine.ElectricMaterial = function()
{
    GASEngine.DielectricMaterial.call(this);
};

GASEngine.ElectricMaterial.prototype = Object.create(GASEngine.DielectricMaterial.prototype);
GASEngine.ElectricMaterial.prototype.constructor = GASEngine.ElectricMaterial;
GASEngine.ElectricMaterial.prototype.typeName = 'electric';
GASEngine.ElectricMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.ElectricMaterial.prototype.typeName);

GASEngine.ElectricMaterial.prototype.getSpecularEnable = function() { return this.specularEnable; }
GASEngine.ElectricMaterial.prototype.getSpecularColor = function() { return [this.specularColor[0], this.specularColor[1], this.specularColor[2]]; }
GASEngine.ElectricMaterial.prototype.getSpecularFactor = function() { return this.specularFactor[0]; }
GASEngine.ElectricMaterial.prototype.getSpecularMapPath = function() { if (this.specularMap) return this.specularMap.texture; else return null; }
GASEngine.ElectricMaterial.prototype.getSpecularMapImage = function() { if (this.specularMap) return this.specularMap.image; else return null; }
GASEngine.ElectricMaterial.prototype.getMetalnessEnable = function() { return null; }
GASEngine.ElectricMaterial.prototype.getMetalnessMapPath = function() { return null; }
GASEngine.ElectricMaterial.prototype.getMetalnessMapImage = function() { return null; }
GASEngine.ElectricMaterial.prototype.getMetalnessFactor = function() { return null; }
GASEngine.ElectricMaterial.prototype.getSpecularF0Enable = function() { return null; }
GASEngine.ElectricMaterial.prototype.getSpecularF0Factor = function() { return null; }
GASEngine.ElectricMaterial.prototype.getSpecularF0MapPath = function() { return null; }
GASEngine.ElectricMaterial.prototype.getSpecularF0MapImage = function() { return null; }
GASEngine.ElectricMaterial.prototype.setSpecularEnable = function(value) {
    this.specularEnable = value;
}
GASEngine.ElectricMaterial.prototype.setSpecularColor = function(r, g, b) {
    this.specularColor[0] = r;
    this.specularColor[1] = g;
    this.specularColor[2] = b;
}
GASEngine.ElectricMaterial.prototype.setSpecularFactor = function(value) {
    this.specularFactor[0] = value;
}
GASEngine.ElectricMaterial.prototype.setMetalnessEnable = function(value) {
    console.log('electric not supported field: metalnessEnable');
}
GASEngine.ElectricMaterial.prototype.setMetalnessFactor = function(value) {
    console.log('electric not supported field: metalnessFactor');
}
GASEngine.ElectricMaterial.prototype.setSpecularF0Enable = function(value) {
    console.log('electric not supported field: specularF0Enable');
}
GASEngine.ElectricMaterial.prototype.setSpecularF0Factor = function(value) {
    console.log('electric not supported field: specularF0Factor');
}

//MatCapMaterial
GASEngine.MatCapMaterial = function()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/MatCapFragment.glsl';

    //MatCapMap
    this.matCapEnable = false;
    this.matCapMap = null;
    this.matCapColor = new Float32Array([1.0, 1.0, 1.0]);
    this.matCapCurvature = new Float32Array([0.0]);

    //DisplacementMap
    this.displacementEnable = false;
    this.displacementDefault = new Float32Array([0.5]);
    this.displacementMap = null;
    this.displacementChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.displacementFactor = new Float32Array([0.5]);

    //NormalMap
    this.normalEnable = false;
    this.normalDefault = new Float32Array([0.500000,0.500000,1.000000]);
    this.normalMap = null;
    this.normalFactor = new Float32Array([1.0]);
    this.normalFlipY = new Int32Array([0]);

    //TransparencyMap
    this.transparencyEnable = false;
    this.transparencyDefault = new Float32Array([1.0,1.0,1.0,1.0]);
    this.transparencyMap = null;
    this.transparencyChannel = new Float32Array([1.0, 0.0, 0.0, 0.0]);
    this.transparencyFactor = new Float32Array([1.0]);
    this.transparencyAlphaInvert = new Int32Array([0]);
    this.transparencyBlendMode = 0;

    //TODO: THIS MUST BE MOVE TO RENDERER
    this.outputLinear = new Int32Array([0]);
    this.rgbmRange = new Float32Array([0.0]);
};

GASEngine.MatCapMaterial.FSMacros =
{
    'MATCAP': 0,
    'MATCAPMAP': 1,
    'NORMAL': 2,
    'NORMALMAP': 3,
    'TRANSPARENCY': 4,
    'TRANSPARENCYMAP': 5,
    'DISPLACEMENT': 6,
    'DISPLACEMENTMAP': 7
};

GASEngine.MatCapMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.MatCapMaterial.prototype.constructor = GASEngine.MatCapMaterial;
GASEngine.MatCapMaterial.prototype.typeName = 'matcap';

GASEngine.MatCapMaterial.prototype.generateFragmentShaderKey = (function ()
{
    return function (item)
    {
        var shaderKey = 0;
        //matcap
        if(this.matCapEnable)
            shaderKey |= (1 << GASEngine.MatCapMaterial.FSMacros.MATCAP);

        if(this.matCapMap != null && this.matCapMap.webglTexture)
            shaderKey |= (1 << GASEngine.MatCapMaterial.FSMacros.MATCAPMAP);

        return shaderKey;
    };
})();

GASEngine.MatCapMaterial.prototype.updateUniforms = (function ()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();
        var matrixViewProjection = camera.getViewProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        //
        if(item.mesh.isSkinned() &&
            GASEngine.PBRPipeline.Instance &&
            GASEngine.PBRPipeline.Instance.boneTexture &&
            GASEngine.PBRPipeline.Instance.boneMatrices)
        {
            var boneTexture = GASEngine.PBRPipeline.Instance.boneTexture;
            var boneTextureSize = GASEngine.PBRPipeline.Instance.boneTextureSize;
            var boneMatrices = GASEngine.PBRPipeline.Instance.boneMatrices;            

            item.mesh.computeSkinningMatrices(boneMatrices);

            gl.bindTexture(gl.TEXTURE_2D, boneTexture);
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

            if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.FLOAT, boneMatrices);
            }
            else
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.UNSIGNED_BYTE, boneMatrices);
            }

            uniformValues['boneTexture'] = boneTexture;
            uniformValues['boneTextureSize'] = boneTextureSize;
        }

        if(SP.fsKey & (1 << GASEngine.MatCapMaterial.FSMacros.MATCAP))
        {
            uniformValues['matCapColor'] = item.material.matCapColor;
            uniformValues['matCapCurvature'] = item.material.matCapCurvature;

            if(SP.fsKey & (1 << GASEngine.MatCapMaterial.FSMacros.MATCAPMAP))
            {
                uniformValues['matCapMap'] = item.material.matCapMap.webglTexture;
            }
        }

        uniformValues['outputLinear'] = this.outputLinear;
        uniformValues['rgbmRange'] = this.rgbmRange;
    };
})();

//Compound Material//
GASEngine.CompoundMaterial = function()
{
    GASEngine.Material.call(this);

    this.materials = new Map();
    this.activeMaterial = null;
};

GASEngine.CompoundMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.CompoundMaterial.prototype.constructor = GASEngine.CompoundMaterial;
GASEngine.CompoundMaterial.prototype.typeName = 'compound';

GASEngine.CompoundMaterial.prototype.addMaterial = function(material)
{
    this.materials.set(material.typeName, material);
}

GASEngine.CompoundMaterial.prototype.getActiveMaterial = function()
{
    return this.activeMaterial;
};

GASEngine.CompoundMaterial.prototype.setActiveMaterial = function(typeName)
{
    var material = this.materials.get(typeName);
    if(material !== undefined)
    {
        this.activeMaterial = material;

        if (this.deferedMaps) {
            var materialMaps = this.deferedMaps[typeName];
            if (materialMaps) {
                //Submit texture load tasks.
                for(var i = 0; i < materialMaps.length; ++i)
                {
                    if(materialMaps[i].texture.length > 0 && !materialMaps[i].webglTexture)
                    {
                        GASEngine.Resources.Instance.loadTextureOnMaterial(materialMaps[i]);
                    }
                }
                delete this.deferedMaps[typeName];
            }
        }
    }
    else
    {
        this.activeMaterial = null;
        console.error('GASEngine.CompoundMaterial.setActive: Failed to set active material. The specified type is not in the record.');
    }
};

GASEngine.CompoundMaterial.prototype.getLinkedTextureMaps = function(list)
{
    this.activeMaterial.getLinkedTextureMaps(list);
};

//SkyboxMaterial//
GASEngine.SkyboxMaterial = function()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/SkyboxVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/SkyboxFragment.glsl';

    this.cubeMap = null;
    this.cubeMapSize = new Float32Array([1.0]);
    this.lightExposure = new Float32Array([1.0]);
    this.backgroundExposure = new Float32Array([1.0]);
    this.orientation = new Float32Array([0.0]);

    this.image = null;
    this.imageWidth = 0;
    this.imageHeight = 0;

    this.solidColor = new Float32Array([0.0, 0.0, 0.0]);

    this.sph = null;

    this.backgroundType = 'SOLIDCOLOR';

    this.backgroundName = '';
    this.cubeMapName = '';
    this.environmentBlur = 0; //0,1,2,3
};

GASEngine.SkyboxMaterial.FSMacros =
{
    'SOLIDCOLOR': 0,
    'IMAGE': 1,
    'CUBEMAP': 2,
    'AMBIENT': 3
};

GASEngine.SkyboxMaterial.CUBEMAPINFO =
[
    {
        name: 'background_cubemap_512_0.02_luv.bin',
        size: 512
    },
    {
        name: 'background_cubemap_256_0.055_luv.bin',
        size: 256
    },
    {
        name: 'background_cubemap_128_0.1_luv.bin',
        size: 128
    },
    {
        name: 'background_cubemap_64_0.15_luv.bin',
        size: 64
    }
];

GASEngine.SkyboxMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.SkyboxMaterial.prototype.constructor = GASEngine.SkyboxMaterial;
GASEngine.SkyboxMaterial.prototype.typeName = 'skybox';
GASEngine.SkyboxMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.SkyboxMaterial.prototype.typeName);

GASEngine.SkyboxMaterial.prototype.generateVertexShaderKey = (function ()
{
    return function (item)
    {
        return 0;        
    };
})();

GASEngine.SkyboxMaterial.prototype.generateFragmentShaderKey = (function ()
{
    return function (item)
    {
        var shaderKey = 0;

        if(this.backgroundType === 'CUBEMAP' && this.cubeMap !== null)
        {
            shaderKey |= (1 << GASEngine.SkyboxMaterial.FSMacros.CUBEMAP);
        }
        else if(this.backgroundType === 'IMAGE' && this.image !== null && this.imageWidth > 0 && this.imageHeight > 0)
        {
            shaderKey |= (1 << GASEngine.SkyboxMaterial.FSMacros.IMAGE);
        }
        else if(this.backgroundType === 'AMBIENT' && this.sph !== null)
        {
            shaderKey |= (1 << GASEngine.SkyboxMaterial.FSMacros.AMBIENT);
        }
        else
        {
            shaderKey |= (1 << GASEngine.SkyboxMaterial.FSMacros.SOLIDCOLOR);
        }

        return shaderKey;
    };
})();

GASEngine.SkyboxMaterial.prototype.updateUniforms = (function ()
{
    var cameraFrustum = new Float32Array(4);
    var cameraRight = new Float32Array(3);
    var cameraUp = new Float32Array(3);
    var cameraFront = new Float32Array(3);
    var imageRatio = new Float32Array(2);

    return function (uniformValues, camera, item, SP)
    {
        if(SP.fsKey & (1 << GASEngine.SkyboxMaterial.FSMacros.CUBEMAP) ||
            SP.fsKey & (1 << GASEngine.SkyboxMaterial.FSMacros.AMBIENT))
        {
            var top = Math.tan(GASEngine.degToRad(camera.fov * 0.5)) * camera.near;
            var right = camera.aspect * top;
            var near = camera.near;
            var far = camera.far;

            cameraFrustum[0] = right;
            cameraFrustum[1] = top;
            cameraFrustum[2] = near;
            cameraFrustum[3] = far;

            var cameraMatrixWorld = camera.getWorldMatrix().elements;

            cameraRight[0] = cameraMatrixWorld[0];
            cameraRight[1] = cameraMatrixWorld[1];
            cameraRight[2] = cameraMatrixWorld[2];

            cameraUp[0] = cameraMatrixWorld[4];
            cameraUp[1] = cameraMatrixWorld[5];
            cameraUp[2] = cameraMatrixWorld[6];

            cameraFront[0] = cameraMatrixWorld[8];
            cameraFront[1] = cameraMatrixWorld[9];
            cameraFront[2] = cameraMatrixWorld[10];

            uniformValues['cameraFrustum'] = cameraFrustum;
            uniformValues['cameraRight'] = cameraRight;
            uniformValues['cameraUp'] = cameraUp;
            uniformValues['cameraFront'] = cameraFront;

            uniformValues['lightExposure'] = this.lightExposure;
            uniformValues['backgroundExposure'] = this.backgroundExposure;
            uniformValues['orientation'] = this.orientation;
        }

        if(SP.fsKey & (1 << GASEngine.SkyboxMaterial.FSMacros.CUBEMAP))
        {
            uniformValues['cubeMap'] = this.cubeMap;
            uniformValues['cubeMapSize'] = this.cubeMapSize;
        }
        else if(SP.fsKey & (1 << GASEngine.SkyboxMaterial.FSMacros.IMAGE))
        {
            uniformValues['image'] = item.material.image;
            //var renderTargetWidth = GASEngine.Renderer.Instance.VRFlag ? (GASEngine.Renderer.Instance.canvasWidth / 2.0) : GASEngine.Renderer.Instance.canvasWidth;
            //var renderTargetHeight = GASEngine.Renderer.Instance.canvasHeight;
            var renderTargetWidth = GASEngine.WebGLDevice.Instance.getCanvasWidth();
            var renderTargetHeight = GASEngine.WebGLDevice.Instance.getCanvasHeight();
            this.updateImageRatio(imageRatio, renderTargetWidth, renderTargetHeight);

            uniformValues['imageRatio'] = imageRatio;
        }
        else if(SP.fsKey & (1 << GASEngine.SkyboxMaterial.FSMacros.AMBIENT))
        {
            uniformValues['sph'] = this.sph;
        }
        else
        {
            uniformValues['solidColor'] = this.solidColor;
        }
    };
})();

GASEngine.SkyboxMaterial.prototype.updateRenderStates = function()
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

GASEngine.SkyboxMaterial.prototype.setCubeMap = function(texture, size)
{
    this.cubeMap = texture;
    this.cubeMapSize[0] = size;
};

GASEngine.SkyboxMaterial.prototype.setImage = function(texture, width, height)
{
    this.image = texture;
    this.imageWidth = width;
    this.imageHeight = height;
};

GASEngine.SkyboxMaterial.prototype.setBackgroundExposure = function(exposure)
{
    this.backgroundExposure[0] = exposure;
};

GASEngine.SkyboxMaterial.prototype.getBackgroundExposure = function()
{
    return this.backgroundExposure[0];
};

GASEngine.SkyboxMaterial.prototype.setLightExposure = function(exposure)
{
    this.lightExposure[0] = exposure;
};

GASEngine.SkyboxMaterial.prototype.setSPH = function(lightExposure, sph)
{
    this.lightExposure[0] = lightExposure;
    this.sph = sph;
};

GASEngine.SkyboxMaterial.prototype.setSolidColor = function(r, g, b)
{
    this.solidColor[0] = r;
    this.solidColor[1] = g;
    this.solidColor[2] = b;
};

GASEngine.SkyboxMaterial.prototype.setOrientation = function(value)
{
    this.orientation[0] = value;
};

GASEngine.SkyboxMaterial.prototype.setBackgroundImage = function(imageName)
{
    this.backgroundName = imageName;
    GASEngine.Resources.Instance.loadTexture
    (
        `/system/backgroundImages/${this.backgroundName}`,
        true,
        function(webglTexture, width, height)
        {
            this.setImage(webglTexture, width, height);
        }.bind(this),
        null, null
    );
};

GASEngine.SkyboxMaterial.prototype.setBackgroundCubeMap = function(cubeMapName)
{
    this.cubeMapName = cubeMapName;
    GASEngine.Resources.Instance.loadSPH
    (
        `/system/backgroundCubes/${this.cubeMapName}/diffuse_sph.json`,
        function (presetExposure, sph)
        {
            this.setSPH(presetExposure, sph);
        }.bind(this)
    );
    this._loadCubeMapFiles();
};

GASEngine.SkyboxMaterial.prototype.setEnvironmentBlur = function(blur)
{
    if(blur >= 0 && blur <= GASEngine.SkyboxMaterial.CUBEMAPINFO.length)
    {
        this.environmentBlur = blur;
        this._loadCubeMapFiles();
    }
};

GASEngine.SkyboxMaterial.prototype._loadCubeMapFiles = function()
{
    const cubeMapInfo = GASEngine.SkyboxMaterial.CUBEMAPINFO[this.environmentBlur];
    const url = `/system/backgroundCubes/${this.cubeMapName}/${cubeMapInfo.name}`;
    const size = cubeMapInfo.size;
    GASEngine.Resources.Instance.loadCubeTexture
    (
        url,
        size,
        function(texture, size)
        {
            this.setCubeMap(texture, size);
        }.bind(this)
    );
};

GASEngine.SkyboxMaterial.prototype.updateImageRatio = function(ratio, renderTargetWidth, renderTargetHeight)
{
    if(this.image != null && this.imageWidth > 0 && this.imageHeight > 0 && renderTargetWidth > 0 && renderTargetHeight > 0)
    {
        var w0 = this.imageWidth;
        var h0 = this.imageHeight;

        var w1 = renderTargetWidth;
        var h1 = renderTargetHeight;

        var w2 = h1 * w0 / h0;
        if(w2 > w1)
        {
            ratio[1] = 1.0;
            ratio[0] = w1 / w2;
        }
        else
        {
            var h2 = w1 * h0 / w0;
            ratio[1] = h1 / h2;
            ratio[0] = 1.0;
        }

        return true;
    }
    else
    {
        return false;
    }
};

//PureColorMaterial
//Author: saralu
//Date: 2019-05-15
GASEngine.PureColorMaterial = function()
{
    GASEngine.Material.call(this);
    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/PureColorFragment.glsl';
    
    this.defaultColor = [1.0, 1.0, 1.0, 1.0];
    this.pureColor = null;
    this.pureColorBase = null;
};

GASEngine.PureColorMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.PureColorMaterial.prototype.constructor = GASEngine.PureColorMaterial;
GASEngine.PureColorMaterial.prototype.typeName = 'pureColor';
GASEngine.PureColorMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.PureColorMaterial.prototype.typeName);

GASEngine.PureColorMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.PureColorMaterial.prototype.setPureColorBase = function(color)
{
    this.pureColorBase = color.slice();
    this.pureColor = color.slice();
}

GASEngine.PureColorMaterial.prototype.getPureColorBase = function()
{
    var color = this.pureColorBase ? this.pureColorBase.slice() : null;
    return color;
}

GASEngine.PureColorMaterial.prototype.setPureColor = function(color)
{
    this.pureColor = color.slice();
}

GASEngine.PureColorMaterial.prototype.getPureColor = function()
{
    var color = this.pureColor ? this.pureColor : null;
    return color;
}

GASEngine.PureColorMaterial.prototype.setDepthTest = function(enable)
{
    this.helperDepthTest = enable;
}

GASEngine.PureColorMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];

        var color = this.pureColor ? this.pureColor : this.defaultColor;
        uniformValues['pureColor'] = color;
    }
})();

GASEngine.PureColorMaterial.prototype.updateRenderStates = function(item)
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

//Author: saralu
//Date: 2019-04-30
//DepthMaterial
GASEngine.DepthMaterial = function()
{
    GASEngine.Material.call(this);
    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/DepthFragment.glsl';
};

GASEngine.DepthMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.DepthMaterial.prototype.constructor = GASEngine.DepthMaterial;
GASEngine.DepthMaterial.prototype.typeName = 'depth';
GASEngine.DepthMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.DepthMaterial.prototype.typeName);


GASEngine.DepthMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.DepthMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];
        
        //
        if(item.mesh.isSkinned() &&
            GASEngine.PBRPipeline.Instance &&
            GASEngine.PBRPipeline.Instance.boneTexture &&
            GASEngine.PBRPipeline.Instance.boneMatrices)
        {
            var boneTexture = GASEngine.PBRPipeline.Instance.boneTexture;
            var boneTextureSize = GASEngine.PBRPipeline.Instance.boneTextureSize;
            var boneMatrices = GASEngine.PBRPipeline.Instance.boneMatrices;            

            item.mesh.computeSkinningMatrices(boneMatrices);

            gl.bindTexture(gl.TEXTURE_2D, boneTexture);
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);

            if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.FLOAT, boneMatrices);
            }
            else
            {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, boneTextureSize[0], boneTextureSize[0], 0, gl.RGBA, gl.UNSIGNED_BYTE, boneMatrices);
            }

            uniformValues['boneTexture'] = boneTexture;
            uniformValues['boneTextureSize'] = boneTextureSize;
        }
    }
})();

GASEngine.DepthMaterial.prototype.updateRenderStates = function()
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

//Author: saralu
//Date: 2019-04-11
GASEngine.HotspotMaterial = function()
{
    GASEngine.Material.call(this);
    this.vertexShaderFile = '/system/shaders/hotspotVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/hotspotFragment.glsl';

    this.image = null;
    this.imageWidth = 0;
    this.imageHeight = 0;
    this.hotspotColorPalette = new Float32Array
    (
        [
            1.0, 1.0, 1.0, 1.0, 
            0.5, 0.5, 0.5, 1.0,
            0.0, 1.0, 1.0, 1.0
        ]
    ); //0 normal, 1 disbled, 2 highlight
};

GASEngine.HotspotMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.HotspotMaterial.prototype.constructor = GASEngine.HotspotMaterial;
GASEngine.HotspotMaterial.prototype.typeName = 'hotspot';
GASEngine.HotspotMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.HotspotMaterial.prototype.typeName);

GASEngine.HotspotMaterial.prototype.setImage = function(texture, width, height)
{
    this.image = texture;
    this.imageWidth = width;
    this.imageHeight = height;
};

GASEngine.HotspotMaterial.prototype.generateVertexShaderKey = function(item)
{
    return 0;
}

GASEngine.HotspotMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.HotspotMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var matrix4 = new GASEngine.Matrix4();
        matrix4.getInverse(camera.getWorldMatrix());
        var matrixProjection = camera.getProjectionMatrix();

        uniformValues['viewMatrix'] =  matrix4.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;

        uniformValues['frameBufferSize'] = new Float32Array([ GASEngine.PBRPipeline.Instance.renderWidth, GASEngine.PBRPipeline.Instance.renderHeight]);
        uniformValues['hotspotSize'] = new Float32Array([GASEngine.HotspotManager.HOTSPOT_SIZE, GASEngine.HotspotManager.HOTSPOT_SIZE]);

        uniformValues['depth'] = GASEngine.PBRPipeline.Instance.depthRT.texture;
        uniformValues['diffuse'] = GASEngine.HotspotManager.Instance.hotspotTexture;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];

        uniformValues['hotspotColorPalette'] = this.hotspotColorPalette;
    }
})();

GASEngine.HotspotMaterial.prototype.updateRenderStates = function()
{
    GASEngine.Material.prototype.updateRenderStates.call(this);

    GASEngine.WebGLRenderStates.Instance.setAlphaBlendEnable(1);
    GASEngine.WebGLRenderStates.Instance.setAlphaBlendMode(1);

    //disable depth will not pass depth test
    GASEngine.WebGLRenderStates.Instance.setDepthTestEnable(0); 
    GASEngine.WebGLRenderStates.Instance.setDepthTestMode(2);
    GASEngine.WebGLRenderStates.Instance.setDepthWriteEnable(0);
};

//UVLayoutMaterial
GASEngine.UVLayoutMaterial = function ()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/uvLayoutVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/WireframeFragment.glsl';

    this.wireframe = true;
    this.showUV = true;

    this.lineColor = new Float32Array([0.0, 1.0, 1.0]);
    this.offset = new Float32Array([0.5, 0.5]);
    this.uvScale = new Float32Array([1.0]);
    //this.depthBias = true;
};

GASEngine.UVLayoutMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.UVLayoutMaterial.prototype.constructor = GASEngine.UVLayoutMaterial;
GASEngine.UVLayoutMaterial.prototype.typeName = 'uvlayout';
GASEngine.UVLayoutMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.UVLayoutMaterial.prototype.typeName);

GASEngine.UVLayoutMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.UVLayoutMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['modelMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['modelViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;   
        uniformValues['uvScale'] = this.uvScale;
        uniformValues['lineColor'] = this.lineColor;
        uniformValues['offset'] = this.offset;
    }
})();

GASEngine.UVLayoutMaterial.prototype.updateRenderStates = function(item)
{
    GASEngine.Material.prototype.updateRenderStates.call(this);
};

//LambertMaterial
GASEngine.LambertMaterial = function ()
{
    GASEngine.Material.call(this);

    this.vertexShaderFile = '/system/shaders/PBRVertex.glsl';
    this.fragmentShaderFile = '/system/shaders/LambertFragment.glsl';

    this.lightVec = new Float32Array([80.0, 100.0, 90.0]);

};

GASEngine.LambertMaterial.prototype = Object.create(GASEngine.Material.prototype);
GASEngine.LambertMaterial.prototype.constructor = GASEngine.LambertMaterial;
GASEngine.LambertMaterial.prototype.typeName = 'lambert';
GASEngine.LambertMaterial.prototype.hashCode = GASEngine.Utilities.hashString(GASEngine.LambertMaterial.prototype.typeName);

GASEngine.LambertMaterial.prototype.generateFragmentShaderKey = function(item)
{
    return 0;
}

GASEngine.LambertMaterial.prototype.updateUniforms = (function()
{
    var matrixWorldView = new GASEngine.Matrix4();
    var matrixNormal = new GASEngine.Matrix3();

    return function (uniformValues, camera, item, SP)
    {
        var gl = GASEngine.WebGLDevice.Instance.gl;

        var matrixWorld = item.matrixWorld;
        var matrixView = camera.getViewMatrix();
        var matrixProjection = camera.getProjectionMatrix();

        matrixWorldView.multiplyMatrices(matrixView, matrixWorld);
        matrixNormal.getNormalMatrix(matrixWorldView);

        uniformValues['worldMatrix'] = matrixWorld.elements;
        uniformValues['viewMatrix'] = matrixView.elements;
        uniformValues['worldViewMatrix'] = matrixWorldView.elements;
        uniformValues['projectionMatrix'] = matrixProjection.elements;
        uniformValues['normalMatrix'] = matrixNormal.elements;

        uniformValues['cameraNearFar'] = [camera.near, camera.far];

        uniformValues['lightVec'] = this.lightVec;

    }
})();

GASEngine.LambertMaterial.prototype.updateRenderStates = function(item)
{
    GASEngine.Material.prototype.updateRenderStates.call(this);

    GASEngine.WebGLRenderStates.Instance.setDepthTestEnable(1);
    GASEngine.WebGLRenderStates.Instance.setCullingFaceEnable(1);
};