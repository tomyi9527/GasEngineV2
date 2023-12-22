(function()
{
    let Application = function(canvas)
    {
        mgs.Object.call(this);

        this._initEngine(canvas, canvas.width, canvas.height);
    };
    mgs.classInherit(Application, mgs.Object);

    Application.prototype._initEngine = function(webglCanvas, canvasWidth, canvasHeight)
    {
        this._webGLDevice = new GASEngine.WebGLDevice();
        this._webGLDevice.init(webglCanvas, canvasWidth, canvasHeight);

        this._webGLTextureManager = new GASEngine.WebGLTextureManager();

        this._pBRPipeline = new GASEngine.PBRPipeline(this._webGLDevice.gl);
        this._pBRPipeline.init(canvasWidth, canvasHeight);
        
        this._webGLShaderManager = new GASEngine.WebGLShaderManager();
        this._webGLBufferManager = new GASEngine.WebGLBufferManager();
        this._componentFactory = new GASEngine.ComponentFactory();
        this._entityFactory = new GASEngine.EntityFactory();
        this._materialFactory = new GASEngine.MaterialFactory();
        this._materialMapFactory = new GASEngine.MaterialMapFactory();
        this._textureFactory = new GASEngine.TextureFactory();
        this._meshFactory = new GASEngine.MeshFactory();
        this._keyframeAnimationFactory = new GASEngine.KeyframeAnimationFactory();
        this._resources = new GASEngine.Resources();
        this._keyboard = new GASEngine.Keyboard(webglCanvas);
        this._mouse = new GASEngine.Mouse(webglCanvas);
        this._cameraManager = new GASEngine.CameraManager();
        this._webglCommon = new GASEngine.WebglCommon();
        this._assetManager = new mgs.AssetManager();

        this._scene = new GASEngine.Scene();

        this.createEnvironmentalLight();
    };

    Application.prototype.createEnvironmentalLight = function()
    {
        //create environment light
        let lightEntity = this.createEntity('Light1');
        let lightComponent = this.createComponent('environmentalLight');
        GASEngine.Resources.Instance.loadCubeTexture(
            '/system/backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin',
            256,
            function (texture, width, height)
            {
                lightComponent.setSpecularCubeMap(texture, width, height);
            });

        GASEngine.Resources.Instance.loadPanorama(
            '/system/backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin',
            function (texture, width, height)
            {
                lightComponent.setSpecularPanoramaTexture(texture, width, height);
            });

        GASEngine.Resources.Instance.loadSPH(
            '/system/backgroundCubes/01_attic_room_with_windows/diffuse_sph.json',
            function (presetExposure, sph)
            {
                lightComponent.setDiffuseSPH(presetExposure, sph);
            });

        GASEngine.Resources.Instance.loadIntegratedBRDF(
            '/system/backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin',
            function (spcularIntegratedBRDF, width, height)
            {
                lightComponent.setSpecularIntegratedBRDF(spcularIntegratedBRDF);
            });
        lightEntity.addComponent(lightComponent);
        if(this._scene) {
            this._scene.appendEntityOnRoot(lightEntity);
        }
        this.lightEntity = lightEntity;
    };


    // -------------------------------------------- getter --------------------------------------------

    Application.prototype.getMeshFactory = function()
    {
        return this._meshFactory;
    };

    Application.prototype.getComponentFactory = function()
    {
        return this._componentFactory;
    };

    Application.prototype.getWebGLDevice = function()
    {
        return this._webGLDevice;
    };


    // -------------------------------------------- update & callback --------------------------------------------

    Application.prototype.destroy = function()
    {
        this._webGLDevice.finl();
    };

    Application.prototype.update = function(delta)
    {
        if(this._scene !== null)
        {
            this._scene.cull();
            
            this._scene.update(delta);
    
            var cameras = this._scene.getCameras();
            var cameraCount = this._scene.getCameraCount();
            if(cameraCount > 0)
            {    
                this._pBRPipeline.render_V1(cameras, cameraCount);
            }
        }
    };

    Application.prototype.onCanvasResize = function(width, height)
    {
        let cameras = this._scene.findComponents('camera');
        for(let i = 0; i < cameras.length; ++i)
        {
            let camera = cameras[i];
            let entity = camera.getParentEntity();
            if (entity && entity.type === 'helper')
            {

            }
            cameras[i].getParentEntity
            cameras[i].aspect = width / height;
            cameras[i]._updateViewMatrix();
            cameras[i]._updateProjectionMatrix();
            cameras[i]._updateViewProjectionMatrix();
        }

        this._webGLDevice.setSize(width, height);
        this._pBRPipeline.onWindowResize(width, height);
    };


    // -------------------------------------------- component & entity --------------------------------------------

    Application.prototype.createComponent = function(name)
    {
        return this._componentFactory.create(name);
    };

    Application.prototype.createEntity = function(name, translation, rotation, scale)
    {
        let entity = this._entityFactory.create();

        entity.uniqueID = GASEngine.generateUUID();

        if (name) entity.name = name;
        if (translation) entity.translation.copy(translation);
        if (rotation) entity.rotation.copy(rotation);
        if (scale) entity.scale.copy(scale);

        return entity;
    };

    Application.prototype.removeEntity = function(entity)
    {
        if (entity.parent)
        {
            entity.parent.removeChild(entity);
        }
        
        this._entityFactory.destroy(entity);
    };

    // -------------------------------------------- scene --------------------------------------------

    Application.prototype.getScene = function()
    {
        return this._scene;
    };

    Application.prototype.closeScene = function()
    {
        if (this._scene !== null)
        {
            this._scene.destroy();
            this._scene = null;
        }
    };

    Application.prototype.openScene = function(sceneInfo, onSuccess)
    {
        let self = this;
        let fileSystem = this._assetManager.getFileSystem();

        function onHierarchySuccess(scene)
        {
            self.closeScene();
            self._scene = scene;
            self._scene.appendEntityOnRoot(self.lightEntity);

            onSuccess(scene);
            console.log('GASEngine.Resources.loadGASScene: Scene hierarchy load finished.');
        };

        if (sceneInfo.type === 'GAS')
        {
            let modelDirectory = sceneInfo.modelDirectory;
            let convertedFile = sceneInfo.convertedFile;
            let sturctureFile = convertedFile + '.structure.json';

            fileSystem.setProjectDirectory(modelDirectory);

            let gasLoader = new GASEngine.GAS1Loader();
            gasLoader.load
            (
                sturctureFile,
                onHierarchySuccess.bind(this)
            );
        }
        else if (sceneInfo.type === 'GLTF')
        {
            let modelDirectory = sceneInfo.modelDirectory;
            let structureFile = sceneInfo.convertedFile;

            fileSystem.setProjectDirectory(modelDirectory);

            let gltfLoader = new GASEngine.GLTFLoader();
            gltfLoader.load
            (
                structureFile,
                onHierarchySuccess.bind(this)
            );
        }
        else if (sceneInfo.type === 'FBX')
        {
            let modelDirectory = sceneInfo.modelDirectory;
            let structureFile = sceneInfo.convertedFile;

            fileSystem.setProjectDirectory(modelDirectory);

            let fbxLoader = new GASEngine.FBXLoader();
            fbxLoader.load
            (
                structureFile,
                onHierarchySuccess.bind(this)
            );
        }
        else if (sceneInfo.type === 'NEO')
        {
            let modelDirectory = sceneInfo.modelDirectory;
            let convertedFile = sceneInfo.convertedFile;
            let sturctureFile = convertedFile + '.structure.json';

            fileSystem.setProjectDirectory(modelDirectory);

            let syncFileCount = 0;
            let asyncFileCount = 0;
            let gas2Loader = new GASEngine.GAS2Loader();
            gas2Loader.load
            (
                sturctureFile,
                onHierarchySuccess.bind(this),
                function onSyncLoading(file, status, totalFiles)
                {
                    ++syncFileCount;
        
                    if(status)
                        console.log('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load successfully.');
                    else
                        console.error('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load failed.');
                },
        
                function onAsyncLoading(file, status, totalFiles)
                {
                    ++asyncFileCount;
        
                    if(status)
                        console.log('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load successfully.');
                    else
                        console.error('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load failed.');
                }
            );
        }
        else if (sceneInfo.type === 'TEST_EMPTY')
        {
            self.closeScene();
            self._scene = new GASEngine.Scene();
            
            // test scene
            let entity0 = self._webglCommon.createCube();
            self._scene.appendEntityOnRoot(entity0);
            entity0.translation.set(-3, 0, 0);

            let entity1 = self._webglCommon.createCube();
            self._scene.appendEntityOnRoot(entity1);
            entity1.translation.set(3, 0, 0);

            onSuccess(self._scene);
            console.log('load test empty: Scene hierarchy load finished.');
        }
        else
        {
            mgs.Log.error('scene type not supported!');
        }
    };
}());