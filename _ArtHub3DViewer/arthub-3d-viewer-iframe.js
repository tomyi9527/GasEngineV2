const MessageType = {
  Inited: "inited",
  StructureLoaded: "structureLoaded",
  FileLoaded: "fileLoaded",
  Shoted: "shoted",
  CaptureGifFinished: "captureGifFinished",
  HotspotAdded: "hotspotAdded",
  HotspotPicked: "hotspotPicked",
  HotspotUnpicked: "hotspotUnpicked",
  HotspotMoved: "hotspotMoved",
  AnimationLoaded: "animationLoaded",
  AnimationProgress: "animationProgress",
  AnimationStart: "animationStart",
  AnimationDelete: 'animationDelete',
  MaterialLoaded: 'materialLoaded'
}
const ViewMode = {
  None: 0,
  Albedo: 1,
  Normals: 2,
  Lit: 3,
  Metal: 4
}
const BackgroundType = {
  SOLIDCOLOR: 'SOLIDCOLOR',
  IMAGE: 'IMAGE',
  CUBEMAP: 'CUBEMAP',
  AMBIENT: 'AMBIENT'
}

const SkeletonSale = [0.1, 10];
const DefaultBackgroundImage = 'SYSTEM_DARK_2.jpg';
const DefaultEnvironmentName = '01_attic_room_with_windows';
//'use strict';
ArtHub3DViewerIFrame = function () {
  this._clock = null;
  this._manipulator = null;
  this._eventInput = null;
  this._gridEntity = null;
  this._boxEntity = null;
  this._axisEntity = null;
  this._scene = null;
  this._canvasContainer_ = null;

  //statistics
  this._fps = 0;
  this._polygonCount = 0;

  //temp vars
  this._resizeCallback = null;
  this._messageCallback = null;
  this._syncFileCount = 0;
  this._asyncFileCount = 0;
  this._dragFlag = false;

  //disable context menu
  document.addEventListener('contextmenu', (e) => {
    e.preventDefault();
    e.stopPropagation();
    return false;
  }, false);

  this._engineInited = false;
  this.initEngine();
};

