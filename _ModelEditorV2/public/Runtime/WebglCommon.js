//Author: saralu
//Date: 2019-5-17
//Notes: used for axes helper and can be selected

//box3helper
GASEngine.WebglCommon = function()
{
    this.mode = 'select';
    this.gl = null;
    this.shaderMap = {};
    this.camera = null;
    this.modelMatrix =new GASEngine.Matrix4();
    this.root = null;
    this.canvas = null;

    this.transformControlsMap = {};
    this.boxCount = 0;
    this.sphereCount = 0;
    this.emptyCount = 0;

    this.initTransformControls();
    GASEngine.WebglCommon.Instance = this;
}

GASEngine.WebglCommon.prototype.constructor = GASEngine.WebglCommon;


GASEngine.WebglCommon.prototype.createEmptyEntity = function()
{
    console.log('webglCommon:addEmptyEntity');
    var entity = GASEngine.EntityFactory.Instance.create();
    this.emptyCount++;
    entity.name = 'Empty' + this.emptyCount;
    entity.uniqueID = GASEngine.generateUUID();
    return entity;
}

GASEngine.WebglCommon.prototype.createCube = function()
{
    console.log('webglCommon:addBox');
    this.boxCount ++;
    // var color = 0.0 | 0.0 << 8 | 0 << 16 | 255 << 24;
    var color = [0.0, 0.0, 0.0, 1.0];
    var size = 1;
    var mesh = new GASEngine.BoxMesh(size);
    mesh.submitToWebGL();
    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    material.setPureColorBase(color);
    var entity = GASEngine.EntityFactory.Instance.create();
    entity.name = 'Box' + this.boxCount;
    entity.type = 'box';
    entity.uniqueID = GASEngine.generateUUID();

    var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilterComponent.setMesh(mesh);
    meshFilterComponent.bbox = new GASEngine.AABB();
    meshFilterComponent.bbox.max.set(size / 2, size / 2, size / 2);
    meshFilterComponent.bbox.min.set(-size / 2, -size / 2, -size / 2);

    var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    meshRendererComponent.addMaterial(material);
    entity.addComponent(meshFilterComponent);
    entity.addComponent(meshRendererComponent);
    return entity;
}

GASEngine.WebglCommon.prototype.createSphere = function()
{
    console.log('webglCommon:addSphere');
    this.sphereCount ++;
    // var color = 0.0 | 0.0 << 8 | 0 << 16 | 255 << 24;
    var color = [0.0, 0.0, 0.0, 1.0];
    var size = 1;
    
    var entity = GASEngine.EntityFactory.Instance.create();
    entity.name = 'Sphere' + this.sphereCount;
    entity.type = 'sphere';
    entity.uniqueID = GASEngine.generateUUID();

    var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
    var mesh = new GASEngine.SphereMesh(size);
    mesh.submitToWebGL();
    meshFilterComponent.setMesh(mesh);
    meshFilterComponent.bbox = new GASEngine.AABB();
    meshFilterComponent.bbox.max.set(size / 2, size / 2, size / 2);
    meshFilterComponent.bbox.min.set(-size / 2, -size / 2, -size / 2);

    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    material.setPureColorBase(color);
    var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    meshRendererComponent.addMaterial(material);
    entity.addComponent(meshFilterComponent);
    entity.addComponent(meshRendererComponent);
    return entity;
}

//cameraHelper
GASEngine.WebglCommon.prototype.createCameraHelper = function()
{
    // var color = 255 | 255.0 << 8 | 0 << 16 | 255 << 24;
    var color = [1.0, 1.0, 1.0, 1.0];
    var mesh = this._createBoxHelperMesh(100);
    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    material.setPureColorBase(color);
    
    var entity = GASEngine.EntityFactory.Instance.create();
    entity.name = 'cameraHelper';
    entity.type = 'helper';

    var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilterComponent.setMesh(mesh);

    var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    meshRendererComponent.addMaterial(material);
    entity.addComponent(meshFilterComponent);
    entity.addComponent(meshRendererComponent);

    var position = new GASEngine.Vector3();
    var cameras = SceneEditor.Editor.Instance.sceneInstance.findComponents('camera');
    var matrix = cameras[0].getWorldMatrix();
    position.setFromMatrixPosition(matrix);
    entity.translation = position;
    return entity;
}

