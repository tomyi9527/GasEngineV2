//Component
GASEngine.Component = function(uniqueID)
{
    GASEngine.Events.attach(this);

    this.mutable = false;
    this.syncLoading = false;
    this.parentEntity = null;
    this.uniqueID = uniqueID;
    this._scene = null;
};

GASEngine.Component.MSG_COMPONENT_CREATED = 'MSG_COMPONENT_CREATED';
GASEngine.Component.MSG_COMPONENT_PROPERTY_CHANGED = 'MSG_COMPONENT_INITED';

GASEngine.Component.prototype =
{
    constructor: GASEngine.Component,
    typeName: 'component',

    get scene() { return this._scene; },
    set scene(value) { this._scene = value; },

    getParentEntity: function()
    {
        return this.parentEntity;
    },

    destroy: function()
    {
        this.parentEntity = null;
        this._scene = null;
    }
};


//Animator
GASEngine.AnimatorComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.clips = [];
    this.activeClip = null;
};

GASEngine.AnimatorComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.AnimatorComponent.prototype.constructor = GASEngine.AnimatorComponent;
GASEngine.AnimatorComponent.prototype.typeName = 'animator';

GASEngine.AnimatorComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);

    for(var i = 0, l = this.clips.length; i < l; i++)
    {
        var clip = this.clips[i];
        GASEngine.KeyframeAnimationFactory.Instance.destroyKeyframeAnimation(clip);
    }
    this.clips.length = 0;
    this.activeClip = null;
}

GASEngine.AnimatorComponent.prototype.takeoutClips = function()
{
    var clipsCopy = [];
    this.clips.forEach(clip=>clipsCopy.push(clip));
    this.clips.length = 0;
    this.activeClip = null;
    return clipsCopy;
}
GASEngine.AnimatorComponent.prototype.setAnimationClip = function(id, clip)
{
    let index = this.getAnimationClipIndex(id);
    if(index !== -1)
    {
        console.info('GASEngine.AnimatorComponent.setAnimationClip: a clip with the same name is already exist!');
    }

    if(this.parentEntity === null)
    {
        console.error('GASEngine.AnimatorComponent.setAnimationClip: animator component must be attached to a entity before the clip to be assigned!');
        return false;
    }

    this.clips.push(clip);

    clip.linkToObjectsAndProperties(this.parentEntity);

    this.emit(GASEngine.Component.MSG_COMPONENT_PROPERTY_CHANGED, name);
    
    return true;
};

GASEngine.AnimatorComponent.prototype.deleteAnimationClip = function(id) 
{
    let index = this.getAnimationClipIndex(id);
    if(index !== -1) 
    {
        var clip = this.clips[index];
        if(this.activeClip === clip)
        {
            this.activeClip = null;
        }
        GASEngine.KeyframeAnimationFactory.Instance.destroyKeyframeAnimation(clip);
        this.clips.splice(index, 1);
        return true;
    }
    else
    {
        console.error('GASEngine.AnimatorComponent.deleteAnimationClip: the clip not exist!');
        return false;
    }
};

GASEngine.AnimatorComponent.prototype.getAnimationClipIndex = function(id) 
{
    let index = this.clips.findIndex(v => v.id === id);
    return index;
},

GASEngine.AnimatorComponent.prototype.getAnimationClip = function(id) 
{
    let index = this.getAnimationClipIndex(id);
    if(index !== -1)
    {     
        let clip = this.clips[index];   
        return clip;
    }
    else
    {
        console.error('GASEngine.AnimatorComponent.getAnimationClip: the clip does not exist!');
        return null;
    }
}

GASEngine.AnimatorComponent.prototype.getAnimationClips = function()
{
    return this.clips;
};

GASEngine.AnimatorComponent.prototype.getActiveAnimationClip = function() 
{
    return this.activeClip;
};

GASEngine.AnimatorComponent.prototype.getActiveAnimationClipIndex = function() 
{
    let index = -1;
    if(this.activeClip)
    {
        index = this.clips.findIndex(v => v === this.activeClip);
    }
    return index;
};

GASEngine.AnimatorComponent.prototype.getAnimationClipCount = function()
{
    return this.clips.length;
};

GASEngine.AnimatorComponent.prototype.getAnimationClipNameList = function()
{
    const nameList = this.clips.map(e => e.name);
    return nameList;
};

