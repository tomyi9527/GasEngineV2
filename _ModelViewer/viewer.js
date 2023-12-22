
//'use strict';
var CANVAS_ID = 'application-canvas';
var canvas;

function createCanvas()
{
    canvas = document.createElement('canvas');
    canvas.setAttribute('id', CANVAS_ID);
    canvas.setAttribute('tabindex', 0);
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    // canvas.style.visibility = 'hidden';

    // Disable I-bar cursor on click+drag
    canvas.onselectstart = function () { return false; };

    document.body.appendChild(canvas);

    return canvas;
};

// you shouldn't need to modify much below here
var G_Clock;
var G_manipulator;
var G_cameraComponent;
var G_cameraEntity;
var G_lightEntity;
var G_gridEntity = null;
var G_boxEntity = null;
var G_axisEntity = null;
var G_radius = 10;
var G_center = new GASEngine.Vector3();
var G_scene;
var G_container;
var G_fps;
var G_polygon;
var G_fpsFrameCount = 0;
var G_fpsAccumulatedTime = 0.0;

let syncFileCount = 0;
let asyncFileCount = 0;
let isLeftDrag = false;
let isRightDrag = false;
let delay = 10.15;
let isDisplay = true;

function createComponent(name)
{
    return GASEngine.ComponentFactory.Instance.create(name);
};

function createEntity(name, translation, rotation, scale)
{
    let entity = GASEngine.EntityFactory.Instance.create();
    entity.uniqueID = GASEngine.generateUUID();

    if (name) entity.name = name;
    if (translation) entity.translation.copy(translation);
    if (rotation) entity.rotation.copy(rotation);
    if (scale) entity.scale.copy(scale);

    return entity;
};

function onCanvasResize(width, height)
{
    canvas.width = width;
    canvas.height = height;

    var container = document.getElementById("application-canvas");
    var canvasWidth = container.clientWidth;
    var canvasHeight = container.clientHeight;

    let cameras = G_scene.findComponents('camera');
    for (let i = 0; i < cameras.length; ++i)
    {
        cameras[i].aspect = width / height;
        cameras[i]._updateViewMatrix();
        cameras[i]._updateProjectionMatrix();
        cameras[i]._updateViewProjectionMatrix();
    }

    GASEngine.WebGLDevice.Instance.setSize(width, height);

    if (GASEngine.HotspotManager.Instance) 
    {
        GASEngine.HotspotManager.Instance.setCanvasSize(canvasWidth, canvasHeight);
    }

    GASEngine.PBRPipeline.Instance.onWindowResize(canvasWidth, canvasHeight);
};

function init()
{
    G_Clock = new GASEngine.Clock();
    canvas = createCanvas();

    var canvasWidth = canvas.width;
    var canvasHeight = canvas.height;

    var options = {
        postprocesseffects: {
            enable: false,
            glow:
            {
                threshold: 0.0,
                intensity: 0.2,
                radius: 1.0
            }
        },
        outline: true
    }

    var webGLDevice = new GASEngine.WebGLDevice();
    webGLDevice.init(canvas, canvasWidth, canvasHeight);
    (new GASEngine.WebGLTextureManager());
    (new GASEngine.PBRPipeline(webGLDevice.gl)).init(canvasWidth, canvasHeight, options);

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
    (new GASEngine.Keyboard(canvas));
    (new GASEngine.Mouse(canvas));
    (new GASEngine.CameraManager());
    (new GASEngine.HotspotManager(canvasWidth, canvasHeight));

    (new GASEngine.FileSystem()).init('', '../system/', '');

    initScene();
    initOptions();

    window.addEventListener('resize', () =>
    {
        onCanvasResize(window.innerWidth, window.innerHeight);
    }, false);
};

//hide context menu
document.oncontextmenu = function(event) {
    event.preventDefault();
    event.stopPropagation();
    return false;
};

//pick object
var G_hotspot;
var G_hotspotDrag;
var G_clientX = 0;
var G_clientY = 0;

