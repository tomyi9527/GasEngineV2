(function()
{
    let AssetManager = function()
    {
        mgs.Object.call(this);

        this._init('', '/system/', '');
    };
    mgs.classInherit(AssetManager, mgs.Object);

    AssetManager.prototype.onDestroy = function()
    {
        this._fileSystem_.Instance.finl();
    };

    AssetManager.prototype._init = function(rootDirectory, systemDirectory, projectDirectory)
    {
        this._fileSystem_ = new GASEngine.FileSystem();
        this._fileSystem_.init(rootDirectory, systemDirectory, projectDirectory);
    };

    AssetManager.prototype.getFileSystem = function()
    {
        return this._fileSystem_;
    };
}());