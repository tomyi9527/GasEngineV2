//Author: saralu
//Date: 2019/5/28
//Delegate of Scene
SceneEditor.DScene = function(engineScene)
{
    // this._DRoot = null;
    // this._DCameras = [];
    // this._DCameraLastIndex = -1;
    // this._DLights = [];
    this.engineScene = engineScene;
    this.removePool = [];
    this.removeParentMap = {};
    this.createPool = [];
    this.createParentMap = {};
    this.init();
    return this;
}

SceneEditor.DScene.prototype.constructor = SceneEditor.DScene;

SceneEditor.DScene.prototype.init = function()
{
    this.root = this.engineScene.root;
    mgsEditor.emit(mgsEditor.EVENTS.onSceneLoaded, this.root); 
}

SceneEditor.DScene.prototype.getRoot = function()
{
    if(!this.engineScene) return undefined;
    return this.engineScene.root;
}
SceneEditor.DScene.prototype.getEntityByID = function(id)
{
    if(!this.engineScene) return false;
    var sceneRoot = this.engineScene.root;
    var entity = sceneRoot.findObjectByID_r(id);
    if(!entity)
    {
        entity = this.getEntityFromRemovePool(id);
    }
    if(!entity)
    {
        entity = this.getEntityFromCreatePool(id);
    }
    return entity;
}

SceneEditor.DScene.prototype.getEntityFromRemovePool = function(id)
{
    var entity;
    for(var i  = 0; i < this.removePool.length; i++)
    {
        entity = this.removePool[i];
        if(entity.uniqueID === id)
        {
            return entity;
        }
    }
    return null;
} 

SceneEditor.DScene.prototype.getEntityFromCreatePool = function(id)
{
    var entity;
    for(var i  = 0; i < this.createPool.length; i++)
    {
        entity = this.createPool[i];
        if(entity.uniqueID === id)
        {
            return entity;
        }
    }
    return null;
} 


SceneEditor.DScene.prototype.getParentOfEntity = function(id)
{
    var parent;
    var entity = this.getEntityByID(id);
    parent = entity.parent;
    if(!parent) 
    {
        parent = this.removeParentMap[id];
    }
    if(!parent)
    {
        parent = this.createParentMap[id];
    }
    return parent;
} 

SceneEditor.DScene.prototype.getNextEntity = function(child, parent)
{
    if(!child || !parent) return false;
    var idx = parent.findChild(child);
    var count = parent.getChildCount();
    var next = null, i = idx + 1, flag = true;
    while(flag)
    {
        next = parent.getChildAt(i);
        i++;
        if(!next || next.type !== 'helper' || i === count)
        {
            flag = false;
        }
        if(next && next.type === 'helper') next = null;
    }
    return next;
}

SceneEditor.DScene.prototype.getCameraCount = function()
{
    return (this._DCameraLastIndex + 1);
}

SceneEditor.DScene.prototype.getBoundingBox = function()
{
    return this._DRoot.bbox;
}

SceneEditor.DScene.prototype._addCamera = function(camera)
{
    ++this._DCameraLastIndex;
    if(this._DCameras[this._DCameraLastIndex] !== undefined)
    {
        this._DCameras[this._DCameraLastIndex] = camera;
    }
    else 
    {
        this._DCameras.push(camera);
    }
}

SceneEditor.DScene.prototype.removeEntity = function(entity)
{
    var parent = entity.parent;
    this.removePool.push(entity);
    this.removeParentMap[entity.uniqueID] = parent;
    parent.removeChild(entity);
    // mgsEditor.emit(mgsEditor.EVENTS.onAssetDeleted, entity);
}

SceneEditor.DScene.prototype.createEntity = function(type)
{
    var entity = null;
    switch(type)
    {
        case 'empty':
            entity = GASEngine.WebglCommon.Instance.createEmptyEntity();
            break;
        case 'cube':
            entity = GASEngine.WebglCommon.Instance.createCube();
            break;
        case 'sphere':
            entity = GASEngine.WebglCommon.Instance.createSphere();
            break;
        default:
            break;
    }
    if(!entity) return undefined;
    this.createPool.push(entity);
    this.createParentMap[entity.uniqueID] = this.engineScene.root;
    return entity.uniqueID;
}

SceneEditor.DScene.prototype.appendEntityOnParent = function(parent, child)
{
    child.setParent(parent, null, true);
};


SceneEditor.DScene.prototype.insertEntityBefore = function(first, next)
{
    var nextParent = next.parent;
    first.setParent(nextParent, next, true);
};



//add nodes and remove nodes
SceneEditor.DScene.prototype.update = function()
{

}


