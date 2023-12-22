
class ArtHubBase {
    constructor(options = {}) {
        this._iframe = options.iframe;
        this._name = options.name;
        this._id = options.id;
        this._parentEntityId = options.parentEntityId;
        
        this._callbacks = new Map();
    }

    get name() {
        return this._name;
    }

    get id() {
        return this._id;
    }

    get parentEntityId() {
        return this._parentEntityId;
    }

    _invokeIFrame(cmd, parameters) {
        if(this._iframe) {
            window.invokeID += 1;
            let invokeID = window.invokeID;

            this._iframe.contentWindow.postMessage({ invokeID, cmd, parameters }, '*');

            return new Promise((rs, rj) => {
                    const timeoutHandler = function () {
                        window.removeEventListener('message', messageHandler);
                        rj({ invokeID, cmd });
                    };
                    const timer = setTimeout(timeoutHandler, 200000);  // TODO(beanpliu): 后续找个方案控制较长的promise问题
        
                    const messageHandler = function (event) {
                        if (event.data.invokeID === invokeID && event.data.cmd === cmd) {
                            // console.log(event.data);
                            window.removeEventListener('message', messageHandler);
                            clearTimeout(timer);
                            rs(event.data.parameters);
                        }
                    };
                    window.addEventListener('message', messageHandler, false);
                });
        }
    }

    _callbackHandlerAdaptor(event) {
        if (event.data.invokeID === -1) {
            this._callbackIFrame(event.data.cmd, event.data.parameters);
        }
    }

    _callbackIFrame(cmd, parameters) {
        const func = this._callbacks.get(cmd);
        if (cmd && cmd.length > 0 && func !== undefined) {
            if(Array.isArray(func)) {
                func.forEach(f => f(parameters));
            } else {
                func(parameters);
            }
        }
    }
    
    addCallback(callbackName, callbackFunction) {
        if (this._callbacks && callbackName && callbackFunction) {
            if(this._callbacks.has(callbackName)) {
                const existCallBackFunction = this._callbacks.get(callbackName);
                if(Array.isArray(existCallBackFunction)) {
                    existCallBackFunction.push(callbackFunction);
                } else {
                    this._callbacks.set(callbackName, [existCallBackFunction, callbackFunction]);
                }
            } else {
                this._callbacks.set(callbackName, callbackFunction);
            }
        }
    }

    removeCallback(callbackName) {
        if (this._callbacks && callbackName) {
          this._callbacks.delete(callbackName);
        }
    }

    addEventHandler() {
        window.addEventListener(
            'message',
            this._callbackHandlerAdaptor.bind(this),
            false
        );
    }

    removeEventHandler() {
        window.removeEventListener(
            'message',
            this._callbackHandlerAdaptor
        );

        this._callbacks.clear();
    }
    
}

export default ArtHubBase;