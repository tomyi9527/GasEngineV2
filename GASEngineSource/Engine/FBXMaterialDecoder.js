GASEngine.FBXMaterialDecoder = function ()
{
    this._fbxTree = null;
    this._connectionMap_ = null;
    this._textureObjects_ = null;
};

GASEngine.FBXMaterialDecoder.prototype = {
    constructor: GASEngine.FBXMaterialDecoder,

    parse: function (materialNode, fbxTree, connections, _textureObjects_)
    {
        this._fbxTree = fbxTree;
        this._connectionMap_ = connections;
        this._textureObjects_ = _textureObjects_;

        var type = materialNode.ShadingModel;
        // Case where FBX wraps shading model in property object.
        if (typeof type === 'object')
        {
            type = type.value;
        }

        var activeMaterialType = '';
        switch (type.toLowerCase())
        {
            case 'phong':
                activeMaterialType = 'blinnPhong';
                break;
            case 'lambert':
                activeMaterialType = 'dielectric';
                break;
            default:
                console.warn('GASEngine.FBXMaterialDecoder: unknown material type "%s". Defaulting to MeshPhongMaterial.', type);
                return null;
        }

        var compoundMaterial = GASEngine.MaterialFactory.Instance.create('compound');

        var blinnPhongMaterial = GASEngine.MaterialFactory.Instance.create('blinnPhong');
        var dielectricMaterial = GASEngine.MaterialFactory.Instance.create('dielectric');

        compoundMaterial.name = materialNode.name;
        compoundMaterial.visible = true;
        compoundMaterial.uniqueID = GASEngine.generateUUID();

        this._parseBlinnPhong(blinnPhongMaterial, materialNode);
        this._parseDielectric(dielectricMaterial, materialNode);

        compoundMaterial.addMaterial(blinnPhongMaterial);
        compoundMaterial.addMaterial(dielectricMaterial);

        compoundMaterial.setActiveMaterial(activeMaterialType);

        return compoundMaterial;
    },

    _parseTextureMap: function (id)
    {
        // if the texture is a layered texture, just use the first layer and issue a warning
        if ('LayeredTexture' in this._fbxTree.Objects && (id in this._fbxTree.Objects.LayeredTexture))
        {
            console.warn('GASEngine.FBXLoader: layered textures are not supported in GASEngine.js. Discarding all but first layer.');
            id = this._connectionMap_.get(id).children[0].ID;
        }
        return this._textureObjects_.get(id);
    },

    _parseBlinnPhong: function (material, materialNode)
    {
        //BlinnPhong        
        material.name = 'Compound-BlinnPhong';
        material.visible = true;
        material.uniqueID = GASEngine.generateUUID();
        material.culling = GASEngine.Material.CullingOnCCW;
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/BlinnPhongFragment.glsl";

        var maps = [];

        //BaseColor
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.albedoFactor = new Float32Array([1.0]);
        if (materialNode.Diffuse)
        {
            material.albedoColor = new Float32Array(materialNode.Diffuse.value);
        }
        else if (materialNode.DiffuseColor && materialNode.DiffuseColor.type === 'Color')
        {
            material.albedoColor = new Float32Array(materialNode.DiffuseColor.value);
        }
        else
        {
            material.albedoColor = new Float32Array([0.588, 0.588, 0.588]);
        }

        //DisplacementMap
        if ( materialNode.DisplacementFactor ) {
            material.displacementEnable = true;
            material.displacementDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.displacementFactor = new Float32Array(materialNode.DisplacementFactor.value);
        }

        //SpecularMap
        material.specularEnable = true;
        material.specularDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.specularColor = new Float32Array([1.0]);
        if ( materialNode.Specular ) {
            material.specularColor = new Float32Array( materialNode.Specular.value );
        } else if ( materialNode.SpecularColor && materialNode.SpecularColor.type === 'Color' ) {
            // The blender exporter exports specular color here instead of in materialNode.Specular
            material.specularColor = new Float32Array(materialNode.SpecularColor.value);
        }

        //TransparencyMap
        material.transparencyEnable = false;
        if (materialNode.TransparentColor && materialNode.TransparentColor.type === 'Color')
        {
            material.transparencyDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.transparencyAlphaInvert = new Int32Array([0]);
            material.transparencyFactor = new Float32Array([parseFloat(materialNode.TransparentColor.value)]);
            material.transparencyBlendMode = 0;
        }

        //NormalMap
        material.normalEnable = false;
        material.normalDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.normalFactor = new Float32Array([1.0]);
        material.normalFlipY = new Int32Array([0]);

        //EmissiveMap
        material.emissiveEnable = false;
        material.emissiveDefault = new Float32Array([1.0, 1.0, 1.0]);
        if ( materialNode.EmissiveFactor ) {
            material.emissiveFactor = new Float32Array([parseFloat( materialNode.EmissiveFactor.value)]);
        }

        if (materialNode.Emissive)
        {
            material.emissiveColor = new Float32Array(materialNode.Emissive.value);
        }
        else if (materialNode.EmissiveColor && materialNode.EmissiveColor.type === 'color')
        {
            material.emissiveColor = new Float32Array(materialNode.EmissiveColor.value);
        }

        var relationship = this._connectionMap_.get(materialNode.id);
        for (var child of relationship.children)
        {
            var type = child.relationship;
            var textureMap = this._parseTextureMap(child.ID);
            maps.push(textureMap);

            switch (type)
            {
                case 'DiffuseColor':
                    material.albedoEnable = true;
                    material.albedoMap = textureMap;
                    break;
                case 'DisplacementColor':
                    material.displacementEnable = true;
                    material.displacementMap = textureMap;
                    break;
                case 'EmissiveColor':
                    material.emissiveEnable = true;
                    material.emissiveMap = textureMap;
                    break;
                case 'NormalMap':
                    material.normalEnable = true;
                    material.normalMap = textureMap;
                    break;
                case 'ReflectionColor':
                    //material.envMap = textureMap;
                    //material.envMap.mapping = GASEngine.EquirectangularReflectionMapping;
                    break;
                case 'SpecularColor':
                    material.specularEnable = true;
                    material.specularMap = textureMap;
                    break;
                case 'TransparentColor':
                    material.transparencyEnable = true;
                    material.transparencyMap = textureMap;
                    //material.transparent = true;
                    break;
                case 'Bump':
                case 'AmbientColor':
                case 'ShininessExponent': // AKA glossiness map
                case 'SpecularFactor': // AKA specularLevel
                case 'VectorDisplacementColor': // NOTE: Seems to be a copy of DisplacementColor
                default:
                    console.warn('GASEngine.FBXLoader: %s map is not supported, skipping texture.', type);
                    break;
            }
        }
        return maps;
    },

    _parseDielectric: function (material, materialNode)
    {
        //Dielectric        
        material.name = 'Compound-Dielectric';
        material.visible = true;
        material.uniqueID = GASEngine.generateUUID();
        material.culling = GASEngine.Material.CullingOnCCW;
        material.vertexShaderFile = "/system/shaders/PBRVertex.glsl";
        material.fragmentShaderFile = "/system/shaders/PBRFragment.glsl";

        var maps = [];
        
        //BaseColor
        material.albedoEnable = true;
        material.albedoDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.albedoFactor = new Float32Array([1.0]);
        if (materialNode.Diffuse)
        {
            material.albedoColor = new Float32Array(materialNode.Diffuse.value);
        }
        else if (materialNode.DiffuseColor && materialNode.DiffuseColor.type === 'Color')
        {
            material.albedoColor = new Float32Array(materialNode.DiffuseColor.value);
        }
        else
        {
            material.albedoColor = new Float32Array([0.588, 0.588, 0.588]);
        }

        //DisplacementMap
        if ( materialNode.DisplacementFactor ) {
            material.displacementEnable = true;
            material.displacementDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.displacementFactor = new Float32Array(materialNode.DisplacementFactor.value);
        }

        //SpecularMap
        material.specularEnable = true;
        material.specularDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.specularColor = new Float32Array([1.0]);
        if ( materialNode.Specular ) {
            material.specularColor = new Float32Array( materialNode.Specular.value );
        } else if ( materialNode.SpecularColor && materialNode.SpecularColor.type === 'Color' ) {
            // The blender exporter exports specular color here instead of in materialNode.Specular
            material.specularColor = new Float32Array(materialNode.SpecularColor.value);
        }

        //Roughness
        material.roughnessEnable = true;
        material.roughnessDefault = new Float32Array([1.0]);
        material.roughnessFactor = new Float32Array([1.0]);

        //TransparencyMap
        material.transparencyEnable = false;
        if(materialNode.TransparentColor && materialNode.TransparentColor.type === 'Color') {
            material.transparencyDefault = new Float32Array([1.0, 1.0, 1.0]);
            material.transparencyAlphaInvert = new Int32Array([0]);
            material.transparencyFactor = new Float32Array([parseFloat(materialNode.TransparentColor.value)]);
            material.transparencyBlendMode = 0;
        }

        //NormalMap
        material.normalEnable = false;
        material.normalDefault = new Float32Array([1.0, 1.0, 1.0]);
        material.normalFactor = new Float32Array([1.0]);
        material.normalFlipY = new Int32Array([0]);

        //EmissiveMap
        material.emissiveEnable = false;
        material.emissiveDefault = new Float32Array([1.0, 1.0, 1.0]);
        if ( materialNode.EmissiveFactor ) {
            material.emissiveFactor = new Float32Array([parseFloat( materialNode.EmissiveFactor.value)]);
        }

        if (materialNode.Emissive)
        {
            material.emissiveColor = new Float32Array(materialNode.Emissive.value);
        }
        else if (materialNode.EmissiveColor && materialNode.EmissiveColor.type === 'color')
        {
            material.emissiveColor = new Float32Array(materialNode.EmissiveColor.value);
        }

        var relationship = this._connectionMap_.get(materialNode.id);
        for (var child of relationship.children)
        {
            var type = child.relationship;
            var textureMap = this._parseTextureMap(child.ID);
            maps.push(textureMap);

            switch (type)
            {
                case 'DiffuseColor':
                    material.albedoEnable = true;
                    material.albedoMap = textureMap;
                    break;
                case 'DisplacementColor':
                    material.displacementEnable = true;
                    material.displacementMap = textureMap;
                    break;
                case 'EmissiveColor':
                    material.emissiveEnable = true;
                    material.emissiveMap = textureMap;
                    break;
                case 'NormalMap':
                    material.normalEnable = true;
                    material.normalMap = textureMap;
                    break;
                case 'ReflectionColor':
                    //material.envMap = textureMap;
                    //material.envMap.mapping = GASEngine.EquirectangularReflectionMapping;
                    break;
                case 'SpecularColor':
                    material.specularEnable = true;
                    material.specularMap = textureMap;
                    break;
                case 'TransparentColor':
                    material.transparencyEnable = true;
                    material.transparencyMap = textureMap;
                    //material.transparent = true;
                    break;
                case 'Bump':
                case 'AmbientColor':
                case 'ShininessExponent': // AKA glossiness map
                case 'SpecularFactor': // AKA specularLevel
                case 'VectorDisplacementColor': // NOTE: Seems to be a copy of DisplacementColor
                default:
                    console.warn('GASEngine.FBXLoader: %s map is not supported, skipping texture.', type);
                    break;
            }
        }
        return maps;
    }
};