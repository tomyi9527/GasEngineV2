GASEngine.GAS1MaterialDecoder = function()
{
};

GASEngine.GAS1MaterialDecoder.prototype = 
{
    constructor: GASEngine.GAS1MaterialDecoder,

    parse: function(materialConfigs, mapConfigs, textureConfigs)
    {
        var materials = [];        
        for(var i = 0; i < materialConfigs.length; ++i)
        {
            var config = materialConfigs[i];
            var compoundMaterial = GASEngine.MaterialFactory.Instance.create('compound');
            var dielectricMaterial = GASEngine.MaterialFactory.Instance.create('dielectric');
            var electricMaterial = GASEngine.MaterialFactory.Instance.create('electric');

            if(config.type === undefined)
            {
                config.type = 1;
            }

            //0: classic   1: dielectric   2: electric
            var activeMaterialType = 'dielectric';
            switch(config.type)
            {
                case 0: activeMaterialType = 'dielectric'; break;
                case 1: activeMaterialType = 'dielectric'; break;
                case 2: activeMaterialType = 'electric'; break;
                default: activeMaterialType = 'dielectric'; break;
            }            

            compoundMaterial.name = config.name;
            compoundMaterial.visible = config.visible;
            compoundMaterial.uniqueID = config.uniqueID;
            compoundMaterial.culling = config.culling;

            var maps;
            maps = this._parseDielectric(dielectricMaterial, config, mapConfigs, textureConfigs);
            maps = this._parseElectric(electricMaterial, config, mapConfigs, textureConfigs);
            
            compoundMaterial.addMaterial(dielectricMaterial);
            compoundMaterial.addMaterial(electricMaterial);
            compoundMaterial.setActiveMaterial(activeMaterialType);

            materials.push(compoundMaterial);
        }

        return materials;
    },

    _findTextureData: function(textureConfigs, textureName)
    {
        var config = null;
        for(var i = 0; i < textureConfigs.length; ++i)
        {
            if(textureConfigs[i].internalName === textureName)
            {
                config = textureConfigs[i];
                break;
            }
        }
        return config;
    },

    _parseTextureMap: function(mapData, pixelChannel, textureConfigs)
    {
        var textureMap = GASEngine.MaterialMapFactory.Instance.create();
        textureMap.wrapModeU = mapData.wrapModeU;
        textureMap.wrapModeV = mapData.wrapModeV;
        textureMap.minFilter = mapData.minFilter;
        textureMap.maxFilter = mapData.maxFilter;
        textureMap.uvSwap = mapData.uvSwap;

        var textureData = this._findTextureData(textureConfigs, mapData.texture);
        if(textureData !== null)
        {
            textureMap.texture = textureData.internalName;
        }
        else
        {
            textureMap.texture = '';
        }

        if(pixelChannel !== undefined)
        {
            textureMap.pixelChannels = new Float32Array([0.0, 0.0, 0.0, 0.0]);
            if(pixelChannel < 0 || pixelChannel > 3)
            {
                pixelChannel = 0;
            }
            textureMap.pixelChannels[pixelChannel] = 1.0;
        }

        textureMap.translation = new Float32Array(mapData.T);
        textureMap.rotation = new Float32Array(mapData.R);
        textureMap.scaling = new Float32Array(mapData.S);
        textureMap.rotationPivot = new Float32Array(mapData.Rp);
        textureMap.scalingPivot = new Float32Array(mapData.Sp);

        return textureMap;
    },

    _parseDielectric: function(material, materialConfig, mapConfigs, textureConfigs)
    {
        //Dielectric        
        material.name = 'Compound-Dielectric';
        material.visible = materialConfig.visible;
        material.uniqueID = materialConfig.uniqueID;
        material.culling = materialConfig.culling;
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/PBRFragment.glsl";

        var maps = [];
        var config = null, textureMapConfig = null, texturMap = null;

        //BaseColor
        config = materialConfig.PBR.Dielectric.BaseColor;
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array(config.color);
        material.albedoColor = new Float32Array(config.colorBias);
        material.albedoFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.albedoMap = texturMap;            
        }

        //MetalnessMap
        config = materialConfig.PBR.Dielectric.Metalness;
        material.metalnessEnable = true;
        material.metalnessDefault = new Float32Array(config.color);
        material.metalnessFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.metalnessMap = texturMap;
        }

        //SpecularF0Map
        config = materialConfig.PBR.Dielectric.SpecularF0;
        material.specularF0Enable = true;
        material.specularF0Default = new Float32Array(config.color);
        material.specularF0Factor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.specularF0Map = texturMap;
        }

        //RoughnessMap
        config = materialConfig.PBR.Dielectric.Roughness;
        material.roughnessEnable = true;
        material.roughnessDefault = new Float32Array(config.color);
        material.roughnessFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.roughnessMap = texturMap;
        }

        //AOMap
        config = materialConfig.PBR.AO;
        material.aoEnable = config.enable;
        material.aoDefault = new Float32Array(config.color);
        material.aoFactor = new Float32Array([config.factor]);
        material.aoOccludeSpecular = new Int32Array([config.occludeSpecular ? 1 : 0]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.aoMap = texturMap;
        }

        //CavityMap
        config = materialConfig.PBR.Cavity;
        material.cavityEnable = config.enable;
        material.cavityDefault = new Float32Array(config.color);
        material.cavityFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.cavityMap = texturMap;
        }

        //DisplacementMap
        config = materialConfig.Displacement;
        material.displacementEnable = config.enable;
        material.displacementDefault = new Float32Array(config.color);
        material.displacementFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.displacementMap = texturMap;
        }

        //NormalMap
        config = materialConfig.Normal;
        material.normalEnable = config.enable;
        material.normalDefault = new Float32Array(config.color);
        material.normalFactor = new Float32Array([config.factor]);
        material.normalFlipY = new Int32Array([config.flipY ? 1 : 0]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.normalMap = texturMap;
        }       

        //TransparencyMap
        config = materialConfig.Transparency;
        material.transparencyEnable = config.enable;
        material.transparencyDefault = new Float32Array(config.color);
        material.transparencyAlphaInvert = new Int32Array([config.invert ? 1 : 0]);
        material.transparencyFactor = new Float32Array([config.factor]);
        material.transparencyBlendMode = config.mode;
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.transparencyMap = texturMap;
        }

        //EmissiveMap
        config = materialConfig.Emissive;
        material.emissiveEnable = config.enable;
        material.emissiveDefault = new Float32Array(config.color);
        material.emissiveFactor = new Float32Array([config.factor]);
        material.emissiveColor = new Float32Array(config.colorBias);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.emissiveMap = texturMap;
        }

        return maps;
    },

    _parseElectric: function(material, materialConfig, mapConfigs, textureConfigs)
    {
        //Electric        
        material.name = 'Compound-Electric';
        material.visible = materialConfig.visible;
        material.uniqueID = materialConfig.uniqueID;
        material.culling = materialConfig.culling;
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/PBRFragment.glsl";

        var maps = [];
        var config = null, textureMapConfig = null, texturMap = null;

        //AlbedoColor
        config = materialConfig.PBR.Electric.Albedo;
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array(config.color);
        material.albedoColor = new Float32Array(config.colorBias);
        material.albedoFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.albedoMap = texturMap;            
        }

        //SpecularMap
        config = materialConfig.PBR.Electric.Specular;
        material.specularEnable = true;
        material.specularDefault = new Float32Array(config.color);
        material.specularColor = new Float32Array(config.colorBias);
        material.specularFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.specularFactor = texturMap;            
        }

        //RoughnessMap
        config = materialConfig.PBR.Electric.Roughness;
        material.roughnessEnable = true;
        material.roughnessDefault = new Float32Array(config.color);
        material.roughnessFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.roughnessMap = texturMap;
        }

        //AOMap
        config = materialConfig.PBR.AO;
        material.aoEnable = config.enable;
        material.aoDefault = new Float32Array(config.color);
        material.aoFactor = new Float32Array([config.factor]);
        material.aoOccludeSpecular = new Int32Array([config.occludeSpecular ? 1 : 0]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.aoMap = texturMap;
        }

        //CavityMap
        config = materialConfig.PBR.Cavity;
        material.cavityEnable = config.enable;
        material.cavityDefault = new Float32Array(config.color);
        material.cavityFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.cavityMap = texturMap;
        }

        //DisplacementMap
        config = materialConfig.Displacement;
        material.displacementEnable = config.enable;
        material.displacementDefault = new Float32Array(config.color);
        material.displacementFactor = new Float32Array([config.factor]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.displacementMap = texturMap;
        }

        //NormalMap
        config = materialConfig.Normal;
        material.normalEnable = config.enable;
        material.normalDefault = new Float32Array(config.color);
        material.normalFactor = new Float32Array([config.factor]);
        material.normalFlipY = new Int32Array([config.flipY ? 1 : 0]);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.normalMap = texturMap;
        }

        //TransparencyMap
        config = materialConfig.Transparency;
        material.transparencyEnable = config.enable;
        material.transparencyDefault = new Float32Array(config.color);
        material.transparencyAlphaInvert = new Int32Array([config.invert ? 1 : 0]);
        material.transparencyFactor = new Float32Array([config.factor]);
        material.transparencyBlendMode = config.mode;
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.transparencyMap = texturMap;
        }

        //EmissiveMap
        config = materialConfig.Emissive;
        material.emissiveEnable = config.enable;
        material.emissiveDefault = new Float32Array(config.color);
        material.emissiveFactor = new Float32Array([config.factor]);
        material.emissiveColor = new Float32Array(config.colorBias);
        if(config.map >= 0 && config.map < mapConfigs.length)
        {
            textureMapConfig = mapConfigs[config.map];   
            texturMap = this._parseTextureMap(textureMapConfig, config.channel, textureConfigs);
            maps.push(texturMap);
            material.emissiveMap = texturMap;
        }

        return maps;
    }
};