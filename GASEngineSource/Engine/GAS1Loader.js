GASEngine.GAS1Loader = function()
{
    GASEngine.Events.attach(this);

    this.BEC =
    {
        'BG': 
        [
            {
                name: 'background_cubemap_512_0.0_luv.bin',
                size: 512
            },
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
        ],
        'SPEC': 
        [
            {
                name: 'specular_panorama_ue4_1024_luv.bin',
                size: 1024
            },
            {
                name: 'specular_cubemap_ue4_256_luv.bin',
                size: 256
            }
        ],
        'BRDF_UE4': 'brdf_ue4.bin',
        'DIFFUSE_SPH': 'diffuse_sph.json'
    };

    this._displayFileInfo_ = null;
    this._convertedFileCount_ = 0;
    this._displaytFileTotalCount_ = 0;
    this._textureDirectory_ = '';
    this._gasMaterialDecoder_ = new GASEngine.GAS1MaterialDecoder();
    this._gasMeshDecoder_ = new GASEngine.GAS1MeshDecoder();
    this._gasAnimationDecoder_ = new GASEngine.GAS1KeyframeAnimationDecoder();

    this._sceneFile_ = '';
    this._modelStructureFile_ = '';
    this._meshFiles_ = [];
    this._animationFiles_ = [];
    this._textureFiles_ = [];
    this._linkedTextureFiles_ = [];

    //For linkage
    this._meshFilterComponents_ = new Map();
    this._meshObjects_ = new Map();
    this._meshRendererComponents_ = new Map();
    this._materialObjects_ = [];
    this._animatorComponent_ = null;
    this._animationClips_ = new Map();

    this._scene_ = null;
    this._onHierarchySuccess_ = null;
    this._onFinished_ = null;

    this.tmpVec = new GASEngine.Vector3();
    this.tmpEuler = new GASEngine.Euler();

    this._rawDisplayFileList_ = [];
    this._sceneConfigData_ = {};

    this._skyboxList_ = [];
    this._environmentalLightList_ = [];

    this.onSceneLoadCallback = null;
    this.onStructureLoadCallback = null;
    this.onMeshLoadCallback = null;
    this.onAnimationLoadCallback = null;
    this.onMaterialLoadCallback = null;
    this.onTextureLoadCallback = null;

    GASEngine.GAS1Loader.Instance = this;
};