function displayObjectProrp_r(obj) 
{
    var datGUI = new dat.GUI();
    var propCount = obj.getPropertyCount();
    for (var i = 0; i < propCount; ++i)
    {
        var prop = obj.getProperty(i);
        var val = prop.get.call(obj);

        var guiObj = {};
        guiObj.prop = prop;

        var fold = datGUI.addFolder(prop.id);

        if (prop.type === 'vector3')
        {
            guiObj.x = val[0];
            guiObj.y = val[1];
            guiObj.z = val[2];

            function vector3Changed(value)
            {
                var x_ = this.object.x;
                var y_ = this.object.y;
                var z_ = this.object.z;
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, [x_, y_, z_]);
                }
            }

            fold.add(guiObj, 'x').onChange(vector3Changed);
            fold.add(guiObj, 'y').onChange(vector3Changed);
            fold.add(guiObj, 'z').onChange(vector3Changed);
        }
        if(prop.type === 'vector2')
        {
            guiObj.x = val[0];
            guiObj.y = val[1];

            function vector2Changed(value)
            {
                var x_ = this.object.x;
                var y_ = this.object.y;
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, [x_, y_]);
                }
            }

            fold.add(guiObj, 'x').onChange(vector2Changed);
            fold.add(guiObj, 'y').onChange(vector2Changed);
        }
        else if (prop.type === 'color')
        {
            var r = Math.round(val[0] * 255);
            var g = Math.round(val[1] * 255);
            var b = Math.round(val[2] * 255);
            guiObj.value = [r, g, b];

            function colorChanged(value)
            {
                var r = this.object.value[0] / 255.0;
                var g = this.object.value[1] / 255.0;
                var b = this.object.value[2] / 255.0;
                var color = [r, g, b];
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, color);
                }
            }

            fold.addColor(guiObj, 'value').onChange(colorChanged);
        }
        else if (prop.type === 'string')
        {
            if(val != undefined)
                guiObj.value = val;
            else
                guiObj.value = '';

            function stringChanged(value)
            {
                var x_ = this.object.value;
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, x_);
                }
            }

            fold.add(guiObj, 'value').onFinishChange(stringChanged);
        }
        else if(prop.type === 'enum')
        {
            guiObj.value = val;

            function numberChanged(value)
            {
                var x_ = parseInt(this.object.value);
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, x_);
                }
            }

            fold.add(guiObj, 'value', prop.range).onChange(numberChanged);
        }
        else if(prop.type === 'number')
        {
            if(val != undefined)
                guiObj.value = val;
            else
                guiObj.value = -1;

            function numberChanged(value)
            {
                var x_ = this.object.value;
                if(this.object.prop.set)
                {
                    this.object.prop.set.call(obj, x_);
                }
            }

            var range = [undefined, undefined];
            var step = undefined;
            if(prop.range)
            {
                range = prop.range;
            }

            if(prop.step)
            {
                step = prop.step;
            }

            fold.add(guiObj, 'value', range[0], range[1], step).onChange(numberChanged);
        }
        else if(prop.type === 'bool')
        {
            if(val === undefined)
            {
                var jjjj = 0;
            }

            guiObj.value = val;
            function numberChanged(value)
            {
                var x_ = this.object.value;
                this.object.prop.set.call(obj, x_);
            }
            fold.add(guiObj, 'value').onChange(numberChanged);
        }
        else if(prop.type === 'array')
        {
            guiObj.value = val;
            function numberChanged(value)
            {
                var x_ = this.object.value;
                this.object.prop.set.call(obj, x_);
            }
            //fold.add(guiObj, 'value').onChange(numberChanged);
        }
        else if(prop.type === 'object')
        {
            if(prop.subType === 'MapDelegate')
            {
                if(val)
                {
                    guiObj.value = val;
                    guiObj.Open = function()
                    {
                        if(this.value)
                        {
                            displayObjectProrp_r(this.value);
                        }
                    };
                    fold.add(guiObj, 'Open');
                }
                else
                {
                    guiObj.New = function()
                    {
                        var newMaterialMapDelegate = H5Editor_V0.globalEditorFramework.createMaterialMapDelegate();
                        this.prop.set.call(obj, newMaterialMapDelegate);
                        displayObjectProrp_r(newMaterialMapDelegate);
                        window.alert("MaterialMapDelegate created successfully!");
                    }
                    fold.add(guiObj, 'New');
                }
            }
            else if(prop.subType === 'TextureDelegate')
            {
                if(val)
                {
                    guiObj.value = val;
                    guiObj.Open = function()
                    {
                        if(this.value)
                        {
                            displayObjectProrp_r(this.value);
                        }
                    };
                    fold.add(guiObj, 'Open');

                    guiObj.Delete = (function(prop, obj)
                    {
                        return function()
                        {
                            prop.set.call(obj, null);
                        }
                    })(prop, obj);
                    fold.add(guiObj, 'Delete');
                }

                guiObj.New = function()
                {
                    var imageChanged = (function(prop, obj)
                    {
                        return function(image, theFile)
                        {
                            var newTextureDelegate = H5Editor_V0.globalEditorFramework.createTextureDelegate(image.fileName__, image.fileName__, image);
                            prop.set.call(obj, newTextureDelegate);
                            displayObjectProrp_r(newTextureDelegate);
                            window.alert("TextureDelegate created successfully!");
                        };
                    })(this.prop, obj);

                    var imageWindow = new H5UI_V0.ImageWindow("TEXTURE_VIEW_WINDOW", 0, 0, 500, 500, "选择纹理图片", "white", 1.0);
                    imageWindow.onImageChanged(imageChanged);

                    imageWindow.setImage(new Image(1, 1));

                    H5UI_V0.DesktopInstance.addChildWindow(imageWindow);

                    H5UI_V0.DesktopInstance.placeWindowAtDesktopCenter(imageWindow);
                    H5UI_V0.DesktopInstance.setActiveWindow(imageWindow);
                }
                fold.add(guiObj, 'New');
            }
            else if(prop.subType === 'MaterialDelegate')
            {
                if(val)
                {
                    guiObj.value = val;
                    guiObj.Open = function()
                    {
                        //if(guiObj.__Tom_uiPanel)
                        //{
                        //    this.__Tom_uiPanel.destroy();
                        //    this.__Tom_uiPanel = null;
                        //}

                        if(this.value)
                        {
                            displayObjectProrp_r(this.value);
                        }
                    };
                    fold.add(guiObj, 'Open');
                }
            }
            else if(prop.subType === 'MeshDelegate')
            {
                if(val)
                {
                    guiObj.value = val;
                    guiObj.Open = function()
                    {
                        //if(this.__Tom_uiPanel)
                        //{
                        //    this.__Tom_uiPanel.destroy();
                        //    this.__Tom_uiPanel = null;
                        //}

                        if(this.value)
                        {
                            displayObjectProrp_r(this.value);
                        }
                    };
                    fold.add(guiObj, 'Open');
                }                        
            }
            else
            {
                console.error('Unsupported object sub type. Cannot be created!');
            }                   
        }
        else if(prop.type === 'image')
        {
            guiObj.open = (function(prop, image)
            {
                return function()
                {
                    var imageChanged = function(image, imageFile)
                    {
                    };

                    var imageWindow = new H5UI_V0.ImageWindow("TEXTURE_VIEW_WINDOW", 0, 0, 500, 500, "选择纹理图片", "white", 1.0);
                    imageWindow.onImageChanged(imageChanged);

                    if(!image)
                    {
                        image = new Image(1, 1);
                    }

                    imageWindow.setImage(image);

                    H5UI_V0.DesktopInstance.addChildWindow(imageWindow);

                    H5UI_V0.DesktopInstance.placeWindowAtDesktopCenter(imageWindow);
                    H5UI_V0.DesktopInstance.setActiveWindow(imageWindow);
                }
            })(prop, val);

            fold.add(guiObj, 'open');
        }
        //fold.open();
    }

    H5Editor_V0.globalAllGUIObject.push(datGUI);
    return datGUI;
}

