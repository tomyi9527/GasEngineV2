ArthubLoader = function () {
  GASEngine.Events.attach(this);

  this._assetNameCache = new Map();

  this._onHierarchySuccess = null;
  this._onSyncLoading = null;
  this._onAsyncLoading = null;
  this._realLoader = null;
  this._rawDisplayFileList_ = [];

  this._networkHub_ = new NetworkHub();

  this._modelFormat = '';

  ArthubLoader.Instance = this;
};

ArthubLoader.prototype = {
  constructor: ArthubLoader,

  get modelId() {
    return this._modelId;
  },

  get assetHub() {
    return this._assetHub;
  },

  get modelFormat() {
    return this._modelFormat;
  },

  get modelStructureUrl() {
    let index = this._rawDisplayFileList_.findIndex(e => e.indexOf('.structure.json') !== -1);
    if (index === -1) return '';
    return this._rawDisplayFileList_[index];
  },

  getRawDisplayFileList: function () {
    return this._rawDisplayFileList_;
  },

  getSkyboxList: function () {
    return this._realLoader.getSkyboxList();
  },

  getEnvironmentalLightList: function () {
    return this._realLoader.getEnvironmentalLightList();
  },

  load: function (scene, modelInfo, onHierarchySuccess, onSyncLoading, onAsyncLoading, onRendererLoaded) {
    if (!modelInfo || !modelInfo.assetHub || !modelInfo.id) {
      console.error('ArthubLoader: load params error!')
      return;
    }
    this._onHierarchySuccess = onHierarchySuccess;
    this._onSyncLoading = onSyncLoading;
    this._onAsyncLoading = onAsyncLoading;
    this._onRendererLoaded = onRendererLoaded;
    this._assetHub = modelInfo.assetHub;
    this._modelId = modelInfo.id;


    this._networkHub_.init(modelInfo);

    return this.loadModelUrl(modelInfo)
      .then(url => {
        return this.getFile(url);
      })
      .then(signedData => {
        signedData.assetHub = this._assetHub;
        signedData.id = this._modelId;

        let modelFormat = signedData.version;
        this._modelFormat = modelFormat;
        switch (modelFormat) {
          case 'gas2': {
            this._realLoader = new ArtHubGas2Loader();
            break;
          }
          default: {
            this._realLoader = new ArtHubGas1Loader();
            break;
          }
        }
        this._realLoader.onStructureLoadCallback = this.onStructureLoadCallback.bind(this);
        this._realLoader.onFileLoaded = this.onFileLoaded.bind(this);
        this._realLoader.onAnimationLoadCallback = this.onAnimationLoadCallback.bind(this);
        this._realLoader.onRendererLoaded = this._onRendererLoaded.bind(this);

        this.getDisplayFileList()
          .then(files => {
            files = files.length === 0 ? signedData.files : files;
            let params = {
              assetHub: this._assetHub,
              id: this._modelId,
              files: files
            }
            this._rawDisplayFileList_ = files;

            this._realLoader.load(
              scene,
              params,
              this._onHierarchySuccess.bind(this),
              this._onSyncLoading.bind(this),
              this._onAsyncLoading.bind(this)
            );
          })

        return true;
      })
      .catch(error => {
        console.log(error);
      })
  },

  loadModelUrl: function (modelInfo) {
    let assetHub = modelInfo.assetHub;
    let objectId = modelInfo.id;
    let payload = [{
      object_id: objectId,
      object_meta: "display_url"
    }];
    return this.getSignature(assetHub, payload);
  },

  //获取外部链接的地址
  //外部引用格式: "@@depot_id/asset_id/asset_file_meta/file_name"
  // asset_file_meta: intermediate_url/origin_url/display_url/preview_url
  // e.g: 贴图引用，"@@AssetHub_Atc/39730009631/intermediate_url/pve_cangzhidu_1_feiting.png"
  // e.g: 贴图引用可支持不加文件名，"@@AssetHub_Atc/39730009631/intermediate_url"
  // e.g: 动画引用："@@AssetHub_Atc/39730021105/display_url/young.fbx.Boxing_Attack_02.335.animation.bin"
  getLinkUrl: function (path) {
    let params = path.split('/');
    let assetHub = params[0];
    let id = parseInt(params[1], 10);
    let assetMeta = params[2];
    let fileName = params[3];
    let payload = [{
      object_id: id,
      object_meta: assetMeta,
      file_name: fileName
    }];
    return this.getSignature(assetHub, payload);
  },

  getArtHubAssetNameByUri(arthubUriArray) {
    let cachedResult = [];
    let cacheAllHit = true;
    let idList = []
    for (let i = 0; i < arthubUriArray.length; ++i) {
      let params = arthubUriArray[i].split('/');
      let id = parseInt(params[1]);
      if (params[0] && this._assetHub !== params[0]) {
        console.warn(`asset: ${id} of ${params[0]} is not allowed to get in another depot.`);
      }
      idList[i] = id;
      let cacheKey = `${this._assetHub}/${id}`;
      cachedResult[i] = this._assetNameCache.get(cacheKey);
    }
    for (let i = 0; i < cachedResult.length; ++i) {
      if (!cachedResult[i]) {
        cacheAllHit = false;
        break;
      }
    }
    if (cacheAllHit) {
      return cachedResult;
    } else {
      return this._networkHub_.getAssetDetailByID(this._assetHub, idList, ['name', 'file_format'])
        .then(resList => {
          let realResult = [];
          resList.forEach(res => {
            let id = res.id;
            let index = res.param_index;
            let cacheKey = `${this._assetHub}/${id}`;
            let nameResult = res.file_format ? `${res.name}.${res.file_format}` : res.name;
            this._assetNameCache.set(cacheKey, nameResult);
            realResult[index] = nameResult;
          });
          return realResult;
        });
    }
  },

  getTextureName: function (pathArray) {
    var filterArray = function (valueArray, cond) {
      var ret = {
        indexTable: [],
        filteredArray: [],
        disposedIndex: []
      };
      for (let i = 0; i < valueArray.length; ++i) {
        if (cond(valueArray[i])) {
          ret.indexTable.push(i);
          ret.filteredArray.push(valueArray[i]);
        } else {
          ret.disposedIndex.push(i);
        }
      }
      return ret;
    }
    // arthub prefix replace
    var filterResult = filterArray(pathArray, path => path.indexOf('@@') === 0);
    filterResult.filteredArray = filterResult.filteredArray.map(value => value.replace('@@', ''));

    return Promise.resolve(this.getArtHubAssetNameByUri(filterResult.filteredArray))
      .then(arthubNames => {
        var returnArray = [];
        filterResult.disposedIndex.forEach(idx => {
          returnArray[idx] = pathArray[idx];
        });
        filterResult.indexTable.forEach(idx => {
          returnArray[idx] = arthubNames[idx];
        });
        return returnArray;
      });
  },

  getSignedUrlByFileName: function (fileName) {
    fileName = GASEngine.Utilities.engineDecodeURIComponent(fileName);
    let displayFileList = this.getRawDisplayFileList();
    if (fileName.indexOf('@@') !== -1) {
      return this.getLinkUrl(fileName.replace('@@', ''));
    }
    if (displayFileList.indexOf(fileName) === -1) {
      return Promise.resolve(fileName);
    }

    let assetHub = this._assetHub;
    let modelId = this._modelId;
    let payload = [{
      object_id: modelId,
      object_meta: "display_url",
      file_name: fileName
    }];
    return this.getSignature(assetHub, payload)
      .then(url => url)
      .catch(error => {
        console.log(error);
      });
  },

  getDisplayFileList() {
    let assetHub = this._assetHub;
    return this._networkHub_.getFileListByIDMeta(assetHub, this._modelId)
  },

  onFileLoaded: function (params) {
    console.log(params);
  },

  onLoadError: function () {

  },

  getFile: function (url) {
    return this._networkHub_.getFile(url);
  },

  getSignature: function (assetHub, payload) {
    return this._networkHub_.getSignature(assetHub, payload);
  },

  // getAssembleConfigurations: function()
  // {
  //     return this._realLoader.getAssembleConfigurations();
  // }
}
