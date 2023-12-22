(function()
{
    let ViewportCamera = function(application)
    {
        mgs.Object.call(this);

        this._application = application;

        this._editorCameras = {};
        this._cameraManipulator = {};
        this._currentCameraName = null;

        // editor camera root
        this._cameraRoot = this._application.createEditorEntity('Editor_Camera_Root');
        this._application.getEditorRoot().addChild(this._cameraRoot);

        // editor main camera
        let manipulator = new GASEngine.SwitchManipulator();
        manipulator.computeHomePosition(new GASEngine.Vector3(0, 0, 10), 1);
        manipulator.getCurrentManipulator().setStepFactor(4);
        this.createCamera('Main', 'perspective', manipulator);

        this.createCamera('Left', 'orthographic');
        this.createCamera('Right', 'orthographic');
        this.createCamera('Top', 'orthographic');
        this.createCamera('Bottom', 'orthographic');

        // set current camera
        let mainCamera = this.getCamera('Main');
        mainCamera.parentEntity.translation.set(0, 0, 10);
        this.setCurrentCanmera('Main');
    };
    mgs.classInherit(ViewportCamera, mgs.Object);

    ViewportCamera.prototype.destroy = function()
    {

    };

    // type: perspective/orthographic
    ViewportCamera.prototype.createCamera = function(name, type, manipulator)
    {
        let cameraEntity = this._application.createEditorEntity('Editor_Camera' + name);
        let cameraComponent = this._application.createComponent('camera');
        cameraEntity.type = type;
        cameraComponent.fov = 45;
        cameraEntity.addComponent(cameraComponent);

        this._cameraRoot.addChild(cameraEntity);

        this._editorCameras[name] = cameraComponent;

        if (manipulator !== undefined)
        {
            manipulator.getCurrentManipulator().setCamera(cameraComponent);
            this._cameraManipulator[name] = manipulator;
        }
    };

    ViewportCamera.prototype.setCurrentCanmera = function(name)
    {
        let camera = this._editorCameras[name];
        if (camera)
        {
            this._application.setCurrentCamera(camera);
        }

        this._currentCameraName = name;
    };

    ViewportCamera.prototype.getCurrentCanmera = function()
    {
        return this._editorCameras[this._currentCameraName];
    };

    ViewportCamera.prototype.getCurrentManipulator = function()
    {
        return this._cameraManipulator[this._currentCameraName];
    };

    ViewportCamera.prototype.getCamera = function(name)
    {
        return this._editorCameras[name];
    };

    ViewportCamera.prototype.update = function(delta)
    {
        let currentCamera = this.getCurrentCanmera();
        let currentManipulator = this.getCurrentManipulator();

        if (currentCamera && currentManipulator)
        {
            currentManipulator.update(delta);
            let cameraMatrixWorld = currentManipulator.getCameraWorldMatrix();
            currentCamera.setWorldMatrix(cameraMatrixWorld);
            currentCamera._updateViewMatrix();
        }
    }
}());