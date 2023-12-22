var SceneEditor =
{
    REVISION: '0',
};

SceneEditor.Editor = function()
{
    this.sceneDelegate = null;
    this.sceneInstance = null;
    this.eventInput = null;
    this.cameraManipulator = null;
    this.clock = new GASEngine.Clock();
    this.currentClipIndex = -1;

    this.lastFpsTimePoint = 0;
    this.frameCount = 0;

    this.mousePos = new GASEngine.Vector2(0, 0);

    SceneEditor.Editor.Instance = this;
};

SceneEditor.Editor.prototype = 
{
    constructor: SceneEditor.Editor,

    init: function()
    {
        this._initUI();

        this._initUIManager();

        this._initEngine(this.canvas, this.container.clientWidth, this.container.clientHeight);
    },

    _initUI: function()
    {
        this.editorLayout = new mgs.UIEditorLayout();
        document.body.appendChild(this.editorLayout.getRoot());
        this.container = this.editorLayout.getCanvasPanel().getRoot();
        this.canvas = this.editorLayout.getCanvas3d().getRoot();

        this.fpsElement = this.editorLayout.getFpsElement().getRoot();
        this.sceneTreePanel = this.editorLayout.getSceneTree();
        this.sceneMenu = this.editorLayout.getSceneMenu();
        this.attributePanel = this.editorLayout.getAttributePanel();

        SceneEditor.AnimationUtils.addAnimationPanel();

        var self = this;
        this.editorLayout.sceneEditor.on('onCanvasResize', function()
        {
            self.onWindowResize();
        });
    },

    _initUIManager: function()
    {
        this.cmdManager= new SceneEditor.CmdManager();
        this.sceneTreeManager = new SceneEditor.SceneTreeManager(this.sceneTreePanel);
        this.attributeManager = new SceneEditor.AttributeManager(this.attributePanel); 
        this.viewportManager = new SceneEditor.ViewportManager(); 
        this.dSceneManager = new SceneEditor.DSceneManager();
        this.dEntityManager = new SceneEditor.DEntityManager();
    },

    _initEngine: function(webglCanvas, canvasWidth, canvasHeight)
    {
        (new GASEngine.FileSystem());
        GASEngine.FileSystem.Instance.init('', '/system/', '');

        (new GASEngine.WebGLDevice());
        GASEngine.WebGLDevice.Instance.init(webglCanvas, canvasWidth, canvasHeight);

        (new GASEngine.WebGLTextureManager());

        (new GASEngine.PBRPipeline(GASEngine.WebGLDevice.Instance.gl));
        GASEngine.PBRPipeline.Instance.init(canvasWidth, canvasHeight);
        
        this.gl = GASEngine.WebGLDevice.Instance.gl;
        (new GASEngine.WebGLShaderManager());
        (new GASEngine.WebGLBufferManager());
        (new GASEngine.ComponentFactory());
        (new GASEngine.EntityFactory());
        (new GASEngine.MaterialFactory());
        (new GASEngine.MaterialMapFactory());
        (new GASEngine.TextureFactory());
        (new GASEngine.MeshFactory());
        (new GASEngine.KeyframeAnimationFactory());
        (new GASEngine.Resources());
        (new GASEngine.Keyboard(webglCanvas));
        (new GASEngine.Mouse(webglCanvas));

        (new GASEngine.HotspotManager(canvasWidth, canvasHeight));
        (new GASEngine.CameraManager());
        
        (new GASEngine.WebglCommon());
    },    

    
    loadSimpleScene: function()
    {
        var scene = new GASEngine.Scene();
        this.sceneInstance = scene;
        
        var entity = GASEngine.WebglCommon.Instance.createCube();
        this.sceneInstance.appendEntityOnRoot(entity);

        var entity = GASEngine.WebglCommon.Instance.createSphere();
        var tmpTranslation = new GASEngine.Vector3(2, 0, 0);
        entity.setLocalTranslation(tmpTranslation);        
        this.sceneInstance.appendEntityOnRoot(entity);

        this.sceneInstance.update();

        this.initDefaultCameraController();

        this.createSkybox();

        GASEngine.HotspotManager.Instance.init(scene);

        this.sceneDelegate = new SceneEditor.DScene(scene);
        
        mgsEditor.on(mgsEditor.EVENTS.toolBar.onSaveBtnClick, function(evt)
        {
            console.log(evt);
        });
    },

    loadGASScene: function(modelDirectory, convertedFile)
    {
        GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);

        var gasLoader = new GASEngine.GAS1Loader();
        gasLoader.load
        (
            convertedFile,

            function onHierarchySuccess(scene)
            {
                this.sceneInstance = scene;

                this.sceneDelegate = new SceneEditor.DScene(scene);
    
                this.sceneInstance.update();
        
                //Camera
                this.initDefaultCameraController();

                // var cameraEntity = GASEngine.EntityFactory.Instance.create();
                // cameraEntity.name = 'MainCamera';
                // var cameraComponent = GASEngine.ComponentFactory.Instance.create('camera');
        
                // cameraEntity.addComponent(cameraComponent);
                // this.sceneInstance.appendEntityOnRoot(cameraEntity);
        
                // var boundingBox = this.sceneInstance.getBoundingBox();
                // var center = new GASEngine.Vector3();
                // boundingBox.getCenter(center);
                // var radius = boundingBox.getRadius();

                // var scope = SceneEditor.Editor.prototype;
                // scope.initDefaultCameraController(center, radius, cameraComponent);

                //scope.createSkybox();
    
                GASEngine.HotspotManager.Instance.init(scene);

                console.log('GASEngine.Resources.loadGASScene: Scene hierarchy load finished.');
            }.bind(this)
        );
    },

    loadScene: function(modelDirectory, modelName)
    {
        GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);

        var syncFileCount = 0;
        var asyncFileCount = 0;
        var sturctureFile = modelName + '.structure.json';
        var gas2Loader = new GASEngine.GAS2Loader();
        gas2Loader.load
        (
            sturctureFile,
    
            function onHierarchySuccess(scene)
            {
                this.sceneInstance = scene;
                this.sceneInstance.update();
                this.sceneDelegate = new SceneEditor.DScene(scene);
        
                //For Animation Panel
                var animators = this.sceneInstance.findComponents('animator');
                for(var i = 0; i < animators.length; ++i)
                {
                    animators[i].on
                    (
                        GASEngine.Component.MSG_COMPONENT_PROPERTY_CHANGED, 
                        function(clipName)
                        {
                            var clip = this.getAnimationClip(clipName);
                            clip.onAnimationUpdateCallback = SceneEditor.AnimationUtils.updateAnimationProgress;
                            this.play(clipName);
                            if(this.currentClipIndex === -1) 
                            {
                                this.currentClipIndex = 0;
                            }
                        }
                    );
                }

                this.initDefaultCameraController();

                this.createSkybox();
    
                GASEngine.HotspotManager.Instance.init(scene);

                console.log('GASEngine.Resources.loadScene: Scene hierarchy load finished.');
            }.bind(this),
    
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
        return false;
    },

    createSkybox: function()
    {
        var mesh = GASEngine.MeshFactory.Instance.create();
        mesh.addStream('position', new Float32Array([1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0]));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': 4 }]);
        mesh.setDrawMode('TRIANGLE_STRIP');
        mesh.submitToWebGL();
    
        var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
        meshFilterComponent.setMesh(mesh);
    
        var material = GASEngine.MaterialFactory.Instance.create('skybox');
        GASEngine.Resources.Instance.loadCubeTexture(
            '/system/backgroundCubes/01_attic_room_with_windows/background_cubemap_512_0.02_luv.bin',
            512,
            function(texture, size)
            {
                material.setCubeMap(texture, size);
            });
    
        GASEngine.Resources.Instance.loadTexture(
            '/system/backgroundCubes/image-2048x1024.jpg',
            true,
            function(webglTexture, width, height)
            {
                material.setImage(webglTexture, width, height);
            },
            null,
            null);
    
        GASEngine.Resources.Instance.loadSPH(
            '/system/backgroundCubes/01_attic_room_with_windows/diffuse_sph.json',
            function (presetExposure, sph)
            {
                material.setSPH(presetExposure, sph);
            });
    
        material.backgroundType = 'CUBEMAP';
    
        var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
        meshRendererComponent.addMaterial(material);
    
        var skyboxEntity = GASEngine.EntityFactory.Instance.create();
        skyboxEntity.name = 'Skybox';
        skyboxEntity.uniqueID = GASEngine.generateUUID();
        skyboxEntity.addComponent(meshFilterComponent);
        skyboxEntity.addComponent(meshRendererComponent);
    
        this.sceneInstance.appendEntityOnRoot(skyboxEntity);
    
        //create environmental light
        var environmentalLightComponent = GASEngine.ComponentFactory.Instance.create('environmentalLight');
    
        GASEngine.Resources.Instance.loadCubeTexture(
            '/system/backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin',
            256,
            function (texture, width, height)
            {
                environmentalLightComponent.setSpecularCubeMap(texture, width, height);
            });
    
        GASEngine.Resources.Instance.loadPanorama(
            '/system/backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin',
            function (texture, width, height)
            {
                environmentalLightComponent.setSpecularPanoramaTexture(texture, width, height);
            });
    
        GASEngine.Resources.Instance.loadSPH(
            '/system/backgroundCubes/01_attic_room_with_windows/diffuse_sph.json',
            function (presetExposure, sph)
            {
                environmentalLightComponent.setDiffuseSPH(presetExposure, sph);
            });
    
        GASEngine.Resources.Instance.loadIntegratedBRDF(
            '/system/backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin',
            function (spcularIntegratedBRDF, width, height)
            {
                environmentalLightComponent.setSpecularIntegratedBRDF(spcularIntegratedBRDF);
            });
    
        var environmentalLightEntity = GASEngine.EntityFactory.Instance.create();
        environmentalLightEntity.name = 'EnvironmentalLight';
        environmentalLightEntity.uniqueID = GASEngine.generateUUID();
        environmentalLightEntity.addComponent(environmentalLightComponent);
        this.sceneInstance.appendEntityOnRoot(environmentalLightEntity);
    
    
        // var directionalLightComponent = GASEngine.ComponentFactory.Instance.create('directionalLight');
        // var directionalLightEntity = GASEngine.EntityFactory.Instance.create();
        // directionalLightEntity.addComponent(directionalLightComponent);
        // H5Editor_V0.sceneInstance.appendEntityOnRoot(directionalLightEntity);
    
        // var pointLightComponent = GASEngine.ComponentFactory.Instance.create('pointLight');
        // var pointLightEntity = GASEngine.EntityFactory.Instance.create();
        // pointLightEntity.addComponent(pointLightComponent);
        // H5Editor_V0.sceneInstance.appendEntityOnRoot(pointLightEntity);
    
        // var spotLightComponent = GASEngine.ComponentFactory.Instance.create('spotLight');
        // var spotLightEntity = GASEngine.EntityFactory.Instance.create();
        // spotLightEntity.addComponent(spotLightComponent);
        // H5Editor_V0.sceneInstance.appendEntityOnRoot(spotLightEntity);
    },

    // 编辑器的相机
    initDefaultCameraController: function()
    {
        var cameraEntity = GASEngine.EntityFactory.Instance.create();
        cameraEntity.name = 'MainCamera';
        cameraEntity.uniqueID = GASEngine.generateUUID();

        var cameraComponent = GASEngine.ComponentFactory.Instance.create('camera');
        cameraComponent.aspect =
            GASEngine.WebGLDevice.Instance.getCanvasWidth() / GASEngine.WebGLDevice.Instance.getCanvasHeight();
            cameraComponent.fov = 45;
        var center = new GASEngine.Vector3(0, 0, 10);
        var radius = 1;
        
        this.cameraManipulator = new GASEngine.SwitchManipulator();
        this.cameraManipulator.setCamera(cameraComponent);
        this.cameraManipulator.computeHomePosition(center, radius);

        this.eventInput = new SceneEditor.EditorModeController(this.cameraManipulator);
        this.eventInput.init(this.canvas);

        cameraEntity.addComponent(cameraComponent);
        this.sceneInstance.appendEntityOnRoot(cameraEntity);
    },

    // 场景计算得到的相机
    initDefaultCameraController_v1: function()
    {
        var cameraEntity = GASEngine.EntityFactory.Instance.create();
        cameraEntity.name = 'MainCamera';
        cameraEntity.uniqueID = GASEngine.generateUUID();
        var cameraComponent = GASEngine.ComponentFactory.Instance.create('camera');

        cameraEntity.addComponent(cameraComponent);
        this.sceneInstance.appendEntityOnRoot(cameraEntity);

        var boundingBox = this.sceneInstance.getBoundingBox();
        var center = new GASEngine.Vector3();
        boundingBox.getCenter(center);
        var radius = boundingBox.getRadius();

        var farPlane = radius * 10.0;
        farPlane = farPlane < 99999.0 ? farPlane : 99999.0;
        var nearPlane = (farPlane / 1000.0 < 0.001) ? 0.001 : (farPlane / 1000.0);
        cameraComponent.far = farPlane;
        cameraComponent.near = nearPlane;
        cameraComponent.aspect =
            GASEngine.WebGLDevice.Instance.getCanvasWidth() / GASEngine.WebGLDevice.Instance.getCanvasHeight();
        cameraComponent.fov = 60.0;

        this.cameraManipulator = new GASEngine.SwitchManipulator();
        this.cameraManipulator.setCamera(cameraComponent);
        this.cameraManipulator.computeHomePosition(center, radius);

        this.eventInput = new SceneEditor.EditorModeController(this.cameraManipulator);
        this.eventInput.init(this.canvas);
    }
};