function initWorldHierarckyTreeView()
{
    H5Editor_V0.globalWorldHierarckyWindow = new H5UI_V0.TreeViewWindow("WORLD_HIERARCKY_WINDOW", 50, 50, 500, 500, "场景结构", "#888888", 1.0);
    H5Editor_V0.globalWorldHierarckyWindow.itemsSelectedChangedCallback = onHierarckyViewSelectedChanged;
    H5UI_V0.DesktopInstance.addChildWindow(H5Editor_V0.globalWorldHierarckyWindow);

    var worldRootDelegate = H5Editor_V0.globalEditorFramework.getWorldRoot();
    var propCount = worldRootDelegate.getPropertyCount();
    if(propCount >= 3)
    {
        var childrenProp = worldRootDelegate.getProperty(2); //children

        var childCount = childrenProp.getElementCount.call(worldRootDelegate);
        for(var i = 0; i < childCount; ++i)
        {
            var childDelegate = childrenProp.get.call(worldRootDelegate, i);
            appendWorldHierarckyTreeView_r(childDelegate, 0);
        }
    }

    H5Editor_V0.globalWorldHierarckyWindow.recalcLayout();
}

function appendWorldHierarckyTreeView_r(nodeDelegate, prarentItemID)
{
    var propCount = nodeDelegate.getPropertyCount();

    var nameProp = nodeDelegate.getProperty(0); //name
    var uuidProp = nodeDelegate.getProperty(1); //UUID
    var objectName = nameProp.get.call(nodeDelegate);
    var ObjectUuid = uuidProp.get.call(nodeDelegate);
    var itemID = H5Editor_V0.globalWorldHierarckyWindow.appendItem(prarentItemID, "A", objectName, nodeDelegate, H5UI_V0.TreeViewItem.TREE_ITEM_STATUS_OPEN);
    if(propCount >= 3)
    {
        var childrenProp = nodeDelegate.getProperty(2); //children
        if(childrenProp.id === 'Children')
        {
            var childCount = childrenProp.getElementCount.call(nodeDelegate);
            for(var i = 0; i < childCount; ++i)
            {
                var childDelegate = childrenProp.get.call(nodeDelegate, i);
                appendWorldHierarckyTreeView_r(childDelegate, itemID);
            }
        }
    }
} 