function onMouseDown(event)
{
    G_clientX = event.clientX;
    G_clientY = event.clientY;

    if (event.button === 0)
    {
        if(GASEngine.HotspotManager.Instance) {
            G_hotspot = GASEngine.HotspotManager.Instance.pickHotspot();

            if (G_hotspot)
            {
                G_hotspotDrag = GASEngine.HotspotManager.Instance.startDrag();
                if (G_hotspotDrag)
                    return;
            }
        }

        if (G_scene) {
            isLeftDrag = true;
            G_manipulator.getRotateInterpolator().reset();
        }
    }
    else if (event.button === 1)
    {
        if (G_container)
        {
            var display = isDisplay ? 'none' : '';
            G_container.setDisplay(display);
            
            focusButton && focusButton.setDisplay(display);
            openButton && openButton.setDisplay(display);
            sceneNameInput && sceneNameInput.setDisplay(display)
            isDisplay = !isDisplay;
        }
    }
    else if(event.button === 2) {
        if (G_scene) {
            isRightDrag = true;
            G_manipulator.getPanInterpolator().reset();
        }
    }
};

function onDoubleClick(event)
{
    var hotspot = GASEngine.HotspotManager.Instance.createHotspot();
    if (hotspot)
    {
        var eye = new GASEngine.Vector3();
        var target = new GASEngine.Vector3();
        G_manipulator.getEyePosition(eye),
            G_manipulator.getTarget(target);
        var type = G_manipulator.getType();

        GASEngine.CameraManager.Instance._storeCameraStatusForTest(hotspot.id, eye, target, type);
    }
};

function onMouseUp(event)
{
    if(G_hotspotDrag)
    {
        GASEngine.HotspotManager.Instance.endDrag();
        G_hotspotDrag = null;
        return;
    }
    
    if (isLeftDrag)
    {
        isLeftDrag = false;
    }
    if (isRightDrag)
    {
        isRightDrag = false;
    }

    if(event.button === 0 && (Math.abs(G_clientX - event.clientX) < 3 && Math.abs(G_clientY - event.clientY) < 3)) {
        var canvasWidth = event.currentTarget.width;
        var canvasHeight = event.currentTarget.height;
        var ratioX = (event.offsetX / canvasWidth) * 2 - 1;
        var ratioY = -(event.offsetY / canvasHeight) * 2 + 1;

        G_scene.pickObject(ratioX, ratioY);

        clientX = event.clientX;
        clientY = event.clientY;
    }
};

function onMouseMove(event)
{
    let rect = canvas.getBoundingClientRect();
    var offsetX = event.x - rect.left;
    var offsetY = event.y - rect.top;

    //update hotspot mouse position
    GASEngine.HotspotManager.Instance.setMousePosition(offsetX, offsetY);
    if(G_hotspotDrag)
    {
        GASEngine.HotspotManager.Instance.drag();
        return;
    }

    if (isLeftDrag)
    {
        let posX = offsetX - (rect.width / 2);
        let posY = offsetY - (rect.height / 2);

        G_manipulator.getRotateInterpolator().setDelay(delay);
        G_manipulator.getRotateInterpolator().setTarget(posX, posY);
        event.preventDefault();
        return;
    }

    if(isRightDrag) {
        let posX = offsetX - (rect.width / 2);
        let posY = offsetY - (rect.height / 2);

        G_manipulator.getPanInterpolator().setDelay(delay);
        G_manipulator.getPanInterpolator().setTarget(posX, posY);
        event.preventDefault();
        return;
    }
}

function onMouseWheel(event)
{
    let delta = 0;

    if (event.wheelDelta)
    {
        delta = event.wheelDelta / 120;
    }

    if (event.detail)
    {
        delta = - event.detail / 3;
    }

    let zoomTarget = G_manipulator.getZoomInterpolator().getTarget()[0] - delta;
    G_manipulator.getZoomInterpolator().setTarget(zoomTarget);
};