GASEngine.AnimatorComponent.prototype.play = function(id)
{
    let clip = this.getAnimationClip(id);
    if(clip)
    {
        this.activeClip = clip;
        this.activeClip.progress = 0.0;
        return true;
    }
    else
    {
        console.error('GASEngine.AnimatorComponent.play: the specified clip is not exist!');
        return false;
    }
};

GASEngine.AnimatorComponent.prototype.playByIndex = function(index)
{
    if(index >= 0 && index < this.clips.length)
    {
        this.activeClip = this.clips[index];
        this.activeClip.progress = 0.0;
        return true;
    }
    else
    {
        console.error('GASEngine.AnimatorComponent.play: the specified clip is not exist!');
        return false;
    }
};

GASEngine.AnimatorComponent.prototype.stop = function(id)
{
    if(this.activeClip !== null && this.activeClip.id === id)
    {
        this.activeClip = null;

        return true;
    }
    else
    {
        console.error('GASEngine.AnimatorComponent.stop: the specified clip is not exist!');
        return false;
    }
};

//Mesh Filter
GASEngine.MeshFilterComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.mesh = null;
    this.bbox = null;
};

GASEngine.MeshFilterComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.MeshFilterComponent.prototype.constructor = GASEngine.MeshFilterComponent;
GASEngine.MeshFilterComponent.prototype.typeName = 'meshFilter';

GASEngine.MeshFilterComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);
    this.mesh.destroy();
    this.mesh = null;
    this.bbox = null;
}

GASEngine.MeshFilterComponent.prototype.setMesh = function(mesh)
{
    this.mesh = mesh;
};

GASEngine.MeshFilterComponent.prototype.getMesh = function()
{
    return this.mesh;
};


//Mesh Renderer
GASEngine.MeshRendererComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.materials = [];
};

GASEngine.MeshRendererComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.MeshRendererComponent.prototype.constructor = GASEngine.MeshRendererComponent;
GASEngine.MeshRendererComponent.prototype.typeName = 'meshRenderer';

GASEngine.MeshRendererComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);
    this.materials.length = 0;
}

GASEngine.MeshRendererComponent.prototype.addMaterial = function(material)
{
    this.materials.push(material);
};
GASEngine.MeshRendererComponent.prototype.setMaterialByIndex = function(material, index)
{
    this.materials[index] = material;
};
GASEngine.MeshRendererComponent.prototype.getMaterials = function()
{
    return this.materials;
};





//Camera
GASEngine.CameraComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.type = 'perspective'; //orthographic
    this.cullingMask = null;
    this.occlusionCulling = false;
    this.hdr = false;

    //this.clearFlag = 'skybox'; //color, image, depth_stencil, none
    //this.backgroundColor = [0.8, 0.8, 0.8, 1.0];

    this.fov = 60.0;    //for perspective
    this.width = 8.0;   //for orthographic

    this.aspect = 1.0;
    this.near = 0.3;
    this.far = 1000.0;
    this.renderingOrder = 0;
    this.viewport = null;
    this.renderPipeline = null;
    this.renderTarget = null;

    this._matrixWorld = null;
    this._matrixView = new GASEngine.Matrix4();
    this._matrixProjection = new GASEngine.Matrix4();
    this._matrixViewProjection = new GASEngine.Matrix4();

    this._skybox = {
        'mesh': null,
        'material': null,
        'subMeshIndex': -1,
        'matrixWorld': null,
        'depth': 1.0
    };

    this._opaqueList = [];
    this._opaqueListLastIndex = -1;

    // this._opaqueTopologyList = [];
    // this._opaqueTopologyListLastIndex = -1;

    // this._opaqueUVTopologyList = [];
    // this._opaqueUVTopologyListLastIndex = -1;

    this._helperList = [];
    this._helperListLastIndex = -1;

    this._hotspotItem = {'mesh': null, 'material': null, 'subMeshIndex': -1};

    this._transparentList = [];
    this._transparentListLastIndex = -1;

    this._meshList = [];
    this._meshListLastIndex = -1;
};

GASEngine.CameraComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.CameraComponent.prototype.constructor = GASEngine.CameraComponent;
GASEngine.CameraComponent.prototype.typeName = 'camera';