SceneEditor.Editor.prototype.tick = function()
{
    var delta = this.clock.getDelta();

    if(this.sceneInstance !== null)
    {
        var cameraMatrixWorld;
        if(this.cameraManipulator !== null)
        {
            this.cameraManipulator.update(delta);
            cameraMatrixWorld = this.cameraManipulator.getCameraWorldMatrix();
        }
        
        var cameras = this.sceneInstance.getCameras();
        var cameraCount = this.sceneInstance.getCameraCount();
        if(cameraCount > 0)
        {    
            cameras[0].setWorldMatrix(cameraMatrixWorld); //overwrite view matrix produced in updating procedure.
            cameras[0]._updateViewMatrix();
        }
        
        cameras = this.sceneInstance.cull();
        
        if(this.eventInput)
        {
            this.eventInput.update(delta);
        }

        this.sceneInstance.update(delta);

        if(cameraCount > 0)
        {    
            if(GASEngine.HotspotManager.Instance && cameras[0])
            {
                GASEngine.HotspotManager.Instance.update(cameras[0]);
            }
            GASEngine.PBRPipeline.Instance.render_V1(cameras, cameraCount);
        }
    }

    if(GASEngine.Keyboard.Instance)
    {
        GASEngine.Keyboard.Instance.update(delta);
    }

    if(GASEngine.Mouse.Instance)
    {
        GASEngine.Mouse.Instance.update(delta);
    }

    ++this.frameCount;
    var now = Date.now();
    if(now - this.lastFpsTimePoint > 1000)
    {
        if(this.fpsElement !== null)
        {
            this.fpsElement.innerText = 'FPS: ' + this.frameCount;
        }

        this.lastFpsTimePoint = now;
        this.frameCount = 0;
    }
};

