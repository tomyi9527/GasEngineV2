(function()
{
    let Editor = function()
    {
        mgs.Events.call(this);
        this._hooks = {};
    };
    mgs.classInherit(Editor, mgs.Events);

    
    Editor.prototype.method = function(name, fn) 
    {
        if (this._hooks[name] !== undefined) 
        {
            throw new Error('can\'t override hook: ' + name);
        }
        this._hooks[name] = fn;
    };


    Editor.prototype.methodRemove = function(name) 
    {
        delete this._hooks[name];
    };


    Editor.prototype.call = function(name) 
    {
        if (this._hooks[name]) 
        {
            var args = Array.prototype.slice.call(arguments, 1);

            try 
            {
                return this._hooks[name].apply(null, args);
            } 
            catch(ex) 
            {
                console.info('%c%s %c(editor.method error)', 'color: #06f', name, 'color: #f00');
                console.log(ex.stack);
            }
        }
        return null;
    };

    let mousePos = new GASEngine.Vector2(0, 0);
    let onMouseMove = function(evt)
    {
        mousePos.set(evt.clientX, evt.clientY);
    };
    window.addEventListener('mousemove', onMouseMove, true);

    let editor = new Editor();
    editor.getMousePos = function()
    {
        return mousePos;
    };
    
    mgs.assign('editor', editor);
})();