(function()
{
    let Viewport = function(canvas3D)
    {
        mgs.Events.call(this);

        this._application = new mgs.EditorApplication(canvas3D.getRoot());

        this._viewportGizmo = new mgs.ViewportGizmo(this._application);
        this._viewportCamera = new mgs.ViewportCamera(this._application); 
        this._viewportModeController = new mgs.ViewportModeController(this, this._application, canvas3D, this._viewportGizmo, this._viewportCamera);

        this._entityPool = new Map();

        let self = this;
        window.addEventListener('keydown', function(evt)
        {
            if (evt.code === 'KeyP')
            {
                self.openScene({type:'TEST_EMPTY'});
            }  
            if (evt.code === 'KeyN')
            {
                self.openScene(
                    {
                        type: 'NEO',
                        modelDirectory: '../../samples/airship/airship.fbx.converted/',
                        convertedFile: 'airship.fbx',
                    }
                );
            }
            if (evt.code === 'KeyM')
            {
                self.openScene(
                    {
                        type: 'GLTF',
                        modelDirectory: '../../samples/gltf/airship-embed/',
                        convertedFile: 'airship.gltf',
                    }
                );
            }
            if (evt.code === 'KeyG')
            {
                self.openScene(
                    {
                        type: 'GLTF',
                        modelDirectory: '../../samples/gltf/DamagedHelmet/',
                        convertedFile: 'DamagedHelmet.gltf',
                    }
                );
            }
            if (evt.code === 'KeyF')
            {
                self.openScene(
                    {
                        type: 'GLTF',
                        modelDirectory: '../../samples/gltf/bristleback-embed/',
                        convertedFile: 'bristleback.gltf',
                    }
                );
            }
            
            if(evt.code === 'KeyH')
            {
                self.openScene(
                    {
                        type: 'FBX',
                        modelDirectory: '../../samples/fbx/',
                        convertedFile: 'tank.fbx',
                    }
                )
            }

            if (evt.code === 'KeyO')
            {
                let entity = self._application._webglCommon.createCube(); // self._application.createEntity(name);
                self.putEntityToEntityPool(entity);

                let sceneID = self.getSceneID();
                let entityID = entity.uniqueID;
                let parentID = self.getRoot().uniqueID;
                
                mgs.editor.delegateManager.sendAddCommands(sceneID, [{objectID: entityID, parentID: parentID, refID: null}]);
            }  
        }, true);

        setTimeout(() => {
            // create init DelegateEntityManager
            let newSceneID = self.getSceneID();
            mgs.editor.delegateManager.createDelegateObjectManager(newSceneID, mgs.DelegateEntityManager, {sourceObject: self._application.getScene()});
            mgs.editor.delegateManager.sendInitCommand(newSceneID);
        }, 0);
    };
    mgs.classInherit(Viewport, mgs.Events);

    Viewport.prototype.destroy = function()
    {
    };
    
    Viewport.prototype.update = function(delta)
    {
        this._viewportModeController.update(delta);
        this._viewportCamera.update(delta);
        this._application.update(delta);
    };

    Viewport.prototype.onCanvasResize = function(width, height)
    {
        this._application.onCanvasResize(width, height);
    };

    Viewport.prototype.openScene = function(sceneInfo)
    {
        let preSceneID = this.getSceneID();

        this._application.openScene(sceneInfo, function(scene)
        {
            // delete pre DelegateEntityManager
            mgs.editor.delegateManager.sendClearCommand(preSceneID);
            mgs.editor.delegateManager.deleteDelegateObjectManager(preSceneID);

            // create DelegateEntityManager
            let newSceneID = scene.root.uniqueID;
            mgs.editor.delegateManager.createDelegateObjectManager(newSceneID, mgs.DelegateEntityManager, {sourceObject: scene});
            mgs.editor.delegateManager.sendInitCommand(newSceneID);
        });
    };

    Viewport.prototype.setLocal = function(isLocal)
    {
        this._viewportModeController.setLocalMode(isLocal);
    };

    Viewport.prototype.setMode = function(mode)
    {
        this._viewportModeController.setCurrentMode(mode);
    };

    Viewport.prototype.setStepValue = function(value)
    {
        let manipulator = this._viewportCamera.getCurrentManipulator();
        manipulator.getCurrentManipulator().setStepFactor(40 * value);
    };

    // -------------------------------------------- util call --------------------------------------------
    Viewport.prototype.getSceneID = function()
    {
        return this._application.getScene().root.uniqueID;
    };

    Viewport.prototype.getEntity = function(id)
    {
        let scene = this._application.getScene();
        let entity = scene.findObjectByID(id);
        return entity;
    };

    Viewport.prototype.getRoot = function()
    {
        return this._application.getScene().root;
    };

    Viewport.prototype.getApplication = function()
    {
        return this._application;
    };

    // -------------------------------------------- viewport events --------------------------------------------
    Viewport.prototype.onEntityTranslate = function(entityID, newTranslate, oldTranslate, isAddHistory)
    {
        let managerID = this.getSceneID();
        let objectID = entityID;
        mgs.editor.delegateManager.sendModifyCommand(managerID, objectID, ['position'], newTranslate, oldTranslate, !isAddHistory);
    };

    Viewport.prototype.onEntityRotate = function(entityID, newRotation, preRotation, isAddHistory)
    {
        let managerID = this.getSceneID();
        let objectID = entityID;
        mgs.editor.delegateManager.sendModifyCommand(managerID, objectID, ['rotation'], newRotation, preRotation, !isAddHistory);
    };

    Viewport.prototype.onEntityScale = function(entityID, newScale, preScale, isAddHistory)
    {
        let managerID = this.getSceneID();
        let objectID = entityID;
        mgs.editor.delegateManager.sendModifyCommand(managerID, objectID, ['scale'], newScale, preScale, !isAddHistory);
    };

    Viewport.prototype.onEntitySelect = function(entityID)
    {
        let managerID = this.getSceneID();
        let objectID = entityID;
        let selectIDs = objectID ? [objectID] : [];
        mgs.editor.selectManager.sendSelectCommand(managerID, selectIDs);
    };

    Viewport.prototype.onEntityDelete = function(entityID, preParentID, refID)
    {
        let sceneID = this.getSceneID();

        mgs.editor.delegateManager.sendDeleteCommands(sceneID, [{objectID: entityID, parentID: preParentID, refID: refID}]);
    };
   
    // -------------------------------------------- viewport modify --------------------------------------------
    Viewport.prototype.selectEntityByID = function(id)
    {
        let scene = this._application.getScene();
        let entity = scene.findObjectByID(id);

        this._viewportModeController.selectEntity(entity);
    };

    Viewport.prototype.clearStatus = function()
    {
        this.selectEntityByID(null);
    };

    // -------------------------------------------- entity pool --------------------------------------------
    Viewport.prototype.getEntityFromEntityPool = function(entityID)
    {
        if (!this._entityPool.has(entityID))
        {
            console.error('entity not exist!');
            return null;
        }

        let entity = this._entityPool.get(entityID);
        this._entityPool.delete(entityID);
        return entity;
    };

    Viewport.prototype.putEntityToEntityPool = function(entity)
    {
        let entityID = entity.uniqueID;
        if (this._entityPool.has(entityID))
        {
            console.error('entity has existed!');
            return false;
        }

        this._entityPool.set(entityID, entity);
        return true;
    };
}());