GASEngine.CameraComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);
    
    this._opaqueList.length = 0;
    this._opaqueListLastIndex = -1;
    
    this._helperList.length = 0;
    this._helperListLastIndex = -1;

    this._transparentList.length = 0;
    this._transparentListLastIndex = -1;

    this._meshList.length = 0;
    this._meshListLastIndex = -1;

    this._skybox.mesh = null;
    this._skybox.material = null;
    
    this._hotspotItem.mesh = null;
    this._hotspotItem.material = null;
}

GASEngine.CameraComponent.prototype.getSkybox = function()
{
    return this._skybox;
};

GASEngine.CameraComponent.prototype.getOpaqueList = function()
{
    return this._opaqueList;
};

GASEngine.CameraComponent.prototype.getOpaqueListLength = function()
{
    return (this._opaqueListLastIndex + 1);
};

GASEngine.CameraComponent.prototype.getMeshList = function()
{
    return this._meshList;
};

GASEngine.CameraComponent.prototype.getMeshListLength = function()
{
    return (this._meshListLastIndex + 1);
};

// GASEngine.CameraComponent.prototype.getOpaqueTopologyList = function()
// {
//     return this._opaqueTopologyList;
// };

// GASEngine.CameraComponent.prototype.getOpaqueTopologyListLength = function()
// {
//     return (this._opaqueTopologyListLastIndex + 1);
// };

// GASEngine.CameraComponent.prototype.getOpaqueUVTopologyList = function()
// {
//     return this._opaqueUVTopologyList;
// };

// GASEngine.CameraComponent.prototype.getOpaqueUVTopologyListLength = function()
// {
//     return (this._opaqueUVTopologyListLastIndex + 1);
// };

GASEngine.CameraComponent.prototype.getTransparentList = function()
{
    return this._transparentList;
};

GASEngine.CameraComponent.prototype.getTransparentListLength = function()
{
    return (this._transparentListLastIndex + 1);
};

GASEngine.CameraComponent.prototype.getHotspotItem = function()
{
    return this._hotspotItem;
};

GASEngine.CameraComponent.prototype.getHelperList = function()
{
    return this._helperList;
};

GASEngine.CameraComponent.prototype.getHelperListLength = function()
{
    return (this._helperListLastIndex + 1);
};

GASEngine.CameraComponent.prototype.addHotspotItem = function(mesh, material, subMeshIndex)
{
    this._hotspotItem.mesh = mesh;
    this._hotspotItem.material = material;
    this._hotspotItem.subMeshIndex = subMeshIndex;
};

GASEngine.CameraComponent.prototype.setWorldMatrix = function(matrix)
{
    this._matrixWorld = matrix;
};

GASEngine.CameraComponent.prototype.getWorldMatrix = function()
{
    if(this._matrixWorld)
    {
        return this._matrixWorld;
    }
    else
    {
        if(this.parentEntity !== null)
        {
            return this.parentEntity.matrixWorld;
        }
        else
        {
            console.error('GASEngine.CameraComponent.getWorldMatrix: Camera component must be attach to an entity at first.');
        }
    }
};

GASEngine.CameraComponent.prototype.getViewMatrix = function()
{
    return this._matrixView;
};

GASEngine.CameraComponent.prototype.getProjectionMatrix = function()
{
    return this._matrixProjection;
};

GASEngine.CameraComponent.prototype.setProjectionMatrix = function (matrix)
{
    this._matrixProjection.copy(matrix);
};

Object.defineProperty(GASEngine.CameraComponent.prototype, "projectionMatrix", { get: function () { return this._matrixProjection; } });

GASEngine.CameraComponent.prototype.getViewProjectionMatrix = function()
{
    return this._matrixViewProjection;
};

GASEngine.CameraComponent.prototype._updateViewMatrix = function()
{
    if(this._matrixWorld)
    {
        this._matrixView.getInverse(this._matrixWorld);
    }
    else
    {
        if(this.parentEntity !== null)
        {
            this._matrixView.getInverse(this.parentEntity.matrixWorld);
        }
        else
        {
            console.error('GASEngine.CameraComponent._updateViewMatrix: Camera component must be attach to an entity at first.');
        }
    }
};

GASEngine.CameraComponent.prototype._updateProjectionMatrix = function()
{
    if(this.type === 'perspective')
    {
        this._matrixProjection.makePerspective(this.fov, this.aspect, this.near, this.far);
    }
    else if(this.type === 'orthographic')
    {
        var width = this.width / 2.0;
        var height = this.width * this.aspect / 2.0;
        this._matrixProjection.makeOrthographic(-width, width, height, -height, this.near, this.far);
    }
    else
    {
        console.error('GASEngine.CameraComponent._updateProjectionMatrix: Unknown camera type.');
    }
};