GASEngine.GAS1Loader.prototype =
{
    constructor: GASEngine.GAS1Loader,

    getRawDisplayFileList: function()
    {
        return this._rawDisplayFileList_;
    },

    getSkyboxList: function()
    {
        return this._skyboxList_;
    },

    getEnvironmentalLightList: function()
    {
        return this._environmentalLightList_;
    },

    load: function(scene, modelName, onHierarchySuccess)
    {
        this._scene_ = scene;
        
        this._displayFileInfo_ = null;

        this._onHierarchySuccess_ = onHierarchySuccess;

        this._loadSuccessInfo_ = {};

        this.loadConvertedFiles(modelName, this.parseScene.bind(this));
    },

    loadConvertedFiles: function(modelName, callback)
    {
        GASEngine.FileSystem.Instance.read
        (
            modelName + '.convertedFiles', 
            function(data)
            {
                callback(data);
                //<
            }.bind(this)
        );
    },

    filterGZFiles: function(files)
    {
        const gzFiles = [];
        for(let i = 0, len = files.length; i < len; i++)
        {
            let filename = files[i];
            if(filename.indexOf(".") !== -1 && filename.split(".").pop().indexOf("gz") !== -1) {
                gzFiles.push(filename);
            } else {
                let gzFilename = `${filename}.gz`;
                let fileIndex = files.indexOf(gzFilename);
                if(fileIndex !== -1) {
                    continue;
                } else {
                    gzFiles.push(filename);
                }
            }
        }

        return gzFiles;
    },

    parseScene: function(data)
    {
        this._displayFileInfo_ = data;
        //this._scene_ = new GASEngine.Scene();

        ////////////////////////////////
        if(this._displayFileInfo_.version === 1)
        {
            this._textureDirectory_ = '$textures/';
        }
        else if(this._displayFileInfo_.version === 3)
        {
            this._textureDirectory_ = '/' + this._displayFileInfo_.texturePath;
        }
        else
        {
            this._textureDirectory_ = '$';
        }
        ////////////////////////////////

        var index = -1, fileName = '', i = 0, fullPath = '';
        this._displaytFileTotalCount_ = 0;
        this._convertedFileCount_ = 0;

        const gzFiles = this.filterGZFiles(this._displayFileInfo_.files);
        for(i = 0; i < gzFiles.length; ++i)
        {
            fileName = gzFiles[i];
            index = fileName.indexOf('scene.json');
            if(index !== -1)
            {
                if(!this._sceneFile_) {
                    this._sceneFile_ = fileName;
                    this._displaytFileTotalCount_++;
                } else {
                    if(fileName.split("scene.json").shift() !== '') {
                        this._sceneFile_ = fileName;
                    }
                }
                // fullPath = fileName.substr(0, index) + 'scene.json';
                // this._sceneFile_ = fullPath;
                continue;
            }

            index = fileName.indexOf('structure.json');
            if(index !== -1)
            {
                if(!this._modelStructureFile_) {
                    this._modelStructureFile_ = fileName;
                    this._displaytFileTotalCount_++;
                } else {
                    if(fileName.split("structure.json").shift() !== '') {
                        this._modelStructureFile_ = fileName;
                    }
                }
                // fullPath = fileName.substr(0, index) + 'structure.json';
                // this._modelStructureFile_ = fullPath;
                continue;
            }

            index = fileName.indexOf('mesh.bin');
            if(index !== -1)
            {
                // fullPath = fileName.substr(0, index) + 'mesh.bin';
                // this._meshFiles_.push(fullPath);
                this._meshFiles_.push(fileName);
                this._displaytFileTotalCount_++;
                continue;
            }

            index = fileName.indexOf('animation.bin');
            if(index !== -1)
            {
                // fullPath = fileName.substr(0, index) + 'animation.bin';
                // this._animationFiles_.push(fullPath);
                this._animationFiles_.push(fileName);
                this._displaytFileTotalCount_++;
                continue;
            }

            index = fileName.indexOf('convertedFiles');
            if(index !== -1)
            {
                continue;
            }

            this._textureFiles_.push(fileName);
        }

        //Load Scene Config
        if(this._sceneFile_ !== '')
        {
            this._loadSceneFile(this._sceneFile_);
        }

        //Load Model Structure
        if(this._modelStructureFile_ !== '')
        {
            this._loadModelStructureFile(this._modelStructureFile_);
        }       

        //Load Mesh Binaries
        for(i = 0; i < this._meshFiles_.length; ++i)
        {
            fullPath = this._meshFiles_[i];
            this._loadMeshBinary(fullPath);
        }

        //Load Animation Binaries
        for(i = 0; i < this._animationFiles_.length; ++i)
        {
            fullPath = this._animationFiles_[i];
            this._loadAnimationBinary(fullPath);
        }
    },

    //TODO: error callback
    beforeCallHierarchySuccess: function(fileName)
    {
        this._loadSuccessInfo_[fileName] = true;
        if(this._loadSuccessInfo_.scene && this._loadSuccessInfo_.structure) {
            this._onHierarchySuccess_(this._scene_);
            if(this.onStructureLoadCallback) 
            {
                this.onStructureLoadCallback();
            }
        }
    },

    onFileLoaded: function(params)
    {
        console.log(params);
    },

    calcFileLoadedCount: function()
    {
        this._convertedFileCount_++;
        this.onFileLoaded({count: this._convertedFileCount_, total: this._displaytFileTotalCount_});
    },

    // getAssembleConfigurations: function()
    // {
    //     let sceneName = this._rawDisplayFileList_.find(name => name.indexOf("scene.json") >= 0);
    //     let sceneConfigData = this._sceneConfigData_;
    //     let hotspots = GASEngine.HotspotManager.Instance.getHotspots();
    //     sceneConfigData.hotspots = hotspots;
    //     return {name: sceneName, scene: sceneConfigData};
    //     // var configurations = this._scene_.__configurations;
    //     // var convertedFiles = this._scene_.__convertedFiles;
    //     // var mapIDs = H5Engine_V1.MaterialManager.Instance.gatherUsedMapID();
    //     // var textureInternalNameDepot = {};
    //     // var maps = [];
    //     // for(var i = 0; i < mapIDs.length; ++i)
    //     // {
    //     //     var mapParam = H5Engine_V1.MaterialMapManager.Instance.getMaterialMapParam(mapIDs[i]);
    //     //     if(mapParam.texture)
    //     //     {
    //     //         textureInternalNameDepot[mapParam.texture] = 1;
    //     //     }
    //     //     maps.push(mapParam);
    //     // }
    //     // // Always make static files linked.
    //     // var linkedFiles = [];
    //     // var staticFileRegex = /((scene\.json)|([\S]*(\.?structure\.json|\.mesh\.bin|\.animation\.bin|\.convertedFiles)))(.gz)?$/i;
    //     // var convertedFileIndex = -1;
    //     // for(var i = 0; i < convertedFiles.length; ++i)
    //     // {
    //     //     if (staticFileRegex.test(convertedFiles[i]))
    //     //     {
    //     //       linkedFiles.push(convertedFiles[i]);
    //     //     }
    //     // }
    //     // var textures = [];
    //     // var localTextures = [];
    //     // var textureInternalNames = Object.keys(textureInternalNameDepot);
    //     // for(var i = 0; i < textureInternalNames.length; ++i)
    //     // {
    //     //     var name = textureInternalNames[i];
    //     //     var textureParam = H5Engine_V1.TextureManager.Instance.getTextureParam(name);
    //     //     textures.push(textureParam);
    //     //     var textureRecord = H5Engine_V1.TextureManager.Instance.getTextureRecord(name);
    //     //     if(textureRecord.localTexture)
    //     //     {
    //     //         localTextures.push(name);
    //     //     }
    //     //     linkedFiles.push(name);
    //     // }

    //     // // Save animation clamped ranges
    //     // var animationClips = H5Engine_V1.Scene.Instance.__animationClips;
    //     // if(animationClips.length > 0)
    //     // {
    //     //     configurations.animationRanges = [];
    //     // }

    //     // for(var clipIndex = 0; clipIndex < animationClips.length; ++clipIndex)
    //     // {
    //     //     var clip = animationClips[clipIndex];
    //     //     configurations.animationRanges.push
    //     //     (
    //     //         { "clipID": clip.clipID, "clampedSF": clip.clampedSF, "clampedEF": clip.clampedEF }
    //     //     );
    //     // }
    //     // ////////////////////////////////////////////////
    //     // configurations.maps = maps;
    //     // configurations.textures = textures;
    //     // //configurations.skinningFlag = H5Renderer_V1.Renderer.Instance.skinningFlag;
    //     // configurations.hotspots = H5Engine_V1.HotspotManager.Instance.getHotspots();
    //     // var sceneConfigurationString = JSON.stringify(configurations);
    //     // var convertedFileString = JSON.stringify
    //     // (
    //     //     { 
    //     //         'files': linkedFiles, 
    //     //         'statistics': H5Engine_V1.Scene.Instance.__statistics,
    //     //         'version': H5Engine_V1.Scene.Instance.__convertedFilesVersion,
    //     //         'texture_path': H5Engine_V1.Scene.Instance.__texturePath
    //     //     }
    //     // );
    //     // return {
    //     //     'scene.json': sceneConfigurationString,
    //     //     'convertedFiles': convertedFileString,
    //     //     'textures': localTextures,
    //     // };    
    // },

    _loadSceneFile: function(path)
    {
        GASEngine.FileSystem.Instance.read
        (
            path, 

            function(data, ext)
            {
                this._sceneConfigData_ = data;

                // var backgroundEntity = this._parseBackground(data);
                // this._scene_.appendEntityOnRoot(backgroundEntity);
                // this._skyboxList_.push(backgroundEntity);

                // var environmentalLightEntity = this._parseEnvironmentalLight(data);
                // this._scene_.appendEntityOnRoot(environmentalLightEntity);
                // this._environmentalLightList_.push(environmentalLightEntity);

                this._parseMaterialConfig(data);

                this._parseHotspotConfig(data);

                this._linkScene();

                this.beforeCallHierarchySuccess('scene');

                this.calcFileLoadedCount();

                if(this.onSceneLoadCallback)
                {
                    this.onSceneLoadCallback();
                }

            }.bind(this),

            null,

            function(path, ext)
            {
            }.bind(this)
        );
    },

    _loadModelStructureFile: function(path)
    {
        GASEngine.FileSystem.Instance.read
        (
            path, 

            function(data, ext)
            {
                var modelRoot = this._parseModelHierarchyStructure(data);

                if(this._animationFiles_.length > 0)
                {
                    var animatorComponent = GASEngine.ComponentFactory.Instance.create('animator', -1);
                    modelRoot.addComponent(animatorComponent);

                    this._animatorComponent_ = animatorComponent;
                }

                this._scene_.setModelRoot(modelRoot);
                this._scene_.appendEntityOnRoot(modelRoot);

                this._linkScene();

                this.beforeCallHierarchySuccess('structure');

                this.calcFileLoadedCount();

            }.bind(this),

            null,

            function(path, ext)
            {
            }.bind(this)
        );
    },

    _loadMeshBinary: function(path)
    {
        GASEngine.FileSystem.Instance.read
        (
            path, 

            function(data, ext)
            {
                var dataView = new DataView(data);

                var meshObject = this._gasMeshDecoder_.parse(dataView);

                meshObject.submitToWebGL();
                var uniqueID = meshObject.uniqueID;
                this._meshObjects_.set(uniqueID, meshObject);

                this._linkScene();

                this.calcFileLoadedCount();

            }.bind(this),

            null,

            function(path, ext)
            {
            }.bind(this)
        );
    },

    _loadAnimationBinary: function(path)
    {
        GASEngine.FileSystem.Instance.read
        (
            path, 

            function(data, ext)
            {
                var dataView = new DataView(data);

                var animationClip = this._gasAnimationDecoder_.parse(dataView);
                // if(animationClip.clipName === '')
                // {
                //     var index = fileName.indexOf('.animation.bin');
                //     var animationPartName = fileName.substr(0, index);
                // }

                var animationRanges = this._displayFileInfo_.animationRanges;                            
                if(animationRanges)
                {
                    for(var rangeIndex = 0; rangeIndex < animationRanges.length; ++rangeIndex)
                    {
                        var rangeInfo = animationRanges[rangeIndex];
                        if(rangeInfo.clipID === animationClip.clipID)
                        {
                            animationClip.clampedSF = rangeInfo.clampedSF;
                            animationClip.clampedEF = rangeInfo.clampedEF;
                            break;
                        }
                    }
                }

                this._animationClips_.set(animationClip.clipID, animationClip);

                this._linkScene();

                this.calcFileLoadedCount();

                if(this.onAnimationLoadCallback)
                {
                    this.onAnimationLoadCallback();
                }

            }.bind(this),

            null,

            function(path, ext)
            {
            }.bind(this)
        );
    },

    _linkScene: function()
    {
        //TODO: need more efficient. need to support same mesh on multiple mesh filters.
        
        //link mesh
        var linkMeshes = [];
        for (var meshName of this._meshFilterComponents_.keys()) 
        {
            var mesh = this._meshObjects_.get(meshName);
            if(mesh !== undefined)
            {
                var meshFilter = this._meshFilterComponents_.get(meshName);
                meshFilter.setMesh(mesh);

                if(mesh.isSkinned())
                {
                    var meshParentEntity = meshFilter.getParentEntity();
                    mesh.linkBones(meshParentEntity);
                }

                var bones = mesh.getBones();
                //GASEngine.SkeletonManager.Instance.appendBones(bones);

                linkMeshes.push(meshName);
            }
        }

        for(var i = 0; i < linkMeshes.length; ++i)
        {
            this._meshFilterComponents_.delete(linkMeshes[i]);
            this._meshObjects_.delete(linkMeshes[i]);
        }

        //link material
        if(this._materialObjects_.length > 0)
        {
            for (var meshRenderer of this._meshRendererComponents_.keys()) 
            {
                var materialIndices = this._meshRendererComponents_.get(meshRenderer);
                for(var i = 0; i < materialIndices.length; ++i)
                {
                    var k = materialIndices[i];
                    var material = this._materialObjects_[k];
                    meshRenderer.addMaterial(material);
                }
                this._meshRendererComponents_.set(meshRenderer, []);
            }
        }

        //link animation
        if(this._animatorComponent_ !== null && this._animationClips_.size > 0)
        {
            for (var clip of this._animationClips_.values()) 
            {
                this._animatorComponent_.setAnimationClip(clip.name, clip);
                this._animatorComponent_.play(clip.id);
            }
            this._animationClips_.clear();
        }
    },

    /////////////////////////////////////////////////////////////////////
    _parseBackground: function(sceneConfig)
    {
        //BG
        if(sceneConfig.background !== undefined)
        {
            var backgroundType = 'IMAGE'; //CUBEMAP';
            // switch(sceneConfig.background.type)
            // {
            //     case 0: backgroundType = 'CUBEMAP'; break;
            //     case 1: backgroundType = 'IMAGE'; break;
            //     case 2: backgroundType = 'SOLIDCOLOR'; break;
            //     case 3: backgroundType = 'AMBIENT'; break;
            //     default: backgroundType = ''; break;
            // } 

            var mesh = GASEngine.MeshFactory.Instance.create();
            mesh.addStream('position', new Float32Array([1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0]));
            mesh.addStream('subMesh', [{ 'start': 0, 'count': 4 }]);
            mesh.setDrawMode('TRIANGLE_STRIP');
            mesh.submitToWebGL();
    
            var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
            meshFilterComponent.setMesh(mesh);            
        
            var material = GASEngine.MaterialFactory.Instance.create('skybox');
            material.backgroundType = backgroundType;

            //Environmental background cube
            if(material.backgroundType === 'CUBEMAP')
            {
                var bg = this.BEC.BG[sceneConfig.background.environmentIndex];
                var environmentBGSize = bg.size;

                var environmentBGCubeFullPath = ''; 
                if(sceneConfig.background.environment.indexOf('$') === 0)
                {                
                    environmentBGCubeFullPath = sceneConfig.background.environment + '/' + bg.name;
                }
                else
                {
                    environmentBGCubeFullPath = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + bg.name;
                }

                GASEngine.Resources.Instance.loadCubeTexture
                (
                    environmentBGCubeFullPath,
                    environmentBGSize,
                    function(texture, size)
                    {
                        material.setCubeMap(texture, size);
                    }
                );
            }
            else if(material.backgroundType === 'IMAGE')
            {
                //Environmental background image
                var environmentBGImageFullPath = ''; 
                if(sceneConfig.background.environment.indexOf('$') === 0)
                {                
                    environmentBGImageFullPath = sceneConfig.background.image;
                }
                else
                {
                    environmentBGImageFullPath = '/system/backgroundImages/' + sceneConfig.background.image;
                }

                GASEngine.Resources.Instance.loadTexture
                (
                    environmentBGImageFullPath,
                    true,
                    function(webglTexture, width, height)
                    {
                        material.setImage(webglTexture, width, height);
                    },
                    null,
                    null
                );
            }
            else if(material.backgroundType === 'SOLIDCOLOR')
            {
                //Environmental background solid color
                var solidColor = sceneConfig.background.color;
                material.setSolidColor(solidColor[0], solidColor[1], solidColor[2]);
            }
            else if(material.backgroundType === 'AMBIENT')
            {
                //Environmental background ambient
                var environmentBGAmbientFullPath = ''; 
                if(sceneConfig.background.environment.indexOf('$') === 0)
                {                
                    environmentBGAmbientFullPath = sceneConfig.background.environment + '/' + this.BEC.DIFFUSE_SPH;
                }
                else
                {
                    environmentBGAmbientFullPath = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + this.BEC.DIFFUSE_SPH;
                }

                GASEngine.Resources.Instance.loadSPH
                (
                    environmentBGAmbientFullPath,
                    function (presetExposure, sph)
                    {
                        material.setSPH(presetExposure, sph);
                    }
                );
            }
        
            var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
            meshRendererComponent.addMaterial(material);
        
            var backgroundEntity = GASEngine.EntityFactory.Instance.create();
            backgroundEntity.name = 'Skybox';
            backgroundEntity.addComponent(meshFilterComponent);
            backgroundEntity.addComponent(meshRendererComponent);
            
            return backgroundEntity;
        }
    },

    _parseEnvironmentalLight: function(sceneConfig)
    {
        //create environmental light
        var environmentalLightComponent = GASEngine.ComponentFactory.Instance.create('environmentalLight');

        var specularPanoramaConfig = this.BEC.SPEC[0];   
        var specularCubeConfig = this.BEC.SPEC[1];

        var fullSpecularPanoramaPath = '';
        var fullSpecularCubePath = '';
        var fullSpecularSPH = '';
        var fullSpecularBRDF_UE4 = '';
        if(sceneConfig.background.environment.indexOf('$') === 0)
        {                
            fullSpecularPanoramaPath = sceneConfig.background.environment + '/' + specularPanoramaConfig.name;
            fullSpecularCubePath = sceneConfig.background.environment + '/' + specularCubeConfig.name;
            fullSpecularSPH = sceneConfig.background.environment + '/' + this.BEC.DIFFUSE_SPH;
            fullSpecularBRDF_UE4 = sceneConfig.background.environment + '/' + this.BEC.BRDF_UE4;
        }
        else
        {
            fullSpecularPanoramaPath = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + specularPanoramaConfig.name;
            fullSpecularCubePath = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + specularCubeConfig.name;
            fullSpecularSPH = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + this.BEC.DIFFUSE_SPH;
            fullSpecularBRDF_UE4 = '/system/backgroundCubes/' + sceneConfig.background.environment + '/' + this.BEC.BRDF_UE4;
        }
    
        GASEngine.Resources.Instance.loadCubeTexture(
            fullSpecularCubePath,
            specularCubeConfig.size,
            function (texture, width, height)
            {
                environmentalLightComponent.setSpecularCubeMap(texture, width, height);
            });
    
        // GASEngine.Resources.Instance.loadPanorama(
        //     fullSpecularPanoramaPath,
        //     function (texture, width, height)
        //     {
        //         environmentalLightComponent.setSpecularPanoramaTexture(texture, width, height);
        //     });
    
        GASEngine.Resources.Instance.loadSPH(
            fullSpecularSPH,
            function (presetExposure, sph)
            {
                environmentalLightComponent.setDiffuseSPH(presetExposure, sph);
            });
    
        GASEngine.Resources.Instance.loadIntegratedBRDF(
            fullSpecularBRDF_UE4,
            function (spcularIntegratedBRDF, width, height)
            {
                environmentalLightComponent.setSpecularIntegratedBRDF(spcularIntegratedBRDF);
            });
    
        var environmentalLightEntity = GASEngine.EntityFactory.Instance.create();
        environmentalLightEntity.name = 'EnvironmentalLight';
        environmentalLightEntity.addComponent(environmentalLightComponent);

        return environmentalLightEntity;
    },

    _parseMaterialConfig: function(sceneConfig)
    {
        var materialConfigs = sceneConfig.materials;
        var mapConfigs = sceneConfig.maps;
        var textureConfigs = sceneConfig.textures;

        var materials = this._gasMaterialDecoder_.parse(materialConfigs, mapConfigs, textureConfigs);
        for(var i = 0; i < materials.length; ++i)
        {
            var maps = [];
            materials[i].getLinkedTextureMaps(maps);
            for(var j = 0; j < maps.length; ++j)
            {
                GASEngine.Resources.Instance.loadTextureOnMaterial(maps[j]);
            }
        }

        this._materialObjects_ = materials;//TODO:
    },

    _parseHotspotConfig: function(sceneConfig)
    {
        if(sceneConfig.hotspots)
        {
            GASEngine.HotspotManager.Instance.setHotspots(sceneConfig.hotspots);
        }
        
    },

    _parseModelHierarchyStructure: function(data)
    {        
        var root = this._walkHierarchy_r(data.nodeTree, null);
        return root;
    },

    _walkHierarchy_r: function(nodeConfig, parent)
    {
        var entity = GASEngine.EntityFactory.Instance.create();
        entity.name = nodeConfig.name;
        entity.uniqueID = nodeConfig.uniqueID;
        entity.guid = nodeConfig.guid;

        if(nodeConfig.MB_PROPS)
        {
            var MB_PROPS = nodeConfig.MB_PROPS;
            entity.MB_PROPS = {};
            entity.MB_PROPS.Visibility = MB_PROPS.Visibility

            // Scale
            var SP = new GASEngine.Vector3(MB_PROPS.ScalingPivot[0], MB_PROPS.ScalingPivot[1], MB_PROPS.ScalingPivot[2]);
            var SP_INV = new GASEngine.Vector3(-MB_PROPS.ScalingPivot[0], -MB_PROPS.ScalingPivot[1], -MB_PROPS.ScalingPivot[2]);
            entity.MB_PROPS.__Sp_M = new GASEngine.Matrix4();
            entity.MB_PROPS.__Sp_M.setPosition(SP);

            entity.MB_PROPS.__Sp_M_INV = new GASEngine.Matrix4();
            entity.MB_PROPS.__Sp_M_INV.setPosition(SP_INV);

            entity.MB_PROPS.__Soff_M = new GASEngine.Matrix4();
            var SOFF = new GASEngine.Vector3(MB_PROPS.ScalingOffset[0], MB_PROPS.ScalingOffset[1], MB_PROPS.ScalingOffset[2]);
            entity.MB_PROPS.__Soff_M.setPosition(SOFF);

            // Rotation
            var RP = new GASEngine.Vector3(MB_PROPS.RotationPivot[0], MB_PROPS.RotationPivot[1], MB_PROPS.RotationPivot[2]);
            var RP_INV = new GASEngine.Vector3(-MB_PROPS.RotationPivot[0], -MB_PROPS.RotationPivot[1], -MB_PROPS.RotationPivot[2]);
            entity.MB_PROPS.__Rp_M = new GASEngine.Matrix4();
            entity.MB_PROPS.__Rp_M.setPosition(RP);

            entity.MB_PROPS.__Rp_M_INV = new GASEngine.Matrix4();
            entity.MB_PROPS.__Rp_M_INV.setPosition(RP_INV);

            entity.MB_PROPS.__Roff_M = new GASEngine.Matrix4();
            var ROFF = new GASEngine.Vector3(MB_PROPS.RotationOffset[0], MB_PROPS.RotationOffset[1], MB_PROPS.RotationOffset[2]);
            entity.MB_PROPS.__Roff_M.setPosition(ROFF);

            var PreRotationX = GASEngine.degToRad(MB_PROPS.PreRotation[0]);
            var PreRotationY = GASEngine.degToRad(MB_PROPS.PreRotation[1]);
            var PreRotationZ = GASEngine.degToRad(MB_PROPS.PreRotation[2]);
            var EULER0 = new GASEngine.Euler(PreRotationX, PreRotationY, PreRotationZ, 'ZYX');
            entity.MB_PROPS.__Rpre_M = new GASEngine.Matrix4();
            entity.MB_PROPS.__Rpre_M.makeRotationFromEuler(EULER0);

            var PostRotationX = GASEngine.degToRad(MB_PROPS.PostRotation[0]);
            var PostRotationY = GASEngine.degToRad(MB_PROPS.PostRotation[1]);
            var PostRotationZ = GASEngine.degToRad(MB_PROPS.PostRotation[2]);
            var EULER1 = new GASEngine.Euler(PostRotationX, PostRotationY, PostRotationZ, 'ZYX');
            var Rpost_M = new GASEngine.Matrix4();
            Rpost_M.makeRotationFromEuler(EULER1);
            entity.MB_PROPS.__Rpost_M_INV = new GASEngine.Matrix4();
            entity.MB_PROPS.__Rpost_M_INV.getInverse(Rpost_M);

            //<
            entity.MB_PROPS.__S = new GASEngine.Matrix4();
            entity.MB_PROPS.__R = new GASEngine.Matrix4();
            entity.MB_PROPS.__T = new GASEngine.Matrix4();

            entity.MB_PROPS.__TMP0 = new GASEngine.Matrix4();
            entity.MB_PROPS.__TMP1 = new GASEngine.Matrix4();

            entity.MB_PROPS.__PREMULT_0 = new GASEngine.Matrix4();
            entity.MB_PROPS.__TMP0.multiplyMatrices(entity.MB_PROPS.__Soff_M, entity.MB_PROPS.__Sp_M);
            entity.MB_PROPS.__TMP1.multiplyMatrices(entity.MB_PROPS.__Rp_M_INV, entity.MB_PROPS.__TMP0);
            entity.MB_PROPS.__PREMULT_0.multiplyMatrices(entity.MB_PROPS.__Rpost_M_INV, entity.MB_PROPS.__TMP1);

            entity.MB_PROPS.__PREMULT_1 = new GASEngine.Matrix4();
            entity.MB_PROPS.__TMP0.multiplyMatrices(entity.MB_PROPS.__Rp_M, entity.MB_PROPS.__Rpre_M);
            entity.MB_PROPS.__PREMULT_1.multiplyMatrices(entity.MB_PROPS.__Roff_M, entity.MB_PROPS.__TMP0);
        }        

        if (nodeConfig.MAX_PROPS) {
            var MAX_PROPS = nodeConfig.MAX_PROPS;

            entity.MAX_PROPS = {};

            entity.MAX_PROPS.__Sa_M = new GASEngine.Matrix4();
            var Sa = new GASEngine.Quaternion(
                MAX_PROPS.ScalingAxis[0],
                MAX_PROPS.ScalingAxis[1],
                MAX_PROPS.ScalingAxis[2],
                MAX_PROPS.ScalingAxis[3]
            );
            entity.MAX_PROPS.__Sa_M.makeRotationFromQuaternion(Sa);
            entity.MAX_PROPS.__Sa_M_INV = new GASEngine.Matrix4();
            entity.MAX_PROPS.__Sa_M_INV.getInverse(entity.MAX_PROPS.__Sa_M);

            entity.MAX_PROPS.__OT_M = new GASEngine.Matrix4();
            var OT = new GASEngine.Vector3(MAX_PROPS.OffsetTranslation[0], MAX_PROPS.OffsetTranslation[1], MAX_PROPS.OffsetTranslation[2]);
            entity.MAX_PROPS.__OT_M.setPosition(OT);

            entity.MAX_PROPS.__OS_M = new GASEngine.Matrix4();
            var OS = new GASEngine.Vector3(MAX_PROPS.OffsetScaling[0], MAX_PROPS.OffsetScaling[1], MAX_PROPS.OffsetScaling[2]);
            entity.MAX_PROPS.__OS_M.scale(OS);

            entity.MAX_PROPS.__OR_M = new GASEngine.Matrix4();
            var OR = new GASEngine.Quaternion(
                MAX_PROPS.OffsetRotation[0],
                MAX_PROPS.OffsetRotation[1],
                MAX_PROPS.OffsetRotation[2],
                MAX_PROPS.OffsetRotation[3]
            );
            entity.MAX_PROPS.__OR_M.makeRotationFromQuaternion(OR);

            entity.MAX_PROPS.__OSa_M = new GASEngine.Matrix4();
            var OSa = new GASEngine.Quaternion(
                MAX_PROPS.OffsetScalingAxis[0],
                MAX_PROPS.OffsetScalingAxis[1],
                MAX_PROPS.OffsetScalingAxis[2],
                MAX_PROPS.OffsetScalingAxis[3]
            );
            entity.MAX_PROPS.__OSa_M.makeRotationFromQuaternion(OSa);
            entity.MAX_PROPS.__OSa_M_INV = new GASEngine.Matrix4();
            entity.MAX_PROPS.__OSa_M_INV.getInverse(entity.MAX_PROPS.__OSa_M);

            //<
            entity.MAX_PROPS.__S = new GASEngine.Matrix4();
            entity.MAX_PROPS.__R = new GASEngine.Matrix4();
            entity.MAX_PROPS.__T = new GASEngine.Matrix4();
            entity.MAX_PROPS.__TMP0 = new GASEngine.Matrix4();
            entity.MAX_PROPS.__TMP1 = new GASEngine.Matrix4();

            entity.MAX_PROPS.__PREMULT_0 = new GASEngine.Matrix4(); // the offset matrix
            entity.MAX_PROPS.__TMP0.multiplyMatrices(entity.MAX_PROPS.__OS_M, entity.MAX_PROPS.__OSa_M);
            entity.MAX_PROPS.__TMP1.multiplyMatrices(entity.MAX_PROPS.__OSa_M_INV, entity.MAX_PROPS.__TMP0);
            entity.MAX_PROPS.__TMP0.multiplyMatrices(entity.MAX_PROPS.__OR_M, entity.MAX_PROPS.__TMP1);
            entity.MAX_PROPS.__PREMULT_0.multiplyMatrices(entity.MAX_PROPS.__OT_M, entity.MAX_PROPS.__TMP0);
        }
       
        // entity.translation.set(nodeConfig.translation[0], nodeConfig.translation[1], nodeConfig.translation[2]);
        this.tmpVec.set(nodeConfig.translation[0], nodeConfig.translation[1], nodeConfig.translation[2]);
        entity.setLocalTranslation(this.tmpVec);

        var rx = GASEngine.degToRad(nodeConfig.rotation[0]);
        var ry = GASEngine.degToRad(nodeConfig.rotation[1]);
        var rz = GASEngine.degToRad(nodeConfig.rotation[2]);
        this.tmpEuler.set(rx, ry, rz, 'ZYX');
        entity.setLocalRotation(this.tmpEuler);
        // entity.rotation.set(rx, ry, rz, 'ZYX');
        
        // entity.scale.set(nodeConfig.scaling[0], nodeConfig.scaling[1], nodeConfig.scaling[2]);
        this.tmpVec.set(nodeConfig.scaling[0], nodeConfig.scaling[1], nodeConfig.scaling[2]);
        entity.setLocalScale(this.tmpVec);

        if(parent !== null)
        {
            parent.addChild(entity);
        }      

        if(nodeConfig.nodeAttr != undefined)
        {
            if(nodeConfig.nodeAttr.type == 'mesh')
            {
                var bbox = nodeConfig.nodeAttr.bbox;
                var uniqueID = nodeConfig.nodeAttr.uniqueID;

                var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter', uniqueID);
                meshFilter.bbox = new GASEngine.AABB();
                meshFilter.bbox.min.set(bbox[0][0], bbox[0][1], bbox[0][2]);
                meshFilter.bbox.max.set(bbox[1][0], bbox[1][1], bbox[1][2]);
                entity.addComponent(meshFilter);

                this._meshFilterComponents_.set(uniqueID, meshFilter);
            }

            var materialIndices = nodeConfig.materials.slice();
            if(materialIndices.length > 0)
            {
                var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer', -1);
                entity.addComponent(meshRenderer);

                this._meshRendererComponents_.set(meshRenderer, materialIndices);
            }
        }

        for(var i = 0; i < nodeConfig.children.length; i++)
        {
            var child = nodeConfig.children[i];
            this._walkHierarchy_r(child, entity);
        }

        return entity;
    }
};