import ArtHubBase from './ArtHubBase.js';

class ArtHubKeyframeAnimation extends ArtHubBase{
    constructor(options) {
        super(options);
    }

    get id() {
        return this._name;
    }
}

export default ArtHubKeyframeAnimation;