SceneEditor.Editor.prototype.onWindowResize = function()
{
    var canvasWidth = this.container.clientWidth;
    var canvasHeight = this.container.clientHeight;

    if(!this.sceneInstance) return;
    var cameras = this.sceneInstance.findComponents('camera');
    for(var i = 0; i < cameras.length; ++i)
    {
        cameras[i].aspect = canvasWidth / canvasHeight;
        cameras[i]._updateProjectionMatrix();
    }
    
    if(GASEngine.HotspotManager.Instance)
    {
        GASEngine.HotspotManager.Instance.setCanvasSize(canvasWidth, canvasHeight);
    }
    
    if(GASEngine.PBRPipeline.Instance)
    {
        GASEngine.WebGLDevice.Instance.setSize(canvasWidth, canvasHeight);
        GASEngine.PBRPipeline.Instance.onWindowResize(canvasWidth, canvasHeight);
    }
};

SceneEditor.Editor.prototype.onClicked = function()
{
    var barType = event.target.id.split('-')[0];
    switch(barType)
    {
        case 'viewport':
            break;
        default:
            event.stopPropagation();
            break;
    }
};

SceneEditor.Editor.prototype.onKeyDown = function(event)
{
    var isCtrlDown = event.ctrlKey || event.metaKey;

    if (isCtrlDown && event.keyCode === 90)
    {
        event.preventDefault();
        event.stopPropagation();
        this.cmdManager.undo();
    }

    else if (isCtrlDown && event.keyCode === 89) 
    {
        event.preventDefault();
        event.stopPropagation();
        this.cmdManager.redo();
    }
};

SceneEditor.Editor.prototype.onMouseMove = function(x, y)
{   
    this.mousePos.set(x, y);
};

SceneEditor.Editor.prototype.createEntity = function(type)
{
    var childID = this.sceneDelegate.createEntity(type);
    return childID;
}

SceneEditor.Editor.prototype.deleteEntity = function(entity)
{
    this.sceneDelegate.removeEntity(entity);
}

SceneEditor.Editor.prototype.onDropHeader = function(parent, child)
{
    this.sceneDelegate.appendEntityOnParent(parent, child);
}

SceneEditor.Editor.prototype.onDropInsertArea = function(first, next)
{
    this.sceneDelegate.insertEntityBefore(first, next);
}