function createDefaultCamera()
{
    //create camera
    G_cameraEntity = createEntity('MainCamera');
    G_cameraComponent = createComponent('camera');
    G_cameraComponent.type = 'perspective';
    G_cameraComponent.fov = 60;
    G_cameraComponent.aspect = canvas.width / canvas.height;
    G_cameraComponent.far = 2000;
    G_cameraEntity.addComponent(G_cameraComponent);

    G_manipulator = new GASEngine.OrbitManipulator();
    G_manipulator.computeHomePosition(new GASEngine.Vector3(0, 0, 0), 40);
    G_manipulator.setAutoPushTarget(true);
    G_manipulator.setCamera(G_cameraComponent);

    G_scene.appendEntityOnRoot(G_cameraEntity);
    return G_cameraEntity;
};

function createDefaultEnvironmentLight()
{
    //create environment light
    G_lightEntity = createEntity('Light1');
    G_lightEntity.type = 'helper';
    let lightComponent = createComponent('environmentalLight');
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

    G_lightEntity.addComponent(lightComponent);
    G_scene.appendEntityOnRoot(G_lightEntity);
    return G_lightEntity;
};

function computeShadingMode()
{
    var isWireframeOverlay = wireframeOverlay.getValue();
    var shadingModeValue = '';
    var shadingOptions = {};

    G_scene.root.traverse(function(entity) {
        var meshFilter = entity.getComponent('meshFilter');
        if(meshFilter) {
            var mesh = meshFilter.getMesh();
            if(mesh) {
                mesh.isWireframeOverlay = isWireframeOverlay;
            }
        }

        if(shadingModeValue === '') {
            var meshRenderer = entity.getComponent('meshRenderer');
            if(meshRenderer) {
                var materials = meshRenderer.getMaterials();
                if(materials.length > 0 && materials[0] instanceof GASEngine.CompoundMaterial) {
                    shadingModeValue = materials[0].getActiveMaterial().typeName;

                    for (var typeName of materials[0].materials.keys()) {
                        shadingOptions[typeName] = typeName;
                    }
                }
            }
        }
    });
    if(shadingModeValue === '') {
        shadingOptions = {
            'wireframe' : 'wireframe',
            'pureColor' : 'pureColor',
            'blinnPhong': 'blinnPhong',
            'dielectric': 'dielectric',
            'electric': 'electric',
            'matcap': 'mapcap',
        }
        shadingModeValue = 'dielectric';
    }
    shadingMode.setOptions(shadingOptions);
    shadingMode.setValue(shadingModeValue);
};

function initOptions()
{
    var postprocesseffects = GASEngine.PBRPipeline.Instance.postprocesseffects;
    if(postprocesseffects) {
        showGlow.setValue(postprocesseffects.enable);
        var glowParameters = postprocesseffects.glow;
        if(glowParameters) {
            glowThreshold.setValue(glowParameters.threshold);
            glowIntensity.setValue(glowParameters.intensity);
            glowRadius.setValue(glowParameters.radius);
        }
    }
    showVRMode.setValue(!!GASEngine.PBRPipeline.Instance.VRMode);
};

//called on setup. Customize this
function initScene()
{
    // create scene
    G_scene = new GASEngine.Scene();

    //add event
    canvas.addEventListener('mousedown', onMouseDown, false);
    canvas.addEventListener('mouseup', onMouseUp, true);
    canvas.addEventListener('mousemove', onMouseMove, false);
    canvas.addEventListener('mousewheel', onMouseWheel, false);
    canvas.addEventListener('dblclick', onDoubleClick, false);

    //create camera
    createDefaultCamera();

    //create environment light
    createDefaultEnvironmentLight();
};

