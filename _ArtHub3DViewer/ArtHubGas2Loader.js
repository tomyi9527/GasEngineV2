//gas2
ArtHubGas2Loader = function () {
  GASEngine.GAS2Loader.call(this);
  this._callBack_ = null;
}
ArtHubGas2Loader.prototype = Object.create(GASEngine.GAS2Loader.prototype);
ArtHubGas2Loader.prototype.constructor = ArtHubGas2Loader;

ArtHubGas2Loader.prototype.loadStructureFiles = function (modelInfo, callback) {
  this._callBack_ = callback;
  let convertedFiles = modelInfo.files;
  let id = modelInfo.id;
  let assetHub = modelInfo.assetHub;

  this._rawDisplayFileList_ = convertedFiles;

  this.loadRendererConf();

  this.loadSceneStructure(() => {
    this.getModelStructureFileSignature(convertedFiles, id, assetHub);
  });
}

ArtHubGas2Loader.prototype.loadRendererConf = function () {
  let index = this._rawDisplayFileList_.findIndex(e => e.indexOf('renderer.conf.json') !== -1);
  if (index === -1) return;
  let renderConfUrl = this._rawDisplayFileList_[index];
  GASEngine.FileSystem.Instance.read(
    renderConfUrl,
    function (data) {
      if (this.onRendererLoaded) {
        this.onRendererLoaded(data);
      }
    }.bind(this)
  );
}

ArtHubGas2Loader.prototype.loadSceneStructure = function (callback) {
  let index = this._rawDisplayFileList_.findIndex(e => e.indexOf('scene.structure.json') !== -1);
  if (index === -1) {
    callback();
    return;
  }
  let sceneStructureUrl = this._rawDisplayFileList_[index];
  GASEngine.FileSystem.Instance.read(
    sceneStructureUrl,
    function (data) {
      const {
        nodeTree
      } = data;
      if (nodeTree) {
        const {
          children
        } = nodeTree;
        if (children) {
          children.forEach(child => {
            if (child) {
              this.parseEntity(child);
            }
          });
        }
      }

      callback();
    }.bind(this)
  );
}

ArtHubGas2Loader.prototype.parseEntity = function (entityConf) {
  let entity = GASEngine.EntityFactory.Instance.create();
  entity.guid = GASEngine.generateUUID();
  this._scene_.appendEntityOnRoot(entity);

  if (entityConf.components) {
    this._walkComponents(entityConf.components, entity);
  }
}

ArtHubGas2Loader.prototype._walkComponents = function (nodeConfigComponents, entity) {
  if (nodeConfigComponents) {
    var componentTypes = Object.keys(nodeConfigComponents);
    for (var j = 0; j < componentTypes.length; ++j) {
      var typeName = componentTypes[j];
      var componentConf = nodeConfigComponents[typeName];
      if (typeName === 'skybox') {
        const mesh = GASEngine.MeshFactory.Instance.create();
        mesh.addStream('position', new Float32Array([1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0]));
        mesh.addStream('subMesh', [{
          'start': 0,
          'count': 4
        }]);
        mesh.setDrawMode('TRIANGLE_STRIP');
        mesh.submitToWebGL();
        const meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
        meshFilterComponent.setMesh(mesh);
        entity.addComponent(meshFilterComponent);

        const skyboxMaterial = GASEngine.MaterialFactory.Instance.create('skybox');
        const meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
        meshRendererComponent.addMaterial(skyboxMaterial);
        entity.addComponent(meshRendererComponent);

        if (componentConf.color !== undefined) {
          const [r, g, b] = componentConf.color;
          skyboxMaterial.setSolidColor(r, g, b);
        }

        if (componentConf.image !== undefined) {
          skyboxMaterial.backgroundName = componentConf.image;
        }

        if (componentConf.backgroundExposure !== undefined) {
          skyboxMaterial.setBackgroundExposure(componentConf.backgroundExposure);
        }

        if (componentConf.environment !== undefined) {
          const {
            environment
          } = componentConf;
          if (environment.name !== undefined) {
            skyboxMaterial.cubeMapName = environment.name;
          }

          if (environment.exposure !== undefined) {
            skyboxMaterial.setLightExposure(environment.exposure);
          }

          if (environment.blur !== undefined) {
            skyboxMaterial.setEnvironmentBlur(environment.blur);
          }

          if (environment.orientation !== undefined) {
            skyboxMaterial.setOrientation(environment.orientation);
          }
        }

        const backgroundType = componentConf.backgroundType || 'IMAGE';
        if (backgroundType === 'IMAGE') {
          const backgroundName = skyboxMaterial.backgroundName || this._defaultBackgroundImage;
          skyboxMaterial.setBackgroundImage(backgroundName);
        } else if (backgroundType === 'CUBEMAP' || backgroundType === 'AMBIENT') {
          const cubeMapName = skyboxMaterial.cubeMapName || this._defaultEnvironmentName;
          skyboxMaterial.setBackgroundCubeMap(cubeMapName);
        }
        skyboxMaterial.backgroundType = backgroundType;
        //skybox
        this._scene_.appendSkybox(entity);
      } else {
        const uniqueID = componentConf.uniqueID;
        const componentObject = GASEngine.ComponentFactory.Instance.create(typeName, uniqueID);
        if (!componentObject) continue;
        componentObject.syncLoading = componentConf.syncLoading;
        entity.addComponent(componentObject);
        this._loadComponent(componentObject, componentConf);
        //environmentalLight
        if (typeName === 'environmentalLight') {
          this._scene_.appendEnvironmentalLight(entity);
        }
      }
    }
  }
}

ArtHubGas2Loader.prototype.getModelStructureFileSignature = function () {
  //先判断是否有scene.structure.json
  let index = this._rawDisplayFileList_.findIndex(e => e.indexOf('.structure.json') !== -1 && e !== 'scene.structure.json');
  if (index === -1) {
    console.error('structure.json not found!');
    return;
  }
  let structureFile = this._rawDisplayFileList_[index];
  GASEngine.FileSystem.Instance.read(
    structureFile,
    function (data) {
      if (this._callBack_) {
        this._callBack_(data);
      }
    }.bind(this)
  );
}

// // 暂时不需要一把拉取所有的文件
// ArtHubGas2Loader.prototype.getModelFilesSignature = function(files, modelId, assetHub)
// {
//     if(!files || !Array.isArray(files) || modelId === -1) return Promise.resolve([]);
//     let payload = [], params;
//     files.forEach(
//         v =>
//         {
//             params = { object_id: modelId, object_meta: "display_url", file_name: v};
//             if(v.indexOf(".") !== -1 && v.split(".").pop().indexOf("gz") !== -1)
//             {
//                 params.content_encoding = "gzip";
//             }
//             payload.push(params);
//         }
//     );

//     return ArthubLoader.Instance.getSignature(assetHub, payload)
//         .then(res => {
//             let resList = res.items;
//             let signedFiles = [];
//             if(resList && Array.isArray(resList)) 
//             {
//                 resList.forEach(v =>
//                 {
//                     signedFiles.push(v.signed_url);
//                 });
//             }
//             let index = signedFiles.findIndex(e => e.indexOf('.structure.json') !== -1);
//             let signedFileUrl = signedFiles[index];
//             this._signedFileList_ = signedFiles;
//             return ArthubLoader.Instance.getFile(signedFileUrl);
//         })
//         .then(res => {
//             if(this._callBack_) {
//                 this._callBack_(res, this._signedFileList_);
//             }
//         })
//         .catch(error => {
//             console.log(error);
//         });
// }