//transformControls
GASEngine.WebglCommon.prototype.initTransformControls = function()
{   
    var modes = ['select', 'translate', 'rotate', 'scale'];
    var types = ['gizmo', 'picker', 'helper'];
    var size = 1; 
    var type, mode, map, entity;

    for (var t = 0, tl = types.length; t < tl; t++)
    {
        map = {};
        type = types[t]; 
        for(var i = 0, l = modes.length; i < l; i++)
        {
            mode = modes[i];
            entity = GASEngine.EntityFactory.Instance.create();
            // entity.name = 'transformControlHelper';
            entity.name = type + ':' + mode;
            entity.type = 'helper';
        
            entity.bbox.max.set(size, size, size);
            entity.bbox.min.set(0, 0, 0);
            
            this.setupGizmo(entity, size, mode, type);
            map[mode] = entity;
        }
        this.transformControlsMap[type] = map;
    }
},

GASEngine.WebglCommon.prototype.createGlobalPlane = function()
{
    // var color = 177.0 | 177 << 8 | 177 << 16 | 25 << 24;
    var color = [0.7, 0.7, 0.7, 0.1];
    var size = 100000;
    
    var entity = GASEngine.EntityFactory.Instance.create();
    entity.type = 'helper';
    entity.name = 'globalPlane';      
    //material
    var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    material.setPureColorBase(color);

    meshRendererComponent.addMaterial(material);
    entity.addComponent(meshRendererComponent);
    //mesh
    var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilterComponent.bbox = new GASEngine.AABB();

  
    mesh = this._createPlaneHelperMesh(size, size, 2, 2, true);
    meshFilterComponent.bbox.max.set(size / 2, size / 2, size / 2);
    meshFilterComponent.bbox.min.set(-size / 2, -size / 2, -size / 2);

    meshFilterComponent.setMesh(mesh);
    entity.addComponent(meshFilterComponent);


    var rotation = [ -Math.PI / 2, 0, 0 ];
    // if (translation) {
    //     entity.translation.set(translation[ 0 ], translation[ 1 ], translation[ 2 ]);
    // }
    // if (rotation) {
        entity.rotation.set(rotation[ 0 ], rotation[ 1 ], rotation[ 2 ], 'XYZ');
    // }
    // if (scale) {
    //     entity.scale.set(scale[ 0 ], scale[ 1 ], scale[ 2 ]);
    // }
    // SceneEditor.Editor.Instance.sceneInstance.appendEntityOnRoot(entity);
    entity.update();
    return entity;
}

GASEngine.WebglCommon.prototype.getTransformControls = function()
{
    var mode = SceneEditor.Editor.Instance.eventInput.getCurrentMode();
    var gizmos = this.transformControlsMap['gizmo'];
    var entity = gizmos[mode];
    return entity;
},

GASEngine.WebglCommon.prototype.getTransformPickers = function()
{
    var mode = SceneEditor.Editor.Instance.eventInput.getCurrentMode();
    var pickers = this.transformControlsMap['picker'];
    var picker = pickers[mode];
    return picker;
}

GASEngine.WebglCommon.prototype.getTransformHelpers = function()
{
    var mode = SceneEditor.Editor.Instance.eventInput.getCurrentMode();
    var helpers = this.transformControlsMap['picker'];
    var helper = helpers[mode];
    return helper;
}