//called on every frame. customize this
function render()
{
    if (G_scene !== null)
    {
        G_scene.cull();

        let delta = G_Clock.getDelta();
        G_scene.update(delta);

        var cameras = G_scene.getCameras();
        var cameraCount = G_scene.getCameraCount();
        if (cameraCount > 0)
        {
            G_manipulator.update(delta);
            var cameraMatrixWorld = G_manipulator.getCameraWorldMatrix();
            G_cameraComponent.setWorldMatrix(cameraMatrixWorld); //overwrite view matrix produced in updating procedure.
            G_cameraComponent._updateViewMatrix();

            if(GASEngine.HotspotManager.Instance && cameras[0])
            {
                GASEngine.HotspotManager.Instance.update(cameras[0]);
            }
            GASEngine.PBRPipeline.Instance.render_V2(cameras, cameraCount);
        }
        ++G_fpsFrameCount;
        if (G_fpsAccumulatedTime > 1.0 - 1.0 / 60.0)
        {
            G_fps && G_fps.setValue(G_fpsFrameCount + 'fps');
            G_fpsFrameCount = 0, G_fpsAccumulatedTime = 0;

            //update triangles count
            if (G_polygon)
            {
                var triangles = G_scene.root.getTrianglesCount();
                G_polygon.setValue(triangles + ' polygons');
            }
        }
        else
        {
            G_fpsAccumulatedTime += delta;
        }
    }
    else if (G_fps) {
        G_fps.setValue('60fps');
    }
    requestAnimationFrame(render);
};

function computeHomeCamera()
{
    if(!G_scene)
        return;

    if(G_manipulator) {
        G_scene.root.updateWorldMatrix_r();
        var boundingBox = G_scene.getBoundingBox();
        boundingBox.getCenter(G_center);
        var radius = boundingBox.getRadius();
        if(radius === 0) {
            radius = 10;
        }
        G_manipulator.computeHomePosition(G_center, radius);
        var farPlane = radius * 10.0;
        farPlane = farPlane > 99999.0 ? 99999.0: farPlane;
        var nearPlane = farPlane / 1000;
        nearPlane = nearPlane < 0.001 ? 0.001: nearPlane;

        G_cameraComponent.far = farPlane;
        G_cameraComponent.near = nearPlane;
        G_cameraComponent.aspect = canvas.width / canvas.height;
        G_cameraComponent.fov = 60.0;

        G_radius = radius;
    }
};

//Accept drop files
var G_filesMap = {};

function loadFiles(files)
{
    for (var i = 0; i < files.length; i++)
    {

        var file = files[i];
        G_filesMap[file.name] = file;
    }

    GASEngine.FileSystem.Instance.setURLModifier(function (path)
    {
        var file = G_filesMap[path];
        if (file)
        {
            console.log('Loading ', path);
            return URL.createObjectURL(file);
        }
        return path;
    });

    for (var i = 0; i < files.length; i++)
    {
        loadFile(files[i]);
    }
};

function onHierarchySuccess(scene)
{
    if (G_scene)
    {
        G_scene.destroy();
        G_scene = null;
    }

    G_scene = scene;

    computeShadingMode();
    computeHomeCamera();

    G_gridEntity = new GASEngine.GridHelper(G_radius*10, G_radius*10, 20, 20, true, [0.5, 0.5, 0.5, 1.0]);
    G_scene.appendEntityOnRoot(G_gridEntity);
    G_gridEntity.enable = false;

    G_boxEntity = new GASEngine.BoxHelper(scene.root);
    G_scene.appendEntityOnRoot(G_boxEntity);
    G_boxEntity.enable = false;

    G_axisEntity = new GASEngine.AxisHelper(scene.root);
    G_scene.appendEntityOnRoot(G_axisEntity);
    G_axisEntity.enable = false;

    //TODO: 初始化HotspotManager，rainyyuan
    GASEngine.HotspotManager.Instance.init(scene);

    show3dGrid.setValue(false);
    showBoundBox.setValue(false);
    show3dAxis.setValue(false);
    wireframeOverlay.setValue(false);
    showUVLayout.setValue(false);

    var cameras = G_scene.findComponents('camera');
    if (cameras.length > 0)
    { //set camera (loaded from scene)to G_manipulator
        G_cameraComponent = cameras[0];
        G_manipulator.setCamera(cameras[0]);
    }
    else
    {
        G_scene.appendEntityOnRoot(G_cameraEntity);
    }

    if (G_scene.findComponents('environmentalLight').length === 0)
    {
        G_scene.appendEntityOnRoot(G_lightEntity);
    }

    console.log('onHierarchySuccess: Scene hierarchy load finished.');
};

