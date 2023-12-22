GASEngine.FileSystem = function()
{
    GASEngine.Events.attach(this);

    this.loadRequest = new Map();
    this.cache = new Map();
    this.cacheData = new Map();

    this.supportedFileType = {
        'bin': 'arraybuffer',
        'json': 'json',
        'convertedfiles': 'json',
        'png': 'arraybuffer',
        'jpg': 'arraybuffer',
        'jpeg': 'arraybuffer',
        'tga': 'arraybuffer',
        'dds': 'arraybuffer',
        'pvr': 'arraybuffer',
        'etc': 'arraybuffer',
        'blob': 'arraybuffer',
        'glsl': 'text',
        "gltf": 'json',
        "glb": "arraybuffer",
        "fbx": "arraybuffer"
    };

    this.rootDirectory = '';
    this.systemDirectory = '';
    this.projectDirectory = '';
    this.textureDirectory = '';

    GASEngine.FileSystem.Instance = this;
};

GASEngine.FileSystem.MSG_FILE_READ_START = 'MSG_FILE_READ_START';
GASEngine.FileSystem.MSG_FILE_READ_SUCCESS = 'MSG_FILE_READ_SUCCESS';
GASEngine.FileSystem.MSG_FILE_READ_PROGRESS = 'MSG_FILE_READ_PROGRESS';
GASEngine.FileSystem.MSG_FILE_READ_ABORT = 'MSG_FILE_READ_ABORT';
GASEngine.FileSystem.MSG_FILE_READ_FAILED = 'MSG_FILE_READ_FAILED';
GASEngine.FileSystem.MSG_FILE_READ_END = 'MSG_FILE_READ_END';

GASEngine.FileSystem.extractFileExt = function(path)
{
    var ext = '';
    var tokenIndex = path.indexOf('?');
    var realModelName = '';
    if(tokenIndex >= 0)
    {
        realModelName = path.substr(0, tokenIndex);
    }
    else
    {
        realModelName = path;
    }

    var index = realModelName.lastIndexOf('.');
    if(index >= 0)
    {
        ext = realModelName.substr(index + 1).toLowerCase();
    }
    if(ext === 'gz')
    {
        realExts = realModelName.split('.');
        if(realExts.length > 2) {
            ext = realExts[realExts.length - 2];
        }
    }
    return ext;
};

GASEngine.FileSystem.isImageExt = function(ext)
{
    return (ext === 'jpg' || ext === 'jpeg' || ext === 'png' || ext === 'blob');
};