GASEngine.CameraComponent.prototype._updateViewProjectionMatrix = function()
{
    this._matrixViewProjection.multiplyMatrices(this._matrixProjection, this._matrixView);
};

GASEngine.CameraComponent.prototype.clearRenderableList = function()
{
    this._opaqueListLastIndex = -1;
    this._opaqueTopologyListLastIndex = -1;
    this._transparentListLastIndex = -1;
    this._meshListLastIndex = -1;
    this._helperListLastIndex = -1;
};

GASEngine.CameraComponent.prototype.appendMesh = function (
    mesh,
    material,
    envirnonmentalLight,
    punctualLights,
    directionalLights,
    pointLights,
    spotLights,
    matrixWorld,
    depth,
    entityType)
{
    if(entityType === 'helper' || material instanceof GASEngine.SkyboxMaterial) {
        return;
    }
    var index = ++(this._meshListLastIndex);
    var item = this._meshList[index];
    if(item !== undefined)
    {
        item.mesh = mesh;
        item.material = material;
        item.envirnonmentalLight = envirnonmentalLight;
        item.punctualLights = punctualLights;
        item.directionalLights = directionalLights[0];
        item.pointLight = pointLights[0],
        item.spotLight = spotLights[0],
        item.matrixWorld = matrixWorld;
        item.depth = depth;
    }
    else
    {
        item = {
            'mesh' : mesh,
            'material': material,
            'subMeshIndex': -1,
            'envirnonmentalLight': envirnonmentalLight,
            'punctualLights': punctualLights,
            'directionalLight': directionalLights[0],
            'pointLight': pointLights[0],
            'spotLight': spotLights[0],
            'matrixWorld': matrixWorld,
            'depth': depth
        };
        this._meshList.push(item);
    }
}