function onSyncLoading(file, status, totalFiles)
{
    ++syncFileCount;
    if (status)
        console.log('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load successfully.');
    else
        console.error('[' + syncFileCount + '/' + totalFiles.size + ']' + 'Sync file: ' + file + ' load failed.');
};

function onAsyncLoading(file, status, totalFiles)
{
    ++asyncFileCount;

    if (status)
        console.log('[' + asyncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load successfully.');
    else
        console.error('[' + asyncFileCount + '/' + totalFiles.size + ']' + 'Async file: ' + file + ' load failed.');
};

function loadFile(file)
{
    var path = file.name;
    var ext = GASEngine.FileSystem.extractFileExt(path);

    var reader = new FileReader();
    reader.addEventListener('progress', function (event)
    {
        var size = '(' + Math.floor(event.total / 1000) + ' KB)';
        var progress = Math.floor((event.loaded / event.total) * 100) + '%';
        console.log('Loading', path, size, progress);
    });

    if(ext === 'gltf' || ext === 'glb' || ext === 'fbx') {
        GASEngine.Resources.Instance.meshCache.clear();
    }

    //load model
    switch (ext)
    {
        case 'gltf': {
            reader.addEventListener('load', function (event)
            {
                var contents = event.target.result;

                let gltfLoader = new GASEngine.GLTFLoader();
                gltfLoader.parse
                    (
                        contents,
                        onHierarchySuccess
                    );
            }, false);
            reader.readAsArrayBuffer(file)
            break;
        }
        case 'glb': {
            reader.addEventListener('load', function (event)
            {
                var contents = event.target.result;

                let gltfLoader = new GASEngine.GLTFLoader();
                gltfLoader.parse
                    (
                        contents,
                        onHierarchySuccess
                    );
            });
            reader.readAsArrayBuffer(file);
            break;
        }
        case 'fbx': {
            reader.addEventListener('load', function (event)
            {
                var contents = event.target.result;

                let fbxLoader = new GASEngine.FBXLoader();
                fbxLoader.parse
                    (
                        contents,
                        onHierarchySuccess
                    );
            });
            reader.readAsArrayBuffer(file);
            break;
        }
    };
};

function loadFileByName(filename)
{
    var ext = GASEngine.FileSystem.extractFileExt(filename);
    filename = filename.replace(/\\/g, "/");

    var lastIndex = filename.lastIndexOf('/');
    var modelDirectory = '../samples/';
    var structureFile;
    if (lastIndex >= 0)
    {
        modelDirectory += filename.substring(0, lastIndex + 1);
        structureFile = filename.substring(lastIndex + 1);
    }
    else
    {
        structureFile = filename;
    }

    GASEngine.Resources.Instance.meshCache.clear();

    //load model
    switch (ext)
    {
        case 'gas': { //Gas Loader
            GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);

            let gasLoader = new GASEngine.GAS1Loader();
            gasLoader.load
                (
                    structureFile,
                    onHierarchySuccess
                );
            break;
        }
        case 'gltf':
            {
                GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);

                let gltfLoader = new GASEngine.GLTFLoader();
                gltfLoader.load
                    (
                        structureFile,
                        onHierarchySuccess
                    );
                break;
            }
        case 'glb':
            {
                GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);

                let gltfLoader = new GASEngine.GLTFLoader();
                gltfLoader.load
                    (
                        structureFile,
                        onHierarchySuccess
                    );
                break;
            }
        case 'json': { //Neo Loader
            GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);
            let gas2Loader = new GASEngine.GAS2Loader();

            gas2Loader.load
                (
                    structureFile,
                    onHierarchySuccess,
                    onSyncLoading,
                    onAsyncLoading
                );
            break;
        }
        case 'fbx': {
            GASEngine.FileSystem.Instance.setProjectDirectory(modelDirectory);
            let fbxLoader = new GASEngine.FBXLoader();

            fbxLoader.load
            (
                structureFile,
                onHierarchySuccess
            );
            break;
        }
    }
};

