//Hotspot Manager
GASEngine.HotspotManager = function(width, height)
{
    this.canvasWidth = width;
    this.canvasHeight = height;

    this.currentMouseX = -1;
    this.currentMouseY = -1;
    this.currentHighlightHotspot = null;

    this.hotspots = [];
    this.MAX_HOTSPOT_COUNT = 50; // create 50 hotspots
    this.hotspotRenderMesh = null;
    this.hotspotMaterial = null;
    this.hotspotTexture = null;
    this.cameraManipulator = null;
    this.scene = null;

    this.currentDragingHotspot = null;
    this.dragPoint = new GASEngine.Vector3(0, 0, 0);
    this.dragBias = new GASEngine.Vector3(0, 0, 0);

    this.status = true;

    GASEngine.HotspotManager.Instance = this;
};

GASEngine.HotspotManager.HOTSPOT_SIZE = 26;

GASEngine.HotspotManager.prototype =
{
    constructor: GASEngine.HotspotManager,
    
    init: function (scene) {
        if (scene === undefined || scene === null)
            return;

        this.scene = scene;
        var hotspotEntity = scene.findObjectByPath('/hotspot');
        if (hotspotEntity) {
            // already exists
            var materials = hotspotEntity.getComponent('meshRenderer').getMaterials();
            if (materials.length > 0) {
                this.hotspotMaterial = materials[0];
                this.hotspotTexture = this.hotspotMaterial.image;
            }
            this.hotspotRenderMesh = hotspotEntity.getComponent('meshFilter').getMesh();
            return Promise.resolve(true);
        } else {
            // create:
            var hotspotPromise = new Promise((resolve, reject)=>{
                this.hotspotMaterial = this.createHotspotMaterial(()=>resolve(), reject);
            });
            this.hotspotRenderMesh = this._createHotspotRenderMesh();

            hotspotEntity = GASEngine.EntityFactory.Instance.create();
            hotspotEntity.name = 'hotspot';
            hotspotEntity.type = 'helper';
            hotspotEntity.uniqueID = GASEngine.generateUUID();
            var meshFilterComponent = GASEngine.ComponentFactory.Instance.create('meshFilter');
            meshFilterComponent.setMesh(this.hotspotRenderMesh);
    
            var meshRendererComponent = GASEngine.ComponentFactory.Instance.create('meshRenderer');
            hotspotEntity.addComponent(meshFilterComponent);
            hotspotEntity.addComponent(meshRendererComponent);
            meshRendererComponent.addMaterial(this.hotspotMaterial);
    
            scene.appendEntityOnRoot(hotspotEntity);
            return hotspotPromise;
        }
    },

    finl: function () {
        this.scene = null;
    },

    getHighlightHotspot: function () {
        return this.currentHighlightHotspot;
    },

    setMousePosition: function (x, y) {
        this.currentMouseX = x;
        this.currentMouseY = y;
    },

    setCanvasSize: function (width, height) {
        if (width <= 0 || height <= 0) {
            console.error('frame buffer size error!');
            return;
        }

        this.canvasWidth = width;
        this.canvasHeight = height;
    },

    setManipulator: function (manipulator) {
        this.cameraManipulator = manipulator;
    },

    addHiddenProperties: function (hotspot) {
        Object.defineProperty(hotspot, '__object', {
            value: null,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__effectivePosition', {
            value: new GASEngine.Vector3(0, 0, 0),
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__visible', {
            value: true,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__screenX', {
            value: 0,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__screenY', {
            value: 0,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__screenZ', {
            value: 0,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__previousScreenX', {
            value: 0,
            enumerable: false,
            writable: true
        });

        Object.defineProperty(hotspot, '__previousScreenY', {
            value: 0,
            enumerable: false,
            writable: true
        });
    },

    createHotspotMaterial: function (onTextureLoaded, onFailed) {
        this.hotspotMaterial = GASEngine.MaterialFactory.Instance.create('hotspot');
        GASEngine.Resources.Instance.loadTexture(
            '/system/hotspot.png',
            true,
            function (webglTexture, width, height) {
                this.hotspotTexture = webglTexture;
                this.hotspotMaterial.setImage(webglTexture, width, height);
                onTextureLoaded(webglTexture, width, height);
            }.bind(this),
            null,
            onFailed);
        return this.hotspotMaterial;
    },

    getHotspotScreenPos: function (hotspot) {
        if (hotspot) {
            return { 'x': hotspot.__screenX, 'y': hotspot.__screenY };
        }
        else {
            return null;
        }
    },

    setHotspots: function (hotspots) {
        this.hotspots = hotspots;

        for (var i = 0; i < this.hotspots.length; ++i) {
            var hotspot = this.hotspots[i];
            this.addHiddenProperties(hotspot);
        }

        // this._createHotspotRenderMesh();
    },

    getHotspots: function () {
        return this.hotspots;
    },

    getHotspotCount: function () {
        return this.hotspots.length;
    },

    createHotspot: function ()//event.offsetX, event.offsetY
    {
        if (!this.scene) {
            console.log('scene is not binded in hotspot manager.');
            return;
        }
        var id = this._getMaxValidID() + 1;

        if (id >= this.MAX_HOTSPOT_COUNT) {
            console.error('GASEngine.HotspotManager.createHotspot: exceed the maximum number!');
            return null;
        }

        var x = (this.currentMouseX / this.canvasWidth) * 2 - 1;
        var y = -(this.currentMouseY / this.canvasHeight) * 2 + 1;

        var pickInfo = this.scene.pickPosition_V2(x, y);

        if (pickInfo) {
            if (pickInfo.object.type === 'helper') {
                return;
            }
            var barycoord = null;
            if (pickInfo.barycoord) {
                barycoord = { 'x': pickInfo.barycoord.x, 'y': pickInfo.barycoord.y, 'z': pickInfo.barycoord.z };
            }

            var interPos = { 'x': pickInfo.point.x, 'y': pickInfo.point.y, 'z': pickInfo.point.z };
            var face = [pickInfo.face.a, pickInfo.face.b, pickInfo.face.c];

            hotspot =
            {
                'id': id,
                'objectID': pickInfo.object.uniqueID,
                'skinned': pickInfo.skinned,
                'morphed': pickInfo.morphed,
                'color': null,
                'face': face,
                'interPos': interPos,
                'barycoord': barycoord ? barycoord : null,
                'status': 0 //0:normal, 1:disabled, 2: highlight
            };

            this.addHiddenProperties(hotspot);

            hotspot.__object = pickInfo.object;
            hotspot.__effectivePosition.x = pickInfo.point.x;
            hotspot.__effectivePosition.y = pickInfo.point.y;
            hotspot.__effectivePosition.z = pickInfo.point.z;

            this.hotspots.push(hotspot);

            return hotspot;
        }

        return null;
    },

    deleteHotspot: function (id) {
        if (id === undefined || id === null || id === -1) {
            return null;
        }

        for (var i = 0; i < this.hotspots.length; ++i) {
            var h = this.hotspots[i];
            if (h.id === id) {
                if (this.currentHighlightHotspot === h) {
                    this.currentHighlightHotspot = null;
                }

                if (this.currentDragingHotspot === h) {
                    this.currentDragingHotspot = null;
                }

                this.hotspots.splice(i, 1);

                return true;
            }
        }

        console.error("Error: can not delete a hotspot that does not exist!");
        return false;
    },

    pickHotspot: function () {
        if (this.currentHighlightHotspot) {
            //console.log('pick Hotspot!!!');
            return this.currentHighlightHotspot;
        }
        else {
            return null;
        }
    },

    startDrag: function () {
        if (!this.camera || !this.camera.type === 'perspective') {
            return false;
        }

        var hotspot = this.pickHotspot();
        if (hotspot) {
            this.currentDragingHotspot = hotspot;

            var ep = hotspot.__effectivePosition.clone();

            if (this.cameraManipulator) {
                var cameraMatrixWorld = this.cameraManipulator.getCameraWorldMatrix();
                this.camera.setWorldMatrix(cameraMatrixWorld); //overwrite view matrix produced in updating procedure.
                this.camera._updateViewMatrix();
            }

            var invWorldMatrix = new GASEngine.Matrix4();
            invWorldMatrix.getInverse(this.camera.getWorldMatrix());
            ep.applyMatrix4(invWorldMatrix);
            var scale = -this.camera.near / ep.z;
            ep.multiplyScalar(scale);

            this._calcMousePositionCameraSpace(this.dragPoint, 0);

            this.dragBias.subVectors(ep, this.dragPoint);

            this.currentDragingHotspot.__object = null;
            this.currentDragingHotspot.objectID = -1;
            this.currentDragingHotspot.skinned = false;
            this.currentDragingHotspot.screenSpace = true;

            this.currentDragingHotspot.interPos.x = this.dragPoint.x + this.dragBias.x;
            this.currentDragingHotspot.interPos.y = this.dragPoint.y + this.dragBias.y;
            this.currentDragingHotspot.interPos.z = this.dragPoint.z + this.dragBias.z;

            return true;
        }
        else {
            return false;
        }
    },

    drag: (function () {
        return function () {
            if (this.currentDragingHotspot) {
                //this.currentDragingHotspot.__object = null;
                //this.currentDragingHotspot.objectID = -1;
                //this.currentDragingHotspot.skinned = false;
                //this.currentDragingHotspot.screenSpace = true;

                this._calcMousePositionCameraSpace(this.dragPoint, 0);

                this.currentDragingHotspot.interPos.x = this.dragPoint.x + this.dragBias.x;
                this.currentDragingHotspot.interPos.y = this.dragPoint.y + this.dragBias.y;
                this.currentDragingHotspot.interPos.z = this.dragPoint.z + this.dragBias.z;
            };
        };
    })(),

    endDrag: function () {
        if (!this.scene) {
            console.log('scene is not binded in hotspot manager.');
            return;
        }
        if (this.currentDragingHotspot) {
            var pos = new GASEngine.Vector3(0, 0, 0);
            pos.copy(this.currentDragingHotspot.__effectivePosition);
            pos.project(this.camera);

            var pickInfo = this.scene.pickPosition_V2(pos.x, pos.y);
            if (pickInfo) {
                var barycoord = null;
                if (pickInfo.barycoord) {
                    barycoord = { 'x': pickInfo.barycoord.x, 'y': pickInfo.barycoord.y, 'z': pickInfo.barycoord.z };
                }
                
                this.currentDragingHotspot.__object = pickInfo.object;
                this.currentDragingHotspot.objectID = pickInfo.object.uniqueID;
                this.currentDragingHotspot.skinned = pickInfo.skinned;
                this.currentDragingHotspot.morphed = pickInfo.morphed;
                this.currentDragingHotspot.screenSpace = false;

                this.currentDragingHotspot.face = [pickInfo.face.a, pickInfo.face.b, pickInfo.face.c];//pickInfo.face.clone(),
                this.currentDragingHotspot.interPos = pickInfo.point.clone(),
                    this.currentDragingHotspot.barycoord = pickInfo.skinned ? barycoord : null,
                    this.currentDragingHotspot.__effectivePosition = pickInfo.point.clone();
            }
            else {
                this.currentDragingHotspot.interPos.x = this.dragPoint.x + this.dragBias.x;
                this.currentDragingHotspot.interPos.y = this.dragPoint.y + this.dragBias.y;
                this.currentDragingHotspot.interPos.z = this.dragPoint.z + this.dragBias.z;
            }

            this.dragPoint.set(0, 0, 0);
            this.dragBias.set(0, 0, 0);

            this.currentDragingHotspot = null;
        }
    },

    update: function (camera) {
        this.camera = camera;
        this._updateHotspotsTransform();
    },

    onHotspotMoved: function(id, x, y) {
        console.log(id, x, y);
    },

    // Functions for internal use
    _calcMousePositionCameraSpace: function (pos, delta) {
        var ymax = this.camera.near * Math.tan(GASEngine.degToRad(this.camera.fov * 0.5));
        var xmax = ymax * this.camera.aspect;

        var x = (this.currentMouseX / this.canvasWidth) * 2 - 1;
        var y = -(this.currentMouseY / this.canvasHeight) * 2 + 1;

        pos.set(xmax * x, ymax * y, -(this.camera.near + delta));
    },

    _findHotspot: function (id) {
        if (id === undefined || id === null || id === -1) {
            return null;
        }

        for (var i = 0; i < this.hotspots.length; ++i) {
            var h = this.hotspots[i];
            if (h.id === id) {
                return h;
            }
        }

        return null;
    },

    _getMaxValidID: function () {
        var id = 0;
        for (var i = 0; i < this.hotspots.length; ++i) {
            var h = this.hotspots[i];
            if (h.id > id) {
                id = h.id;
            }
        }

        return id;
    },

    _pickHotspotInternal: function (x, y) {
        if (this.hotspots) {
            var hotspotCount = this.hotspots.length;
            for (var i = 0; i < hotspotCount; ++i) {
                var hotspot = this.hotspots[i];

                var deltaX = x - hotspot.__screenX;
                var deltaY = y - hotspot.__screenY;

                if (deltaX * deltaX + deltaY * deltaY <=
                    GASEngine.HotspotManager.HOTSPOT_SIZE * GASEngine.HotspotManager.HOTSPOT_SIZE / 4) {
                    return hotspot;
                }
            }
        }

        return null;
    },

    _updateHotspotSharedVertexBuffer: function () {
        for (var i = 0; i < this.MAX_HOTSPOT_COUNT; i++) {
            var id = 0, x = 0, y = 0, z = 0, visible = 0, status = 0;
            if (i < this.hotspots.length) {
                var index = this.hotspots.length - 1 - i;
                var hotspot = this.hotspots[index];
                id = hotspot.id;
                x = hotspot.__effectivePosition.x;
                y = hotspot.__effectivePosition.y;
                z = hotspot.__effectivePosition.z;
                visible = hotspot.__visible ? 1 : 0;
                status = hotspot.status;
            }

            var byteOffset = i * 64;
            this.sharedVertexBufferView.setFloat32(byteOffset + 0, x, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 4, y, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 8, z, true);
            var packedColor0 = id | (0 << 8) | (255 << 16) | (visible << 30) | (status << 24);
            this.sharedVertexBufferView.setUint32(byteOffset + 12, packedColor0, true);

            this.sharedVertexBufferView.setFloat32(byteOffset + 16, x, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 20, y, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 24, z, true);
            var packedColor1 = id | (255 << 8) | (255 << 16) | (visible << 30) | (status << 24);
            this.sharedVertexBufferView.setUint32(byteOffset + 28, packedColor1, true);

            this.sharedVertexBufferView.setFloat32(byteOffset + 32, x, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 36, y, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 40, z, true);
            var packedColor2 = id | (0 << 8) | (0 << 16) | (visible << 30) | (status << 24);
            this.sharedVertexBufferView.setUint32(byteOffset + 44, packedColor2, true);

            this.sharedVertexBufferView.setFloat32(byteOffset + 48, x, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 52, y, true);
            this.sharedVertexBufferView.setFloat32(byteOffset + 56, z, true);
            var packedColor3 = id | (255 << 8) | (0 << 16) | (visible << 30) | (status << 24);
            this.sharedVertexBufferView.setUint32(byteOffset + 60, packedColor3, true);
        }
    },

    _createHotspotRenderMesh: function () {
        var mesh = GASEngine.MeshFactory.Instance.create();
        this.sharedVertexBuffer = new ArrayBuffer(this.MAX_HOTSPOT_COUNT * 64);
        this.sharedVertexBufferView = new DataView(this.sharedVertexBuffer);
        this._updateHotspotSharedVertexBuffer();

        // indices
        // 0 ----- 1
        // |     / |
        // |  /    |
        // 2 ----- 3
        var indexBufferArray = new Uint32Array(this.MAX_HOTSPOT_COUNT * 6);
        for (var i = 0; i < this.MAX_HOTSPOT_COUNT; i++) {
            indexBufferArray[i * 6 + 0] = i * 4 + 0;
            indexBufferArray[i * 6 + 1] = i * 4 + 2;
            indexBufferArray[i * 6 + 2] = i * 4 + 1;

            indexBufferArray[i * 6 + 3] = i * 4 + 1;
            indexBufferArray[i * 6 + 4] = i * 4 + 2;
            indexBufferArray[i * 6 + 5] = i * 4 + 3;
        }

        mesh.addStream('position', this.sharedVertexBuffer);
        mesh.addStream('color', this.sharedVertexBuffer);
        mesh.addStream('index', indexBufferArray);
        mesh.addStream('subMesh', [{ 'start': 0, 'count': this.MAX_HOTSPOT_COUNT * 6 }]);
        mesh._sharedVertexBuffer = true;
        mesh.submitToWebGL('dynamic');

        return mesh;
    },

    _updateHotspotsTransform: (function () {
        var pos = new GASEngine.Vector3(0, 0, 0);

        return function () {
            if (!this.scene) {
                console.log('scene is not binded in hotspot manager.');
                return;
            }
            this.currentHighlightHotspot = null;
            for (var i = 0; i < this.hotspots.length; ++i) {
                //object, skinned, hotspotID, color, face, hotspotPosition, barycoord
                var hotspot = this.hotspots[i];

                if (!hotspot.__object && hotspot.objectID >= 0) {
                    if (this.scene) {
                        hotspot.__object = this.scene.root.findObjectByID_r(hotspot.objectID);
                    }
                }

                if (hotspot.__object) {
                    if (hotspot.skinned && this.status) {
                        this._cpuSkinningVertex(hotspot.__object, hotspot.face, hotspot.barycoord, hotspot.__effectivePosition);
                    }
                    else if (hotspot.morphed && this.status) {
                        this._cpuMorphingVertex(hotspot.__object, hotspot.face, hotspot.barycoord, hotspot.__effectivePosition);
                    }
                    else {
                        //TODO: MORPH AND STATIC MESH BUT WITH ANIMATION
                        hotspot.__effectivePosition.x = hotspot.interPos.x;
                        hotspot.__effectivePosition.y = hotspot.interPos.y;
                        hotspot.__effectivePosition.z = hotspot.interPos.z;

                        // // hotspot.__object.updateMatrixWorld();
                        // if(hotspot.__object.update) {
                        //     hotspot.__object.update();
                        // }
                        if(hotspot.__effectivePosition.applyMatrix4 && hotspot.__object.matrixWorld) {
                            hotspot.__effectivePosition.applyMatrix4(hotspot.__object.matrixWorld);
                        }
                        // hotspot.__object.update();
                        // hotspot.__effectivePosition.applyMatrix4(hotspot.__object.matrixWorld);
                    }

                    hotspot.__visible = true;
                }
                else {
                    if (hotspot.screenSpace) {
                        hotspot.__effectivePosition.x = hotspot.interPos.x;
                        hotspot.__effectivePosition.y = hotspot.interPos.y;
                        hotspot.__effectivePosition.z = hotspot.interPos.z;

                        hotspot.__effectivePosition.applyMatrix4(this.camera.getWorldMatrix());

                        hotspot.__visible = true;
                    }
                    else {
                        hotspot.__visible = false;
                    }
                }
                //<

                pos.copy(hotspot.__effectivePosition);
                
                //the data structure changes, so add this snippet
                this.camera.matrixWorld = this.camera._matrixWorld.clone();
                this.camera.projectionMatrix = this.camera._matrixProjection.clone();

                pos.project(this.camera);
                hotspot.__screenX = Math.floor((pos.x + 1) / 2 * this.canvasWidth);
                hotspot.__screenY = Math.floor((1 - pos.y) / 2 * this.canvasHeight);
                hotspot.__screenZ = pos.z;

                if (hotspot.__screenX != hotspot.__previousScreenX ||
                    hotspot.__screenY != hotspot.__previousScreenY) {
                    if(hotspot) {
                        this.onHotspotMoved(hotspot.id, hotspot.__screenX, hotspot.__screenY);
                    }

                    hotspot.__previousScreenX = hotspot.__screenX;
                    hotspot.__previousScreenY = hotspot.__screenY;
                }

                var deltaX = this.currentMouseX - hotspot.__screenX;
                var deltaY = this.currentMouseY - hotspot.__screenY;

                if (deltaX * deltaX + deltaY * deltaY <=
                    GASEngine.HotspotManager.HOTSPOT_SIZE * GASEngine.HotspotManager.HOTSPOT_SIZE / 4) {
                    if (!this.currentHighlightHotspot) {
                        this.currentHighlightHotspot = hotspot;
                    }
                }

                if (hotspot.screenSpace) {
                    hotspot.status = 1;
                }
                else {
                    hotspot.status = 0;
                }

                if (this.currentHighlightHotspot) {
                    this.currentHighlightHotspot.status = 2;
                }
            }

            if (this.hotspots.length > 1) {
                this.hotspots.sort(function (a, b) {
                    return a.__screenZ - b.__screenZ;
                });
            }

            if (this.sharedVertexBufferView && this.sharedVertexBuffer) {
                this._updateHotspotSharedVertexBuffer();
                GASEngine.WebGLBufferManager.Instance.updateWebglBuffer(this.sharedVertexBuffer, 'position');
            }
        }
    })(),

    _cpuSkinningVertex: (function () {
        var tmp_0_0 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_1 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_2 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_3 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_4 = new GASEngine.Vector4(0, 0, 0, 1);

        var tmp_1 = new GASEngine.Vector4(0, 0, 0, 0);
        var tmp_2 = new GASEngine.Vector4(0, 0, 0, 0);

        var boneMatrices = [];
        var skinnedPosition = [];
        for (var i = 0; i < 3; ++i) {
            skinnedPosition.push(new GASEngine.Vector4(0, 0, 0, 0));
        }

        return function (entity, face, barycoord, output) {
            var meshFilterComponent = null;
            if(entity.typeName === 'meshFilter')
            {
                meshFilterComponent = entity;
            } 
            else if(entity.getComponent)
            {
                meshFilterComponent = entity.getComponent('meshFilter');
            }

            if (!meshFilterComponent)
            {
                console.warn('HotspotManager cpuskinning: the meshFilter load error!');
                return;
            }

            var mesh = meshFilterComponent.getMesh();

            if(!mesh)
            {
                console.warn('HotspotManager cpuskinning: the mesh load error!');
                return;
            }
            var bones = mesh.bones || [];
            var invMatrices = mesh.matricesWorld2Bone || [];

            //refresh bone matrices            
            for (var j = boneMatrices.length; j < bones.length; ++j) {
                boneMatrices.push(new GASEngine.Matrix4());
            }

            if(bones.length !== invMatrices.length)
            {
                console.warn('HotspotManager cpuskinning: the mesh bones match matrices error!');
                return;
            }

            for (var j = 0; j < bones.length; ++j)
            {
                var matrix = bones[j].matrixWorld;
                boneMatrices[j].multiplyMatrices(matrix, invMatrices[j]);
            }

            var positionData = mesh.getStream('position'); //Float32Array
            var positionItemSize = mesh.getVertexElementCount();

            var skinWeightData = mesh.getStream('skinWeight'); //Float32Array

            var skinIndexData = mesh.getStream('skinIndex'); //Float32Array
            var skinIndexItemSize = mesh.getSkinIndexItemSize();

            if (!positionData || !skinWeightData || !skinIndexData)
            {
                console.warn('HotspotManager cpuskinning: the mesh does not have necessary streams!');
                return;
            }

            var mat4MulVec4 = function (mat4, vec4, factor) {
                var x = vec4.x;
                var y = vec4.y;
                var z = vec4.z;
                var w = vec4.w;

                var e = mat4.elements;

                vec4.x = (e[0] * x + e[4] * y + e[8] * z + e[12] * w) * factor;
                vec4.y = (e[1] * x + e[5] * y + e[9] * z + e[13] * w) * factor;
                vec4.z = (e[2] * x + e[6] * y + e[10] * z + e[14] * w) * factor;
                vec4.w = (e[3] * x + e[7] * y + e[11] * z + e[15] * w) * factor;

                return vec4;
            }

            for (var j = 0; j < face.length; ++j) {
                var k = face[j] * skinIndexItemSize;
                var i = face[j] * positionItemSize;

                tmp_0_0.x = positionData[i + 0];
                tmp_0_0.y = positionData[i + 1];
                tmp_0_0.z = positionData[i + 2];
                tmp_0_0.w = 1.0;

                tmp_0_1.x = positionData[i + 0];
                tmp_0_1.y = positionData[i + 1];
                tmp_0_1.z = positionData[i + 2];
                tmp_0_1.w = 1.0;

                tmp_0_2.x = positionData[i + 0];
                tmp_0_2.y = positionData[i + 1];
                tmp_0_2.z = positionData[i + 2];
                tmp_0_2.w = 1.0;

                tmp_0_3.x = positionData[i + 0];
                tmp_0_3.y = positionData[i + 1];
                tmp_0_3.z = positionData[i + 2];
                tmp_0_3.w = 1.0;

                tmp_1.x = skinWeightData[k + 0];
                tmp_1.y = skinWeightData[k + 1];
                tmp_1.z = skinWeightData[k + 2];
                tmp_1.w = skinWeightData[k + 3];

                tmp_2.x = skinIndexData[k + 0];
                tmp_2.y = skinIndexData[k + 1];
                tmp_2.z = skinIndexData[k + 2];
                tmp_2.w = skinIndexData[k + 3];

                mat4MulVec4(boneMatrices[tmp_2.x], tmp_0_0, tmp_1.x);
                mat4MulVec4(boneMatrices[tmp_2.y], tmp_0_1, tmp_1.y);
                mat4MulVec4(boneMatrices[tmp_2.z], tmp_0_2, tmp_1.z);
                mat4MulVec4(boneMatrices[tmp_2.w], tmp_0_3, tmp_1.w);

                tmp_0_4.x = tmp_0_0.x + tmp_0_1.x + tmp_0_2.x + tmp_0_3.x;
                tmp_0_4.y = tmp_0_0.y + tmp_0_1.y + tmp_0_2.y + tmp_0_3.y;
                tmp_0_4.z = tmp_0_0.z + tmp_0_1.z + tmp_0_2.z + tmp_0_3.z;
                tmp_0_4.w = 1.0;

                skinnedPosition[j].copy(tmp_0_4);
            }

            output.set(0, 0, 0);
            output.addScaledVector(skinnedPosition[0], barycoord.x).addScaledVector(skinnedPosition[1], barycoord.y).addScaledVector(skinnedPosition[2], barycoord.z);
        };
    })(),

    _cpuMorphingVertex: (function () {
        var tmpArr = [];
        var pA = new GASEngine.Vector3(0, 0, 0);
        var pB = new GASEngine.Vector3(0, 0, 0);
        var pC = new GASEngine.Vector3(0, 0, 0);
        var activateMorphWeights = new Float32Array(4);
        var activateMorphTargets = [null, null, null, null];
        var positionData, m, n, morphPositionData, morphPositionItemSize, positionItemSize;
        var meshFilterComponent, mesh, matrixWorld;

        return function(entity, face, barycoord, output)
        {
            meshFilterComponent = entity.getComponent('meshFilter');
            mesh = meshFilterComponent.getMesh();
            positionData = mesh.getStream('position');

            positionItemSize = mesh.getVertexElementCount();
           
            mesh.sortMorphWeights(activateMorphWeights, activateMorphTargets);
            
            pA.fromArray(positionData, face[0] * positionItemSize);
            pB.fromArray(positionData, face[1] * positionItemSize);
            pC.fromArray(positionData, face[2] * positionItemSize);

            for(var i = 0; i < 9; i++) {
                tmpArr[i] = 0;
            }

            for (var i = 0; i < activateMorphTargets.length; ++i)
            {
                if (activateMorphTargets[i] !== null)
                {
                    morphPositionData = activateMorphTargets[i].getStream('position');
                    morphPositionItemSize = activateMorphTargets[i].getVertexElementCount();
    
                    for(var j = 0; j < face.length; j++)
                    {
                        m = face[j] * morphPositionItemSize;
                        n = face[j] * positionItemSize;

                        tmpArr[j * 3 + 0] += (morphPositionData[m + 0] - positionData[n + 0]) * activateMorphWeights[i];
                        tmpArr[j * 3 + 1] += (morphPositionData[m + 1] - positionData[n + 1]) * activateMorphWeights[i];
                        tmpArr[j * 3 + 2] += (morphPositionData[m + 2] - positionData[n + 2]) * activateMorphWeights[i];
                    }
                }
            }

            pA.x += tmpArr[0];
            pA.y += tmpArr[1];
            pA.z += tmpArr[2];

            pB.x += tmpArr[3];
            pB.y += tmpArr[4];
            pB.z += tmpArr[5];

            pC.x += tmpArr[6];
            pC.y += tmpArr[7];
            pC.z += tmpArr[8];

            output.set(0,0,0);
            output.addScaledVector(pA, barycoord.x).addScaledVector(pB, barycoord.y).addScaledVector(pC, barycoord.z);
            
            matrixWorld = entity.matrixWorld;
            output.applyMatrix4(matrixWorld);
        }
    })()

    
};