ArtHub3DViewerIFrame.prototype = {
  constructor: ArtHub3DViewerIFrame,

  initEngine: function () {
    let container = document.body;
    let baseURL = this._getBaseURL();
    let rootDirectory = `//${baseURL}/`;
    let systemDirectory = `//${baseURL}/system/`;
    let gifWorkerPath = `//${baseURL}/_ArtHub3DViewer/vendors/gif.worker.js`;
    let projectDirectory = '';
    let textureDirectory = '';
    this._canvasContainer_ = container;

    this._clock = new GASEngine.Clock();

    this._sceneLoader_ = null;

    //create canvas
    this._canvas = document.createElement('canvas');
    this._canvas.setAttribute('id', 'arthub-3d-viewer-canvas');
    this._canvas.setAttribute('tabindex', 0);
    this._canvas.width = window.innerWidth;
    this._canvas.height = window.innerHeight;
    //disable I-bar cursor on click+drag
    this._canvas.onselectstart = function () {
      return false;
    };
    this._canvasContainer_.appendChild(this._canvas);

    //init renderer
    var webGLDevice = new GASEngine.WebGLDevice();
    var initResult = webGLDevice.init(this._canvas, this._canvas.width, this._canvas.height);
    //
    if (!initResult) {
      this._returnParent(-1, MessageType.Inited, false);
      return;
    }

    (new GASEngine.WebGLTextureManager()).init();
    (new GASEngine.WebGLShaderManager()).init();
    (new GASEngine.WebGLBufferManager()).init();

    (new GASEngine.MaterialFactory()).init();
    (new GASEngine.MaterialMapFactory()).init();

    (new GASEngine.PBRPipeline(webGLDevice.gl)).init(this._canvas.width, this._canvas.height, {});

    (new GASEngine.ComponentFactory()).init();

    (new GASEngine.EntityFactory()).init();

    (new GASEngine.TextureFactory()).init();

    (new GASEngine.MeshFactory()).init();

    (new GASEngine.KeyframeAnimationFactory()).init();

    (new ArtHubFileSystem()).init(rootDirectory, systemDirectory, projectDirectory, textureDirectory);

    (new GASEngine.Resources()).init(); //TODO:
    (new GASEngine.UniqueIDGenerator()).init();

    (new GASEngine.Keyboard(this._canvas));
    (new GASEngine.Mouse(this._canvas));
    (new GASEngine.CameraManager());
    (new GASEngine.HotspotManager(this._canvas.width, this._canvas.height));
    (new GASEngine.SkeletonManager());

    GASEngine.HotspotManager.Instance.onHotspotMoved = this.onHotspotMoved.bind(this);

    this._manipulator = new GASEngine.SwitchManipulator();
    this._eventInput = new GASEngine.StandardMouseKeyboard(this._manipulator);
    //this._eventInput = new GASEngine.DeviceOrientation(this._manipulator);
    //this._eventInput = new GASEngine.HammerController(this._manipulator);
    this._manipulator.setManipulatorType(GASEngine.Manipulator.ORBIT);
    this._eventInput.init({
      'mouseEventNode': this._canvas,
      'eventNode': this._canvas
    });
    this._eventInput.setEnable(true);

    //hook messages
    this._resizeCallback = () => {
      this._onWindowResize();
    };

    window.addEventListener('resize', this._resizeCallback, false);

    this._messageCallback = (event) => {
      this._onIFrameMessage(event.data.invokeID, event.data.cmd, event.data.parameters);
    };
    window.addEventListener("message", this._messageCallback, false);

    //enable canvas mouse event
    this._initCanvasMouseEventHandler();

    this._recordFrame = false;
    this._captureGif = new GifCapture({
      path: gifWorkerPath
    });
    this._captureGif.onFinishedCallback = this.onFinishedCaptureCallback.bind(this);

    this.isAddHotspot = false;
    this.preSelectHotspot = null;
    this._models = [];

    this._skeletonEnable = false;

    this._engineInited = true;
    this._returnParent(-1, MessageType.Inited, true);
  },

  onHotspotMoved: function (id, x, y) {
    this._returnParent(-1, MessageType.HotspotMoved, {
      id,
      x,
      y
    });
  },

  destroyEngine: function () {
    this._engineInited = false;
    if (GASEngine.PBRPipeline.Instance) {
      GASEngine.PBRPipeline.Instance.finl();
      GASEngine.PBRPipeline.Instance = null;
    }

    if (GASEngine.SkeletonManager.Instance) {
      GASEngine.SkeletonManager.Instance.finl();
      GASEngine.SkeletonManager.Instance = null;
    }

    if (GASEngine.HotspotManager.Instance) {
      GASEngine.HotspotManager.Instance.finl();
      GASEngine.HotspotManager.Instance = null;
    }

    if (GASEngine.CameraManager.Instance) {
      GASEngine.CameraManager.Instance.finl();
      GASEngine.CameraManager.Instance = null;
    }

    if (GASEngine.Mouse.Instance) {
      GASEngine.Mouse.Instance.finl();
      GASEngine.Mouse.Instance = null;
    }

    if (GASEngine.Keyboard.Instance) {
      GASEngine.Keyboard.Instance.finl();
      GASEngine.Keyboard.Instance = null;
    }

    if (GASEngine.Resources.Instance) {
      GASEngine.Resources.Instance.finl();
      GASEngine.Resources.Instance = null;
    }

    if (GASEngine.UniqueIDGenerator.Instance) {
      GASEngine.UniqueIDGenerator.Instance.finl();
      GASEngine.UniqueIDGenerator.Instance = null;
    }

    if (GASEngine.FileSystem.Instance) {
      GASEngine.FileSystem.Instance.finl();
      GASEngine.FileSystem.Instance = null;
    }

    if (GASEngine.KeyframeAnimationFactory.Instance) {
      GASEngine.KeyframeAnimationFactory.Instance.finl();
      GASEngine.KeyframeAnimationFactory.Instance = null;
    }

    if (GASEngine.MeshFactory.Instance) {
      GASEngine.MeshFactory.Instance.finl();
      GASEngine.MeshFactory.Instance = null;
    }

    if (GASEngine.TextureFactory.Instance) {
      GASEngine.TextureFactory.Instance.finl();
      GASEngine.TextureFactory.Instance = null;
    }

    if (GASEngine.EntityFactory.Instance) {
      GASEngine.EntityFactory.Instance.finl();
      GASEngine.EntityFactory.Instance = null;
    }

    if (GASEngine.ComponentFactory.Instance) {
      GASEngine.ComponentFactory.Instance.finl();
      GASEngine.ComponentFactory.Instance = null;
    }

    if (GASEngine.MaterialMapFactory.Instance) {
      GASEngine.MaterialMapFactory.Instance.finl();
      GASEngine.MaterialMapFactory.Instance = null;
    }

    if (GASEngine.MaterialFactory.Instance) {
      GASEngine.MaterialFactory.Instance.finl();
      GASEngine.MaterialFactory.Instance = null;
    }

    if (GASEngine.WebGLBufferManager.Instance) {
      GASEngine.WebGLBufferManager.Instance.finl();
      GASEngine.WebGLBufferManager.Instance = null;
    }

    if (GASEngine.WebGLShaderManager.Instance) {
      GASEngine.WebGLShaderManager.Instance.finl();
      GASEngine.WebGLShaderManager.Instance = null;
    }

    if (GASEngine.WebGLTextureManager.Instance) {
      GASEngine.WebGLTextureManager.Instance.finl();
      GASEngine.WebGLTextureManager.Instance = null;
    }

    if (GASEngine.WebGLDevice.Instance) {
      GASEngine.WebGLDevice.Instance.finl();
      GASEngine.WebGLDevice.Instance = null;
    }

    if (this._canvas) {
      this._canvasContainer_.removeChild(this._canvas);
      this._canvas = null;
    }

    if (this._messageCallback) {
      window.removeEventListener("message", this._messageCallback);
      this._messageCallback = null;
    }

    if (this._resizeCallback) {
      window.removeEventListener("message", this._resizeCallback);
      this._resizeCallback = null;
    }


    this._clock = null;

    this._syncFileCount = 0;
    this._asyncFileCount = 0;
    this._fps = 0;
    this._polygonCount = 0;
  },

  createScene: function () {
    this._scene = new GASEngine.Scene();
    var hotspotPromise = GASEngine.HotspotManager.Instance.init(this._scene);
    return hotspotPromise;
  },

  destroyScene: function () {
    if (this._scene) {
      this._scene.destroy();
      this._scene = null;
    }
  },

  run: function () {
    ArtHub3DViewerIFrame._mainLoop = function () {
      requestAnimationFrame(ArtHub3DViewerIFrame._mainLoop);

      if (!this._engineInited) {
        return;
      }

      let delta = this._clock.getDelta();

      if (this._scene !== null && GASEngine.PBRPipeline.Instance) {
        this._scene.cull();
        this._scene.update(delta);

        var cameras = this._scene.getCameras();
        var cameraCount = this._scene.getCameraCount();

        if (this._eventInput) {
          this._eventInput.update(delta);
        }

        if (this._manipulator) {
          this._manipulator.update(delta);
        }

        if (cameraCount > 0 && this._manipulator) {
          var cameraMatrixWorld = this._manipulator.getCameraWorldMatrix();
          cameras[0].setWorldMatrix(cameraMatrixWorld);
          cameras[0]._updateViewMatrix();
          if (GASEngine.HotspotManager.Instance && cameras[0]) {
            GASEngine.HotspotManager.Instance.update(cameras[0]);
          }
          GASEngine.PBRPipeline.Instance.render_V1(cameras, cameraCount);
        }

        if (GASEngine.Keyboard.Instance) {
          GASEngine.Keyboard.Instance.update(delta);
        }

        if (GASEngine.Mouse.Instance) {
          GASEngine.Mouse.Instance.update(delta);
        }
      }
    }.bind(this);

    ArtHub3DViewerIFrame._mainLoop();
  },

  load: function (parameters) {
    if (!this._engineInited) {
      return Promise.reject({
        message: '引擎尚未初始化'
      });
    }
    var filename = parameters.filename;
    var format = parameters.format;

    this._syncFileCount = 0;
    this._asyncFileCount = 0;

    //var ext = GASEngine.FileSystem.extractFileExt(filename);
    filename = filename ? filename.replace(/\\/g, "/") : '';

    var lastIndex = filename.lastIndexOf('/');
    var modelDirectory = '';
    var modelFile = '';
    var textureDirectory = '';
    if (lastIndex >= 0) {
      modelDirectory += filename.substring(0, lastIndex + 1);
      textureDirectory = filename.substring(0, lastIndex);
      var mLastIndex = textureDirectory.lastIndexOf('/');
      textureDirectory = textureDirectory.substring(0, mLastIndex + 1);
      modelFile = filename.substring(lastIndex + 1);
    } else {
      modelFile = filename;
    }

    GASEngine.Resources.Instance.meshCache.clear();
    GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory, textureDirectory);

    let loadParams = modelFile;
    switch (format) {
      case 'gas1': {
        this._sceneLoader_ = new GASEngine.GAS1Loader();
        break;
      }
      case 'gltf': {
        this._sceneLoader_ = new GASEngine.GLTFLoader();
        break;
      }
      case 'glb': {
        this._sceneLoader_ = new GASEngine.GLTFLoader();
        break;
      }
      case 'gas2': {
        this._sceneLoader_ = new GASEngine.GAS2Loader();
        break;
      }
      case 'fbx': {
        this._sceneLoader_ = new GASEngine.FBXLoader();
        break;
      }
      case 'arthub': {
        this._sceneLoader_ = new ArthubLoader();
        loadParams = parameters.options;
        GASEngine.FileSystem.Instance.getFullPath = this.getFullPath.bind(this);
        break;
      }
    }
    // for local mode
    if (!this._sceneLoader_.getTextureName) {
      this._sceneLoader_.getTextureName = function (pathArray) {
        var takeName = function (path) {
          var startIdx = path.indexOf('/');
          var endIdx = path.lastIndexOf('?');
          if (endIdx === -1)
            endIdx = path.length;
          return path.slice(startIdx + 1, endIdx);
        };
        return pathArray.map(path => takeName(path));
      }
    }

    this._sceneLoader_.onStructureLoadCallback = this._onStructureLoadSuccess.bind(this);
    this._sceneLoader_.onFileLoaded = this._onFileLoaded.bind(this);
    this._sceneLoader_.onAnimationLoadCallback = this.onAnimationLoaded.bind(this);

    this._sceneLoader_.load(
      this._scene,
      loadParams,
      this._onHierarchySuccess.bind(this),
      this._onSyncLoading.bind(this),
      this._onAsyncLoading.bind(this),
      this._onRendererLoaded.bind(this)
    );
    return Promise.resolve(true);
  },

  getFullPath: function (fileName) {
    return this._sceneLoader_.getSignedUrlByFileName(fileName);
  },

  // unload all
  unload: function () {
    this.enableAnimation(false); // 清除前停止播放动画
    GASEngine.SkeletonManager.Instance.finl(); // 清除前解绑骨骼
    this._models.forEach(e => {
      this._scene.removeEntityOnRoot(e);
      GASEngine.EntityFactory.Instance.destroy(e);
    });
    this._models.length = 0;
  },

  showUVLayout: function (flag) {
    //this._scene.enableUVLayout(flag);
    GASEngine.PBRPipeline.Instance.isShowUVTopologyObjects = flag;
  },

  showTopology: function (flag, isSaveMode = true) {
    //this._scene.enableWireframeOverlay(flag);
    GASEngine.PBRPipeline.Instance.isShowTopologyObjects = flag;
    if (isSaveMode) {
      this.storeRendererChange('wireframeEnable', flag);
    }
  },

  setTopologyColor: function (rgb, isSaveMode = true) {
    GASEngine.PBRPipeline.Instance.topologyLineColor[0] = rgb[0];
    GASEngine.PBRPipeline.Instance.topologyLineColor[1] = rgb[1];
    GASEngine.PBRPipeline.Instance.topologyLineColor[2] = rgb[2];
    if (isSaveMode) {
      this.storeRendererChange('wireframeColor', rgb);
    }
  },

  getTopologyColor: function () {
    var rgba = GASEngine.PBRPipeline.Instance.topologyLineColor;
    return [rgba[0], rgba[1], rgba[2]];
  },

  setTopologyAlpha: function (alpha, isSaveMode = true) {
    GASEngine.PBRPipeline.Instance.topologyLineColor[3] = alpha;
    if (isSaveMode) {
      this.storeRendererChange('wireframeOpacity', alpha);
    }
  },

  getTopologyAlpha: function () {
    return GASEngine.PBRPipeline.Instance.topologyLineColor[3];
  },

  showTPose: function (flag, isSaveMode = true) {
    if (GASEngine.PBRPipeline.Instance.skinningFlag === flag) return;
    GASEngine.PBRPipeline.Instance.skinningFlag = flag;
    GASEngine.HotspotManager.Instance.status = flag;
    if (isSaveMode) {
      this.storeRendererChange('tposeEnable', flag);
    }
    // //T-Pose的时候禁用动画
    // this.enableAnimation(flag);
  },

  showSkeleton: function (flag, isSaveMode = true) {
    this._skeletonEnable = flag;
    var skeletonHelper = this._scene.root.findChildByName('skeletonHelper');
    if (skeletonHelper !== null)
      skeletonHelper.enable = flag;
    else {
      skeletonHelper = GASEngine.SkeletonManager.Instance.init();
      if (skeletonHelper) {
        this._scene.appendEntityOnRoot(skeletonHelper);
        skeletonHelper.enable = flag;
      } else {
        console.log('This model has no skeletons!');
      }

    }
    if (isSaveMode) {
      this.storeRendererChange('skeletonEnable', flag);
    }
  },

  setSkeletonScale: function (scale, isSaveMode = true) {
    GASEngine.SkeletonManager.Instance.scale = scale;
    if (isSaveMode) {
      this.storeRendererChange('skeletonScale', scale);
    }
  },

  getSkeletonScale: function () {
    return GASEngine.SkeletonManager.Instance.scale;
  },

  changeRenderMode: function (mode, isSaveMode = true) {
    GASEngine.PBRPipeline.Instance.viewMode = mode;
    if (isSaveMode) {
      this.storeRendererChange('renderMode', mode);
    }
  },

  enableAnimation: function (flag) {
    let playMode = flag ? 'Play' : 'Pause';
    this._scene.setAnimationGlobalPlayMode(playMode);
  },

  enableHotspotAdd: function (flag) {
    this.isAddHotspot = flag;
  },

  createEntity: function (name, translation, rotation, scale) {
    let entity = GASEngine.EntityFactory.Instance.create();
    entity.guid = GASEngine.generateUUID();

    if (name) {
      entity.name = name;
    }

    if (translation) {
      entity.translation.copy(translation);
    }

    if (rotation) {
      entity.rotation.copy(rotation);
    }

    if (scale) {
      entity.scale.copy(scale);
    }

    return entity;
  },


  createSkyboxEntity: function () {
    var skyboxEntity = this.createEntity('Skybox');

    var mesh = GASEngine.MeshFactory.Instance.create();
    mesh.addStream('position', new Float32Array([1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0]));
    mesh.addStream('subMesh', [{
      'start': 0,
      'count': 4
    }]);
    mesh.setDrawMode('TRIANGLE_STRIP');
    mesh.submitToWebGL();

    var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilterComponent.setMesh(mesh);

    var material = GASEngine.MaterialFactory.Instance.create('skybox');
    material.backgroundType = 'IMAGE';
    material.setBackgroundImage(DefaultBackgroundImage); //TODO: 后期需要存储及读取

    var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    meshRendererComponent.addMaterial(material);

    skyboxEntity.addComponent(meshFilterComponent);
    skyboxEntity.addComponent(meshRendererComponent);
    return skyboxEntity;
  },

  createDefaultEnvironmentLightEntity: function () {
    var lightEntity = this.createEntity('EnvironmentalLight');

    let lightComponent = GASEngine.ComponentFactory.Instance.create('environmentalLight');
    lightComponent.setEnvironmentName(DefaultEnvironmentName);
    lightEntity.addComponent(lightComponent);
    return lightEntity;
  },

  createDefaultCameraEntity: function () {
    var cameraEntity = this.createEntity('CameraEntity');

    var cameraComponent = GASEngine.ComponentFactory.Instance.create('camera');
    var boundingBox = this._scene.getBoundingBox();
    var center = new GASEngine.Vector3();
    boundingBox.getCenter(center);
    var radius = boundingBox.getRadius();

    var farPlane = radius * 10.0;
    farPlane = farPlane < 99999.0 ? farPlane : 99999.0;
    var nearPlane = (farPlane / 1000.0 < 0.001) ? 0.001 : (farPlane / 1000.0);
    cameraComponent.far = farPlane;
    cameraComponent.near = nearPlane;
    cameraComponent.aspect = GASEngine.WebGLDevice.Instance.getCanvasWidth() / GASEngine.WebGLDevice.Instance.getCanvasHeight();
    cameraComponent.fov = 60.0;

    cameraEntity.addComponent(cameraComponent);
    return cameraEntity;
  },

  _onHierarchySuccess: function () {
    this._scene.setAnimationUpdateCallback(this.updateAnmationProgress.bind(this));
    this._scene.setAnimationStartCallback(this.updateAnimationIndex.bind(this));

    this._scene.update();
    this._models.push(this._scene.getModelRoot());

    let skyboxList = this._scene.getSkyboxList();
    if (skyboxList.length === 0) {
      var skyboxEntity = this.createSkyboxEntity();
      this._scene.appendEntityOnRoot(skyboxEntity);
      this._scene.appendSkybox(skyboxEntity);
    }

    let environmentLightList = this._scene.getEnvironmentalLightList();
    if (environmentLightList.length === 0) {
      var lightEntity = this.createDefaultEnvironmentLightEntity();
      this._scene.appendEntityOnRoot(lightEntity);
      this._scene.appendEnvironmentalLight(lightEntity);
    }

    let cameraEntityList = this._scene.getCameraEntityList();
    var cameraEntity;
    if (cameraEntityList.length === 0) {
      cameraEntity = this.createDefaultCameraEntity();
      this._scene.appendEntityOnRoot(cameraEntity);
      this._scene.appendCameraEntity(cameraEntity);
    } else {
      cameraEntity = cameraEntityList[0];
    }

    var cameraComponent = cameraEntity.getComponent('camera');
    this._manipulator.setCamera(cameraComponent);
    this.setCameraPosition();

    this.showSkeleton(this._skeletonEnable, false);

    // var gridEntity = new GASEngine.GridHelper(G_radius*10, G_radius*10, 20, 20, true, [0.5, 0.5, 0.5, 1.0]);
    // this._scene.appendEntityOnRoot(gridEntity);
    // gridEntity.enable = false;

    // var boxEntity = new GASEngine.BoxHelper(scene.root);
    // this._scene.appendEntityOnRoot(boxEntity);
    // boxEntity.enable = false;

    // var axisEntity = new GASEngine.AxisHelper(scene.root);
    // this._scene.appendEntityOnRoot(axisEntity);
    // axisEntity.enable = false;

    // console.log('onHierarchySuccess: Scene hierarchy load finished.');
  },

  onAnimationLoaded() {
    this._returnParent(-1, MessageType.AnimationLoaded);
  },

  getDefaultCameraParams() {
    let configData = this.getAssembleConfigurations();
    let sceneConfigData = configData.scene || {};
    let defaultCameraParams;
    if (sceneConfigData && "cameras" in sceneConfigData) {
      if (sceneConfigData.cameras && "default" in sceneConfigData.cameras) {
        defaultCameraParams = sceneConfigData.cameras.default;
      }
    }
    return defaultCameraParams;
  },

  setCameraPosition: function () {
    let defaultCameraParams = this.getDefaultCameraParams();
    if (defaultCameraParams) {
      var positionVec = new GASEngine.Vector3(defaultCameraParams.position.x, defaultCameraParams.position.y, defaultCameraParams.position.z);
      var targetVec = new GASEngine.Vector3(defaultCameraParams.orbitTarget.x, defaultCameraParams.orbitTarget.y, defaultCameraParams.orbitTarget.z);
      this._manipulator.cameraGoto(positionVec, targetVec);
    } else {
      var boundingBox = this._scene.getBoundingBox();
      var center = new GASEngine.Vector3();
      boundingBox.getCenter(center);
      var radius = boundingBox.getRadius();
      this._manipulator.computeHomePosition(center, radius);
    }
  },

  getCurrentAnimationClipIndex: function () {
    return this._scene.getCurrentAnimationClipIndex();
  },

  getAnimationClips: function () {
    const clipInfo = this._scene.getAllAnimationClipInfo();
    return clipInfo;
  },

  setAnimationLoopMode: function (loopMode) {
    this._scene.setAnimationGlobalLoopMode(loopMode);
  },

  setSpeedFactor: function (value) {
    this._scene.setAnimationGlobalSpeedFactor(value);
  },

  setClipClamp: function (value) {
    this._scene.setAnimationGlobalClamp(value);
  },

  setClipIndex: function (value) {
    this._scene.changeActiveAnimaitonClipByIndex(value);
  },

  setClipProgress: function (value) {
    this._scene.setActiveAnimationProgress(value);
  },

  _onStructureLoadSuccess: function () {
    this._returnParent(-1, MessageType.StructureLoaded);
  },

  _onFileLoaded: function (params) {
    this._returnParent(-1, MessageType.FileLoaded, params);
  },

  onFinishedCaptureCallback: function (data) {
    this._returnParent(-1, MessageType.CaptureGifFinished, data);
    if (data.type === 'finish') {
      this._nextFrame(() => {
        this._showHotspot();
      });
    }
  },

  _onSyncLoading: function (file, status, totalFiles) {
    ++this._syncFileCount;
    this._onFileLoaded({
      count: this._syncFileCount,
      total: totalFiles.size
    });
    if (status)
      console.log('[' + this._syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load successfully.');
    else
      console.error('[' + this._syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load failed.');
  },

  _onAsyncLoading: function (file, status, totalFiles) {
    ++this._asyncFileCount;
    this._onFileLoaded({
      count: this._asyncFileCount,
      total: totalFiles.size
    });
    if (status)
      console.log('[' + this._asyncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load successfully.');
    else
      console.error('[' + this._asyncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load failed.');
  },

  _onWindowResize: function () {
    var newWidth = window.innerWidth;
    var newHeight = window.innerHeight;

    this._canvas.width = newWidth;
    this._canvas.height = newHeight;

    let cameras = this._scene.findComponents('camera');
    for (let i = 0; i < cameras.length; ++i) {
      cameras[i].aspect = newWidth / newHeight;
      cameras[i]._updateViewMatrix();
      cameras[i]._updateProjectionMatrix();
      cameras[i]._updateViewProjectionMatrix();
    }

    GASEngine.WebGLDevice.Instance.setSize(newWidth, newHeight);

    if (GASEngine.HotspotManager.Instance) {
      GASEngine.HotspotManager.Instance.setCanvasSize(newWidth, newHeight);
    }

    GASEngine.PBRPipeline.Instance.onWindowResize(newWidth, newHeight);
  },


  _initCanvasMouseEventHandler: function () {
    var clientX = 0;
    var clientY = 0;
    if (this._canvas != undefined && this._canvas != null) {
      this._canvas.addEventListener("mousedown", function (event) {
        if (event.button == 0) {
          clientX = event.clientX;
          clientY = event.clientY;
        }

        if (GASEngine.HotspotManager.Instance) {
          if (event.button == 0) {
            var hotspot = GASEngine.HotspotManager.Instance.pickHotspot();
            if (hotspot) {
              this._eventInput.setEnable(false);
              GASEngine.HotspotManager.Instance.startDrag();
              this._dragFlag = true;
            }

            if (hotspot) {
              if (this.preSelectHotspot && hotspot.id !== this.preSelectHotspot.id || !this.preSelectHotspot) {
                let hotspots = this.getHotspots();
                let index = hotspots.findIndex(e => e.id === hotspot.id);
                hotspot.index = index;
                this._returnParent(-1, MessageType.HotspotPicked, hotspot);
                this.preSelectHotspot = hotspot;
              }
            } else if (!hotspot && this.preSelectHotspot) {
              this._returnParent(-1, MessageType.HotspotUnpicked, this.preSelectHotspot);
              this.preSelectHotspot = null;
            }
          }
        }
      }.bind(this));

      this._canvas.addEventListener("mousemove", function (event) {
        if (GASEngine.HotspotManager.Instance) {
          GASEngine.HotspotManager.Instance.setMousePosition(event.offsetX, event.offsetY);
          if (this._dragFlag)
            GASEngine.HotspotManager.Instance.drag();
        }
      }.bind(this));

      this._canvas.addEventListener("mouseup", function (event) {
        if (GASEngine.HotspotManager.Instance) {
          if (this._dragFlag) {
            this._eventInput.setEnable(true);
            GASEngine.HotspotManager.Instance.endDrag();
            this._dragFlag = false;
          }
        }
      }.bind(this));

      this._canvas.addEventListener('click', function (event) {
        if (event.ctrlKey) {
          var hotspot = GASEngine.HotspotManager.Instance.pickHotspot();
          if (hotspot) {
            var storedCameraStatus = GASEngine.CameraManager.Instance._retrieveStoredCameraStatus(hotspot.id);
            if (storedCameraStatus) {
              this._manipulator.cameraGoto(storedCameraStatus.eye, storedCameraStatus.target);
            }
          }
        }

        if (this.isAddHotspot) {
          if (GASEngine.HotspotManager.Instance) {
            if (this._manipulator.getCurrentManipulatorType() !== GASEngine.Manipulator.ORBIT) {
              console.error('Hotspot must be created under orbit controller.');
              return;
            }

            var hotspot = GASEngine.HotspotManager.Instance.createHotspot();
            if (hotspot) {
              this._returnParent(-1, MessageType.HotspotAdded, hotspot);
              this.isAddHotspot = false;

              var eye = new GASEngine.Vector3();
              var target = new GASEngine.Vector3();
              this._manipulator.getEyePosition(eye),
                this._manipulator.getTarget(target);

              GASEngine.CameraManager.Instance._storeCameraStatusForTest(
                hotspot.id,
                eye,
                target,
                this._manipulator.getCurrentManipulatorType());
            }
          }
        }
      }.bind(this), false);

      this._canvas.addEventListener('dblclick', function (event) {
        this.setCameraPosition();
      }.bind(this), false);
    }

    //for debug
    // window.addEventListener('keydown', function(event)
    // {
    //     if(event.code === 'KeyO') {
    //         this._manipulator.setManipulatorType(GASEngine.Manipulator.ORBIT);
    //     } else if(event.code === 'KeyF') {
    //         this._manipulator.setManipulatorType(GASEngine.Manipulator.FPS);
    //     }
    // }.bind(this), false)
  },

  updateAnmationProgress: function (clip) {
    var options = {
      progress: clip.progress,
      localTime: clip.localTime
    }
    this._returnParent(-1, MessageType.AnimationProgress, options);
  },

  updateAnimationIndex: function (clip) {
    this._returnParent(-1, MessageType.AnimationStart, clip.id);
  },

  shot(options = {}) {
    this._hideHotspot();
    const maxSize = options.maxSize || 480;
    const widthImage = options.withImage;
    const width = this._canvas.width;
    const height = this._canvas.height;
    let scaleWidth = width > height ? maxSize : maxSize * (width / height);
    let scaleHeight = width > height ? maxSize * (height / width) : maxSize;

    const formatOption = {
      withCover: true,
      withCameraParam: true,
      withImage: widthImage,
      withHotspot: false,
      width: scaleWidth,
      height: scaleHeight
    };

    this._nextFrame(() => {
      this.capture(formatOption);
    });
  },

  capture(opt) {
    const data = {};
    if (opt.withCover) {
      data.coverImage = this._canvas.toDataURL();
    }

    if (opt.withImage) {
      data.image = this.captureImage(opt.width, opt.height);
    }

    if (opt.withCameraParam) {
      var eye = new GASEngine.Vector3();
      var target = new GASEngine.Vector3();
      var manipulator = this._manipulator.getCurrentManipulator();
      manipulator.getEyePosition(eye),
        manipulator.getTarget(target);
      const cameraParam = {
        version: 2.1,
        position: eye,
        orbitTarget: target
      };

      data.cameraParam = JSON.stringify(cameraParam);
    }
    this._returnParent(-1, MessageType.Shoted, data);

    this._nextFrame(() => {
      this._showHotspot();
    });
  },

  _hideHotspot() {
    this._isShowHotspot = GASEngine.PBRPipeline.Instance.showHotspot;
    if (this._isShowHotspot) {
      this._isShowHotspot = false;
      GASEngine.PBRPipeline.Instance.showHotspot = this._isShowHotspot;
    }
  },

  _showHotspot() {
    if (!this._isShowHotspot) {
      this._isShowHotspot = true;
      GASEngine.PBRPipeline.Instance.showHotspot = this._isShowHotspot;
    }
  },

  _nextFrame(fn) {
    setTimeout(fn, 100);
  },

  captureImage(width, height) {
    if (this._canvas) {
      var w = width,
        h = height,
        cw = this._canvas.width,
        ch = this._canvas.height;
      var sx, sy, sw, sh;
      //console.log('capture', w, h, cw, ch);

      if (w / h > cw / ch) {
        sx = 0;
        sy = (ch - h / w * cw) / 2.0;
        sw = cw;
        sh = (h / w * cw);
      } else {
        sx = (cw - w / h * ch) / 2.0;
        sy = 0;
        sw = (w / h * ch);
        sh = ch;
      }

      var canvas = document.createElement('canvas');
      canvas.width = w;
      canvas.height = h;
      context = canvas.getContext('2d');
      context.drawImage(this._canvas, sx, sy, sw, sh, 0, 0, w, h);

      var imageData = canvas.toDataURL('image/jpeg', 0.7);
      return imageData;
    } else {
      return undefined;
    }
  },

  captureGifStart(callback) {
    this._hideHotspot();

    this._nextFrame(() => {
      this._captureGif.start(this._canvas, callback);
    });
  },

  captureGifStop() {
    this._captureGif.stop();
  },

  getAssembleConfigurations() {
    return {};
  },

  // setAssembleConfigurations(options)
  // {
  //     this._sceneLoader_.setAssembleConfigurations(options);
  // },

  // Editor
  deleteModelByEntity(entity) {
    const sceneRoot = this._scene.root;
    if (entity && sceneRoot.findChild(entity) !== -1) {
      this.enableAnimation(false); // 清除前停止播放动画
      GASEngine.SkeletonManager.Instance.finl(); // 清除前解绑骨骼
      if (this._scene.getModelRoot() === entity)
        this._scene.setModelRoot(null);
      this._scene.removeEntityOnRoot(entity);
      for (var i = 0; i < this._models.length; ++i) {
        if (this._models[i] === entity) {
          this._models.splice(i, 1);
          break;
        }
      }
      GASEngine.EntityFactory.Instance.destroy(entity);
      return true;
    }
    return false;
  },
  deleteCurrentModel() {
    const modelRoot = this._scene.getModelRoot();
    return this.deleteModelByEntity(modelRoot);
  },

  deleteModelByUniqueId(uniqueId) {
    const sceneRoot = this._scene.root;
    const entity = sceneRoot.findChildByID(uniqueId);
    return this.deleteModelByEntity(entity);
  },

  deleteAnimationClipById(clipId) {
    const modelRoot = this._scene.getModelRoot();
    if (!modelRoot) return false;
    const animatorComponent = modelRoot.getComponent('animator');
    if (!animatorComponent) return false;
    let activeClip = animatorComponent.getActiveAnimationClip();
    if (activeClip) {
      animatorComponent.stop(activeClip.id);
    }

    animatorComponent.deleteAnimationClip(clipId);
    this._returnParent(-1, MessageType.AnimationDelete, clipId);

    if (!activeClip || activeClip.id === clipId) {
      const clips = animatorComponent.getAnimationClips();
      activeClip = clips.length > 0 ? clips[0] : null;
    }

    if (activeClip) {
      animatorComponent.play(activeClip.id);
    }
  },

  getRootDirectory() {
    return GASEngine.FileSystem.Instance.rootDirectory;
  },

  getModels() {
    const modelInfos = [];
    this._models.forEach(m => {
      modelInfos.push({
        id: m.uniqueID,
        name: m.name
      });
    });
    return modelInfos;
  },

  getModelStructure(uniqueID) {
    const meshInfoList = this.getModelMeshList(uniqueID);

    const materialInfoList = this.getModelMaterialList(uniqueID);

    const animationclipsInfo = this.getModelAnimationClips(uniqueID);

    return {
      mesh: meshInfoList,
      material: materialInfoList,
      animationClips: animationclipsInfo
    };
  },

  getModelMeshList(uniqueID) {
    const index = this._models.findIndex(e => e.uniqueID === uniqueID);
    if (index === -1) return false;
    const model = this._models[index];
    const meshList = [];
    model.getMeshList_r(meshList);
    const meshInfoList = meshList.map(e => {
      const {
        parentEntity
      } = e;
      const {
        mesh
      } = e;
      return {
        name: mesh.name,
        id: mesh.uniqueID,
        parentEntityId: parentEntity.uniqueID
      };
    });
    return meshInfoList;
  },

  showMesh({
    id,
    visible
  }) {
    const sceneRoot = this._scene.getModelRoot();
    if (!sceneRoot) return false;
    const entity = sceneRoot.findObjectByID_r(id);
    if (!entity) return false;
    entity.enable = visible;
    return true;
  },

  getMeshUVData(id) {
    const sceneRoot = this._scene.getModelRoot();
    if (!sceneRoot) return null;
    const meshList = [];
    sceneRoot.getMeshList_r(meshList);
    const index = meshList.findIndex(e => e.mesh.uniqueID === id);
    if (index === -1) return null;
    const {
      mesh
    } = meshList[index];
    if (!mesh) return null;
    const topologyStream = mesh.getStream('uvtopology');
    const uvStream1 = mesh.getStream('uv');
    const uvStream2 = mesh.getStream('uv2');
    return {
      1: [uvStream1, topologyStream],
      2: [uvStream2, topologyStream]
    }
  },

  getModelMaterialList(uniqueID) {
    const index = this._models.findIndex(e => e.uniqueID === uniqueID);
    if (index === -1) return false;
    const model = this._models[index];

    const materialList = [];
    model.getMaterialList_r(materialList);

    const materialInfoList = [];
    materialList.forEach(e => {
      const {
        parentEntity
      } = e;
      const {
        materials
      } = e;
      materials.forEach(m => {
        materialInfoList.push({
          name: m.name,
          id: m.uniqueID,
          type: m.typeName,
          parentEntityId: parentEntity.uniqueID
        });
      });
    });

    return materialInfoList;
  },

  getModelAnimationClips(uniqueID) {
    const index = this._models.findIndex(e => e.uniqueID === uniqueID);
    if (index === -1) return false;
    const model = this._models[index];

    const animatorComponent = model.getComponent('animator');
    let animationclips = [];
    if (animatorComponent) {
      animationclips = animatorComponent.getAnimationClips();
    }

    const animationclipsInfo = animationclips.map(e => {
      return {
        name: e.name,
        id: e.id
      };
    });

    return animationclipsInfo;
  },

  _getSkyboxMaterial() {
    const skyboxList = this._scene.getSkyboxList();
    if (skyboxList.length === 0) return false;
    const skyboxEntity = skyboxList[0];
    if (!skyboxEntity) return false;
    const materialComponent = skyboxEntity.getComponent('meshRenderer');
    if (!materialComponent) return false;
    const materials = materialComponent.getMaterials();
    if (!materials.length === 0) return false;
    const skyboxMaterial = materials[0];
    if (!skyboxMaterial) return false;
    return skyboxMaterial;
  },

  getBackgroundInfo() {
    const skyboxMaterial = this._getSkyboxMaterial();
    if (!skyboxMaterial) return false;
    let type = skyboxMaterial.backgroundType;
    let lightEnable = false;
    if (type === BackgroundType.AMBIENT) {
      type = BackgroundType.CUBEMAP;
      lightEnable = true;
    }
    const params = {};
    const solidColor = skyboxMaterial.solidColor;
    params.backgroundColor = {
      r: solidColor[0],
      g: solidColor[1],
      b: solidColor[2]
    };
    params.backgroundImage = skyboxMaterial.backgroundName;

    params.cubeMapName = skyboxMaterial.cubeMapName;
    params.lightEnable = lightEnable;
    params.environmentBrightness = Math.round(100 * skyboxMaterial.getBackgroundExposure()) / 100;
    params.environmentBlur = skyboxMaterial.environmentBlur;

    const backgroundInfo = {
      type,
      ...params
    };
    return backgroundInfo;
  },

  setBackground(backgroundParams) {
    const skyboxMaterial = this._getSkyboxMaterial();
    if (!skyboxMaterial) return false;

    let {
      type
    } = backgroundParams;

    const {
      lightEnable
    } = backgroundParams;
    if (lightEnable !== undefined) {
      type = lightEnable ? BackgroundType.AMBIENT : BackgroundType.CUBEMAP;
      this.storeSceneStructureChange('backgroundCubeAmbientEnable', lightEnable);
    }

    if (type !== undefined) {
      skyboxMaterial.backgroundType = type;
      this.storeSceneStructureChange('backgroundType', type);
      switch (type) {
        case BackgroundType.CUBEMAP: {
          const cubeMapName = skyboxMaterial.cubeMapName || DefaultEnvironmentName;
          skyboxMaterial.setBackgroundCubeMap(cubeMapName);
          break;
        }
        case BackgroundType.IMAGE: {
          const backgroundName = skyboxMaterial.backgroundName || DefaultBackgroundImage;
          skyboxMaterial.setBackgroundImage(backgroundName);
          break;
        }
      }
    }
    const {
      backgroundColor
    } = backgroundParams;
    if (backgroundColor !== undefined) {
      const r = backgroundColor.r;
      const g = backgroundColor.g;
      const b = backgroundColor.b;
      skyboxMaterial.setSolidColor(r, g, b);
      this.storeSceneStructureChange('backgroundColor', [r, g, b]);
    }
    const {
      backgroundImage
    } = backgroundParams;
    if (backgroundImage !== undefined) {
      skyboxMaterial.setBackgroundImage(backgroundImage);
      this.storeSceneStructureChange('backgroundImage', backgroundImage);
    }
    const {
      cubeMapName
    } = backgroundParams;
    if (cubeMapName !== undefined) {
      skyboxMaterial.setBackgroundCubeMap(cubeMapName);
      this.storeSceneStructureChange('backgroundCubeName', cubeMapName);
    }

    const {
      environmentBrightness
    } = backgroundParams;
    if (environmentBrightness !== undefined) {
      skyboxMaterial.setBackgroundExposure(environmentBrightness);
      this.storeSceneStructureChange('backgroundExposure', environmentBrightness);
    }

    const {
      lightBrightness
    } = backgroundParams;
    if (lightBrightness !== undefined) {
      skyboxMaterial.setLightExposure(lightBrightness);
      this.storeSceneStructureChange('backgroundCubeExposure', lightBrightness);
    }

    const {
      environmentBlur
    } = backgroundParams;
    if (environmentBlur !== undefined) {
      skyboxMaterial.setEnvironmentBlur(environmentBlur);
      this.storeSceneStructureChange('environmentBlur', environmentBlur);
    }

    const {
      orientation
    } = backgroundParams;
    if (orientation !== undefined) {
      skyboxMaterial.setOrientation(orientation);
      this.storeSceneStructureChange('backgroundCubeOrientation', orientation);
    }
    return true;
  },

  _getEnvironmentalLight() {
    const environmentLightList = this._scene.getEnvironmentalLightList();
    if (environmentLightList.length === 0) return false;
    const environmentLightEntity = environmentLightList[0];
    if (!environmentLightEntity) return false;
    const environmentLightComponent = environmentLightEntity.getComponent('environmentalLight');
    if (!environmentLightComponent) return false;
    return environmentLightComponent;
  },

  getEnvironmentInfo() {
    const environmentLightComponent = this._getEnvironmentalLight();
    if (!environmentLightComponent) return false;
    const environmentName = environmentLightComponent.environmentName;
    const orientation = environmentLightComponent.orientation;
    const intensity = Math.round(100 * environmentLightComponent.getEnvironmentExposure()) / 100;
    const environmentLightInfo = {
      environmentName,
      orientation,
      intensity
    };
    return environmentLightInfo;
  },

  setEnvironmentInfo(options) {
    const environmentLightComponent = this._getEnvironmentalLight();
    if (!environmentLightComponent) return false;
    if (options.environmentName !== undefined) {
      const {
        environmentName
      } = options;
      environmentLightComponent.setEnvironmentName(environmentName);
      this.storeSceneStructureChange('environmentName', environmentName);
      this.setBackground({
        cubeMapName: environmentName
      });
    }
    if (options.intensity !== undefined) {
      environmentLightComponent.setEnvironmentExposure(options.intensity);
      this.storeSceneStructureChange('environmentExposure', options.intensity);
    }
    if (options.orientation !== undefined) {
      environmentLightComponent.orientation = options.orientation;
      this.storeSceneStructureChange('environmentOrientation', options.orientation);
    }
  },

  getRendererInfo() {
    const renderInfo = {};
    renderInfo.renderMode = GASEngine.PBRPipeline.Instance.viewMode;
    const rgba = GASEngine.PBRPipeline.Instance.topologyLineColor;
    renderInfo.wireframeColor = {
      r: rgba[0],
      g: rgba[1],
      b: rgba[2]
    };
    renderInfo.wireframeOpacity = GASEngine.PBRPipeline.Instance.topologyLineColor[3];
    renderInfo.wireframeEnable = GASEngine.PBRPipeline.Instance.isShowTopologyObjects;
    renderInfo.showTPose = GASEngine.PBRPipeline.Instance.skinningFlag;
    renderInfo.showSkelentor = this._skeletonEnable;
    renderInfo.skelentorScale = GASEngine.SkeletonManager.Instance.scale;
    return renderInfo;
  },

  _onRendererLoaded: function (renderConf) {
    if (renderConf.renderMode !== undefined) {
      const {
        renderMode
      } = renderConf;
      this.changeRenderMode(renderMode, false);
    }
    if (renderConf.wireframe !== undefined) {
      const {
        wireframe
      } = renderConf;
      if (wireframe.enable !== undefined) {
        const {
          enable
        } = wireframe;
        this.showTopology(enable, false);
      }
      if (wireframe.color !== undefined) {
        const {
          color
        } = wireframe;
        this.setTopologyColor(color, false);
      }
      if (wireframe.opacity !== undefined) {
        const {
          opacity
        } = wireframe;
        this.setTopologyAlpha(opacity, false);
      }
    }
    if (renderConf.skeleton !== undefined) {
      const {
        skeleton
      } = renderConf;
      if (skeleton.scale !== undefined) {
        const {
          scale
        } = skeleton;
        this.setSkeletonScale(scale, false);
      }
      if (skeleton.enable !== undefined) {
        const {
          enable
        } = skeleton;
        this.showSkeleton(enable, false);
      }
    }
    if (renderConf.tpose !== undefined) {
      const {
        tpose
      } = renderConf;
      if (tpose.enable !== undefined) {
        const {
          enable
        } = tpose;
        this.showTPose(enable, false);
      }
    }
  },

  getSceneStructure() {
    const sceneStructure = {};
    return sceneStructure;
  },

  deleteHotspot(hotspotId) {
    GASEngine.HotspotManager.Instance.deleteHotspot(hotspotId);
  },

  createHotspot: function () {
    return GASEngine.HotspotManager.Instance.createHotspot();
  },

  getHotspots: function () {
    return GASEngine.HotspotManager.Instance.getHotspots();
  },

  _onIFrameMessage: function (invokeID, cmd, parameters) {
    if ((typeof cmd) === 'string' && cmd.length > 0) {
      if (this[cmd]) {
        var result = this[cmd](parameters);
        if (result instanceof Promise) {
          result.then((res) => {
            this._returnParent(invokeID, cmd, res);
          }).catch((error) => {
            this._returnParent(invokeID, cmd, error);
          })
        } else {
          this._returnParent(invokeID, cmd, result);
        }
      } else {
        console.error('invoked function is invalid.');
      }
    }
  },

  _returnParent: function (invokeID, cmd, parameters) {
    if (window.parent && window !== window.parent) {
      window.parent.postMessage({
          'invokeID': invokeID,
          'cmd': cmd,
          'parameters': parameters
        },
        "*"
      );
    } else {
      console.error('Can not find the parent window of ArtHub 3D viewer iframe.');
    }
  },

  getMaterialByID: function (uniqueID, srcEntity) {
    if (!srcEntity) {
      srcEntity = this._scene.root;
    }
    const materialComponent = srcEntity.getComponent('meshRenderer');
    if (materialComponent) {
      var materials = materialComponent.getMaterials();
      for (var i = 0; i < materials.length; ++i) {
        if (materials[i] && materials[i].uniqueID && materials[i].uniqueID === uniqueID) {
          return materials[i];
        }
      }
    }

    for (var i = 0; i < srcEntity.children.length; i++) {
      var mat = this.getMaterialByID(uniqueID, srcEntity.children[i]);
      if (mat) {
        return mat;
      }
    }

    return null;
  },

  invokeMaterialField: function (options) {
    // value can be null
    if (options.field && options.id) {
      // console.log(options.field);
      var mat = this.getMaterialByID(options.id);

      if (options.field.startsWith('set')) {
        this.storeMaterialChange(mat, options.field, options.value);
      }

      var target = new ArtHubMaterialAdapter(mat, this);
      if (Reflect.has(target.getAdapter(), options.field)) {
        return (target.getAdapter())[options.field](options.value);
      } else {
        return null;
      }
    } else {
      return null;
    }
  },

  storeMaterialChange: function (material, field, rawValue) {
    const rawField = field.slice(3);
    const key = rawField.charAt(0).toLowerCase() + rawField.slice(1);
    const value = key.endsWith('Color') ? color2RGB(rawValue) : rawValue;
    this.commitToSaver('material', {
      material,
      key,
      value
    });
  },

  storeRendererChange: function (key, value) {
    this.commitToSaver('renderer', {
      key,
      value
    });
  },

  storeSceneStructureChange: function (key, value) {
    this.commitToSaver('scene', {
      key,
      value
    });
  },

  commitToSaver(type, params = {}) {
    const saverInstance = GASEngine.GAS2IncrementalSaver.Instance;
    if (saverInstance) {
      saverInstance.storeChange(type, params);
    } else {
      console.log('saverInstance should init first!');
    }
  },

  getTextureName: function (pathArray) {
    return this._sceneLoader_.getTextureName(pathArray);
  },

  // get texture already in cache
  getTextureList: function () {
    if (!GASEngine.Resources.Instance) {
      return [];
    }
    var ret = [];
    GASEngine.Resources.Instance.textureCache.forEach((value, key) => {
      if (key.indexOf('/system') === -1 && value && value.image)
        ret.push({
          path: key,
          image: encodeImageFileAsURL(value.image)
        });
    })
    return ret;
  },

  removeTextureByPath: function (path) {
    if (!GASEngine.Resources.Instance) {
      return false;
    }
    return GASEngine.Resources.Instance.textureCache.delete(path);
  },

  clearTextureList: function () {
    var textureList = this.getTextureList();
    textureList.forEach(texture => this.removeTextureByPath(texture.path));
    return true;
  },

  initIncrementalSaver: function () {
    new ArtHubIncrementalSaver();
    GASEngine.GAS2IncrementalSaver.Instance.init();
  },

  destroyIncrementalSaver: function () {
    if (GASEngine.GAS2IncrementalSaver.Instance) {
      GASEngine.GAS2IncrementalSaver.Instance.destroy();
      GASEngine.GAS2IncrementalSaver.Instance = null;
    }
  },

  saverApplyChange: function () {
    if (GASEngine.GAS2IncrementalSaver.Instance) {
      return GASEngine.GAS2IncrementalSaver.Instance.applyChange();
    } else {
      console.error('The saver should init first!');
    }
  },

  _getBaseURL() {
    let pathName = window.location.pathname;
    if (pathName.startsWith('/')) {
      pathName = pathName.slice(1);
    }
    if (pathName.endsWith('/')) {
      pathName = pathName.slice(0, pathName.length - 1);
    }
    const pathArr = pathName ? pathName.split('/') : [];
    const depotName = pathArr.length >= 0 ? pathArr[0] : '';
    let baseURL = depotName ? `${window.location.host}/${depotName}/gas` : `${window.location.host}`;
    return baseURL;
  }
};

var viewer = new ArtHub3DViewerIFrame();
viewer.run();
window.addEventListener('unload', () => {
  viewer.destroyEngine();
  viewer.destroyIncrementalSaver();
});
window.iframe = viewer;