GASEngine.FileSystem.prototype =
{
    constructor: GASEngine.FileSystem,

    init: function(rootDirectory, systemDirectory, projectDirectory, textureDirectory)
    {
        this.rootDirectory = rootDirectory;
        this.systemDirectory = systemDirectory;
        this.projectDirectory = projectDirectory;
        this.textureDirectory = textureDirectory;
    },

    finl: function()
    {
        this.rootDirectory = '';
        this.systemDirectory = '';
        this.projectDirectory = '';
        this.textureDirectory = '';

        this.loadRequest = null;
        this.cache = null;
        this.cacheData = null;
    },

    setSystemDirectory: function(path)
    {
        this.systemDirectory = path;    
    },

    setProjectDirectory: function(path, texturePath = '')
    {
        this.projectDirectory = path;
        this.textureDirectory = texturePath;
    },

    setURLModifier: function(transform)
    {
        this.urlModifier = transform;
        return this;
    },   

    _translatePath: (function(path)
    {
        var root = '/';
        var system = '/system';
        var home = '$';
        var baseUrl = '//';
        var texture = '/texture';

        return function(path)
        {
            if(this.urlModifier) {
                return this.urlModifier(path);
            }

            var index = path.indexOf(system);
            var textureIndex = path.indexOf(texture);
            if(index === 0)
            {
                return this.systemDirectory + path.substr(system.length + 1);
            }
            else if(textureIndex === 0)
            {
                path = path.replace('/texture/', '');
                return  this.textureDirectory + path;
            }
            else
            {
                index = path.indexOf(baseUrl);
                if(index === 0)
                {
                    return path;
                }
                else
                {
                    index = path.indexOf(root);
                    if(index === 0)
                    {
                        return this.rootDirectory + path.substr(root.length);
                    }
                    else
                    {
                        return this.projectDirectory + path;
                    }
                }
            }
        }
    })(),

    writeOT: function(filePath, propertyPath, value)
    {

    },

    write: function(filePath, data, overwrite)
    {

    },

    emitMessage: function(msgName, filePath)
    {
        this.emit(msgName, filePath);
    },

    read: function(path, onSuccess, onProgress, onError)
    {
        'use strict';

        var ext;
        var _url;
        // Check for data: URI
        var dataUriRegex = /^data:(.*?)(;base64)?,(.*)$/;
        var dataUriRegexResult = path.match(dataUriRegex);

        if (dataUriRegexResult)
        {   //data: URI
            var mimeType = dataUriRegexResult[1];
            if(mimeType === "application/octet-stream") {
                var isBase64 = !!dataUriRegexResult[2];
                var data = dataUriRegexResult[3];

                data = GASEngine.Utilities.engineDecodeURIComponent(data);

                if (isBase64) data = window.atob(data);

                try
                {
                    var view = new Uint8Array(data.length);
                    for (var i = 0; i < data.length; i++)
                    {
                        view[i] = data.charCodeAt(i);
                    }
                    this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, path);
                }
                catch (error)
                {
                    console.error('GASEngine.FileSystem.read: failed to read data uri: ' + error.message);
                    onError(null, ext);
                    return;
                }
                onSuccess(view.buffer, ext);
                return;
            }

            if(mimeType === 'image/png')
                ext = 'png';
            else if (mimeType === 'image/jpeg')
                ext = 'jpeg';
            _url = path;
        }
        else {
            if(/^blob:.*$/i.test(path)) {
                ext = 'blob';
                _url = path;
            }
            else {
                ext = GASEngine.FileSystem.extractFileExt(path);
                _url = this._translatePath(path);
                if(/^blob:.*$/i.test(_url) && GASEngine.FileSystem.isImageExt(ext)) {
                    ext = 'blob';
                }
            }
        }

        var type = this.supportedFileType[ext];
        if(type === undefined)
        {
            console.error('GASEngine.FileSystem.read: \"' + ext + '\" file is not supported.');
            return null;
        }

        var hitCount = this.cache.get(path);
        if(hitCount !== undefined)
        {
            this.cache.set(path, hitCount + 1);

            var existData = this.cacheData.get(path);
            if (existData)
            {
                onSuccess(existData.data, existData.ext);
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, path);
            }
            return;
        }

        var record = this.loadRequest.get(_url);
        if(record !== undefined)
        {
            record.successCallbacks.push(onSuccess);
            record.progressCallbacks.push(onProgress);
            record.errorCallbacks.push(onError);
            return;
        }

        this.loadRequest.set(_url,
            {
                'successCallbacks': [onSuccess],
                'progressCallbacks': [onProgress],
                'errorCallbacks': [onError]
            });

        if(!GASEngine.FileSystem.isImageExt(ext))
        {
            var _xhr = new XMLHttpRequest();

            _xhr.addEventListener("error", function(event)
            {
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_FAILED, path);

                var record = this.loadRequest.get(_url);
                for(var i = 0; i < record.errorCallbacks.length; i++)
                {
                    if(record.errorCallbacks[i])
                    {
                        record.errorCallbacks[i](path, ext);
                    }
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _xhr.addEventListener("abort", function(event)
            {
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_ABORT, path);

                var record = this.loadRequest.get(_url);
                for(var i = 0; i < record.errorCallbacks.length; i++)
                {
                    if(record.errorCallbacks[i])
                    {
                        record.errorCallbacks[i](path, ext);
                    }
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _xhr.addEventListener("timeout", function(event)
            {
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_FAILED, path);

                var record = this.loadRequest.get(_url);
                for(var i = 0; i < record.errorCallbacks.length; i++)
                {
                    if(record.errorCallbacks[i])
                    {
                        record.errorCallbacks[i](path, ext);
                    }
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _xhr.addEventListener("readystatechange", function(event)
            {
            }.bind(this), false);

            _xhr.addEventListener("loadstart", function(event)
            {
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_START, path);

            }.bind(this), false);

            _xhr.addEventListener("progress", function(event)
            {
                var ajaxObject = event.currentTarget;
                if(ajaxObject.readyState == XMLHttpRequest.LOADING && ajaxObject.status == 200)
                {
                    this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_PROGRESS, path, event.loaded, event.total);
                }

            }.bind(this), false);

            _xhr.addEventListener("loadend", function(event)
            {
                var ajaxObject = event.currentTarget;
                if(ajaxObject.readyState == XMLHttpRequest.DONE && ajaxObject.status == 404)
                {
                    this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_FAILED, path);

                    var record = this.loadRequest.get(_url);
                    for(var i = 0; i < record.errorCallbacks.length; i++)
                    {
                        if(record.errorCallbacks[i])
                        {
                            record.errorCallbacks[i](path, ext);
                        }
                    }
                    //<
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _xhr.addEventListener("load", function(event)
            {
                var ajaxObject = event.currentTarget;
                if(ajaxObject.readyState == XMLHttpRequest.DONE && ajaxObject.status == 200)
                {
                    if(ajaxObject.response)
                    {
                        this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, path);

                        this.cache.set(path, 1);
                        this.cacheData.set(path, {data: ajaxObject.response, ext: ext});

                        var record = this.loadRequest.get(_url);
                        for(var i = 0; i < record.successCallbacks.length; i++)
                        {
                            if(record.successCallbacks[i])
                            {
                                record.successCallbacks[i](ajaxObject.response, ext);
                            }
                        }

                        this.loadRequest.delete(_url);
                    }
                }
            }.bind(this), false);

            _xhr.open("GET", _url, true);
            _xhr.responseType = type;
            _xhr.send(null);
        }
        else
        {
            var _image = document.createElement('img');

            _image.addEventListener('load', function(event)
            {
                var image = event.currentTarget;
                
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_SUCCESS, path);

                //this.cache.set(path, ajaxObject.response);
                this.cache.set(path, 1);
                this.cacheData.set(path, {data: image, ext: ext});

                if(ext === 'blob') { //free blob
                    URL.revokeObjectURL(_url);
                }

                var record = this.loadRequest.get(_url);
                for(var i = 0; i < record.successCallbacks.length; i++)
                {
                    if(record.successCallbacks[i])
                    {
                        record.successCallbacks[i](image, ext);
                    }
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _image.addEventListener('error', function(event) //404
            {
                this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_FAILED, path);

                var record = this.loadRequest.get(_url);
                for(var i = 0; i < record.errorCallbacks.length; i++)
                {
                    if(record.errorCallbacks[i])
                    {
                        record.errorCallbacks[i](path, ext);
                    }
                }

                this.loadRequest.delete(_url);

            }.bind(this), false);

            _image.crossOrigin = '';
            _image.src = _url;

            this.emitMessage(GASEngine.FileSystem.MSG_FILE_READ_START, path);
        }
    }
};