GASEngine.WebglCommon.prototype.setupGizmo = function(entity, size, mode, type)
{
    var rColor = [1.0, 0.0, 0.0, 1.0];
    var gColor = [0.0, 1.0, 0.0, 1.0];
    var bColor = [0.0, 0.0, 1.0, 1.0];

    var whiteColor = [1.0, 1.0, 1.0, 0.25];
    var xyColor = [1.0, 1.0, 0.0, 0.25];
    var yzColor = [0.0, 1.0, 1.0, 0.25];
    var xzColor = [1.0, 0.0, 1.0, 0.25];

    var otherColor = [0.7, 0.7, 0.7, 0.0];
    var sphereColor = [0.5, 0.5, 0.5, 0.0];
    
    var gizmoSelect = {
        X: [
            [ 'line', rColor,  [ 1, 0, 0 ] ]
            // [ 'line', rColor,  [ 1, 0, 0 ], [ 0, 0, -Math.PI / 2 ]]
        ],
        Y: [
            [ 'line', gColor, [ 0, 1, 0 ], [ 0, 0, Math.PI / 2 ] ]
            // [ 'line', gColor, [ 0, 1, 0 ], null ]
        ],
        Z: [
            [ 'line', bColor, [ 0, 0, 1 ], [ 0, -Math.PI / 2, 0 ] ]
            // [ 'line', bColor, [ 0, 0, 1 ], [ Math.PI / 2, 0, 0 ] ]
        ]
    };

    var pickerSelect = {
        X: [
            [ 'cone', rColor,  [ size * 0.6, 0, 0 ], [ 0, 0, -Math.PI / 2 ], null],
        ],
        Y: [
            [ 'cone', gColor, [ 0, size * 0.6, 0 ],  null, null],
        ],
        Z: [
            [ 'cone', bColor, [ 0, 0, size * 0.6 ], [ Math.PI / 2, 0, 0 ], null]
        ]
    };

    var gizmoTranslate = {
        X: [
            [ 'arrow', rColor, [ size, 0, 0 ], [ 0, 0, -Math.PI / 2 ], null, 'fwd' ],
            // [ meshArrow, [ 1, 0, 0 ], [ 0, 0, Math.PI / 2 ], null, 'bwd' ],
            [ 'line', rColor ]
        ],
        Y: [
            [ 'arrow', gColor, [ 0, size, 0 ], null, null, 'fwd' ],
            // [ meshArrow, [ 0, 1, 0 ], [ Math.PI, 0, 0 ], null, 'bwd' ],
            [ 'line', gColor, null, [ 0, 0, Math.PI / 2 ] ]
        ],
        Z: [
            [ 'arrow', bColor, [ 0, 0, size ], [ Math.PI / 2, 0, 0 ], null, 'fwd' ],
            // [ meshArrow, [ 0, 0, 1 ], [ -Math.PI / 2, 0, 0 ], null, 'bwd' ],
            [ 'line', bColor, null, [ 0, -Math.PI / 2, 0 ] ]
        ],
        XY: [
            [ 'plane', xyColor ]
        ],
        YZ: [
            [ 'plane', yzColor, [ 0, 0, size/3 ], [ 0, Math.PI / 2, 0 ] ]
        ],
        XZ: [
            [ 'plane', xzColor, [ 0, 0, size/3 ], [ -Math.PI / 2, 0, 0 ] ]
        ]
    };

    var pickerTranslate = {
        X: [
            [ 'cone', rColor, [ size * 0.6, 0, 0 ], [ 0, 0, -Math.PI / 2 ], null],
        ],
        Y: [
            [ 'cone', gColor, [ 0, size * 0.6, 0 ], null, null],
        ],
        Z: [
            [ 'cone', bColor, [ 0, 0, size * 0.6 ], [ Math.PI / 2, 0, 0 ], null],
        ],
        XY: [
            [ 'plane', xyColor ]
        ],
        YZ: [
            [ 'plane', yzColor, [ 0, 0, size/3 ], [ 0, Math.PI / 2, 0 ] ]
        ],
        XZ: [
            [ 'plane', xzColor, [ 0, 0, size/3 ], [ -Math.PI / 2, 0, 0 ] ]
        ]
    };

    //v2
    var gizmoRotate = {
        X: [
            [ 'circle', rColor, null, null, null, false ],
            // [ 'circle', otherColor, null, null, null, false ]
        ],
        Y: [
            [ 'circle', gColor, null, [ 0, 0, Math.PI / 2], null, false ],
            // [ 'circle', otherColor, null, [ 0, 0, Math.PI / 2], null, false ]
        ],
        Z: [
            [ 'circle', bColor, null, [ 0, Math.PI / 2, 0 ], null, false ],
            // [ 'circle', otherColor, null, [ 0, Math.PI / 2, 0 ], null, false ]
        ],
        E1: [
            [ 'circle', whiteColor, null, [ 0, Math.PI / 2, 0 ], null, false ]
        ],
        E: [
            [ 'circle', whiteColor, null, [ 0, Math.PI / 2, 0 ], [1.25, 1.25, 1.25], false ]
        ],
        XYZE: [
            [ 'sphere', sphereColor, null, null, null, false ]
        ]
    };

    var pickerRotate = {
        X: [
            [ 'torus', rColor,  null, [ 0, Math.PI / 2, 0 ], null], //(x轴)
        ],
        Y: [
            [ 'torus', gColor, null, [ Math.PI / 2, 0, 0 ], null], //（y轴）
        ],
        Z: [
            [ 'torus', bColor,  null, null, null], //(z轴)
        ],
        E: [
            [ 'torus', xyColor,  null, null, [1.25, 1.25, 1.25]], //(正对相机)
        ],
        XYZE: [
            [ 'sphere', sphereColor, null, null, null, false ]
        ]
    };

    var gizmoScale = {
        X: [
            [ 'box', rColor, [ size, 0, 0 ], null, null, 'fwd' ],
            // [ meshArrow, [ 1, 0, 0 ], [ 0, 0, Math.PI / 2 ], null, 'bwd' ],
            [ 'line', rColor ]
        ],
        Y: [
            [ 'box', gColor, [ 0, size, 0 ], null, null, 'fwd' ],
            // [ meshArrow, [ 0, 1, 0 ], [ Math.PI, 0, 0 ], null, 'bwd' ],
            [ 'line', gColor, null, [ 0, 0, Math.PI / 2 ] ]
        ],
        Z: [
            [ 'box', bColor, [ 0, 0, size ], null, null, 'fwd' ],
            // [ meshArrow, [ 0, 0, 1 ], [ -Math.PI / 2, 0, 0 ], null, 'bwd' ],
            [ 'line', bColor, null, [ 0, -Math.PI / 2, 0 ] ]
        ],
        XY: [
            [ 'plane', xyColor ]
        ],
        YZ: [
            [ 'plane', yzColor, [ 0, 0, size/3 ], [ 0, Math.PI / 2, 0 ] ]
        ],
        XZ: [
            [ 'plane', xzColor, [ 0, 0, size/3 ], [ -Math.PI / 2, 0, 0 ] ]
        ]
    };


    var pickerScale = {
        X: [
            [ 'cone', rColor,  [ size * 0.6, 0, 0 ], [ 0, 0, -Math.PI / 2 ], null],
        ],
        Y: [
            [ 'cone', gColor, [ 0, size * 0.6, 0 ], null, null],
        ],
        Z: [
            [ 'cone', bColor, [ 0, 0, size * 0.6 ], [ Math.PI / 2, 0, 0 ], null],
        ],
        XY: [
            [ 'plane', xyColor ]
        ],
        YZ: [
            [ 'plane', yzColor, [ 0, 0, size/3 ], [ 0, Math.PI / 2, 0 ] ]
        ],
        XZ: [
            [ 'plane', xzColor, [ 0, 0, size/3 ], [ -Math.PI / 2, 0, 0 ] ]
        ]
    };

    // var mode = SceneEditor.Editor.Instance.eventInput.getCurrentMode();
    var gizmoMap = mode === 'select' ? gizmoSelect : mode === 'translate' ? gizmoTranslate : mode === 'rotate' ? gizmoRotate : gizmoScale;
    var pickerMap = mode === 'select' ? pickerSelect : mode === 'translate' ? pickerTranslate : mode === 'rotate' ? pickerRotate : pickerScale;

    var map = type === 'gizmo' ? gizmoMap : pickerMap;
    this._setupGizmoInner(map, entity, size);
}