document.addEventListener('dragover', function (event)
{
    event.preventDefault();
    event.dataTransfer.dropEffect = 'copy';
}, false);

document.addEventListener('drop', function (event)
{
    event.preventDefault();
    loadFiles(event.dataTransfer.files);
}, false);


G_fps = new UI.Text('60fps');
G_fps.setId('fps');
document.body.append(G_fps.dom);

G_polygon = new UI.Text('0 polygons');
G_polygon.setId('polygon');
document.body.append(G_polygon.dom);

let sceneNameInput = new UI.Input('gltf/airship-embed/airship.gltf');
sceneNameInput.setId('sceneNameInput');
document.body.append(sceneNameInput.dom);

let openButton = new UI.Button('加载');
openButton.setId('open_scene');
document.body.append(openButton.dom);
openButton.onClick(function ()
{
    let sceneName = sceneNameInput.getValue();
    if(sceneName !== '') {
        loadFileByName(sceneName);
    }
});

var focusButton = new UI.Button('聚焦');
focusButton.setId('focus_scene');
document.body.append(focusButton.dom);
focusButton.onClick(function ()
{
    if(G_manipulator) {
        G_manipulator._distance = G_manipulator.getHomeDistance(G_radius);
        G_manipulator.setTarget(G_center);
    }
});

if(!G_container) {
    G_container = new UI.Panel();
    G_container.setId('sidebar');
    document.body.append(G_container.dom);
}

//add camera option
var cameraRow = new UI.Row();
cameraRow.setMarginTop('10px').setMarginLeft('10px');

var cameraType = new UI.Select().setOptions({

    'Perspective': 'Perspective',
    'Top': 'Top',
    'Front': 'Front',
    'Left': 'Left'
}).setLeft('200px').setWidth('120px');

cameraType.onChange(function ()
{
    //onFogChanged();
    //refreshFogUI();
});
cameraType.setValue('Perspective');

cameraRow.add(new UI.Text('Camera').setWidth('150px'));
cameraRow.add(cameraType);
G_container.add(cameraRow);

//show 3d grid
var show3dGridRow = new UI.Row();
show3dGridRow.setMarginLeft('10px');
var show3dGrid = new UI.Checkbox(false).setLeft('200px');
show3dGrid.onChange(function ()
{
    if(G_gridEntity !== null) {
        G_gridEntity.enable = show3dGrid.getValue();
    }
});

show3dGridRow.add(new UI.Text('Show 3d grid').setWidth('200px'));
show3dGridRow.add(show3dGrid);
G_container.add(show3dGridRow);

//show 3d axis
var show3dAxisRow = new UI.Row();
show3dAxisRow.setMarginLeft('10px');
var show3dAxis = new UI.Checkbox(false).setLeft('200px');
show3dAxis.onChange(function ()
{
    if(G_axisEntity !== null) {
        G_axisEntity.enable = show3dAxis.getValue();
    }
});
show3dAxisRow.add(new UI.Text('Show 3d axis').setWidth('200px'));
show3dAxisRow.add(show3dAxis);
G_container.add(show3dAxisRow);

//show 3d axis
var showVRModeRow = new UI.Row();
showVRModeRow.setMarginLeft('10px');
var showVRMode = new UI.Checkbox(false).setLeft('200px');
showVRMode.onChange(function ()
{
    GASEngine.PBRPipeline.Instance.enableVR(showVRMode.getValue());
});
showVRModeRow.add(new UI.Text('Show VR Mode').setWidth('200px'));
showVRModeRow.add(showVRMode);
G_container.add(showVRModeRow);

