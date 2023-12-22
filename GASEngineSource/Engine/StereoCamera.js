GASEngine.StereoCamera = function ()
{
    this.type = 'StereoCamera';
    this.aspect = 0.5;
    this.eyeSep = 0.064;

    this.cameraL = GASEngine.ComponentFactory.Instance.create('camera');
    this.cameraL.type = 'perspective';
    this.cameraL.fov = 60;
    this.cameraL.aspect = 1;

    this.cameraR = GASEngine.ComponentFactory.Instance.create('camera');
    this.cameraR.type = 'perspective';
    this.cameraR.fov = 60;
    this.cameraR.aspect = 1;

    GASEngine.StereoCamera.Instance = this;
};

GASEngine.StereoCamera.prototype = {
    constructor: GASEngine.StereoCamera,
    update: function ()
    {
        var fov, aspect, near, far;
        var eyeRight = new GASEngine.Matrix4();
        var eyeLeft = new GASEngine.Matrix4();
        var matrixWorldLeft = new GASEngine.Matrix4();
        var matrixWorldRight = new GASEngine.Matrix4();

        return function update(camera)
        {
            var needsUpdate = fov !== camera.fov ||
                aspect !== camera.aspect * this.aspect || near !== camera.near ||
                far !== camera.far;

            if (needsUpdate)
            {
                //focus = camera.focus;
                fov = camera.fov;
                aspect = camera.aspect * this.aspect;
                near = camera.near;
                far = camera.far;
                //zoom = camera.zoom;

                // Off-axis stereoscopic effect based on
                // http://paulbourke.net/stereographics/stereorender/
                var projectionMatrix = camera.getProjectionMatrix().clone();
                var eyeSep = this.eyeSep / 2;
                var eyeSepOnProjection = eyeSep * near / 10.0;
                var ymax = (near * Math.tan(GASEngine.degToRad(fov * 0.5))) / 1.0;
                var xmin, xmax;

                // translate xOffset
                eyeLeft.elements[12] = - eyeSep;
                eyeRight.elements[12] = eyeSep;

                // for left eye
                xmin = - ymax * aspect + eyeSepOnProjection;
                xmax = ymax * aspect + eyeSepOnProjection;

                projectionMatrix.elements[0] = 2 * near / (xmax - xmin);
                projectionMatrix.elements[8] = (xmax + xmin) / (xmax - xmin);

                this.cameraL.setProjectionMatrix(projectionMatrix);

                // for right eye
                xmin = - ymax * aspect - eyeSepOnProjection;
                xmax = ymax * aspect - eyeSepOnProjection;

                projectionMatrix.elements[0] = 2 * near / (xmax - xmin);
                projectionMatrix.elements[8] = (xmax + xmin) / (xmax - xmin);

                this.cameraR.setProjectionMatrix(projectionMatrix);
            }

            var matrixWorld = camera.getWorldMatrix();
            if(matrixWorld !== null) {
                matrixWorldLeft.copy(matrixWorld);
                matrixWorldRight.copy(matrixWorld);
            }
            matrixWorldLeft.multiply(eyeLeft);
            matrixWorldRight.multiply(eyeRight);

            this.cameraL.setWorldMatrix(matrixWorldLeft);
            this.cameraR.setWorldMatrix(matrixWorldRight);

            this.cameraL._opaqueList = camera._opaqueList;
            this.cameraL._opaqueListLastIndex = camera._opaqueListLastIndex;
            this.cameraL._transparentList = camera._transparentList;
            this.cameraL._transparentListLastIndex = camera._transparentListLastIndex;
            this.cameraL._helperList = camera._helperList;
            this.cameraL._helperListLastIndex = camera._helperListLastIndex;
            
            this.cameraL._skybox = camera._skybox;
            this.cameraL._hotspotItem = camera._hotspotItem;

            this.cameraR._opaqueList = camera._opaqueList;
            this.cameraR._opaqueListLastIndex = camera._opaqueListLastIndex;
            this.cameraR._transparentList = camera._transparentList;
            this.cameraR._transparentListLastIndex = camera._transparentListLastIndex;
            this.cameraR._helperList = camera._helperList;
            this.cameraR._helperListLastIndex = camera._helperListLastIndex;

            this.cameraR._skybox = camera._skybox;
            this.cameraR._hotspotItem = camera._hotspotItem;

            this.cameraL._updateViewMatrix();
            //TODO: DONT call stereoCamera.cameraL._updateProjectionMatrix, projectMatrix has updated through stereoCamera.update
            this.cameraL._updateViewProjectionMatrix();
    
            this.cameraR._updateViewMatrix();
            //TODO: DONT call stereoCamera.cameraR._updateProjectionMatrix, projectMatrix has updated through stereoCamera.update
            this.cameraR._updateViewProjectionMatrix();
        }
    }()
};