GASEngine.WebglCommon.prototype._setupGizmoInner = function(gizmoMap, entity, size)
{ 
    for ( var name in gizmoMap ) {
        var gizmo = GASEngine.EntityFactory.Instance.create();
        gizmo.name = name;
        gizmo.type = 'helper';

        var gizmoLength = gizmoMap[name].length;

        for ( var i = 0; i < gizmoLength; i++ ) 
        {
            var meshType = gizmoMap[name][i][0];
            var color = gizmoMap[name][i][1];
            var translation = gizmoMap[name][i][2];
            var rotation = gizmoMap[name][i][3];
            var scale = gizmoMap[name][i][4];
            var halfFlag = gizmoMap[name][i][5];
            // var tag = gizmoMap[ name ][ i ][ 4 ];

            var childEntity = GASEngine.EntityFactory.Instance.create();
            childEntity.type = 'helper';
            childEntity.name = gizmo.name;      
           
            //mesh
            var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
            meshFilterComponent.bbox = new GASEngine.AABB();
            var mesh;

            //material
            var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
            var material = GASEngine.MaterialFactory.Instance.create('pureColor');
            material.setPureColorBase(color);
            if(meshType === 'arrow')
            {
                mesh = new GASEngine.ArrowMesh(0, size/25, size/6);
            } 
            else if(meshType === 'line')
            {
                mesh = new GASEngine.LineMesh(size);
            } 
            else if(meshType === 'plane')
            {
                mesh = new GASEngine.PlaneMesh(size / 3, size / 3);
                meshFilterComponent.bbox.max.set(size / 3, size / 3, size / 3);
                meshFilterComponent.bbox.min.set(0, 0, 0);
            }
            else if(meshType === 'circle')
            {
                mesh = new GASEngine.CircleMesh(size, 100, halfFlag);
                material.depthTest = true;

                // mesh = this._createCircleHelperMesh_v2(size, 200, halfFlag);
                // var tube = 20;
                // // mesh = this._createCircleHelperMesh(size, 100, halfFlag);
                // mesh = this._createTorusHelperMesh(size, tube, 200, 10);
                // meshFilterComponent.bbox.max.set(size + tube, size + tube, tube);
                // meshFilterComponent.bbox.min.set(-(size + tube), -(size + tube), -tube);
            }
            else if(meshType === 'box')
            {
                mesh = new GASEngine.BoxMesh(size/10);
            }
            else if(meshType === 'cone')
            {
                mesh = new GASEngine.ArrowMesh(size/7.5, 0, size);
                meshFilterComponent.bbox.max.set(size/7.5, size, size/7.5);
                meshFilterComponent.bbox.min.set(-size/7.5, 0, -size/7.5);
            }
            else if(meshType === 'torus')
            {   
                mesh = new GASEngine.TorusMesh(size, size/15, 4, 24);
                meshFilterComponent.bbox.max.set(size + size/15, size + size/15, size/15);
                meshFilterComponent.bbox.min.set(-(size + size/15), -(size + size/15), -size/15);
            }
            else if(meshType === 'sphere')
            {
                var tmpSize = 0.99 * size;
                mesh = new GASEngine.SphereMesh(tmpSize);
                meshFilterComponent.bbox.max.set(tmpSize, tmpSize, tmpSize);
                meshFilterComponent.bbox.min.set(-tmpSize, -tmpSize, -tmpSize);
                material.depthTest = true;
            }

            if(mesh) {
                mesh.submitToWebGL();
                meshFilterComponent.setMesh(mesh);
            }
            childEntity.addComponent(meshFilterComponent);

            meshRendererComponent.addMaterial(material);
            childEntity.addComponent(meshRendererComponent);

            if (translation) {
                childEntity.translation.set(translation[ 0 ], translation[ 1 ], translation[ 2 ]);
            }
            if (rotation) {
                childEntity.rotation.set(rotation[ 0 ], rotation[ 1 ], rotation[ 2 ], 'XYZ');
            }
            if (scale) {
                childEntity.scale.set(scale[ 0 ], scale[ 1 ], scale[ 2 ]);
            }

            childEntity.update();   

            gizmo.addChild(childEntity);
        }
        entity.addChild(gizmo);
    }
};