//show bound box
var showBoundBoxRow = new UI.Row();
showBoundBoxRow.setMarginLeft('10px');
var showBoundBox = new UI.Checkbox(false).setLeft('200px');
showBoundBox.onChange(function ()
{
    if(G_boxEntity !== null) {
        G_boxEntity.enable = showBoundBox.getValue();
    }
});
showBoundBoxRow.add(new UI.Text('Show bound box').setWidth('200px'));
showBoundBoxRow.add(showBoundBox);
G_container.add(showBoundBoxRow);

//show glow
var showGlowRow = new UI.Row();
showGlowRow.setMarginLeft('10px');
var showGlow = new UI.Checkbox(false).setLeft('200px');
showGlow.onChange(function ()
{
    var value = showGlow.getValue();
    GASEngine.PBRPipeline.Instance.postprocesseffects.enable = !!value;
});

showGlowRow.add(new UI.Text('Show glow').setWidth('200px'));
showGlowRow.add(showGlow);
G_container.add(showGlowRow);

//glow threshold
var glowThresholdRow = (new UI.Row()).setMarginLeft('10px');
var glowThreshold = (new UI.Number()).setLeft('100px').setWidth('100px').setFontSize( '16px' );
glowThreshold.onChange(function ()
{
    var value = glowThreshold.getValue();
    GASEngine.PBRPipeline.Instance.postprocesseffects.glow.threshold = value;
});
glowThresholdRow.add(new UI.Text('Glow threshold').setWidth('150px'));
glowThresholdRow.add(glowThreshold);
G_container.add(glowThresholdRow);

//glow intensity
var glowIntensityRow = (new UI.Row()).setMarginLeft('10px');
var glowIntensity = (new UI.Number()).setLeft('100px').setWidth('100px').setFontSize( '16px' );
glowIntensity.onChange(function ()
{
    var value = glowIntensity.getValue();
    GASEngine.PBRPipeline.Instance.postprocesseffects.glow.intensity = value;
});
glowIntensityRow.add(new UI.Text('Glow intensity').setWidth('150px'));
glowIntensityRow.add(glowIntensity);
G_container.add(glowIntensityRow);

//glow radius
var glowRadiusRow = (new UI.Row()).setMarginLeft('10px');
var glowRadius = (new UI.Number()).setLeft('100px').setWidth('100px').setFontSize( '16px' );
glowThreshold.onChange(function ()
{
    var value = glowRadius.getValue();
    GASEngine.PBRPipeline.Instance.postprocesseffects.glow.radius = value;
});
glowRadiusRow.add(new UI.Text('Glow radius').setWidth('150px'));
glowRadiusRow.add(glowRadius);
G_container.add(glowRadiusRow);

//show hot spot
var showHotspotRow = (new UI.Row()).setMarginLeft('10px');
var showHotspot = new UI.Checkbox(true).setLeft('200px');
showHotspot.onChange(function ()
{
    GASEngine.PBRPipeline.Instance.showHotspot = showHotspot.getValue();
});
showHotspotRow.add(new UI.Text('Show hot spot').setWidth('200px'));
showHotspotRow.add(showHotspot);
G_container.add(showHotspotRow);

//Enable UV Layout
var showUVLayoutRow = new UI.Row();
showUVLayoutRow.setMarginLeft('10px');
var showUVLayout = new UI.Checkbox(false).setLeft('200px');
showUVLayout.onChange(function ()
{
    G_scene.enableUVLayout(showUVLayout.getValue());
});
showUVLayoutRow.add(new UI.Text('Show total uv layout').setWidth('200px'));
showUVLayoutRow.add(showUVLayout);
G_container.add(showUVLayoutRow);

//show scene statistics
var showSceneStatRow = new UI.Row();
showSceneStatRow.setMarginLeft('10px');
var showSceneStat = new UI.Checkbox(true).setLeft('200px');
showSceneStat.onChange(function ()
{
    let isShowSceneStat = showSceneStat.getValue();
    G_fps.setDisplay(isShowSceneStat?'': 'none');
    G_polygon.setDisplay(isShowSceneStat?'': 'none');
});
showSceneStatRow.add(new UI.Text('Show scene statistics').setWidth('200px'));
showSceneStatRow.add(showSceneStat);
G_container.add(showSceneStatRow);

