(function ()
{
    let DelegateManager = function()
    {
        mgs.Object.call(this);

        this._delegateObjectContainerMap_ = new Map();

        let self = this;
        this._cmdHander = function(cmd)
        {
            let managerID = cmd.managerID;
            let delegateObjectManager = self.getDelegateObjectManager(managerID);
            if (!delegateObjectManager)
            {
                return;
            }

            switch(cmd.type)
            {
                case 'init':
                {
                    delegateObjectManager.init();
                }
                break;
                case 'clear':
                {
                    delegateObjectManager.clear();
                }
                break;
                case 'add':
                {
                    let viewport = mgs.editor.viewportManager.getViewportBySceneID(managerID);
                    for (let i = 0;i < cmd.param.idInfos.length;i ++)
                    {
                        let idInfo = cmd.param.idInfos[i];

                        // get from pool
                        let entity = viewport.getEntityFromEntityPool(idInfo.objectID);
 
                        // get parent
                        let parent = viewport.getEntity(idInfo.parentID);
                        if (!parent)
                        {
                            parent = viewport.getRoot();
                        }

                        // get ref
                        let refEntity = viewport.getEntity(idInfo.refID);

                        // add to parent
                        entity.setParent(parent, refEntity);

                        // add delegate
                        delegateObjectManager.createEntityDelegates(entity);
                    } 
                }
                break;
                case 'delete':
                {
                    let viewport = mgs.editor.viewportManager.getViewportBySceneID(managerID);
                    for (let i = 0;i < cmd.param.idInfos.length;i ++)
                    {
                        let idInfo = cmd.param.idInfos[i];

                        // get from viewport
                        let entity = viewport.getEntity(idInfo.objectID);

                        // remove from parent
                        entity.parent.removeChild(entity);

                        // remove delegate
                        delegateObjectManager.deleteEntityDelegates(entity);

                        // put to pool
                        viewport.putEntityToEntityPool(entity);
                    } 
                }
                break;
                case 'move':
                {
                    let viewport = mgs.editor.viewportManager.getViewportBySceneID(managerID);
                    for (let i = 0;i < cmd.param.idInfos.length;i ++)
                    {
                        let idInfo = cmd.param.idInfos[i];

                        let fromEntity = viewport.getEntity(idInfo.fromID);
                        let toEntity = viewport.getEntity(idInfo.toID);
                        let refEntity = viewport.getEntity(idInfo.refID);
                        fromEntity.setParent(toEntity, refEntity, true);
                    } 
                }
                break;
                case 'modify':
                {
                    let objectID = cmd.objectID;
                    let delegateObject = delegateObjectManager.get(objectID);
                    if (delegateObject)
                    {
                        let property = delegateObject.getPropertyByFullPath(cmd.param.fullPath);
                        let value = cmd.param.value;
                        property.setValue(value);
                    }
                }
                break;
            }
        };
        mgs.editor.commandManager.on('onCommand', this._cmdHander);
    };
    mgs.classInherit(DelegateManager, mgs.Object);

    DelegateManager.prototype.onDestroy = function()
    {
        mgs.editor.commandManager.unbind('onCommand', this._cmdHander);
    };

    DelegateManager.prototype.createDelegateObjectManager = function(id, typeClass, params)
    {
        if (this._delegateObjectContainerMap_.has(id))
        {
            console.error('id has existed! : ' + id);
            return null;
        }

        params = params ? params : {};
        params.id = id;
        let manager = new typeClass(params);
        this._delegateObjectContainerMap_.set(id, manager);
        return manager;
    };

    DelegateManager.prototype.getDelegateObjectManager = function(id)
    {
        let manager = this._delegateObjectContainerMap_.get(id);
        return manager;
    };

    DelegateManager.prototype.deleteDelegateObjectManager = function(id)
    {
        if (!this._delegateObjectContainerMap_.has(id))
        {
            console.error('id not existed! : ' + id);
            return null;
        }

        let manager = this._delegateObjectContainerMap_.get(id);
        this._delegateObjectContainerMap_.delete(id);
        return manager;
    };

    // ------ command ------ //
    DelegateManager.prototype.generateModifyCommand = function(managerID, objectID, fullPath, value, oldValue)
    {
        let delegateObjectManager = this.getDelegateObjectManager(managerID);
        if (!delegateObjectManager)
        {
            return;
        }

        let delegateObject = delegateObjectManager.get(objectID);
        if (!delegateObjectManager)
        {
            return;
        }

        let delegateProperty = delegateObject.getPropertyByFullPath(fullPath);
        if (!delegateProperty)
        {
            return;
        }

        if (oldValue === undefined)
        {
            oldValue = delegateProperty.getValue();
        }
        
        let command =
        {
            cmd:
            {
                managerID: managerID,
                objectID: objectID,
                type: 'modify',
                param: 
                {
                    fullPath: fullPath,
                    value: mgs.Util.deepCopy(value),
                }
            },

            reverse:
            {
                managerID: managerID,
                objectID: objectID,
                type: 'modify',
                param: 
                {
                    fullPath: fullPath,
                    value: mgs.Util.deepCopy(oldValue),
                }
            },
        };

        return command;
    };

    DelegateManager.prototype.sendModifyCommand = function(managerID, objectID, fullPath, value, oldValue, ignoreHistory)
    {
        let command = this.generateModifyCommand(managerID, objectID, fullPath, value, oldValue);
        mgs.editor.commandManager.processCommands([command], false, ignoreHistory);
    };

    DelegateManager.prototype.generateInitCommand = function(managerID)
    {
        let command =
        {
            cmd:
            {
                managerID: managerID,
                type: 'init'
            },

            reverse:
            {
                managerID: managerID,
                type: 'init'
            },
        };

        return command;
    };

    DelegateManager.prototype.sendInitCommand = function(managerID)
    {
        let command = this.generateInitCommand(managerID);
        mgs.editor.commandManager.processCommands([command], false, true);
    };

    DelegateManager.prototype.generateClearCommand = function(managerID)
    {
        let command =
        {
            cmd:
            {
                managerID: managerID,
                type: 'clear'
            },

            reverse:
            {
                managerID: managerID,
                type: 'clear'
            },
        };

        return command;
    };

    DelegateManager.prototype.sendClearCommand = function(managerID)
    {
        let command = this.generateClearCommand(managerID);
        mgs.editor.commandManager.processCommands([command], false, true);
    };

    DelegateManager.prototype.generateAddCommands = function(managerID, idInfos)
    {
        let commandAdd =
        {
            cmd:
            {
                managerID: managerID,
                type: 'add',
                param:
                {
                    idInfos: mgs.Util.deepCopy(idInfos),
                }
            },

            reverse:
            {
                managerID: managerID,
                type: 'delete',
                param:
                {
                    idInfos: mgs.Util.deepCopy(idInfos),
                }
            },
        };

        let selectIDs = [];
        for (let i = 0;i < idInfos.length;i ++)
        {
            let idInfo = idInfos[i];
            selectIDs.push(idInfo.objectID);
        }
        let commandSelect = mgs.editor.selectManager.generateSelectCommand(managerID, selectIDs); 

        return [commandAdd, commandSelect];
    };

    DelegateManager.prototype.sendAddCommands = function(managerID, idInfos)
    {
        let commands = this.generateAddCommands(managerID, idInfos);
        mgs.editor.commandManager.processCommands(commands, false);
    };

    DelegateManager.prototype.generateDeleteCommands = function(managerID, idInfos)
    {
        let commandDelete =
        {
            cmd:
            {
                managerID: managerID,
                type: 'delete',
                param:
                {
                    idInfos: mgs.Util.deepCopy(idInfos),
                }
            },

            reverse:
            {
                managerID: managerID,
                type: 'add',
                param:
                {
                    idInfos: mgs.Util.deepCopy(idInfos),
                }
            },
        };

        let selectIDs = [];
        let commandSelect = mgs.editor.selectManager.generateSelectCommand(managerID, selectIDs); 

        return [commandSelect, commandDelete];
    };

    DelegateManager.prototype.sendDeleteCommands = function(managerID, idInfos)
    {
        let commands = this.generateDeleteCommands(managerID, idInfos);
        mgs.editor.commandManager.processCommands(commands, false);
    };

    DelegateManager.prototype.generateMoveCommand = function(managerID, idInfos, oldIDInfos)
    {
        let command =
        {
            cmd:
            {
                managerID: managerID,
                type: 'move',
                param:
                {
                    idInfos: mgs.Util.deepCopy(idInfos),
                }
            },

            reverse:
            {
                managerID: managerID,
                type: 'move',
                param:
                {
                    idInfos: mgs.Util.deepCopy(oldIDInfos),
                }
            },
        };

        return [command];
    };

    DelegateManager.prototype.sendMoveCommand = function(managerID, idInfos, oldIDInfos)
    {
        let commands = this.generateMoveCommand(managerID, idInfos, oldIDInfos);
        mgs.editor.commandManager.processCommands(commands, false);
    };
}());