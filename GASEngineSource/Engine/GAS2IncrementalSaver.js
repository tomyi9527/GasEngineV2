GASEngine.GAS2IncrementalSaver = function()
{
    if(GASEngine.GAS2IncrementalSaver.Instance) {
        return GASEngine.GAS2IncrementalSaver.Instance;
    }
    GASEngine.Events.attach(this);
    GASEngine.GAS2IncrementalSaver.Instance = this;
};

GASEngine.GAS2IncrementalSaver.prototype =
{
    constructor: GASEngine.GAS2IncrementalSaver,

    init: function() {
        this._streamsMap = new Map();

        this._materialMappingTable = {
            name: 'name',
            uniqueID: 'uniqueID',
            culling: 'culling',
            activeMaterialType: 'activeMaterial',
            // albedo
            albedoEnable: 'albedo/enable',
            albedoColor: 'albedo/tint',
            albedoFactor: 'albedo/factor',
            albedoMapPath: 'albedo/map/texture',
            // specular
            specularEnable: 'specular/enable',
            specularFactor: 'specular/factor',
            specularMapPath: 'specular/map/texture',
            // metalness
            metalnessEnable: 'metalness/enable',
            metalnessFactor: 'metalness/factor',
            metalnessMapPath: 'metalness/map/texture',
            // specularF0
            specularF0Enable: 'specularF0/enable',
            specularF0Factor: 'specularF0/factor',
            specularF0MapPath: 'specularF0/map/texture',
            // roughness and glossiness
            roughnessEnable: 'roughness/enable',
            roughnessFactor: 'roughness/factor',
            roughnessMapPath: 'roughness/map/texture',
            glossinessEnable: 'glossiness/enable',
            glossinessFactor: 'glossiness/factor',
            glossinessMapPath: 'glossiness/map/texture',
            // ao
            aoEnable: 'ao/enable',
            aoFactor: 'ao/factor',
            aoOccludeSpecular: 'ao/occludeSpecular',
            aoMapPath: 'ao/map/texture',
            // cavity
            cavityEnable: 'cavity/enable',
            cavityFactor: 'cavity/factor',
            cavityMapPath: 'cavity/map/texture',
            // normal Map
            normalEnable: 'normal/enable',
            normalFlipY: 'normal/map/flipY',
            normalFactor: 'normal/factor',
            normalMapPath: 'normal/map/texture',
            // transparency/opacity
            opacityEnable: 'transparency/enable',
            opacityAlphaInvert: 'transparency/invert',
            opacityFactor: 'transparency/factor',
            opacityMapPath: 'transparency/map/texture',
        };

        this._rendererMappingTable = {
            renderMode: 'renderMode',
            wireframeEnable: 'wireframe/enable',
            wireframeColor: 'wireframe/color',
            wireframeOpacity: 'wireframe/opacity',
            skeletonEnable: 'skeleton/enable',
            skeletonScale: 'skeleton/scale',
            tposeEnable: 'tpose/enable',
        };

        this._sceneStructureMappingTable = {
            environmentName: 'nodeTree/children/0/components/environmentalLight/name',
            environmentOrientation: 'nodeTree/children/0/components/environmentalLight/orientation',
            environmentExposure: 'nodeTree/children/0/components/environmentalLight/exposure',
            backgroundType: 'nodeTree/children/1/components/skybox/backgroundType',
            backgroundExposure: 'nodeTree/children/1/components/skybox/backgroundExposure',
            backgroundCubeName: 'nodeTree/children/1/components/skybox/environment/name',
            backgroundCubeOrientation: 'nodeTree/children/1/components/skybox/environment/orientation',
            backgroundCubeExposure: 'nodeTree/children/1/components/skybox/environment/exposure',
            backgroundCubeAmbientEnable: 'nodeTree/children/1/components/skybox/environment/ambient',
            environmentBlur: 'nodeTree/children/1/components/skybox/environment/blur',
            backgroundImage: 'nodeTree/children/1/components/skybox/image',
            backgroundColor: 'nodeTree/children/1/components/skybox/color',
        };

        this._structureMappingTable = {
            name: 'name',
            uniqueID: 'uniqueID',
            animator: 'nodeTree/components/animator/clips',
            mesh: '',//mesh visible
            node: 'nodeTree'//nodeDelete, nodeReplace
        };
    },

    destroy: function() {
        for(let key of this._streamsMap.keys()) {
            const map = this._streamsMap.get(key);
            map && map.clear();
        }
        this._streamsMap.clear();
    },

    _getMaterialFullPath: function(material) {
        const filePath = GASEngine.Resources.Instance.getMaterialPath(material);
        const fullPath = GASEngine.FileSystem.Instance.getFullPathByPath(filePath);
        return fullPath;
    },

    _addToMap: function(url, key, value) {
        let fileMap;
        if(this._streamsMap.has(url)) {
            fileMap = this._streamsMap.get(url);
        } else {
            fileMap = new Map();
            this._streamsMap.set(url, fileMap);
        }
        fileMap.set(key, value);
        // console.log('saverSet:', url, '-----; key: ', key, '-----; vlaue: ', value);
    },

    // Material. new if property not exist
    updateMaterialPropertyValue: function(material, materialID, propertyPath, propertyValue)
    {
        const mappingPath = this._materialMappingTable[propertyPath];
        if(!mappingPath) return;

        const filePath = this._getMaterialFullPath(material);

        let typeName = material.typeName;
        let fullPropertyPath = mappingPath;
        if(typeName === 'compound') {

            const activeMaterial = material.getActiveMaterial();
            typeName = activeMaterial.typeName;
            fullPropertyPath = `${typeName}/${mappingPath}`;
        }
       
        const key = `${materialID}/${fullPropertyPath}`;

        this._addToMap(filePath, key, propertyValue);
    },

    // Scene Structure
    // "w": fail if exist
    // "w+": override if exist
    createSceneStructure: function(model, mode) 
    {
        return false;
    },

    deleteSceneStructureProperty: function(model, objectUniqueID, propertyPath)
    {
        return false;
    },

    updateSceneStructurePropertyValue: function(sceneStructureUrl, propertyPath, propertyValue)
    {
        const mappingPath = this._sceneStructureMappingTable[propertyPath];
        if(!mappingPath) return;
        const key = `sceneStructureID/${mappingPath}`;
        this._addToMap(sceneStructureUrl, key, propertyValue);
    },

    getSceneStructurePropertyValue: function(model, objectUniqueID, propertyPath, propertyValue)
    {
        return false;
    },

    // Model Structure. new if property not exist
    updateModelStructurePropertyValue: function(model, modelName, modelID, objectUniqueID, propertyPath, propertyValue)
    {
        return false;
    },

    // Renderer
    // "w": fail if exist
    // "w+": override if exist
    createRenderer: function(model, mode)
    {
        return false;
    },

    deleteRendererProperty: function(model, propertyPath)
    {
        return false;
    },

    updateRendererPropertyValue: function(renderConfUrl, propertyPath, propertyValue)
    {
        const mappingPath = this._rendererMappingTable[propertyPath];
        if(!mappingPath) return;
        const key = `renderer/${mappingPath}`;
        this._addToMap(renderConfUrl, key, propertyValue);
    },

    getRendererPropertyValue: function(model, propertyPath, propertyValue)
    {
        return false;
    },

    // Submit change
    applyChange: function()
    {
        console.log(this._streamsMap)
        this._streamsMap.clear();
        return Promise.resolve(true);
    }

    // Hotspot
    // updateHotspot: function()
    // {
    //     return false;
    // },
}