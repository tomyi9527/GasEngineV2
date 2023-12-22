
GASEngine.BoxHelper = function (entity, color)
{
    GASEngine.Entity.call(this);

    var mesh = GASEngine.MeshFactory.Instance.create();

    var size = 10;
    var max = new GASEngine.Vector3(), min = new GASEngine.Vector3();
    if (!entity || entity.bbox === null)
    {
        max.set(size / 2, size / 2, size / 2);
        min.set(-size / 2, -size / 2, -size / 2);
    }
    else
    {
        max = entity.bbox.max.clone();
        min = entity.bbox.min.clone();
    }
    mesh = new GASEngine.Box3Mesh(min, max);
    mesh.submitToWebGL();

    this.type = 'helper';

    var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilter.uniqueID = GASEngine.generateUUID();
    meshFilter.bbox = new GASEngine.AABB();
    meshFilter.bbox.max.set(max.x, max.y, max.z);
    meshFilter.bbox.min.set(min.x, min.y, min.z);
    this.addComponent(meshFilter);
    meshFilter.setMesh(mesh);

    var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    if (color === undefined)
    {
        color = [0.0, 0.0, 1.0, 1];
    }
    material.setPureColorBase(color);
    material.culling = GASEngine.Material.CullingOff;
    meshRenderer.addMaterial(material);
    this.addComponent(meshRenderer);
};

GASEngine.BoxHelper.prototype = Object.create(GASEngine.Entity.prototype);
GASEngine.BoxHelper.constructor = GASEngine.BoxHelper;

//GridHelper
GASEngine.GridHelper = function (width, height, widthSegments, heightSegments, halfFlag, color)
{
    GASEngine.Entity.call(this);

    width = width || 10;
    height = height || 10;

    let mesh = new GASEngine.GridMesh(width, height, widthSegments, heightSegments, halfFlag);
    mesh.submitToWebGL();
    this.type = 'helper';

    var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter');
    meshFilter.uniqueID = GASEngine.generateUUID();
    meshFilter.bbox = new GASEngine.AABB();
    meshFilter.bbox.max.set(width / 2, 0, height / 2);
    meshFilter.bbox.min.set(-width / 2, 0, -height / 2);
    this.addComponent(meshFilter);
    meshFilter.setMesh(mesh);

    var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer');
    var material = GASEngine.MaterialFactory.Instance.create('pureColor');
    material.setPureColorBase(color);
    material.culling = GASEngine.Material.CullingOff;
    meshRenderer.addMaterial(material);
    this.addComponent(meshRenderer);

    this.setLocalTranslation(new GASEngine.Vector3(0, -0.01, 0));
};

GASEngine.GridHelper.prototype = Object.create(GASEngine.Entity.prototype);
GASEngine.GridHelper.constructor = GASEngine.GridHelper;

//三维坐标系
GASEngine.AxisHelper = function (entity, showPlane)
{
    GASEngine.Entity.call(this);

    var rColor = [1.0, 0.0, 0.0, 1.0];
    var gColor = [0.0, 1.0, 0.0, 1.0];
    var bColor = [0.0, 0.0, 1.0, 1.0];

    var xyColor = [1.0, 1.0, 0.0, 0.25];
    var yzColor = [0.0, 1.0, 1.0, 0.25];
    var xzColor = [1.0, 0.0, 1.0, 0.25];

    var size = 10;
    if (entity && entity.bbox !== null)
    {
        size = entity.bbox.getRadius() / 2;
        size = size || 10;
    }

    var gizmoMap = {
        X: [
            {
                type: 'arrow',
                color: rColor,
                translation: [size, 0, 0],
                rotation: [0, 0, -Math.PI / 2]
            },
            {
                type: 'line',
                color: rColor
            }
        ],
        Y: [
            {
                type: 'arrow',
                color: gColor,
                translation: [0, size, 0],
            },
            {
                type: 'line',
                color: gColor,
                rotation: [0, 0, Math.PI / 2]
            }
        ],
        Z: [
            {
                type: 'arrow',
                color: bColor,
                translation: [0, 0, size],
                rotation: [Math.PI / 2, 0, 0]
            },
            {
                type: 'line',
                color: bColor,
                rotation: [0, -Math.PI / 2, 0]
            }
        ],
        XY: [
            {
                type: 'plane',
                color: xyColor
            }
        ],
        YZ: [
            {
                type: 'plane',
                color: yzColor,
                translation: [0, 0, size / 3],
                rotation: [0, Math.PI / 2, 0]
            }
        ],
        XZ: [
            {
                type: 'plane',
                color: xzColor,
                translation: [0, 0, size / 3],
                rotation: [-Math.PI / 2, 0, 0]
            }
        ]
    };

    this.type = 'helper';

    var localTranslation = new GASEngine.Vector3();
    var localRotation = new GASEngine.Euler();
    var localScale = new GASEngine.Vector3();

    for (var name in gizmoMap)
    {
        if(name.length === 2 && !showPlane) //TODO:跳过显示面片，只现在坐标轴
            continue;

        for (var i = 0; i < gizmoMap[name].length; i++) 
        {
            var gizmoConfig = gizmoMap[name][i];

            var meshType = gizmoConfig.type;
            var color = gizmoConfig.color;
            var translation = gizmoConfig.translation;
            var rotation = gizmoConfig.rotation;
            var scale = gizmoConfig.scale;

            var childEntity = GASEngine.EntityFactory.Instance.create();
            childEntity.type = 'helper';
            childEntity.name = name + '_' + meshType;

            //mesh
            var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
            meshFilterComponent.bbox = new GASEngine.AABB();
            var mesh;

            //material
            var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
            var material = GASEngine.MaterialFactory.Instance.create('pureColor');
            material.setPureColorBase(color);
            if (meshType === 'arrow')
            {
                mesh = new GASEngine.ArrowMesh(0, size / 25, size / 6);
            }
            else if (meshType === 'line')
            {
                mesh = new GASEngine.LineMesh(size);
            }
            else if (meshType === 'plane')
            {
                mesh = new GASEngine.PlaneMesh(size / 3, size / 3);
                meshFilterComponent.bbox.max.set(size / 3, size / 3, size / 3);
                meshFilterComponent.bbox.min.set(0, 0, 0);
            }
            else if (meshType === 'cone')
            {
                mesh = new GASEngine.ArrowMesh(size / 7.5, 0, size);
                meshFilterComponent.bbox.max.set(size / 7.5, size, size / 7.5);
                meshFilterComponent.bbox.min.set(-size / 7.5, 0, -size / 7.5);
            }

            if (mesh)
            {
                mesh.submitToWebGL();
                meshFilterComponent.setMesh(mesh);
                childEntity.addComponent(meshFilterComponent);
            }

            meshRendererComponent.addMaterial(material);
            childEntity.addComponent(meshRendererComponent);

            if (translation)
            {
                localTranslation.set(translation[0], translation[1], translation[2]);
                childEntity.setLocalTranslation(localTranslation);
            }
            if (rotation)
            {
                localRotation.set(rotation[0], rotation[1], rotation[2], 'XYZ');
                childEntity.setLocalRotation(localRotation);
            }
            if (scale)
            {
                localScale.set(scale[0], scale[1], scale[2]);
                childEntity.setLocalScale(localScale);
            }
            childEntity.updateWorldMatrix_r();
            this.addChild(childEntity);
        }
    }
};

GASEngine.AxisHelper.prototype = Object.create(GASEngine.Entity.prototype);
GASEngine.AxisHelper.constructor = GASEngine.AxisHelper;



