
ArtHubFileSystem = function()
{
    GASEngine.FileSystem.call(this);
    this._fileMap = new Map();
}
ArtHubFileSystem.prototype = Object.create(GASEngine.FileSystem.prototype);
ArtHubFileSystem.prototype.constructor = ArtHubFileSystem;

//should be overwrite
ArtHubFileSystem.prototype.getFullPath = function(filePath)
{
    return Promise.resolve(filePath);
}

ArtHubFileSystem.prototype.setFullPath = function(fullPath, path)
{
    this._fileMap.set(fullPath, path);
}

ArtHubFileSystem.prototype.read = function(path, onSuccess, onProgress, onError)
{
    return this.getFullPath(path)
        .then(fullPath => {
            this._fileMap.set(fullPath, path);
            GASEngine.FileSystem.prototype.read.call(this, fullPath, onSuccess, onProgress, onError);
        })
        .catch(error => {
            console.log('fileSystem read error: ', error);
        })
}

ArtHubFileSystem.prototype.emitMessage = function(msgName, filePath, ...args)
{
    if(this._fileMap.has(filePath)) {
        const fileName = this._fileMap.get(filePath);
        this.emit(msgName, fileName, ...args);
    } else {
        this.emit(msgName, filePath, ...args);
    }
}

ArtHubFileSystem.prototype.getFullPathByPath = function(path)
{
    for(var key of this._fileMap.keys()) {
        var value = this._fileMap.get(key);
        if(value === path) {
            return key;
        }
    }
    return path;
}