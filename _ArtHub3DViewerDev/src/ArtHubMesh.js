import ArtHubBase from './ArtHubBase.js';

class ArtHubMesh extends ArtHubBase{
    constructor(options) {
        super(options);
    }

    hide() {
        return this._invokeIFrame('showMesh', {id: this.parentEntityId, visible: false});
    }

    show() {
        return this._invokeIFrame('showMesh', {id: this.parentEntityId, visible: true});
    }
    getUVData() {
        return this._invokeIFrame('getMeshUVData', this.id)
            .then(uvData => {
                return uvData;
            })
    }
}

export default ArtHubMesh;