GASEngine.CameraComponent.prototype.appendRenderables = function (
    mesh,
    material,
    subMeshIndex,
    envirnonmentalLight,
    punctualLights,
    directionalLights,
    pointLights,
    spotLights,
    matrixWorld,
    depth,
    entityType)
{
    if(material instanceof GASEngine.SkyboxMaterial)
    {
        this._skybox.mesh = mesh;
        this._skybox.material = material;
        this._skybox.subMeshIndex = subMeshIndex;
        this._skybox.matrixWorld = matrixWorld;
        return;
    }

    var array, index;
    //let isTopo = false;
    if(material !== null && material.transparencyEnable)
    {
        array = this._transparentList;
        index = ++(this._transparentListLastIndex);
    }
    else if(entityType === 'helper')
    {
        array = this._helperList;
        index = ++(this._helperListLastIndex);
    }
    else
    {
        array = this._opaqueList;
        index = ++(this._opaqueListLastIndex);
        //isTopo = true;
    }

    var item = array[index];

    if(item !== undefined)
    {
        item.mesh = mesh;
        item.material = material;
        item.subMeshIndex = subMeshIndex;
        item.envirnonmentalLight = envirnonmentalLight;
        item.punctualLights = punctualLights;
        item.directionalLights = directionalLights[0];
        item.pointLight = pointLights[0],
        item.spotLight = spotLights[0],
        item.matrixWorld = matrixWorld;
        item.depth = depth;
    }
    else
    {
        item = {
            'mesh' : mesh,
            'material': material,
            'subMeshIndex': subMeshIndex,
            'envirnonmentalLight': envirnonmentalLight,
            'punctualLights': punctualLights,
            'directionalLight': directionalLights[0],
            'pointLight': pointLights[0],
            'spotLight': spotLights[0],
            'matrixWorld': matrixWorld,
            'depth': depth
        };
        array.push(item);
    }

    // if (isTopo)
    // {
    //     this._opaqueTopologyListLastIndex = index;
    //     this._opaqueUVTopologyListLastIndex = index;

    //     var topologyItem = this._opaqueTopologyList[index];
    //     var uvTopologyItem = this._opaqueUVTopologyList[index];

    //     // var topologyMaterial = GASEngine.MaterialFactory.Instance.create('wireframe');
    //     // var uvTopologyMaterial = GASEngine.MaterialFactory.Instance.create('uvlayout');

    //     if(topologyItem !== undefined)
    //     {
    //         topologyItem.mesh = mesh;
    //         topologyItem.material = null;
    //         topologyItem.subMeshIndex = subMeshIndex;
    //         topologyItem.envirnonmentalLight = envirnonmentalLight;
    //         topologyItem.punctualLights = punctualLights;
    //         topologyItem.directionalLights = directionalLights[0];
    //         topologyItem.pointLight = pointLights[0],
    //         topologyItem.spotLight = spotLights[0],
    //         topologyItem.matrixWorld = matrixWorld;
    //         topologyItem.depth = depth;
    //     }
    //     else
    //     {
    //         topologyItem = {
    //             'mesh' : mesh,
    //             'material': null,
    //             'subMeshIndex': subMeshIndex,
    //             'envirnonmentalLight': envirnonmentalLight,
    //             'punctualLights': punctualLights,
    //             'directionalLight': directionalLights[0],
    //             'pointLight': pointLights[0],
    //             'spotLight': spotLights[0],
    //             'matrixWorld': matrixWorld,
    //             'depth': depth,
    //         };
    //         this._opaqueTopologyList.push(topologyItem);
    //     }

    //     if(uvTopologyItem !== undefined)
    //     {
    //         uvTopologyItem.mesh = mesh;
    //         uvTopologyItem.material = null;
    //         uvTopologyItem.subMeshIndex = subMeshIndex;
    //         uvTopologyItem.envirnonmentalLight = envirnonmentalLight;
    //         uvTopologyItem.punctualLights = punctualLights;
    //         uvTopologyItem.directionalLights = directionalLights[0];
    //         uvTopologyItem.pointLight = pointLights[0],
    //         uvTopologyItem.spotLight = spotLights[0],
    //         uvTopologyItem.matrixWorld = matrixWorld;
    //         uvTopologyItem.depth = depth;
    //     }
    //     else
    //     {
    //         uvTopologyItem = {
    //             'mesh' : mesh,
    //             'material': null,
    //             'subMeshIndex': subMeshIndex,
    //             'envirnonmentalLight': envirnonmentalLight,
    //             'punctualLights': punctualLights,
    //             'directionalLight': directionalLights[0],
    //             'pointLight': pointLights[0],
    //             'spotLight': spotLights[0],
    //             'matrixWorld': matrixWorld,
    //             'depth': depth,
    //         };
    //         this._opaqueUVTopologyList.push(uvTopologyItem);
    //     }
    // }
};



////Skybox Component
//GASEngine.SkyboxComponent = function(uniqueID)
//{
//    GASEngine.Component.call(this, uniqueID);
//};

//GASEngine.SkyboxComponent.prototype = Object.create(GASEngine.Component.prototype);
//GASEngine.SkyboxComponent.prototype.constructor = GASEngine.SkyboxComponent;
//GASEngine.SkyboxComponent.prototype.typeName = 'skybox';


//Light
GASEngine.PunctualLightComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.type = 'hemisphere'; //direction, point, spot
};

GASEngine.PunctualLightComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.PunctualLightComponent.prototype.constructor = GASEngine.PunctualLightComponent;
GASEngine.PunctualLightComponent.prototype.typeName = 'punctualLight';


//Environmental Light
GASEngine.EnvironmentalLightComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.environmentExposure = new Float32Array(1.0);

    this.specularCubeMap = null;
    this.specularCubeMapSize = new Float32Array([0.0, 0.0]);
    this.specularCubeMapLODRange = new Float32Array([0.0, 0.0]);

    this.specularPanorama = null;
    this.specularPanoramaSize = new Float32Array([0.0, 0.0]);
    this.specularPanoramaLODRange = new Float32Array([0.0, 0.0]);

    this.specularIntegratedBRDF = null;

    this.sph = null;
    this.orientation = 0.0;   /*  0 -> 360  */
    this.environmentName = '';
};

GASEngine.EnvironmentalLightComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.EnvironmentalLightComponent.prototype.constructor = GASEngine.EnvironmentalLightComponent;
GASEngine.EnvironmentalLightComponent.prototype.typeName = 'environmentalLight';

GASEngine.EnvironmentalLightComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);
    this.environmentExposure = null;
    this.specularCubeMap = null;
    this.specularCubeMapSize = null;
    this.specularCubeMapLODRange = null;

    this.specularPanorama = null;
    this.specularPanoramaSize = null;
    this.specularPanoramaLODRange = null;

    this.specularIntegratedBRDF = null;

    this.sph = null;
}

