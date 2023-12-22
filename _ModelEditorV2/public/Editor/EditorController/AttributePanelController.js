(function ()
{
    let AttributePanelController = function (options) 
    {
        mgs.ControllerBase.call(this, options);

        // --------- process cmd --------- //
        this._currentDelegateObject_ = null;

        let self = this;
        this._handler = function(cmd)
        {
            // find delegate
            let managerID = cmd.managerID;
            let delegateObjectManager = mgs.editor.delegateManager.getDelegateObjectManager(managerID);
            if (!delegateObjectManager)
            {
                if (self._currentDelegateObject_)
                {
                    self.clearContent();
                }
                
                return;
            }

            switch(cmd.type)
            {
                case 'clear':
                {
                    self.clearContent();
                }
                break;
                case 'select':
                {
                    self.clearContent();
                    
                    let selectIDs = cmd.param.selectIDs;
                    if (selectIDs.length === 1)
                    {
                        // todo: 暂时支持单Object属性select
                        let delegateObject = delegateObjectManager.get(selectIDs[0]);
                        self._currentDelegateObject_ = delegateObject;
    
                        self.showDelegateObject(self._currentDelegateObject_);
                    }
                }
                break;
                case 'modify':
                {
                    let objectID = cmd.objectID;
                    let delegateObject = delegateObjectManager.get(objectID);
                    if (delegateObject === self._currentDelegateObject_)
                    {
                        let id = self.fullPathToID(cmd.param.fullPath);
                        let value = cmd.param.value;
                        self.onSetValue(id, value, cmd.param.fullPath);
                    }
                }
                break;
            }
        };
        mgs.editor.commandManager.on('onCommand', this._handler);

        // --------- attributes map --------- //
        this._attributeGridMap_ = new Map();
    };
    mgs.classInherit(AttributePanelController, mgs.ControllerBase);

    // --------- inherit --------- //
    AttributePanelController.prototype.onDestroy = function()
    {
        mgs.editor.commandManager.unbind('onCommand', this._handler);
    };

    AttributePanelController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UIPanel(viewOptions);
        return view;
    };

    // --------- attributes ops --------- //
    AttributePanelController.prototype.clearContent = function()
    {
        let self = this;
        this._attributeGridMap_.forEach(function(value, key, map)
        {
            let grid = value;
            grid.unbind('onChange', self.onChange.bind(self));
        });
        this._attributeGridMap_.clear();
        let view = this.getView();
        view.contentClear();

        self._currentDelegateObject_ = null;
    };

    AttributePanelController.prototype.onChange = function(id, value, ignoreHistory, oldValue)
    {
        // send cmd
        let grid = this._attributeGridMap_.get(id);
        if (grid && this._currentDelegateObject_)
        {
            let managerID = this._currentDelegateObject_.getManagerID();
            let objectID = this._currentDelegateObject_.getID();
            let fullPath = grid.getFullPath();
            mgs.editor.delegateManager.sendModifyCommand(managerID, objectID, fullPath, value, oldValue, ignoreHistory);
        }
    };

    AttributePanelController.prototype.isHideValue = function(value, editParam)
    {
        if (editParam.hideCondition)
        {
            if (editParam.hideCondition.values)
            {
                return mgs.Util.arrayExist(editParam.hideCondition.values, value);
            }
        }

        return false;
    };

    let panelPath = [];
    AttributePanelController.prototype.onSetValue = function(id, value, fullPath)
    {
        let grid = this._attributeGridMap_.get(id);

        // check remove/add grid
        let property = this._currentDelegateObject_.getPropertyByFullPath(fullPath);
        let editParam = property.getEditorParam();
        if (editParam && editParam.hideCondition)
        {
            // get panel
            mgs.Util.arrayCopy(fullPath, panelPath);
            panelPath.pop();

            let panel = null;
            if (panelPath.length <= 0)
            {
                panel = this.getView();
            }
            else
            {
                let panelID = this.fullPathToID(panelPath);
                panel = this._attributeGridMap_.get(panelID);
            }

            if (this.isHideValue(value, editParam))
            {
                if (grid)
                {
                    // remove grid
                    if (panel)
                    {
                        this._attributeGridMap_.delete(id);
                        panel.content.remove(grid);
                        grid.unbind('onChange', this.onChange.bind(this));
                        grid = null;
                    }
                }
            }
            else
            {
                if (!grid)
                {
                    // add grid
                    if (panel)
                    {
                        this.showDelegateProperty(property, panel.content, true);
                    }  
                }
            }
        }

        // set value
        if (grid)
        {
            grid.setValue(value);
        }
    };

    AttributePanelController.prototype.fullPathToID = function(fullPath)
    {
        let id = '';
        for (let i = 0;i < fullPath.length;i ++)
        {
            id += '.' + fullPath[i];
        }
        return id
    };

    AttributePanelController.prototype.getPreGrid = function(targetProperty)
    {
        let delegateObject = targetProperty.getDelegateObject();
        let propertyList = delegateObject.getPropertyList();
        let id = null;
        for (let i = 0;i < propertyList.length;i ++)
        {
            let property = propertyList[i];
            if (targetProperty === property)
            {
                break;
            }

            id = this.fullPathToID(property.getFullPath());
        }

        let grid = this._attributeGridMap_.get(id);
        return grid;
    };

    AttributePanelController.prototype.showDelegateProperty = function(property, content, isInsert)
    {
        let editParam = property.getEditorParam();
        if (!editParam)
        {
            return;
        }

        // checkHide
        if (this.isHideValue(property.getValue(), editParam))
        {
            return;
        }

        let gridOption = editParam.gridOption;
        let grid = null;
        switch(editParam.type)
        {
            case 'Label':
            {
                grid = new mgs.UIAttributeGrid_Label(gridOption);
            }
            break;
            case 'Input':
            {
                grid = new mgs.UIAttributeGrid_TextField(gridOption);
            }
            break;
            case 'Slider':
            {
                grid = new mgs.UIAttributeGrid_Slider(gridOption);
            }
            break;
            case 'Vector3':
            {
                grid = new mgs.UIAttributeGrid_Vector3(gridOption);
            }
            break;
            case 'DegreeVector':
            {
                grid = new mgs.UIAttributeGrid_DegreeVector(gridOption);
            }
            break;
            case 'Panel':
            {
                grid = new mgs.UIAttributeGrid_Panel(gridOption);
                let nestDelegateObject = property.getNestDelegateObject();
                if (nestDelegateObject)
                {
                    this.showDelegateObject(nestDelegateObject, grid.content);
                }
            }
            break;
            case 'Array':
            {
                grid = new mgs.UIAttributeGrid_Array(gridOption);
                // let nestDelegateObject = property.getNestDelegateObject(); // array delegate object
                // let elementDelegateObject = property.getElementProperty();
                // if (elementDelegateObject)
                // {
                //     this.showDelegateObject(nestDelegateObject, grid.content);
                // }
            }
            break;
        }

        if (grid)
        {
            let value = property.getValue();
            let fullPath = property.getFullPath();
            let id = this.fullPathToID(fullPath);

            grid.setID(id);
            grid.setTitle(property.getPath());
            grid.setValue(value);
            grid.setFullPath(fullPath);
            grid.setNeedUpdate(editParam.needUpdate);

            if (isInsert)
            {
                let preGird = this.getPreGrid(property);
                content.insertAfter(grid, preGird);
            }
            else
            {
                content.append(grid);
            }
            
            grid.on('onChange', this.onChange.bind(this));
            
            this._attributeGridMap_.set(id, grid);
        }
    };

    AttributePanelController.prototype.showDelegateObject = function(delegateObject, content)
    {
        if (!content)
        {
            let view = this.getView();
            content = view.content;
        }

        let propertyList = delegateObject.getPropertyList();
        for (let i = 0;i < propertyList.length;i ++)
        {
            let property = propertyList[i];
            this.showDelegateProperty(property, content);
        }
    };

    AttributePanelController.prototype.update = function(delta)
    {
        if (this._currentDelegateObject_)
        {
            let self = this;
            this._attributeGridMap_.forEach(function(value, key, map)
            {
                let grid = value;
                if (grid.getNeedUpdate())
                {
                    let property = self._currentDelegateObject_.getPropertyByFullPath(grid.getFullPath());
                    grid.setValue(property.getValue());
                }
            });
        }
    };
}());