(function ()
{
    // -------------------------- options -------------------------- //
    let leftPartOptions =
    {
        id:'leftPart',
        styleOptions:
        {
            'flex-flow':'column nowrap',
        }
    };

    let scenePanelOptions =
    {
        id:'sceneEditor',
        text:'场景编辑器',
        childStyleOptions:
        {
            content:
            {
                margin: '0px',
                display: 'flex',
                'flex-flow':'row nowrap',
            },
            header:
            {
                height : '0px',
            },
        }
    };

    let assetPanelOptions =
    {
        id:'assetPanel',
        text:'资源',
        styleOptions:
        {
            height:'30%', // height:'313px',
            'border-top-width':'1px',
        },
        handlerOption:
        {
            direction:'top',
            min:100,
            // max:500,
        },
        childStyleOptions:
        {
            content:
            {
                margin: '0px',
                display: 'flex',
                'flex-flow':'row nowrap',
            },
            header:
            {
                height:'25px',
            },
        },
        foldOption:
        {
            horizontal:true,
        }
    };

    let attributePanelOptions ={
        id:'attributePanel',
        text:'属性',
        styleOptions:
        {
            width: '30%', // width:'330px',
            'border-left-width':'1px',
        },

        childStyleOptions:
        {
            content:
            {
                padding:'5px 5px 15px 5px',
                margin:'0px',
                color: 'white'
            },
        },

        handlerOption:
        {
            direction:'left',
            min:100,
            // max:500,
        },
        foldOption:
        {
            vertial:true,
            buttonDirection:'Bottom',
        },
    };

    // -------------------------- EditorController -------------------------- //
    let EditorController = function (options) 
    {
        mgs.ControllerBase.call(this, options);
   
        // --------- managers --------- //
        mgs.editor.viewportManager = new mgs.ViewportManager();
        mgs.editor.assetManager = new mgs.AssetManager();
        mgs.editor.commandManager = new mgs.CommandManager();
        mgs.editor.delegateManager = new mgs.DelegateManager();
        mgs.editor.selectManager = new mgs.SelectManager();
        
        // --------- global events --------- //
        let self = this;
        window.addEventListener('keydown', function(evt)
        {
            let isCtrlDown = evt.ctrlKey || evt.metaKey;
            if (isCtrlDown)
            {
                let commandHistory = mgs.editor.commandManager.getCommandHistory();
                if (evt.code === 'KeyZ')
                {
                    commandHistory.undo();
                    evt.preventDefault(); // disable default undo
                }

                if (evt.code === 'KeyY')
                {
                    commandHistory.redo();
                    evt.preventDefault(); // disable default redo
                }  
            }
        }, true);

        // disable contextmenu
        window.addEventListener('contextmenu', event => event.preventDefault());

        // --------- sub controllers --------- //
        let view = this.getView();
        let leftPart = new mgs.UILayout(leftPartOptions);
        view.append(leftPart);

        this._scenePanelController_ = new mgs.ScenePanelController({viewOptions: scenePanelOptions});
        leftPart.append(this._scenePanelController_.getView());

        this._assetPanelController_ = new mgs.AssetPanelController({viewOptions: assetPanelOptions});
        leftPart.append(this._assetPanelController_.getView());
        
        this._attributePanelController_ = new mgs.AttributePanelController({viewOptions: attributePanelOptions});
        view.append(this._attributePanelController_.getView());

        // --------- test --------- //
        // let managerID = 0;
        // let testManager = mgs.editor.delegateManager.createDelegateObjectManager(managerID, mgs.DelegateTestManager);
        // let testObject = new mgs.TestSourceObject();
        // testManager.add(testObject.id, {sourceObject: testObject});
        // this._testManagerID = managerID;
        // this._testObjectID = testObject.id;
        // this._testNum = 500;

        // let self = this;
        // window.addEventListener('keydown', function(evt)
        // {
        //     if (evt.code === 'KeyT')
        //     {
        //         this.console.log(testObject);
        //     }

        //     if (evt.code === 'KeyA')
        //     {
        //         mgs.editor.delegateManager.sendModifyCommand(self._testManagerID, self._testObjectID, ['a'], self._testNum ++);
        //     }
        //     if (evt.code === 'KeyB')
        //     {
        //         mgs.editor.delegateManager.sendModifyCommand(self._testManagerID, self._testObjectID, ['testNest'], new mgs.TestSourceObjectNest());
        //     }

        //     if (evt.code === 'KeyC')
        //     {
        //         mgs.editor.delegateManager.sendModifyCommand(self._testManagerID, self._testObjectID, ['testNest'], null);
        //     }
        // }, true);

        // mgs.editor.selectManager.sendSelectCommand(managerID, [testObject.id]);
    };
    mgs.classInherit(EditorController, mgs.ControllerBase);

    EditorController.prototype.createView = function(viewOptions)
    {
        let view = new mgs.UILayout(viewOptions);
        view.getRoot().classList.add('EditorControllerView');
        return view;
    };

    EditorController.prototype.update = function(delta)
    {
        this._scenePanelController_.update(delta);
        this._assetPanelController_.update(delta);
        this._attributePanelController_.update(delta);
    };
}());