//Shading

//Select shading mode
var shadingModeRow = new UI.Row();
shadingModeRow.setMarginTop('10px').setMarginLeft('10px');

var shadingMode = new UI.Select().setOptions({
}).setLeft('200px').setWidth('120px');

shadingMode.onChange(function ()
{
    G_scene.setShadingMode(shadingMode.getValue());
});

shadingModeRow.add(new UI.Text('Shading mode').setWidth('150px'));
shadingModeRow.add(shadingMode);
G_container.add(shadingModeRow);

//Enable wireframe overlay
var wireframeOverlayRow = new UI.Row();
wireframeOverlayRow.setMarginLeft('10px');
var wireframeOverlay = new UI.Checkbox(false).setLeft('200px');
wireframeOverlay.onChange(function ()
{
    let isWireframeOverlay = wireframeOverlay.getValue();
    G_scene.enableWireframeOverlay(isWireframeOverlay);
});

wireframeOverlayRow.add(new UI.Text('Enable wireframe overlay').setWidth('200px'));
wireframeOverlayRow.add(wireframeOverlay);
G_container.add(wireframeOverlayRow);

//Enable scene lights
var sceneLightsRow = new UI.Row();
sceneLightsRow.setMarginLeft('10px');
var sceneLight = new UI.Checkbox(true).setLeft('200px');
sceneLight.onChange(function ()
{
});
sceneLightsRow.add(new UI.Text('Enable scene lights(Unrealized)').setWidth('200px'));
sceneLightsRow.add(sceneLight);
G_container.add(sceneLightsRow);

//Enable real-time shadows
var realtimeShadowsRow = new UI.Row();
realtimeShadowsRow.setMarginLeft('10px');
var realtimeShadows = new UI.Checkbox(true).setLeft('200px');
realtimeShadows.onChange(function ()
{
});
realtimeShadowsRow.add(new UI.Text('Enable real-time shadows(Unrealized)').setWidth('200px'));
realtimeShadowsRow.add(realtimeShadows);
G_container.add(realtimeShadowsRow);

//set theme
if(window.localStorage['theme'] == undefined) {
    window.localStorage['theme'] = 'css/light.css';
}

function setTheme(value) {
    document.getElementById('theme').href = value;
};

setTheme(window.localStorage['theme']);

//theme row
var themeRow = new UI.Row();
themeRow.setMarginLeft('10px');
var theme = new UI.Select().setLeft('200px').setWidth('120px');
theme.setOptions(
    {
        'css/light.css': 'light',
        'css/dark.css': 'dark'
    }
);
theme.setValue(window.localStorage['theme']);

theme.onChange(function ()
{
    var value = theme.getValue();
    setTheme(value);
    window.localStorage['theme'] = value;
});

themeRow.add(new UI.Text('Theme').setWidth('150px'));
themeRow.add(theme);
G_container.add(themeRow);


//add button [Export Scene]
var exportButton = new UI.Button('导出');
exportButton.setId('export_scene');
exportButton.onClick(function ()
{
    exportGLTF();
});
G_container.add(exportButton);

//add a link
var link = document.createElement('a');
link.style.display = 'none';
document.body.appendChild(link);

//export scene to gltf
function exportGLTF()
{
    if (!G_scene)
        return;

    var path = prompt("请输入保存的文件名", "scene");
    if (!path)
        return;

    var gltfExporter = new GASEngine.GLTFExporter();
    gltfExporter.parse(G_scene, function (result)
    {
        var blob, filename;
        if (result instanceof ArrayBuffer)
        {
            blob = new Blob([result], { type: 'application/octet-stream' });
            filename = path + '.glb';
        }
        else
        {
            var output = JSON.stringify(result, null, 2);
            blob = new Blob([output], { type: 'text/plain' });
            filename = path + '.gltf';
        }

        link.href = URL.createObjectURL(blob);
        link.download = filename;
        link.click();
    });
};

init();
render();
