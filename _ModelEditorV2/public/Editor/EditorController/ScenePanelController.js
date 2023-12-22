(function ()
{
    let style = getComputedStyle(document.body);
    let itemColor = style.getPropertyValue('--item-color');

    // -------------------------- options -------------------------- //
    let sceneTreeOption =
    {
        id:'viewTree',
        text:'场景树',
        styleOptions:
        {
            width: '25%', // width:'256px',
            'border-right-width':'1px',
        },
        handlerOption:
        {
            direction:'right',
            min:100,
            max:400,
        },
        foldOption:
        {
            vertial:true,
            buttonDirection:'Top',
        },
        childStyleOptions:
        {
            content:
            {
                margin: '0px',
                background: itemColor,
            },
        },
    };

    let sceneViewportOption =
    {
        id:'toolBarViewport',
        styleOptions:
        {
            'flex-flow':'column nowrap',
        },
    };


    // -------------------------- SceneTreeController -------------------------- //
    let findTreeRule = function(id, data)
    {
        return id === data.id;
    };

    let SceneTreeController = function(options) 
    {
        this._viewport_ = options.viewport;
        mgs.ControllerBase.call(this, options);

        let self = this;
        this._cmdHander = function(cmd)
        {
                // find delegate
                let managerID = cmd.managerID;
                let delegateObjectManager = mgs.editor.delegateManager.getDelegateObjectManager(managerID);

                if (!delegateObjectManager)
                {
                    self.rootTree._selectTrees([]);
                    return;
                }

                // check entity delegate manager
                if (!delegateObjectManager.isInstanceOf(mgs.DelegateEntityManager))
                {
                    return;
                }
    
                switch(cmd.type)
                {
                    case 'init':
                    {
                        let rootEntityDelegate = delegateObjectManager.getRootDelegate();
                        self.rootTree.data = {id: rootEntityDelegate.getID()};

                        let childDelegates = rootEntityDelegate.getChildDelegates();
                        for (let i = 0;i < childDelegates.length;i ++)
                        {
                            let childDelegate = childDelegates[i];
                            self._addDelegateEntityTree(childDelegate, self.rootTree);
                        }
                    }
                    break;
                    case 'clear':
                    {
                        self.rootTree.clearTree();
                    }
                    break;
                    case 'select':
                    {
                        let selectIDs = cmd.param.selectIDs;
                        let trees = [];
                        for (let i = 0;i < selectIDs.length;i ++)
                        {
                            let tree = self._findTreeByID(selectIDs[i]);
                            trees.push(tree);
                        }
                        
                        self.rootTree._selectTrees(trees);
                    }
                    break;
                    case 'move':
                    {
                        for (let i = 0;i < cmd.param.idInfos.length;i ++)
                        {
                            let idInfo = cmd.param.idInfos[i];
    
                            let fromTree = self._findTreeByID(idInfo.fromID);
                            let toTree = self._findTreeByID(idInfo.toID);
                            let refTree = self._findTreeByID(idInfo.refID);

                            if (refTree)
                            {
                                toTree.insertTree(fromTree, refTree);
                            }
                            else
                            {
                                toTree.appendTree(fromTree);
                            }
                        } 
                    }
                    break;
                    case 'add':
                    {
                        for (let i = 0;i < cmd.param.idInfos.length;i ++)
                        {
                            let idInfo = cmd.param.idInfos[i];
                            let delegateObject = delegateObjectManager.get(idInfo.objectID);
                            let parentTree = idInfo.parentID ? self._findTreeByID(idInfo.parentID) : self.rootTree;
                            let refTree = idInfo.refID ? self._findTreeByID(idInfo.refID) : null;
                            self._addDelegateEntityTree(delegateObject, parentTree, refTree);
                        } 
                    }
                    break;
                    case 'delete':
                    {
                        for (let i = 0;i < cmd.param.idInfos.length;i ++)
                        {
                            let idInfo = cmd.param.idInfos[i];
                            let tree = self._findTreeByID(idInfo.objectID);
                            self.rootTree.removeTree(tree);
                        } 
                    }
                    break;
                }
        };
        mgs.editor.commandManager.on('onCommand', this._cmdHander);
    };
    mgs.classInherit(SceneTreeController, mgs.ControllerBase);

    SceneTreeController.prototype.onDestroy = function()
    {
        mgs.editor.commandManager.unbind('onCommand', this._cmdHander);
    };

    SceneTreeController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UIPanel(viewOptions);

        this.rootTree = new mgs.UITree
        (
            {
                styleOptions:{}, 
                childStyleOptions:
                {
                    header:{display:'none',}, 
                    insertArea:{display:'none',},
                    content:{'padding-left':'0px',},
                },
                canFocus: true,
            }
        ); 

        let self = this;
        this.rootTree.on('onTreeSelect', function(trees)
        {
            let sceneID = self._viewport_.getSceneID();
            let selectIDs = [];
            for (let i = 0;i < trees.length;i ++)
            {
                selectIDs.push(trees[i].data.id)
            }
            mgs.editor.selectManager.sendSelectCommand(sceneID, selectIDs);
        });

        this.rootTree.on('onTreeDelete', function(trees)
        {
            let sceneID = self._viewport_.getSceneID();
            let idInfos = [];
            for (let i = 0;i < trees.length;i ++)
            {
                let tree = trees[i];
                let parentTree = tree.getParentTree();

                let nextTreeIndex = parentTree.getChildTreeIndex(tree) + 1;
                let nextTree = parentTree.getChildrenTreeByIndex(nextTreeIndex);
                let refID = nextTree ? nextTree.data.id : null;

                let idInfo = 
                {
                    objectID: tree.data.id,
                    parentID: parentTree.data.id,
                    refID: refID,
                }
                idInfos.push(idInfo)
            }
            mgs.editor.delegateManager.sendDeleteCommands(sceneID, idInfos);
        });

        this.rootTree.on('onTreeMove', function(fromTrees, toTree, refTree)
        {
            let sceneID = self._viewport_.getSceneID();
            let newParentID = toTree.data.id;
            let refID = refTree ? refTree.data.id : null;

            let idInfos = [];
            let oldIDInfos = [];
            for (let i = 0;i < fromTrees.length;i ++)
            {
                let tree = fromTrees[i];
                let oldParentTree = tree.getParentTree();

                let nextTreeIndex = oldParentTree.getChildTreeIndex(tree) + 1;
                let nextTree = oldParentTree.getChildrenTreeByIndex(nextTreeIndex);
                let oldRefID = nextTree ? nextTree.data.id : null;

                let idInfo =
                {
                    fromID: tree.data.id,
                    toID: newParentID,
                    refID: refID,
                };
                idInfos.push(idInfo);

                let oldIDInfo =
                {
                    fromID: tree.data.id,
                    toID: oldParentTree.data.id,
                    refID: oldRefID,
                };
                oldIDInfos.push(oldIDInfo);
            }

            mgs.editor.delegateManager.sendMoveCommand(sceneID, idInfos, oldIDInfos);
        });

        view.content.append(this.rootTree);
        view.content.getRoot().addEventListener('mouseup', function(evt)
        {            
            let sceneID = self._viewport_.getSceneID();
            let selectIDs = [];
            mgs.editor.selectManager.sendSelectCommand(sceneID, selectIDs);

        }, false);
        return view;
    };

    SceneTreeController.prototype._addDelegateEntityTree = function(delegateEntity, parentTree, refTree)
    {
        let nameProperty = delegateEntity.getProperty('name');
        let name = nameProperty.getValue();

        let id = delegateEntity.getID();
        let data = {id: id};

        let tree = new mgs.UITree({text: name, rootTree:this.rootTree, data: data });
        if (refTree)
        {
            parentTree.insertTree(tree, refTree);
        }
        else
        {
            parentTree.appendTree(tree); 
        }

        let childDelegates = delegateEntity.getChildDelegates();
        for (let i = 0;i < childDelegates.length;i ++)
        {
            let childDelegate = childDelegates[i];
            this._addDelegateEntityTree(childDelegate, tree);
        }
    };

    SceneTreeController.prototype._findTreeByDelegate = function(delegateEntity)
    {
        let id = delegateEntity.getID();
        let parentTree = this.rootTree.findTree(id, findTreeRule);
        return parentTree;
    };

    SceneTreeController.prototype._findTreeByID = function(id)
    {
        let tree = this.rootTree.findTree(id, findTreeRule);
        return tree;
    };

    // -------------------------- SceneViewportController -------------------------- //
    let SceneViewportController = function (options) 
    {
        mgs.ControllerBase.call(this, options);

        let self = this;
        this._cmdHander = function(cmd)
        {
            // find delegate
            let managerID = cmd.managerID;
            let delegateObjectManager = mgs.editor.delegateManager.getDelegateObjectManager(managerID);
            if (!delegateObjectManager)
            {
                self._viewport_.selectEntityByID(null);
                return;
            }

            switch(cmd.type)
            {
                case 'clear':
                {
                    self._viewport_.clearStatus();
                }
                break;
                case 'select':
                {
                    let selectIDs = cmd.param.selectIDs;
                    
                    self._viewport_.selectEntityByID(null);

                    if (selectIDs.length === 1)
                    {
                        // todo: 暂时支持单Object属性select
                        self._viewport_.selectEntityByID(selectIDs[0]);
                    }
                }
                break;
            }
        };
        mgs.editor.commandManager.on('onCommand', this._cmdHander);
    };
    mgs.classInherit(SceneViewportController, mgs.ControllerBase);

    SceneViewportController.prototype.onDestroy = function()
    {
        mgs.editor.commandManager.unbind('onCommand', this._cmdHander);
    };

    SceneViewportController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UILayout(viewOptions);

        ////////////////////// tool bar //////////////////////
        this.toolBar = new mgs.UILayout
        (
            {
                id:'toolBar',
                styleOptions:
                {
                    height:'30px',
                    'border-bottom-width':'1px',
                }
            }
        );
        view.append(this.toolBar);

        let leftPanel = new mgs.UILayout
        (
            {
                id:'toolBar-leftPanel',
                styleOptions:
                {
                    height : '30px',
                    'border-right-width' : '1px',
                    'flex-grow' : '1',
                    'position' : 'relative',
                }
            }
        );
        this.toolBar.append(leftPanel);

        let leftContentPanel = new mgs.UILayout
        (
            {
                id:'toolBar-leftContentPanel',
                styleOptions:
                {
                    height : '30px',
                    'display' : 'flex',
                    'align-items' : 'center',
                    'position' : 'absolute',
                    'overflow' : 'auto',
                    'left' : '0px',
                    'right' : '0px',
                    'top' : '0px',
                    'bottom' : '0px',
                    'white-space' : 'nowrap',
                }
            }
        );
        leftPanel.append(leftContentPanel);

        ////////////////////// trs button //////////////////////
        let toolBarStateBtnStyle = 
        {
            width:'25px',
            height:'25px',
            'line-height':'25px',
            'flex-grow':'0',
            'flex-shrink':'0',
            'padding':'0px',
            'margin' : '0px 1.5px',
            'font-family' : 'mgs-icon',
            'font-size' : '18px',
            'cursor' : 'pointer',
            'font-weight' : 'normal',
        };

        let mutexButtonInfos =
        [
            { selectID:'translate', text: mgs.ICONS.moveBtn },
            { selectID:'rotate', text: mgs.ICONS.rotateBtn },
            { selectID:'scale', text: mgs.ICONS.scaleBtn }
        ];
        let mutexButtons = {};
        this._selectID_ = 'translate';

        let self = this;
        let onStateChange = function(ui, selected)
        {
            if (selected)
            {
                for (let key in mutexButtons)
                {
                    let mutexButton = mutexButtons[key];
                    if (mutexButton !== ui)
                    {
                        if (mutexButton.selected)
                        {
                            mutexButton.setSelected(false, true);
                        }
                    }
                }
            }
            else
            {
                mutexButtons[ui.selectID].setSelected(true, true);
            }

            self._selectID_ = ui.selectID;
            self._onTRSSelect(ui.selectID);
        };

        for (let i = 0;i < mutexButtonInfos.length;i ++)
        {
            let mutexButtonInfo = mutexButtonInfos[i];
            let btn = new mgs.UIStateButton(
                {
                    text:mutexButtonInfo.text,
                    styleOptions : toolBarStateBtnStyle,
                }
            );
            btn.selectID = mutexButtonInfo.selectID;
            btn.onStateChange = onStateChange;
            leftContentPanel.append(btn);
            mutexButtons[mutexButtonInfo.selectID] = btn;
        }

        let checkbox = new mgs.UICheckbox();
        checkbox.onChange = function(value)
        {
            self._onLocalModeChanged(value);
        };

        leftContentPanel.append(checkbox);

        let slider = new mgs.UISlider
        (
            {
                styleOptions:
                {
                    'margin-left': '15px',
                },
                precision: 1,
                min: 0.1,
                max: 1,
            }
        );
        leftContentPanel.append(slider);
        this.slider = slider;
        this.slider.on('onValueChange', function(value)
        {
            self._onStepValueChanged(value);
        });

        let saveBtn = new mgs.UIButton
        (
            {
                styleOptions: toolBarStateBtnStyle,
                text: mgs.ICONS.saveBtn,
            }
        );
        leftContentPanel.append(saveBtn);
        this.saveBtn = saveBtn;
        this.saveBtn.on('onClick', function(evt)
        {
            self._onSaveBtnClicked();
        });

        ////////////////////// viewport //////////////////////
        this._viewportPanel_ = new mgs.UILayout();

        this._canvas3d_ = new mgs.UICanvas({canFocus: true});
        this._viewportPanel_.append(this._canvas3d_);

        this._fpsElement_ = new mgs.UIBase
        (
            {
                id:'FPS',
                styleOptions: {
                    position: 'absolute',
                    color: 'red',
                    margin: '5px'
                }
            }
        );
        this._viewportPanel_.append(this._fpsElement_);
        view.append(this._viewportPanel_);

        this._viewport_ = mgs.editor.viewportManager.createViewport(this._canvas3d_);

        // onresize
        let viewportPanelRoot = this._viewportPanel_.getRoot();
        let viewportPanelWidth = 0;
        let viewportPanelHeight = 0;
   
        this.viewportResizeInterval = setInterval(() => {
            let viewportPanelRect = viewportPanelRoot.getBoundingClientRect();
            if (viewportPanelWidth !== viewportPanelRect.width ||
                viewportPanelHeight !== viewportPanelRect.height)
            {
                viewportPanelWidth = viewportPanelRect.width;
                viewportPanelHeight = viewportPanelRect.height;
                self._viewport_.onCanvasResize(viewportPanelWidth, viewportPanelHeight);
            }
        }, 1000/60);

        mutexButtons[mutexButtonInfos[0].selectID].setSelected(true); 

        return view;
    };

    SceneViewportController.prototype._onTRSSelect = function(selectID)
    {
        this._viewport_.setMode(selectID);
    };

    SceneViewportController.prototype._onLocalModeChanged = function(isLocalMode)
    {
        this._viewport_.setLocal(isLocalMode);
    };

    SceneViewportController.prototype._onStepValueChanged = function(value)
    {
        this._viewport_.setStepValue(value);
    };

    SceneViewportController.prototype._onSaveBtnClicked = function(isLocalMode)
    {
        
    };

    SceneViewportController.prototype.getTRSSelectID = function(selectID)
    {
        return this._selectID_;
    };

    SceneViewportController.prototype.update = function(delta)
    {
        this._viewport_.update(delta);
    };

    SceneViewportController.prototype.getViewport = function()
    {
        return this._viewport_;
    };

    // -------------------------- ScenePanelController -------------------------- //
    let ScenePanelController = function (options) 
    {
        mgs.ControllerBase.call(this, options);

        let view = this.getView();
  
        this._sceneViewportController_ = new mgs.SceneViewportController({viewOptions: sceneViewportOption});
        view.content.append(this._sceneViewportController_.getView());

        this._sceneTreeController_ = new mgs.SceneTreeController({viewOptions: sceneTreeOption, viewport: this._sceneViewportController_.getViewport()});
        view.content.insert(this._sceneTreeController_.getView(), 0);
    };
    mgs.classInherit(ScenePanelController, mgs.ControllerBase);

    ScenePanelController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UIPanel(viewOptions);
        return view;
    };

    ScenePanelController.prototype.update = function(delta)
    {
        this._sceneTreeController_.update(delta);
        this._sceneViewportController_.update(delta);
    };
    
}());