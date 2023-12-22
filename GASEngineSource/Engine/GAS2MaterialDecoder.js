//Author: tomyi
//Date: 2017-07-03

GASEngine.GAS2MaterialDecoder = function()
{
    this._parseFunctionTable = {};

    this._parseFunctionTable.compound = function(material, config)
    {
        material.name = config.name;
        material.uniqueID = GASEngine.UniqueIDGenerator.Instance.checkAndGetID(config.uniqueID);

        var mapping = {};

        var blinnPhongMaterial = GASEngine.MaterialFactory.Instance.create(config.blinnPhong.type);
        var blinnPhongMaps = this._parseFunctionTable[config.blinnPhong.type](blinnPhongMaterial, config.blinnPhong);
        mapping[config.blinnPhong.type] = blinnPhongMaps;
        blinnPhongMaterial.uniqueID = material.uniqueID;
        material.addMaterial(blinnPhongMaterial);

        var dielectricMaterial = GASEngine.MaterialFactory.Instance.create(config.dielectric.type);
        var dielectricMaps = this._parseFunctionTable[config.dielectric.type](dielectricMaterial, config.dielectric);
        mapping[config.dielectric.type] = dielectricMaps;
        dielectricMaterial.uniqueID = material.uniqueID;
        material.addMaterial(dielectricMaterial);

        var electricMaterial = GASEngine.MaterialFactory.Instance.create(config.electric.type);
        var electricMaps = this._parseFunctionTable[config.electric.type](electricMaterial, config.electric);
        mapping[config.electric.type] = electricMaps;
        electricMaterial.uniqueID = material.uniqueID;
        material.addMaterial(electricMaterial);

        var matcapMaterial = GASEngine.MaterialFactory.Instance.create(config.matcap.type);
        var matcapMaps = this._parseFunctionTable[config.matcap.type](matcapMaterial, config.matcap);
        mapping[config.matcap.type] = matcapMaps;
        matcapMaterial.uniqueID = material.uniqueID;
        material.addMaterial(matcapMaterial);

        material.setActiveMaterial(config.activeMaterial);
        material.deferedMaps = mapping;
        var materialsToLoad = material.deferedMaps[config.activeMaterial];
        delete material.deferedMaps[config.activeMaterial];

        return materialsToLoad;
        // return [].concat(blinnPhongMaps, dielectricMaps, electricMaps, matcapMaps);  // load all on start

    }.bind(this);

    this._parseFunctionTable.blinnPhong = function(material, config)
    {
        material.name = config.name;
        material.uniqueID = GASEngine.UniqueIDGenerator.Instance.checkAndGetID(config.uniqueID);
        material.culling = config.culling;        
        material.vertexShaderFile = config.vertexShaderFile;
        material.fragmentShaderFile = config.fragmentShaderFile;

        var maps = [];

        material.reflectiveRatio = config.reflectiveRatio;

        //AlbedoMap
        material.albedoEnable = config.albedo.enable;
        material.albedoDefault = new Float32Array(config.albedo.default);  //the default color without texture
        material.albedoColor = new Float32Array(config.albedo.tint);
        material.albedoFactor = new Float32Array([config.albedo.factor]);
        material.albedoMap = this._parseTextureMap(config.albedo.map);
        maps.push(material.albedoMap);

        //SpecularMap
        material.specularEnable = config.specular.enable;
        material.specularDefault = new Float32Array(config.specular.default);
        material.specularColor = new Float32Array(config.specular.tint);
        material.specularFactor = new Float32Array([config.specular.factor]);
        material.specularMap = this._parseTextureMap(config.specular.map);
        maps.push(material.specularMap);

        //GlossinessMap
        material.glossinessEnable = config.glossiness.enable;
        material.glossinessFactor = new Float32Array([config.glossiness.factor]);
        material.glossinessDefault = new Float32Array(config.glossiness.default);
        material.glossinessMap = this._parseTextureMap(config.glossiness.map);
        maps.push(material.glossinessMap);

        //DisplacementMap
        material.displacementEnable = config.displacement.enable;
        material.displacementFactor = new Float32Array([config.displacement.factor]);
        material.displacementDefault = new Float32Array(config.displacement.default);
        material.displacementMap = this._parseTextureMap(config.displacement.map);
        maps.push(material.displacementMap);

        //NormalMap
        material.normalEnable = config.normal.enable;
        material.normalFlipY = new Float32Array([config.normal.map.flipY ? 0.0 : 1.0]);
        material.normalDefault = new Float32Array(config.normal.default);
        material.normalFactor = new Float32Array([config.normal.factor]);
        material.normalMap = this._parseTextureMap(config.normal.map);
        maps.push(material.normalMap);

        //LightMap
        material.lightEnable = config.light.enable;
        material.lightDefault = new Float32Array(config.light.default);
        material.lightColor = new Float32Array(config.light.tint);
        material.lightFactor = new Float32Array([config.light.factor]);
        material.lightMap = this._parseTextureMap(config.light.map);
        maps.push(material.lightMap);

        //TransparencyMap
        material.transparencyEnable = config.transparency.enable;
        material.transparencyDefault = new Float32Array(config.transparency.default);
        material.transparencyAlphaInvert = new Int32Array([config.transparency.invert ? 1 : 0]);
        material.transparencyFactor = new Float32Array([config.transparency.factor]);
        material.transparencyBlendMode = config.transparency.mode;
        material.transparencyMap = this._parseTextureMap(config.transparency.map);
        maps.push(material.transparencyMap);

        //EmissiveMap
        material.emissiveEnable = config.emissive.enable;
        material.emissiveDefault = new Float32Array(config.emissive.default);
        material.emissiveMultiplicative = new Float32Array([config.emissive.multiplicative ? 0.0 : 1.0]);
        material.emissiveColor = new Float32Array(config.emissive.tint);
        material.emissiveFactor = new Float32Array([config.emissive.factor]);
        material.emissiveMap = this._parseTextureMap(config.emissive.map);
        maps.push(material.emissiveMap);        

        return maps;

    }.bind(this);

    this._parseFunctionTable.dielectric = function(material, config)
    {
        material.name = config.name;
        material.uniqueID = GASEngine.UniqueIDGenerator.Instance.checkAndGetID(config.uniqueID);
        material.culling = config.culling;

        material.vertexShaderFile = config.vertexShaderFile;
        material.fragmentShaderFile = config.fragmentShaderFile;

        var maps = [];

        //AlbedoMap
        material.albedoEnable = config.albedo.enable;
        material.albedoDefault = new Float32Array(config.albedo.default);
        material.albedoColor = new Float32Array(config.albedo.tint);
        material.albedoFactor = new Float32Array([config.albedo.factor]);
        material.albedoMap = this._parseTextureMap(config.albedo.map);
        maps.push(material.albedoMap);

        if(material.typeName === 'dielectric')
        {
            //MetalnessMap
            material.metalnessEnable = config.metalness.enable;
            material.metalnessDefault = new Float32Array(config.metalness.default);
            material.metalnessFactor = new Float32Array([config.metalness.factor]);
            material.metalnessMap = this._parseTextureMap(config.metalness.map);
            maps.push(material.metalnessMap);

            //SpecularF0Map
            material.specularF0Enable = config.specularF0.enable;
            material.specularF0Default = new Float32Array(config.specularF0.default);
            material.specularF0Factor = new Float32Array([config.specularF0.factor]);
            material.specularF0Map = this._parseTextureMap(config.specularF0.map);
            maps.push(material.specularF0Map);
        }
        else if(material.typeName === 'electric')
        {
            //SpecularMap
            material.specularEnable = config.specular.enable;
            material.specularDefault = new Float32Array(config.specular.default);
            material.specularColor = new Float32Array(config.specular.tint);
            material.specularFactor = new Float32Array([config.specular.factor]);
            material.specularMap = this._parseTextureMap(config.specular.map);  
            maps.push(material.specularMap);
        }
        else
        {
            console.error('GAS2MaterialDecoder parse material failed.');
            return null;
        }

        //RoughnessMap
        material.roughnessEnable = config.roughness.enable;
        material.roughnessDefault = new Float32Array(config.roughness.default);
        material.roughnessFactor = new Float32Array([config.roughness.factor]);
        material.roughnessMap = this._parseTextureMap(config.roughness.map);
        maps.push(material.roughnessMap);

        //DisplacementMap
        material.displacementEnable = config.displacement.enable;
        material.displacementDefault = new Float32Array(config.displacement.default);
        material.displacementFactor = new Float32Array([config.displacement.factor]);
        material.displacementMap = this._parseTextureMap(config.displacement.map);
        maps.push(material.displacementMap);       

        //NormalMap
        material.normalEnable = config.normal.enable;
        material.normalDefault = new Float32Array(config.normal.default);
        material.normalFactor = new Float32Array([config.normal.factor]);
        material.normalFlipY = new Int32Array([config.normal.map.flipY ? 1 : 0]);
        material.normalMap = this._parseTextureMap(config.normal.map);
        maps.push(material.normalMap);

        //AOMap
        material.aoEnable = config.ao.enable;
        material.aoDefault = new Float32Array(config.ao.default);
        material.aoFactor = new Float32Array([config.ao.factor]);
        material.aoOccludeSpecular = new Int32Array([config.ao.occludeSpecular ? 1 : 0]);
        material.aoMap = this._parseTextureMap(config.ao.map);
        maps.push(material.aoMap);

        //CavityMap
        material.cavityEnable = config.cavity.enable;
        material.cavityDefault = new Float32Array(config.cavity.default);
        material.cavityFactor = new Float32Array([config.cavity.factor]);
        material.cavityMap = this._parseTextureMap(config.cavity.map);
        maps.push(material.cavityMap);

        //TransparencyMap
        material.transparencyEnable = config.transparency.enable;
        material.transparencyDefault = new Float32Array(config.transparency.default);
        material.transparencyAlphaInvert = new Int32Array([config.transparency.invert ? 1 : 0]);
        material.transparencyFactor = new Float32Array([config.transparency.factor]);
        material.transparencyBlendMode = config.transparency.mode;
        material.transparencyMap = this._parseTextureMap(config.transparency.map);
        maps.push(material.transparencyMap);

        //EmissiveMap
        material.emissiveEnable = config.emissive.enable;
        material.emissiveDefault = new Float32Array(config.emissive.default);
        material.emissiveFactor = new Float32Array([config.emissive.factor]);
        material.emissiveColor = new Float32Array(config.emissive.tint);
        material.emissiveMap = this._parseTextureMap(config.emissive.map);
        maps.push(material.emissiveMap);

        return maps;

    }.bind(this);

    this._parseFunctionTable.electric = function(material, config)
    {        
        return this._parseFunctionTable.dielectric(material, config);
    }.bind(this);

    this._parseFunctionTable.matcap = function(material, config)
    {
        material.name = config.name;
        material.uniqueID = GASEngine.UniqueIDGenerator.Instance.checkAndGetID(config.uniqueID);
        material.culling = config.culling;
        material.vertexShaderFile = config.vertexShaderFile;
        material.fragmentShaderFile = config.fragmentShaderFile;

        var maps = [];
        //MatCap
        material.matCapEnable = config.matsphere.enable;
        material.matCapColor = new Float32Array(config.matsphere.tint);
        material.matCapCurvature = new Float32Array([config.matsphere.curvature]);
        material.matCapMap = this._parseTextureMap(config.matsphere.map);
        maps.push(material.matCapMap);

        //DisplacementMap
        material.displacementEnable = config.displacement.enable;
        material.displacementDefault = new Float32Array(config.displacement.default);
        material.displacementFactor = new Float32Array([config.displacement.factor]);
        material.displacementMap = this._parseTextureMap(config.displacement.map);
        maps.push(material.displacementMap);

        //NormalMap
        material.normalEnable = config.normal.enable;
        material.normalDefault = new Float32Array(config.normal.default);
        material.normalFactor = new Float32Array([config.normal.factor]);
        material.normalFlipY = new Int32Array([config.normal.map.flipY ? 1 : 0]);
        material.normalMap = this._parseTextureMap(config.normal.map);
        maps.push(material.normalMap);

        //TransparencyMap
        material.transparencyEnable = config.transparency.enable;
        material.transparencyDefault = new Float32Array(config.transparency.default);
        material.transparencyAlphaInvert = new Int32Array([config.transparency.invert ? 1 : 0]);
        material.transparencyFactor = new Float32Array([config.transparency.factor]);
        material.transparencyBlendMode = config.transparency.mode;
        material.transparencyMap = this._parseTextureMap(config.transparency.map);
        maps.push(material.transparencyMap);

        return maps;

    }.bind(this);
};

