(function()
{
    let EditorApplication = function(canvas)
    {
        mgs.Application.call(this, canvas);

        this._currentCamera = null;
        this._currentCameraList = null;

        this._editorEntitys = {};
        this._editorRoot = this.createEditorEntity('Editor_Root');

        // let entity0 = this._webglCommon.createCube();
        // this._scene.appendEntityOnRoot(entity0);
        // entity0.translation.set(-3, 0, 0);

        // let entity1 = this._webglCommon.createCube();
        // this._scene.appendEntityOnRoot(entity1);
        // entity1.translation.set(3, 0, 0);

        // default scene append editor root;
        this._scene.appendEntityOnRoot(this._editorRoot);
    };
    mgs.classInherit(EditorApplication, mgs.Application);

    // -------------------------------------------- overwrite --------------------------------------------

    EditorApplication.prototype.destroy = function()
    {
        mgs.Application.prototype.destroy(this);
    };

    EditorApplication.prototype.onCanvasResize = function(width, height)
    {
        // resize editor camera
        let scene = this.getScene();
        if (scene !== null)
        {
            let cameras = scene.findComponents('camera');
            for(let i = 0; i < cameras.length; ++i)
            {
                if (this.isEditorEntity(cameras[i].getParentEntity()))
                {
                    cameras[i].aspect = width / height;
                    cameras[i]._updateViewMatrix();
                    cameras[i]._updateProjectionMatrix();
                    cameras[i]._updateViewProjectionMatrix();
                }
            }
        }
                
        this._webGLDevice.setSize(width, height);
        this._pBRPipeline.onWindowResize(width, height);
    };

    EditorApplication.prototype.update = function(delta)
    {
        if(this._scene !== null)
        {
            this._scene.cull();
            
            this._scene.update(delta);
    
            if (this._currentCameraList !== null)
            {
                this._pBRPipeline.render_V1(this._currentCameraList, 1);
            }
        }
    };

    EditorApplication.prototype.closeScene = function()
    {
        // remove editor entities
        if (this._scene !== null)
        {
            this._scene.root.removeChild(this._editorRoot);
        }
        
        mgs.Application.prototype.closeScene.call(this);
    };

    EditorApplication.prototype.openScene = function(sceneInfo, onSuccess)
    {
        let self = this;
        mgs.Application.prototype.openScene.call(this, sceneInfo, function(scene)
        {
            // append editor entities
            if (scene !== null)
            {
                scene.appendEntityOnRoot(self._editorRoot);
            }

            onSuccess(scene);
        });
    };

    // -------------------------------------------- extend --------------------------------------------

    EditorApplication.prototype.getSceneRootChildrenEntities = function(camera)
    {
        let entities = [];
        for(var i = 0; i < this._scene.root.children.length; i++)
        {
            let entity = this._scene.root.children[i];
            if (entity.uniqueID !== this._editorRoot.uniqueID)
            {
                entities.push(entity);
            }
        }

        return entities;
    };

    EditorApplication.prototype.setCurrentCamera = function(camera)
    {
        this._currentCamera = camera;
        this._currentCameraList = [this._currentCamera];
    };

    EditorApplication.prototype.getCurrentCamera = function(camera)
    {
        return this._currentCamera;
    };

    EditorApplication.prototype.getEditorRoot = function(camera)
    {
        return this._editorRoot;
    };

    EditorApplication.prototype.isEditorEntity = function(entity)
    {
        return entity && (this._editorEntitys[entity.uniqueID] !== undefined);
    };

    EditorApplication.prototype.createEditorEntity = function(name, translation, rotation, scale)
    {
        let entity = this.createEntity(name, translation, rotation, scale);
        entity.type = 'helper';
        this._editorEntitys[entity.uniqueID] = entity;
        return entity;
    };

    EditorApplication.prototype.removeEditorEntity = function(entity)
    {
        delete this._editorEntitys[entity.uniqueID];
        this.destroy(entity);
    };
}());