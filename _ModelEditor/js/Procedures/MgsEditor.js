(function (global)
{
    /*************************************** MgsEditor ***************************************/
    function MgsEditor() 
    {
        mgs.Events.call(this);

        this._hooks = { };
    }
    mgs.classInherit(MgsEditor, mgs.Events);

    MgsEditor.prototype.method = function(name, fn) {
        if (this._hooks[name] !== undefined) {
            throw new Error('can\'t override hook: ' + name);
        }
        this._hooks[name] = fn;
    };


    MgsEditor.prototype.methodRemove = function(name) {
        delete this._hooks[name];
    };


    MgsEditor.prototype.call = function(name) {
        if (this._hooks[name]) {
            var args = Array.prototype.slice.call(arguments, 1);

            try {
                return this._hooks[name].apply(null, args);
            } catch(ex) {
                console.info('%c%s %c(editor.method error)', 'color: #06f', name, 'color: #f00');
                console.log(ex.stack);
            }
        }
        return null;
    };

    MgsEditor.prototype.assign = function(name, value)
    {
        if (this[name])
        {
            mgs.Log.error('mgs editor namespace props name:' + '"' + name + '"' + ' is eixsted, try assign other name!');
        }
    
        this[name] = value;
    }

    /*************************************** editor instance ***************************************/
    global.mgsEditor = new mgs.MgsEditor();

}(this));