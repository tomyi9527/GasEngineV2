GASEngine.GAS2Loader = function()
{
    GASEngine.Events.attach(this); 
    this.tmpEuler = new GASEngine.Euler();
    this.tmpVec = new GASEngine.Vector3();

    this._rawDisplayFileList_ = [];
    this._signedFileList_ = [];
    this._skyboxList_ = [];
    this._environmentalLightList_ = [];

    this._scene_ = null;

    this.onStructureLoadCallback = null;
    this.onMeshLoadCallback = null;
    this.onAnimationLoadCallback = null;
    this.onMaterialLoadCallback = null;
    this.onTextureLoadCallback = null;

    this._defaultBackgroundImage = 'SYSTEM_DARK_2.jpg';
    this._defaultEnvironmentName = '01_attic_room_with_windows';
};

GASEngine.GAS2Loader.prototype =
{
    constructor: GASEngine.GAS2Loader,

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

    load: function(scene, modelName, onHierarchySuccess, onSyncLoading, onAsyncLoading)
    {
        this._scene_ = scene;

        this._onHierarchySuccess = onHierarchySuccess;
        this._onSyncLoading = onSyncLoading;
        this._onAsyncLoading = onAsyncLoading;
        this.loadStructureFiles(modelName, this.parseHierarchy.bind(this));
    },

    loadStructureFiles: function(modelName, callback)
    {
        GASEngine.FileSystem.Instance.read
        (
            modelName + '.structure.json', 
            function(data)
            {
                callback(data);
                //<
            }.bind(this)
        );
    },

    parseHierarchy: function(hierarchy)
    {
        var syncComponents = new Map();
        var asyncComponents = new Map();

        var hierarchyInstance = this._walkSceneJSON(hierarchy, syncComponents, asyncComponents);
        this._scene_.setModelRoot(hierarchyInstance);
        this._scene_.appendEntityOnRoot(hierarchyInstance);
        
        this._onHierarchySuccess(this._scene_);
        
        // GASEngine.SkeletonManager.Instance.init();

        if(syncComponents.size > 0)
        {
            this._loadComponents(syncComponents, (function(){

                var loadedCount = 0;
                return (function(file, status, syncFiles)
                {
                    ++loadedCount;

                    this._onSyncLoading(file, status, syncFiles);

                    if(loadedCount === syncFiles.size && asyncComponents.size > 0)
                    {
                        this._loadComponents(asyncComponents, this._onAsyncLoading.bind(this));
                    }
                }.bind(this));

            }.bind(this))());
        }
        else
        {
            if(asyncComponents.size > 0)
            {
                this._loadComponents(asyncComponents, this._onAsyncLoading.bind(this));
            }
        }

        if(this.onStructureLoadCallback) 
        {
            this.onStructureLoadCallback();
        }
    },

    // getAssembleConfigurations: function()
    // {
    //     return {};
    // },

    _walkHierarchy_r: function(nodeConfig, parent, syncComponents, asyncComponents)
    {
        var entity = GASEngine.EntityFactory.Instance.create();

        entity.name = nodeConfig.name;
        entity.uniqueID = nodeConfig.uniqueID;
        GASEngine.UniqueIDGenerator.Instance.updateID(entity.uniqueID);
        entity.guid = nodeConfig.guid;

        if(entity.skeletonName !== "eNone")
        {
            GASEngine.SkeletonManager.Instance.appendBones([entity]);
        }

        if(entity.skeletonName !== "eNone")
        {
            GASEngine.SkeletonManager.Instance.appendBones([entity]);
        }

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
        // entity.rotation.set(rx, ry, rz, 'ZYX');
        // entity.quaternion.setFromEuler(entity.rotation, false);
        //entity.rotation.setFromQuaternion(entity.quaternion, undefined, false);
        this.tmpEuler.set(rx, ry, rz, 'ZYX');
        entity.setLocalRotation(this.tmpEuler);

        // entity.scale.set(nodeConfig.scaling[0], nodeConfig.scaling[1], nodeConfig.scaling[2]);
        this.tmpVec.set(nodeConfig.scaling[0], nodeConfig.scaling[1], nodeConfig.scaling[2]);
        entity.setLocalScale(this.tmpVec);

        if(parent !== null)
        {
            parent.addChild(entity);
        }      

        if(nodeConfig.components)
        {
            var componentTypes = Object.keys(nodeConfig.components);
            for(var j = 0; j < componentTypes.length; ++j)
            {
                var typeName = componentTypes[j];
                var componentConf = nodeConfig.components[typeName];

                var uniqueID = componentConf.uniqueID;
                var newComponent = GASEngine.ComponentFactory.Instance.create(typeName, uniqueID);
                if(!newComponent) continue;
                newComponent.syncLoading = componentConf.syncLoading;

                if(newComponent instanceof GASEngine.MeshFilterComponent)
                {
                    newComponent.bbox = new GASEngine.AABB();
                    newComponent.bbox.min.set(componentConf.bbox[0][0], componentConf.bbox[0][1], componentConf.bbox[0][2]);
                    newComponent.bbox.max.set(componentConf.bbox[1][0], componentConf.bbox[1][1], componentConf.bbox[1][2]);
                }

                entity.addComponent(newComponent);

                newComponent.emit(GASEngine.Component.MSG_COMPONENT_CREATED);

                if(newComponent.syncLoading)
                {
                    syncComponents.set(newComponent, componentConf);
                }
                else
                {
                    asyncComponents.set(newComponent, componentConf);
                }
            }
        }
        
        for(var i = 0; i < nodeConfig.children.length; i++)
        {
            var child = nodeConfig.children[i];
            this._walkHierarchy_r(child, entity, syncComponents, asyncComponents);
        }

        return entity;
    },    

    _walkSceneJSON: function(sceneJSON, syncComponents, asyncComponents)
    {
        var root = this._walkHierarchy_r(sceneJSON.nodeTree, null, syncComponents, asyncComponents);

        return root;
    },        

    _loadComponents: function(components, onLoading)
    {
        var loadingFiles = new Map();
        var loadedCount = 0;

        if(components.size > 0)
        {
            function onLoadSuccess(file)
            {
                if(loadingFiles.has(file))
                {
                    ++loadedCount;

                    if(loadedCount === loadingFiles.size)
                    {
                        GASEngine.FileSystem.Instance.off(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, onLoadSuccess, this)
                    }

                    onLoading(file, true, loadingFiles);
                }
            }

            function onLoadFailed(file)
            {
                if(loadingFiles.has(file))
                {
                    ++loadedCount;

                    if(loadedCount === loadingFiles.size)
                    {
                        GASEngine.FileSystem.Instance.off(GASEngine.FileSystem.MSG_FILE_READ_FAILED, onLoadFailed, this)
                    }

                    onLoading(file, false, loadingFiles);
                }
            }

            GASEngine.FileSystem.Instance.on(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, onLoadSuccess, this);
            GASEngine.FileSystem.Instance.on(GASEngine.FileSystem.MSG_FILE_READ_FAILED, onLoadFailed, this);
            for (var [componentObject, componentConf] of components.entries())
            {
               this._loadComponent(componentObject, componentConf, loadingFiles);
            }
        }
    },

    _loadComponent: function(componentObject, componentConf, loadingFiles) {
        if(componentObject instanceof GASEngine.AnimatorComponent) {
            //TODO: WATCH OUT! ALL EXTERNAL FILES SHOULD BE COLLECTED BEFORE LOADING!
            for(var i = 0; i < componentConf.clips.length; ++i) {
                var path = this._getFilePath(componentConf.clips[i]);

                var that = this;
                GASEngine.Resources.Instance.loadAnimationClip(path, (function(compoment, clipName)
                {
                    return function(clip)
                    {
                        compoment.setAnimationClip(clip.name, clip);
                        compoment.play(clip.id);

                        if(that.onAnimationLoadCallback)
                        {
                            that.onAnimationLoadCallback();
                        }
                    };

                })(componentObject, componentConf.clips[i]));

                if(loadingFiles) {
                    loadingFiles.set(path, 0);
                }
            }
        }
        else if(componentObject instanceof GASEngine.MeshFilterComponent) {
            var path = this._getFilePath(componentConf.mesh);
            GASEngine.Resources.Instance.loadMesh(path, (function(component, meshName, boundingBox)
            {

                return function(mesh)
                {
                    mesh.submitToWebGL();
                    component.setMesh(mesh);
                    if(mesh.isSkinned())
                    {
                        mesh.linkBones(component.getParentEntity());
                    }
                    var bones = mesh.getBones();
                    //GASEngine.SkeletonManager.Instance.appendBones(bones);
                };

            })(componentObject, componentConf.mesh, componentConf.bbox));

            if(loadingFiles) {
                loadingFiles.set(path, 0);
            }
        }
        else if(componentObject instanceof GASEngine.MeshRendererComponent) {
            for(var i = 0; i < componentConf.materials.length; ++i)
            {
                var path = this._getFilePath(componentConf.materials[i]);
                GASEngine.Resources.Instance.loadMaterial(path, (function(compoment, materialName, index)
                {
                
                    return function(material)
                    {
                        compoment.setMaterialByIndex(material, index);
                    };
                
                })(componentObject, componentConf.materials[i], i));
            }

            if(loadingFiles) {
                loadingFiles.set(path, 0);
            }
        }
        else if(componentObject instanceof GASEngine.EnvironmentalLightComponent) {
            const environmentName = componentConf.name || this._defaultEnvironmentName;
            componentObject.setEnvironmentName(environmentName)
            .then(() => {
                if(componentConf.exposure !== undefined) {
                    componentObject.setEnvironmentExposure(componentConf.exposure);
                }
            })
            if(componentConf.orientation !== undefined) {
                componentObject.orientation = componentConf.orientation;
            }
        }
        else if(componentObject instanceof GASEngine.CameraComponent) {
            console.error("GASEngine.Resources._loadComponents: This component has not been supported by loader.");
        }
        else {
            console.error("GASEngine.Resources._loadComponents: This component has not been supported by loader.");
        }
    },

    _getFilePath: function(rawName)
    {
        var pathName = encodeURIComponent(rawName);
        var index = this._signedFileList_.findIndex(e => e.indexOf(pathName) !== -1);
        var path = index !== -1 ? this._signedFileList_[index] : pathName;
        return path;
    }
};