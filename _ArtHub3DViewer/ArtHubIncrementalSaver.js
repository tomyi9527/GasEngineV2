// tab1: renderer 的参数更新，background 和 light更新
// tab2: material 的参数更新
// tab3: structure 的参数更新，animation 的列表更新, mesh 隐藏，model 删除与替换

// renderer.conf.json 渲染设置
// scene.structure.json 背景，灯光
// xxx.mat.json 材质编辑
// xxx.fbx.structure.json 场景结构（model的结构）

ArtHubIncrementalSaver = function()
{
    this._networkHub = new NetworkHub();
    this._networkHub.init();

    this._rendererConfBaseUrl = 'renderer.conf.json';
    this._sceneStructureBaseUrl = 'scene.structure.json';
    this._needCreateScene = false;
    this._needCreateRenderer = false;

    GASEngine.GAS2IncrementalSaver.call(this);
}
ArtHubIncrementalSaver.prototype = Object.create(GASEngine.GAS2IncrementalSaver.prototype);
ArtHubIncrementalSaver.prototype.constructor = ArtHubIncrementalSaver;

ArtHubIncrementalSaver.prototype.applyChange = function()
{
    const modelFormat = ArthubLoader.Instance.modelFormat;
    if(modelFormat !== 'gas2') {
        return Promise.reject(this._formatResponse(false, '暂不支持该格式文件的保存'));
    }
    return this._createFiles()
        .then(() => {
            return this._updateChanges()
        });
}

ArtHubIncrementalSaver.prototype._createFiles = function()
{
    const createPayloadItems = [];

    const baseStrcutureUrl = ArthubLoader.Instance.modelStructureUrl
    let strcutureFullUrl = GASEngine.FileSystem.Instance.getFullPathByPath(baseStrcutureUrl)
    strcutureFullUrl = GASEngine.Utilities.engineDecodeURIComponent(strcutureFullUrl);
    const index = strcutureFullUrl.indexOf(baseStrcutureUrl);
    const baseUrl = strcutureFullUrl.substring(0, index);

    if(!baseUrl) {
        return Promise.reject(this._formatResponse(false, '文件路径获取失败'));
    }

    const fileFullPathList = [];

    if(this._needCreateRenderer) {
        const rendererFullUrl = `${baseUrl}${this._rendererConfBaseUrl}`;
        createPayloadItems.push({file_name: rendererFullUrl, content: JSON.stringify({})});
        fileFullPathList.push({url: this._rendererConfBaseUrl, fullUrl: rendererFullUrl});
    }
    if(this._needCreateScene) {
        const sceneStructureFullUrl = `${baseUrl}${this._sceneStructureBaseUrl}`;
        createPayloadItems.push({file_name: sceneStructureFullUrl, content: JSON.stringify({})});
        fileFullPathList.push({url: this._sceneStructureBaseUrl, fullUrl: sceneStructureFullUrl});
    }

    if(createPayloadItems.length === 0) return Promise.resolve();

    const assetId = Number(ArthubLoader.Instance.modelId);
    const assetHub = ArthubLoader.Instance.assetHub;
    const createPayload = {
        asset_id: assetId,
        override: true,
        items: createPayloadItems
    };
    return this._networkHub.createFile(createPayload, assetHub)
        .then(res => {
            fileFullPathList.forEach(e => {
                GASEngine.FileSystem.Instance.setFullPath(e.fullUrl, e.url);
            });
            return res;
        })
}

ArtHubIncrementalSaver.prototype._updateChanges = function()
{
    const assetId = Number(ArthubLoader.Instance.modelId);
    const fileNames = []; const operations = [];
    for(let filePath of this._streamsMap.keys()) {
        let fileMap = this._streamsMap.get(filePath);
        const items = [];
        let nodeId;
        for(let propPath of fileMap.keys()) {
            let arr = propPath.split('/');
            nodeId = parseInt(arr[0]);
            let path = arr.slice(1).join('/');
            let value = fileMap.get(propPath);
            const obj = {};
            obj[path] = value;
            items.push(obj);
        }

        if(filePath === this._rendererConfBaseUrl || filePath === this._sceneStructureBaseUrl) {
            filePath = GASEngine.FileSystem.Instance.getFullPathByPath(filePath);
        }

        fileNames.push(filePath);
        operations.push({
            items: items
        });
    }

    

    if(fileNames.length === 0) return Promise.resolve(this._formatResponse(true, ''));
    

    const payload = {
        asset_id: assetId,
        filenames: fileNames,
        operations,
    };
    const assetHub = ArthubLoader.Instance.assetHub;
    
    return this._networkHub.updateJsonValueByPath(payload, assetHub)
        .then(res => {
            if(res) {
                this._streamsMap.clear();
                return this._formatResponse(true, '');
            } else {
                return this._formatResponse(false, '保存失败');
            }
        })
        .catch(error => {
            return this._formatResponse(false, error.message);
        })
}

ArtHubIncrementalSaver.prototype._formatResponse = function(state, message) {
    return {
        state, message
    }
}

ArtHubIncrementalSaver.prototype.storeChange = function(type, params = {})
{
    const {key, value} = params;
    switch(type) {
        case 'material': {
            const {material} = params;
            this.updateMaterialPropertyValue(material, material.uniqueID, key, value);
            break;
        }
        case 'scene': {
            const url = this._sceneStructureBaseUrl;
            const fullUrl = GASEngine.FileSystem.Instance.getFullPathByPath(url);
            let storeUrl = fullUrl || url;
            this._needCreateScene = storeUrl === url;
            this.updateSceneStructurePropertyValue(storeUrl, key, value);
            break;
        }
        case 'renderer': {
            const url = this._rendererConfBaseUrl;
            const fullUrl = GASEngine.FileSystem.Instance.getFullPathByPath(url);
            let storeUrl = fullUrl || url;
            this._needCreateRenderer = storeUrl === url;
            this.updateRendererPropertyValue(storeUrl, key, value);
            break;
        }
        default:
            break;
    }
}