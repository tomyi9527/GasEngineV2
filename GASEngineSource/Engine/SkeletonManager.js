//Skeleton Manager
GASEngine.SkeletonManager = function ()
{
    this.boneList = [];
    this.skeletonEntity = [];
    this.updatePairs = [];
    this.scale = 1.0;
    this.min_size = Number.MAX_SAFE_INTEGER;
    this.max_size = 0;
    this.skeletonHelper = null;

    this.unitSphereMesh = new GASEngine.SphereMesh(1.0);
    this.unitSphereMesh.submitToWebGL();

    this.unitOctahedronMesh = new GASEngine.OctahedronMesh(new GASEngine.Vector3(0,0,1), 0.5);
    this.unitOctahedronMesh.submitToWebGL();

    GASEngine.SkeletonManager.Instance = this;
};

GASEngine.SkeletonManager.prototype =
{
    constructor: GASEngine.SkeletonManager,

    init: function()
    {
        if (this.boneList.length > 0)
        {
            this.createSkeleton();

            this.skeletonHelper = GASEngine.EntityFactory.Instance.create();
            this.skeletonHelper.type = 'helper';
            this.skeletonHelper.name = 'skeletonHelper';
            this.skeletonHelper.uniqueID = GASEngine.generateUUID();

            this.skeletonHelper.children = this.skeletonEntity;
            this.skeletonHelper.enable = true;
            this.skeletonEntity.forEach(v=>v.parent = this.skeletonHelper);
            //console.log(skeletonHelper);
            //var bone = this.createBone(this.bonelist[0]);
            return this.skeletonHelper;
        }
        else
        {
            console.error("This model has no skeletons!");  
            return null;  
        }
    },

    finl: function()
    {
        if (this.skeletonHelper !== null) {
            if (this.skeletonHelper.parent !== null) {
                this.skeletonHelper.parent.removeChild(this.skeletonHelper);
            }
            GASEngine.EntityFactory.Instance.destroy(this.skeletonHelper);
            this.skeletonHelper = null;
        }
        // skeletonEntity均为helper的child，上面的destroy会清理掉的。
        this.boneList.length = 0;
        this.skeletonEntity.length = 0;
        this.updatePairs.length = 0;
        this.scale = 1.0;
    },

    update: function()
    {
        if (this.min_size > this.max_size) {
            this.updateMinMax();
        }
        if (this.min_size <= this.max_size) {
            this.updatePairs.forEach(v =>{
                if (v[0] == 1) {
                    this.applyTransformToBone(v[1], v[2]);
                } else if (v[0] == 2) {
                    this.applyTransformToJoint(v[1], v[2]);
                }
            });
        }
    },

    appendBones: function (bones)
    {
        for (var i = 0; i < bones.length; ++i)
        {
            if (!this.boneList.includes(bones[i]))
            {
                this.boneList.push(bones[i]);
            }
        }
    },

    createJoint: function (entity)
    {
        var jointEntity = GASEngine.EntityFactory.Instance.create();
        jointEntity.name = entity.name;
        jointEntity.type = 'helper';
        jointEntity.uniqueID = GASEngine.generateUUID();
        jointEntity.boneID = entity.uniqueID;

        var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
        meshFilterComponent.uniqueID = GASEngine.generateUUID();
        meshFilterComponent.setMesh(this.unitSphereMesh);

        var meshRenderComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
        var material = GASEngine.MaterialFactory.Instance.create('lambert');
        //material.culling = GASEngine.Material.CullingOff;
        meshRenderComponent.addMaterial(material);

        jointEntity.addComponent(meshFilterComponent);
        jointEntity.addComponent(meshRenderComponent);

        return jointEntity;
    },

    createBone: function (child, parent)
    {
        //var skeleton = this.bonelist[i].parent.clone();
        var boneEntity = GASEngine.EntityFactory.Instance.create();
        boneEntity.name = child.name;
        boneEntity.type = 'helper';
        boneEntity.setLocalTranslation(new GASEngine.Vector3(0,0,0));
        //skeleton.setLocalQuaternion(this.bonelist[i].getLocalQuaternion());

        //skeleton.parent = this.root;
        boneEntity.uniqueID = GASEngine.generateUUID();
        //skeleton.boneID = this.bonelist[i].uniqueID;

        var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
        meshFilterComponent.uniqueID = GASEngine.generateUUID();
        var dir = child.getLocalTranslation().clone();//.sub(this.bonelist[i].parent.getWorldTranslation().clone());
        meshFilterComponent.setMesh(this.unitOctahedronMesh);

        var meshRenderComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
        var material = GASEngine.MaterialFactory.Instance.create('lambert');
        //material.culling = GASEngine.Material.CullingOff;
        meshRenderComponent.addMaterial(material);

        boneEntity.addComponent(meshFilterComponent);
        boneEntity.addComponent(meshRenderComponent);

        return boneEntity;
    },

    createSkeleton: function ()
    {
        for (var i = 0; i < this.boneList.length; ++i)
        {
            var src_entity = this.boneList[i];
            if (src_entity.parent && this.boneList.includes(src_entity.parent))
            {
                var boneEntity = this.createBone(src_entity, src_entity.parent);
                this.skeletonEntity.push(boneEntity);
                this.updatePairs.push([1, src_entity, boneEntity]);
                var jointEntity = this.createJoint(src_entity);
                this.skeletonEntity.push(jointEntity);
                this.updatePairs.push([2, src_entity, jointEntity]);
            }
        }
    },

    updateMinMax: function ()
    {
        this.min_size = Number.MAX_SAFE_INTEGER;
        this.max_size = 0;
        for (var i = 0; i < this.boneList.length; ++i)
        {
            if (this.boneList[i].parent && this.boneList.includes(this.boneList[i].parent))
            {
                // var distance = this.boneList[i].getLocalTranslation().length();
                var distance_vector = this.boneList[i].getWorldTranslation().clone();
                distance_vector.sub(this.boneList[i].parent.getWorldTranslation());
                var distance_world = distance_vector.length();
                // console.log('distance is ' + distance);
                if (distance_world > 0) {
                    this.min_size = Math.min(this.min_size, distance_world);
                    this.max_size = Math.max(this.max_size, distance_world);
                }
            }
        }
    },

    applyTransformToBone: function(src_entity, dest_entity) {
        var pos = src_entity.getWorldTranslation().clone();
        pos.sub(src_entity.parent.getWorldTranslation());
        var distance = pos.length();
        var xy_scale = this.scale * Math.clamp(distance, this.min_size, this.max_size) / 15.0;
        var model_scale = new GASEngine.Matrix4();
        model_scale.scale(new GASEngine.Vector3(xy_scale, xy_scale, distance));
        pos.normalize();
        var model_rotate = new GASEngine.Matrix4();
        if (pos.z < 0.9999) {
            var rotate_axis = new GASEngine.Vector3(-pos.y, pos.x, 0).normalize();
            var cos_theta_d2 = Math.sqrt((1 + pos.z) / 2);
            var sin_theta_d2 = Math.sqrt(1 - cos_theta_d2 * cos_theta_d2);
            var quat = new GASEngine.Quaternion(sin_theta_d2 * rotate_axis.x,
                 sin_theta_d2 * rotate_axis.y,
                 sin_theta_d2 * rotate_axis.z,
                 cos_theta_d2);
            model_rotate.makeRotationFromQuaternion(quat);
        }
        var final_matrix = new GASEngine.Matrix4();
        final_matrix.setPosition(src_entity.parent.getWorldTranslation());
        final_matrix.multiplyMatrices(final_matrix, model_rotate);
        final_matrix.multiplyMatrices(final_matrix, model_scale);
        dest_entity.setLocalMatrix(final_matrix);
        dest_entity.matrixWorld.copy(final_matrix);
    },

    applyTransformToJoint: function(src_entity, dest_entity) {
        var pos = src_entity.getWorldTranslation().clone();
        pos.sub(src_entity.parent.getWorldTranslation());
        var distance = pos.length();

        var radius = this.scale * Math.clamp(distance, this.min_size, this.max_size) / 2 / 15.0;
        var model_scale = new GASEngine.Matrix4();
        model_scale.scale(new GASEngine.Vector3(radius, radius, radius));
        var final_matrix = new GASEngine.Matrix4();
        final_matrix.setPosition(src_entity.parent.getWorldTranslation());
        final_matrix.multiplyMatrices(final_matrix, model_scale);
        dest_entity.setLocalMatrix(final_matrix);
        dest_entity.matrixWorld.copy(final_matrix);
    }
};