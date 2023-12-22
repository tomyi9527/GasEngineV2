import ArtHubEnvironmetalLight from './ArtHubEnvironmetalLight.js';
import ArtHubSkybox from './ArtHubSkybox.js';
import ArtHubModel from './ArtHubModel.js';
import ArtHubBase from './ArtHubBase.js';
const MessageType = {
    StructureLoaded: 'structureLoaded',
};

class ArtHubScene extends ArtHubBase{
    constructor(options) {
        super(options);

        this.init();
    }

    init() {
        this._environmentalLight = new ArtHubEnvironmetalLight({iframe: this._iframe});
        this._skybox = new ArtHubSkybox({iframe: this._iframe});
        this._modelList = [];

        this.addEventHandler();
    }

    destroy() {
        this.removeEventHandler();
    }

    
    loadModel(modelInfo) {
        return new Promise((resolve, reject) => {
            this._invokeIFrame('load', modelInfo);
            this.addCallback(MessageType.StructureLoaded, () => {
                this.removeCallback(MessageType.StructureLoaded);
                this.setModelList();
                resolve(true);
            });
        });
    }

    unloadModel(model) {
        return this._invokeIFrame('deleteCurrentModel');
    }

    setModelList() {
        return this._invokeIFrame('getModels')
            .then(models => {
                this._modelList = [];
                models.forEach(e => {
                    const model = new ArtHubModel({iframe: this._iframe, ...e});
                    this._modelList.push(model);
                });
                return this._modelList;
            });
    }

    getEnvironmentalLight() {
        return Promise.resolve(this._environmentalLight);
    }

    getSkybox() {
        return Promise.resolve(this._skybox);
    }

    getModelList() {
        return Promise.resolve(this._modelList);
    }

    getModelById(id) {
        let index = this._modelList.findIndex(e => e.id === id);
        const model = index !== -1 ? this._modelList[index] : null;
        return model;
    }

    //params: uniqueId
    deleteModelByUniqueId(id) {
        return this._invokeIFrame('deleteModelByUniqueId', id)
            .then(() => {
                let index = this._modelList.findIndex(e => e.id === id);
                if(index === -1) return false;
                this._modelList.splice(index, 1);
            });
    }

    getTextureList() {
        return this._invokeIFrame('getTextureList').then(textureList=>{
            var pathes = []
            textureList.forEach(textureInfo=>{
                // image, path
                pathes.push(textureInfo.path);
            })
            return this._invokeIFrame('getTextureName', pathes).then(textureNames=>{
                for(let i = 0;i<pathes.length;++i) {
                    textureList[i].name = textureNames[i];
                }
                return textureList;
            });
        });
    }

    getTextureLinkByPath(path) {
        return this.getModelList().then(models=>{
            var promises = models.map(model=>model.getTextureLinkByPath(path));
            return Promise.all(promises).then(values=>{
                var ret = [];
                values.forEach(value=>{ret = ret.concat(value);});
                return ret;
            });
        });
    }

    removeTextureByPath(path) {
        return this._invokeIFrame('removeTextureByPath', path);
    }

    clearTextureList() {
        return this._invokeIFrame('clearTextureList');
    }

    // load newModel
    // animation append to newModel
    // delete oldModel
    // params: same as load
    // WIP: 还未完成请不要用它
    replaceModel() {
        // get current state.
        var s = this._model.getScene();
        var root = s.getModelRoot();
        var animatorComponent = root.getComponent('animator');
        var onAnimationLoaded = function() {
            // new data loaded.
            var newRoot = s.getModelRoot();
            var newAnimatorComponent = newRoot.getComponent('animator');
            // move clips to new root.
            var clips = animatorComponent.takeoutClips();
            // old clips is now empty, and it's ok to destroy
            clips.forEach(clip=>newAnimatorComponent.setAnimationClip(clip.getName(), clip));
            // clear old data.
            this._invokeIFrame('deleteModelByUniqueId', new_root);
            s.setModelRoot(newRoot);
        };
        // set loaded callback

        // begin to load
        this._invokeIFrame('load', modelInfo);
    }

}

export default ArtHubScene;