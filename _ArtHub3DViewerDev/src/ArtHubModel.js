import ArtHubMesh from './ArtHubMesh.js';
import ArtHubMaterial from './ArtHubMaterial.js';
import ArtHubKeyframeAnimation from './ArtHubKeyframeAnimation.js';
import ArtHubBase from './ArtHubBase.js';

class ArtHubModel extends ArtHubBase{
    constructor(options) {
        super(options);

        this._meshList = [];
        this._materialList = [];
        this._animationClips = [];
    }

    getModelStructure() {
        return this._invokeIFrame('getModelStructure', this.id)
            .then(modelInfo => {

                const meshList = modelInfo.mesh;
                this._meshList = meshList.map(e => new ArtHubMesh({iframe: this._iframe, ...e}));
                const materials = modelInfo.material;
                this._materialList = materials.map(e => new ArtHubMaterial({iframe: this._iframe, ...e}));
                const clips = modelInfo.animationClips;
                this._animationClips = clips.map(e => new ArtHubKeyframeAnimation({iframe: this._iframe, ...e}));

                return Promise.all(this._materialList.map(v=>v.init())).then(()=>{
                    return {
                        name: this.name,
                        id: this.id,
                        ...modelInfo
                    }
                });
            });
    }

    getMeshById(id) {
        let index = this._meshList.findIndex(e => e.id === id);
        const mesh = index !== -1 ? this._meshList[index] : null;
        return Promise.resolve(mesh);
    }

    getMaterialById(id) {
        let index = this._materialList.findIndex(e => e.id === id);
        const material = index !== -1 ? this._materialList[index] : null;
        return Promise.resolve(material);
    }

    getMaterialByIndex(index) {
        const material = index >= 0 && index < this._materialList.length ? this._materialList[index] : null;
        return material;
    }

    getAnimationClipById(id) {
        let index = this._animationClips.findIndex(e => e.id === id);
        const clip = index !== -1 ? this._animationClips[index] : null;
        return Promise.resolve(clip);
    }

    getMeshList() {
        return Promise.resolve(this._meshList);
    }

    getMaterialList() {
        return Promise.resolve(this._materialList);
    }

    getAnimationClips() {
        return Promise.resolve(this._animationClips);
    }

    deleteAnimationClipById(clipId) {
        return this._invokeIFrame('deleteAnimationClipById', clipId)
            .then(() => {
                let index = this._animationClips.findIndex(e => e.id === clipId);
                if(index !== -1) {
                    this._animationClips.splice(index, 1);
                }
                return true;
            })
    }

    getTextureLinkByPath(path) {
        var promises = this._materialList.map(material=>material.getTextureLinkByPath(path));
        return Promise.all(promises).then(values=>{
            var ret = [];
            values.forEach(value=>{ret = ret.concat(value);});
            return ret;
        });
    }

    // load newModel
    // animation append to currentModel
    // delete newModel
    // params: loadParams
    // WIP: 还未完成请不要用它
    addAnimationClips(modelInfo) {
        // get current state.
        var s = this._model.getScene();
        var root = s.getModelRoot();
        var animatorComponent = root.getComponent('animator');
        var onAnimationLoaded = function() {
            // new data loaded.
            var newRoot = s.getModelRoot();
            // move clips to old root.
            var newAnimatorComponent = newRoot.getComponent('animator');
            // new clips is now empty, and it's ok to destroy
            var clips = newAnimatorComponent.takeoutClips();
            clips.forEach(clip=>animatorComponent.setAnimationClip(clip.getName(), clip));
            // set root to old root and clear new data.
            this._invokeIFrame('deleteModelByUniqueId', new_root);
            s.setModelRoot(root);
        };
        // set loaded callback
        this.addCallback(MessageType.FileLoaded, (param) => {
            if (param.count == param.total)
                onAnimationLoaded();
        });
        // begin to load
        this._invokeIFrame('load', modelInfo);
    }
}

export default ArtHubModel;