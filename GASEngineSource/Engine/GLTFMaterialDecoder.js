GASEngine.GLTFMaterialDecoder = function ()
{
};

GASEngine.GLTFMaterialDecoder.prototype = {
    constructor: GASEngine.GLTFMaterialDecoder,

    parse: function (materialDef, textureObjects)
    {
        var compoundMaterial = GASEngine.MaterialFactory.Instance.create('compound');

        var blinnPhongMaterial = GASEngine.MaterialFactory.Instance.create('blinnPhong');
        var dielectricMaterial = GASEngine.MaterialFactory.Instance.create('dielectric');

        compoundMaterial.name = materialDef.name;
        compoundMaterial.visible = true;
        compoundMaterial.uniqueID = GASEngine.generateUUID();

        this._parseBlinnPhong(blinnPhongMaterial, materialDef, textureObjects);
        this._parseDielectric(dielectricMaterial, materialDef, textureObjects);

        compoundMaterial.addMaterial(blinnPhongMaterial);
        compoundMaterial.addMaterial(dielectricMaterial);
        compoundMaterial.setActiveMaterial('dielectric');

        return compoundMaterial;
    },

    _parseTextureMap: function (textureObject)
    {
        var textureMap = GASEngine.MaterialMapFactory.Instance.create();

        textureMap.texture = textureObject.uri;
        textureMap.minFilter = 'LINEAR_MIPMAP_LINEAR';
        textureMap.maxFilter = 'LINEAR';

        var pixelChannel = 0;
        textureMap.pixelChannels = new Float32Array([0.0, 0.0, 0.0, 0.0]);
        if (pixelChannel < 0 || pixelChannel > 3)
            pixelChannel = 0;

        textureMap.pixelChannels[pixelChannel] = 1.0;
        return textureMap;
    },

    _parseBlinnPhong: function (material, materialDef, textureObjects)
    {
        //BlinnPhong        
        material.name = 'Compound-BlinnPhong';
        material.visible = true;
        material.uniqueID = GASEngine.generateUUID();
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/BlinnPhongFragment.glsl";

        var maps = [];
        var textureObject;
        var textureMap;

        //culling
        //TODO: 临时，总是设置为双面材质，rainyyuan
        material.culling = GASEngine.Material.CullingOff;
        //material.culling = materialDef.doubleSided?GASEngine.Material.CullingOff: GASEngine.Material.CullingOnCCW; 

        //BaseColor
        var metallicRoughness = materialDef.pbrMetallicRoughness || {};
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.albedoFactor = new Float32Array([1.0]);
        material.albedoColor = new Float32Array([0.588, 0.588, 0.588]);
        if (Array.isArray(metallicRoughness.baseColorFactor))
        {
            for(var j = 0; j < 3; j++) {
                material.albedoColor[j] = metallicRoughness.baseColorFactor[j];
            }
        }

        if (metallicRoughness.baseColorTexture !== undefined)
        {
            textureObject = textureObjects.get(metallicRoughness.baseColorTexture.index);
            textureMap = this._parseTextureMap(textureObject);
            maps.push(textureMap);
            material.albedoMap = textureMap;
        }
       
        //NormalMap
        if (materialDef.normalTexture !== undefined)
        {
            material.normalEnable = true;
            material.normalDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.normalFactor = new Float32Array([1.0]);
            material.normalFlipY = new Int32Array([1]);

            textureObject = textureObjects.get(materialDef.normalTexture.index);
            texturMap = this._parseTextureMap(textureObject);
            maps.push(texturMap);
            material.normalMap = texturMap;

            if (materialDef.normalTexture.scale !== undefined)
            {
                textureMap.scaling = new Float32Array([materialDef.normalTexture.scale, materialDef.normalTexture.scale]);
            }
        }
       
        //EmissiveMap
        if (materialDef.emissiveFactor !== undefined)
        {
            material.emissiveEnable = true;
            material.emissiveDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.emissiveFactor = new Float32Array([1.0]);
            material.emissiveColor = new Float32Array(materialDef.emissiveFactor);

            if (materialDef.emissiveTexture !== undefined)
            {
                var textureObject = textureObjects.get(materialDef.emissiveTexture.index);
                texturMap = this._parseTextureMap(textureObject);
                maps.push(texturMap);
                material.emissiveMap = texturMap;
            }
        }

        return maps;
    },

    _parseDielectric: function (material, materialDef, textureObjects)
    {
        //Dielectric        
        material.name = 'Compound-Dielectric';
        material.visible = true;
        material.uniqueID = GASEngine.generateUUID();
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/PBRFragment.glsl";

        var maps = [];
        var textureObject;
        var textureMap;

        //culling
        //TODO: 临时，总是设置为双面材质，rainyyuan
        material.culling = GASEngine.Material.CullingOff;
        //material.culling = materialDef.doubleSided?GASEngine.Material.CullingOff: GASEngine.Material.CullingOnCCW; 

        //BaseColor
        var metallicRoughness = materialDef.pbrMetallicRoughness || {};
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.albedoFactor = new Float32Array([1.0]);
        material.albedoColor = new Float32Array([0.588, 0.588, 0.588])
        if (Array.isArray(metallicRoughness.baseColorFactor))
        {
            for(var j = 0; j < 3; j++) {
                material.albedoColor[j] = metallicRoughness.baseColorFactor[j];
            }
        }
        
        if (metallicRoughness.baseColorTexture !== undefined)
        {
            textureObject = textureObjects.get(metallicRoughness.baseColorTexture.index);
            textureMap = this._parseTextureMap(textureObject);
            maps.push(textureMap);
            material.albedoMap = textureMap;
        }

         //MetalnessMap
         material.metalnessEnable = true;
         material.metalnessDefault = new Float32Array([1.0]);
         var metalness = metallicRoughness.metallicFactor !== undefined ? metallicRoughness.metallicFactor : 1.0;
         material.metalnessFactor = new Float32Array([metalness]);

         //RoughnessMap
         material.roughnessEnable = true;
         material.roughnessDefault = new Float32Array([1.0]);
         var roughness = metallicRoughness.roughnessFactor !== undefined ? metallicRoughness.roughnessFactor : 1.0;
         material.roughnessFactor = new Float32Array([roughness]);

        if (metallicRoughness.metallicRoughnessTexture !== undefined)
        {
            var textureIndex = metallicRoughness.metallicRoughnessTexture.index;
            textureObject = textureObjects.get(textureIndex);
            texturMap = this._parseTextureMap(textureObject);
            maps.push(texturMap);
            material.metalnessMap = texturMap;
            material.roughnessMap = texturMap;
        }

        //AOMap
        if (materialDef.occlusionTexture !== undefined)
        {
            material.aoEnable = true;
            material.aoDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.aoFactor = new Float32Array([1.0]);
            material.aoOccludeSpecular = new Int32Array([1]);

            textureObject = textureObjects.get(materialDef.occlusionTexture.index);
            texturMap = this._parseTextureMap(textureObject);
            maps.push(texturMap);
            material.aoMap = texturMap;
        }

        //NormalMap
        if (materialDef.normalTexture !== undefined)
        {
            material.normalEnable = true;
            material.normalDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.normalFactor = new Float32Array([1.0]);
            material.normalFlipY = new Int32Array([0]);

            textureObject = textureObjects.get(materialDef.normalTexture.index);
            texturMap = this._parseTextureMap(textureObject);
            maps.push(texturMap);
            material.normalMap = texturMap;

            if (materialDef.normalTexture.scale !== undefined)
            {
                textureMap.scaling = new Float32Array([materialDef.normalTexture.scale, materialDef.normalTexture.scale]);
            }
        }

        //EmissiveMap
        if (materialDef.emissiveFactor !== undefined)
        {
            material.emissiveEnable = true;
            material.emissiveDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.emissiveFactor = new Float32Array([1.0]);
            material.emissiveColor = new Float32Array(materialDef.emissiveFactor);

            if (materialDef.emissiveTexture !== undefined)
            {
                var textureObject = textureObjects.get(materialDef.emissiveTexture.index);
                texturMap = this._parseTextureMap(textureObject);
                maps.push(texturMap);
                material.emissiveMap = texturMap;
            }
        }

        return maps;
    }
};