GASEngine.EnvironmentalLightComponent.prototype.setEnvironmentName = function(name)
{
    this.environmentName = name;
    GASEngine.Resources.Instance.loadCubeTexture
    (
        `/system/backgroundCubes/${this.environmentName}/specular_cubemap_ue4_256_luv.bin`,
        256,
        function (texture, width, height)
        {
            this.setSpecularCubeMap(texture, width, height);
        }.bind(this)
    );

    GASEngine.Resources.Instance.loadPanorama
    (
        `/system/backgroundCubes/${this.environmentName}/specular_panorama_ue4_1024_luv.bin`,
        function (texture, width, height)
        {
            this.setSpecularPanoramaTexture(texture, width, height);
        }.bind(this)
    );

    GASEngine.Resources.Instance.loadIntegratedBRDF
    (
        `/system/backgroundCubes/${this.environmentName}/brdf_ue4.bin`,
        function (spcularIntegratedBRDF, width, height)
        {
            this.setSpecularIntegratedBRDF(spcularIntegratedBRDF);
        }.bind(this)
    );

    return new Promise((resolve, reject) => {
        GASEngine.Resources.Instance.loadSPH
        (
            `/system/backgroundCubes/${this.environmentName}/diffuse_sph.json`,
            function (presetExposure, sph)
            {
                this.setDiffuseSPH(presetExposure, sph);
                resolve();
            }.bind(this)
        );
    });
};

GASEngine.EnvironmentalLightComponent.prototype.setEnvironmentExposure = function(value)
{
    this.environmentExposure[0] = value;
};

GASEngine.EnvironmentalLightComponent.prototype.getEnvironmentExposure = function()
{
    return this.environmentExposure[0];
};

GASEngine.EnvironmentalLightComponent.prototype.setSpecularCubeMap = function (texture, width, height)
{
    var gl = GASEngine.WebGLDevice.Instance.gl;
    var OT = gl.getParameter(gl.TEXTURE_BINDING_CUBE_MAP);
    gl.bindTexture(gl.TEXTURE_CUBE_MAP, texture);
    gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    //FOR LOD TEXTURE FETCH
    gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
    gl.bindTexture(gl.TEXTURE_CUBE_MAP, OT);

    this.specularCubeMap = texture;
    this.specularCubeMapSize[0] = width;
    this.specularCubeMapSize[1] = height;

    var nbLod = Math.log(width) / Math.LN2;
    var maxLod = nbLod - Math.log(8.0) / Math.LN2;
    this.specularCubeMapLODRange[0] = nbLod;
    this.specularCubeMapLODRange[1] = maxLod;
};

GASEngine.EnvironmentalLightComponent.prototype.setSpecularPanoramaTexture = function (texture, width, height)
{
    this.specularPanorama = texture;
    this.specularPanoramaSize[0] = width;
    this.specularPanoramaSize[1] = height;

    var nbLod = Math.log(width) / Math.LN2;
    var maxLod = nbLod - Math.log(32.0) / Math.LN2;
    this.specularPanoramaLODRange[0] = nbLod;
    this.specularPanoramaLODRange[1] = maxLod;
};

GASEngine.EnvironmentalLightComponent.prototype.setDiffuseSPH = function (presetExposure, sph)
{
    this.environmentExposure[0] = presetExposure;
    this.sph = sph;
};

GASEngine.EnvironmentalLightComponent.prototype.setSpecularIntegratedBRDF = function (specularIntegratedBRDF)
{
    this.specularIntegratedBRDF = specularIntegratedBRDF;
};

//Resource Component
GASEngine.ResourceComponent = function(uniqueID)
{
    GASEngine.Component.call(this, uniqueID);

    this.resources = [];
};

GASEngine.ResourceComponent.prototype = Object.create(GASEngine.Component.prototype);
GASEngine.ResourceComponent.prototype.constructor = GASEngine.ResourceComponent;
GASEngine.ResourceComponent.prototype.typeName = 'resource';

GASEngine.ResourceComponent.prototype.destroy = function()
{
    GASEngine.Component.prototype.destroy.call(this);
    this.resources.length = 0;
}

GASEngine.ResourceComponent.prototype.addResources = function(url)
{
    this.resources.push(url);
};