GASEngine.GAS2MaterialDecoder.prototype = {

    constructor: GASEngine.GAS2MaterialDecoder,

    parse: function(materialJSON)
    {
        var material = GASEngine.MaterialFactory.Instance.create(materialJSON.type);

        var materialMaps = this._parseFunctionTable[materialJSON.type](material, materialJSON);

        //Submit texture load tasks.
        for(var i = 0; i < materialMaps.length; ++i)
        {
            if(materialMaps[i].texture.length > 0 && !materialMaps[i].webglTexture)
            {
                GASEngine.Resources.Instance.loadTextureOnMaterial(materialMaps[i]);
            }
        }

        return material;
    },

    _parseTextureMap: function(mapData)
    {
        var textureMap = GASEngine.MaterialMapFactory.Instance.create();
        textureMap.wrapModeU = mapData.wrapModeU;
        textureMap.wrapModeV = mapData.wrapModeV;
        textureMap.minFilter = mapData.minFilter;
        textureMap.maxFilter = mapData.maxFilter;
        textureMap.uvSwap = mapData.uvSwap;
        textureMap.texture = mapData.texture;

        textureMap.pixelChannels = new Float32Array([0.0, 0.0, 0.0, 0.0]);
        var pixelChannel = 0;
        if(mapData.pixelChannels >= 0 && mapData.pixelChannels < 4)
        {
            pixelChannel = mapData.pixelChannels;
        }
        textureMap.pixelChannels[pixelChannel] = 1.0;

        textureMap.translation = new Float32Array(mapData.T);
        textureMap.rotation = new Float32Array(mapData.R);
        textureMap.scaling = new Float32Array(mapData.S);
        textureMap.rotationPivot = new Float32Array(mapData.Rp);
        textureMap.scalingPivot = new Float32Array(mapData.Sp);

        return textureMap;
    },
};