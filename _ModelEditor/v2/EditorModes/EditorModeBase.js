//Author: saralu
//Date: 2019-5-27
//EditorModeBase, the base class of editor mode

SceneEditor.EditorModeBase = function()
{
    // this.canvas = document.getElementById('viewport-canvas');
    this.canvas = SceneEditor.Editor.Instance.canvas;
}
SceneEditor.EditorModeBase.prototype.constructor = SceneEditor.EditorModeBase;


SceneEditor.EditorModeBase.prototype.getMode = function()
{
    return this.mode;
}

SceneEditor.EditorModeBase.prototype.mousedown = function()
{
    
}

SceneEditor.EditorModeBase.prototype.mousemove = function()
{

}

SceneEditor.EditorModeBase.prototype.mouseup = function()
{

}

SceneEditor.EditorModeBase.prototype.mousewheel = function()
{

}

SceneEditor.EditorModeBase.prototype.click = function()
{

}

SceneEditor.EditorModeBase.prototype.dbclick = function()
{
    
}

SceneEditor.EditorModeBase.prototype.keydown = function()
{

}

SceneEditor.EditorModeBase.prototype.keyup = function()
{

}

SceneEditor.EditorModeBase.prototype.blur = function()
{

}

SceneEditor.EditorModeBase.prototype.update = function()
{

}