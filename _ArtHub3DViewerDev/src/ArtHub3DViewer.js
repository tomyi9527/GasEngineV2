
import ArtHubScene from './ArtHubScene.js';
import ArtHubRenderer from './ArtHubRenderer.js';
import ArtHubBase from './ArtHubBase.js';

const MessageType = {
    Inited: 'inited',
    Loaded: "loaded",
    MaterialLoaded: 'materialLoaded'
};

window.invokeID = 1;
class ArtHub3DViewer extends ArtHubBase {
    constructor(options) {
        super();
        const {container, baseUrl} = options;
        this._container = container;
        this._baseUrl = baseUrl || this._getBaseURL();

    }

    _getIframeSrc() {
        let baseURL = this._baseUrl;
        return `//${baseURL}/_ArtHub3DViewer/arthub-3d-viewer-iframe.html`;
    }

    _getBaseURL() {
        let pathName = window.location.pathname;
        if (pathName.startsWith('/')) {
            pathName = pathName.slice(1);
        }
        if (pathName.endsWith('/')) {
            pathName = pathName.slice(0, pathName.length - 1);
        }
        const pathArr = pathName ? pathName.split('/') : [];
        const depotName = pathArr.length >= 0 ? pathArr[0] : '';
        let baseURL = depotName ? `${window.location.host}/${depotName}/gas` : `${window.location.host}`;   
        return baseURL;
    }

    _createIframe() {
        const iframe = document.createElement('iframe');
        iframe.id = 'arthub-3d-viewer';
        iframe.style.borderWidth = '0px';
        iframe.style.width = '100%';
        iframe.style.height = '100%';
        return iframe;
    }

    // apis
    init() {
        this._iframe = this._createIframe();
        if (this._container) {
            this._container.appendChild(this._iframe);
        }
        this.addEventHandler();
    }

    destroy() {
        if (this._container && this._iframe) {
            this._container.removeChild(this._iframe);
            this._iframe = null;
            this._container = null;

            this.removeEventHandler();
        }
    }
    
    initEngine(isSaveEnabled = false) {
        return new Promise((resolve, reject) => {

            this._iframe.src = this._getIframeSrc();
    
            this.addCallback(MessageType.Inited, (success) => {
                this.removeCallback(MessageType.Inited);
                if (success) {
                    this._renderer = new ArtHubRenderer({iframe: this._iframe});
                    if(isSaveEnabled) {
                        this.initIncrementalSaver();
                    }
                    resolve(true);
                } else {
                    reject(false);
                }
            });

            // this.addCallback(MessageType.MaterialLoaded, (matId) => {
            //     console.log(`material id: ${matId} loaded now.`);
            // });
        })
    }

    destroyEngine() {
        return this._invokeIFrame('destroyEngine');
    }

    createScene() {
        return this._invokeIFrame('createScene')
            .then(() => {
                this._scene = new ArtHubScene({iframe: this._iframe});
                return this._scene;
            });
    }

    //TODO: iframe传scene.id
    destroyScene(scene) {
        return this._invokeIFrame('destroyScene');
    }

    getScene() {
        return Promise.resolve(this._scene);
    }

    getRenderer() {
        return Promise.resolve(this._renderer);
    }

    // shot and gifCapture
    shot(options) {
        return this._invokeIFrame('shot', options);
    }

    captureGifStart() {
        return this._invokeIFrame('captureGifStart');
    }

    captureGifStop() {
        return this._invokeIFrame('captureGifStop');
    }

    setAnimationLoopMode(options) {
        return this._invokeIFrame('setAnimationLoopMode', options);
    }

    setSpeedFactor(options) {
        return this._invokeIFrame('setSpeedFactor', options);
    }

    setClipClamp(options) {
        return this._invokeIFrame('setClipClamp', options);
    }

    getAnimationClips() {
        return this._invokeIFrame('getAnimationClips');
    }

    enableAnimation(options) {
        return this._invokeIFrame('enableAnimation', options);
    }

    setClipIndex(options) {
        return this._invokeIFrame('setClipIndex', options);
    }

    setClipProgress(options) {
        return this._invokeIFrame('setClipProgress', options);
    }

    enableHotspotAdd(options) {
        return this._invokeIFrame('enableHotspotAdd', options);
    }

    deleteHotspot(options) {
        return this._invokeIFrame('deleteHotspot', options);
    }

    getHotspots() {
        return this._invokeIFrame('getHotspots');
    }

    getAssembleConfigurations() {
        return this._invokeIFrame('getAssembleConfigurations');
    }

    initIncrementalSaver() {
        return this._invokeIFrame('initIncrementalSaver');
    }

    saverApplyChange() {
        return this._invokeIFrame('saverApplyChange')
            .then(res => {
                if(res.state) {
                    return true;
                } else {
                    throw new Error(res.message);
                }
            })
    }


}

if(window.ArtHubWebSDK) {
    window.ArtHubWebSDK.ModelViewer = ArtHub3DViewer;
} else {
    window.ArtHubWebSDK = {
        ModelViewer: ArtHub3DViewer
    };
}
export default ArtHub3DViewer;