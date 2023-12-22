GASEngine.GLTFLoader = function ()
{
    GASEngine.Events.attach(this);

    this._gltfMaterialDecoder_ = new GASEngine.GLTFMaterialDecoder();
    this._gltfMeshDecoder_ = new GASEngine.GLTFMeshDecoder();
    this._gltfAnimationDecoder_ = new GASEngine.GLTFKeyframeAnimationDecoder();

    //Accessor
    this._binaryBuffer = null;
    this._bufferObjects_ = new Map();
    this._bufferViewObjects_ = new Map();
    this._accessorObjects_ = new Map();

    //For linkage
    this._textureObjects_ = new Map();
    this._materialObjects_ = new Map();
    this._skinObjects_ = new Map();

    this._meshObjects_ = new Map();
    this._meshFilterComponents_ = new Map();
    this._meshRendererComponents_ = new Map();
    this._animatorComponent_ = null;
    this._animationClips_ = new Map();

    this._nodeObjects_ = new Map();
    this._sceneObjects_ = new Map();
    this._cameraObjects_ = new Map();

    this._scene_ = null;
    this._onSuccess_ = null;
    this._onFinished_ = null;

    GASEngine.GLTFLoader.Instance = this;
};

GASEngine.GLTFLoader.prototype = {
    constructor: GASEngine.GLTFLoader,

    load: function (modelName, onSuccess)
    {
        this._onSuccess_ = onSuccess;

        GASEngine.FileSystem.Instance.read
            (
                modelName,
                function (data)
                {
                   this.parse(data, onSuccess);
                }.bind(this),
                null,
                function (path, ext) {
                }.bind(this)
            );
    },

    parse: function (data, onSuccess) {
        this._onSuccess_ = onSuccess;

        if (data instanceof ArrayBuffer) //GLB
        {
            var magic = GASEngine.Utilities.decodeText(new Uint8Array(data, 0, 4));

            var content;
            if (magic === GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_MAGIC)
            {
                var result = this._readBinary(data);
                content = result.content;
                this._binaryBuffer = result.buffer;
            }
            else
            {
                content = GASEngine.Utilities.decodeText(new Uint8Array(data));
            }

            if(content !== undefined) {
                this._json = JSON.parse(content);

                //this._json is not Object
                if (this._json instanceof Object) {

                    //Load all buffer
                    if(this._json.buffers instanceof Array) {
                        for (var bufferIndex = 0; bufferIndex < this._json.buffers.length; bufferIndex++)
                        {
                            this._loadBuffer(bufferIndex, function ()
                            {
                                if (this._bufferObjects_.size === this._json.buffers.length)
                                {
                                    this._parse();
                                }
                            }.bind(this));
                        }
                    }
                }
            }
        }
        else { //parse gltf
            if(typeof data === 'string') {
                this._json = JSON.parse(data);
            }
            else {
                this._json = data;
            }

            //this._json MUST be Object
            if (this._json instanceof Object) {

                //Load all buffer
                for (var bufferIndex = 0; bufferIndex < this._json.buffers.length; bufferIndex++)
                {
                    this._loadBuffer(bufferIndex, function ()
                    {
                        // loaded all buffers 
                        if (this._bufferObjects_.size === this._json.buffers.length)
                        {
                            this._parse();
                        }
                    }.bind(this));
                }
            }
        }
    },

    _readBinary: function (data) {
		var headerView = new DataView( data, 0, GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_LENGTH );

		var header = {
			magic: GASEngine.Utilities.decodeText( new Uint8Array( data.slice( 0, 4 ) ) ),
			version: headerView.getUint32( 4, true ),
			length: headerView.getUint32( 8, true )
		};

		if (header.magic !== GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_MAGIC ) {
			console.error( 'GASEngine.GLTFLoader: Unsupported glTF-Binary header.' );
		} else if ( header.version < 2.0 ) {
			console.error('GASEngine.GLTFLoader: Legacy binary file detected. Use LegacyGLTFLoader instead.' );
		}

		var chunkView = new DataView( data, GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_LENGTH );
        var chunkIndex = 0;
        var content, buffer;

		while ( chunkIndex < chunkView.byteLength ) {

			var chunkLength = chunkView.getUint32( chunkIndex, true );
			chunkIndex += 4;

			var chunkType = chunkView.getUint32( chunkIndex, true );
			chunkIndex += 4;

			if ( chunkType === GASEngine.GLTFConsts.BINARY_EXTENSION_CHUNK_TYPES.JSON ) {

				var contentArray = new Uint8Array( data, GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_LENGTH + chunkIndex, chunkLength );
				content = GASEngine.Utilities.decodeText( contentArray );

			} else if ( chunkType === GASEngine.GLTFConsts.BINARY_EXTENSION_CHUNK_TYPES.BIN ) {

				var byteOffset = GASEngine.GLTFConsts.BINARY_EXTENSION_HEADER_LENGTH + chunkIndex;
				buffer = data.slice( byteOffset, byteOffset + chunkLength );
			}

			// Clients must ignore chunks with unknown types.
			chunkIndex += chunkLength;
		}

        return {'content': content, 'buffer': buffer };
    },

    _parse: function ()
    {
        var index = 0;
        var il = 0;
        var json = this._json;

        //Load all bufferView
        for (index = 0, il = json.bufferViews.length; index < il; index++)
        {
            this._loadBufferView(index);
        }

        //Load all accessor
        for (index = 0, il = json.accessors.length; index < il; index++)
        {
            this._loadAccessor(index);
        }

        //Load Texture config
        if (json.textures !== undefined)
        {
            for (index = 0, il = json.textures.length; index < il; index++)
            {
                this._loadTexture(index);
            }
        }

        //Load Materials
        for (index = 0, il = json.materials.length; index < il; index++) 
        {
            this._loadMaterial(index);
        }

        //Load Mesh
        for (index = 0, il = json.meshes.length; index < il; index++) 
        {
            this._loadMesh(index);
        }

        //Load Skin
        if (json.skins !== undefined)
        {
            for (index = 0, il = json.skins.length; index < il; index++)
            {
                this._loadSkin(index);
            }
        }

        //Load Camera
        if (json.cameras !== undefined)
        {
            for (index = 0, il = json.cameras.length; index < il; index++)
            {
                this._loadCamera(index);
            }
        }

        //Load Light
        if (json.lights !== undefined)
        {
            for (index = 0, il = json.lights.length; index < il; index++)
            {
                this._loadLight(index);
            }
        }

        //Load Node
        if (json.nodes !== undefined)
        {
            for (index = 0, il = json.nodes.length; index < il; index++)
            {
                this._loadNode(index);
            }
        }

        //Load Scenes
        for (index = 0, il = json.scenes.length; index < il; index++)
        {
            this._loadScene(index);
        }

        //Load Animation Binaries
        if (json.animations !== undefined)
        {
            for (index = 0, il = json.animations.length; index < il; index++) 
            {
                this._loadAnimation(index);
            }

            this._linkScene();
        }

        if (this._sceneObjects_.size > 0)
        {
            this._scene_ = this._sceneObjects_.get(json.scene || 0);
            this._onSuccess_(this._scene_);
        }
    },

    _loadBuffer: function (bufferIndex, onSuccess)
    {
        var bufferDef = this._json.buffers[bufferIndex];
        if (bufferDef.type && bufferDef.type !== 'arraybuffer')
        {
            console.error('GASEngine.GLTFLoader: buffer' + bufferIndex + ' : ' + bufferDef.type + ' buffer type is not supported.');
            return;
        }

        // If present, GLB container is required to be the first buffer.
		if ( bufferDef.uri === undefined && bufferIndex === 0 ) {
            this._bufferObjects_.set(bufferIndex, this._binaryBuffer);
            onSuccess(this._binaryBuffer);
            return;
		}

        GASEngine.FileSystem.Instance.read
            (
                bufferDef.uri,

                function (data, ext)
                {
                    var view = new Uint8Array(data);
                    this._bufferObjects_.set(bufferIndex, view.buffer);
                    onSuccess(view.buffer);

                }.bind(this),

                null,

                function (path, ext)
                {
                }.bind(this)
            );
    },

    _loadBufferView: function (bufferViewIndex)
    {
        if (this._bufferObjects_.size === 0)
        {
            console.error('GASEngine.GLTFLoader._loadAccessor : There are no buffers.');
            return;
        }

        var bufferViewDef = this._json.bufferViews[bufferViewIndex];

        var buffer = this._bufferObjects_.get(bufferViewDef.buffer);
        if (buffer)
        {
            var byteLength = bufferViewDef.byteLength || 0;
            var byteOffset = bufferViewDef.byteOffset || 0;
            var bufferView = buffer.slice(byteOffset, byteOffset + byteLength);
            this._bufferViewObjects_.set(bufferViewIndex, bufferView);
        }
        else {
            console.error('GASEngine.GLTFLoader._loadAccessor : bufferViewIndex ' + bufferViewIndex + "not exist");
        }
    },

    _loadAccessor: function (accessorIndex)
    {
        if (this._bufferViewObjects_.size === 0)
        {
            console.error('GASEngine.GLTFLoader._loadAccessor : There are no bufferViews.');
            return;
        }

        var json = this._json;

        var accessorDef = json.accessors[accessorIndex];

        if (accessorDef.bufferView === undefined && accessorDef.sparse === undefined)
        {
            // Ignore empty accessors, which may be used to declare runtime
            // information about attributes coming from another source (e.g. Draco
            // compression extension).
            return;
        }

        var bufferViews = [];
        if (accessorDef.bufferView !== undefined)
        {
            bufferViews.push(this._bufferViewObjects_.get(accessorDef.bufferView));
        }
        else
        {
            bufferViews.push(null);
        }

        if (accessorDef.sparse !== undefined)
        {
            console.error('GASEngine.GLTFLoader : TODO: parse accessorDef.sparse ');
            // bufferViews.push(this._bufferViewObjects_.get(accessorDef.sparse.indices.bufferView));
            // bufferViews.push(this._bufferViewObjects_.get(accessorDef.sparse.sparse.values.bufferView));
        }

        // prepare accessor from bufferviews
        var bufferView = bufferViews[0];

        var itemSize = GASEngine.GLTFConsts.WEBGL_TYPE_SIZES[accessorDef.type];
        var TypedArray = GASEngine.GLTFConsts.WEBGL_COMPONENT_TYPES[accessorDef.componentType];

        // For VEC3: itemSize is 3, elementBytes is 4, itemBytes is 12.
        var elementBytes = TypedArray.BYTES_PER_ELEMENT;
        var itemBytes = elementBytes * itemSize;
        var byteOffset = accessorDef.byteOffset || 0;
        var byteStride = accessorDef.bufferView !== undefined ? json.bufferViews[accessorDef.bufferView].byteStride : undefined;
        //var normalized = accessorDef.normalized === true;

        var array;

        // The buffer is not interleaved if the stride is the item size in bytes.
        if (byteStride && byteStride !== itemBytes)
        {
            //TODO: support interleaved buffer
            console.error("GASEngine.GLTFLoader : TODO: The buffer is interleaved.")
        } else
        {
            if (bufferView === null)
            {
                array = new TypedArray(accessorDef.count * itemSize);
            } else
            {
                array = new TypedArray(bufferView, byteOffset, accessorDef.count * itemSize);
            }
        }

        this._accessorObjects_.set(accessorIndex, array);
        return array;
    },

    _buildNodeHierachy: function (nodeId, parentObject)
    {
        var node = this._nodeObjects_.get(nodeId);
        var nodeDef = this._json.nodes[nodeId];

        // build skeleton here as well
        if (nodeDef && nodeDef.skin !== undefined)
        {
            var meshEntities = node.getComponent('meshFilter') ? [node] : node.children;

            for (var i = 0, il = meshEntities.length; i < il; i++)
            {
                var meshFilter = meshEntities[i].getComponent('meshFilter');
                if (!meshFilter)
                    continue;

                var mesh = meshFilter.getMesh();
                if (!mesh)
                    continue;

                var skinEntry = this._skinObjects_.get(nodeDef.skin);

                var bones = [];

                for (var j = 0, jl = skinEntry.joints.length; j < jl; j++)
                {
                    var jointId = skinEntry.joints[j];
                    var jointNode = this._nodeObjects_.get(jointId);

                    if (jointNode)
                    {
                        var bone = {
                            "uniqueID": jointNode.uniqueID,
                            "name": jointNode.name
                        };

                        var mat = new GASEngine.Matrix4();
                        if (skinEntry.inverseBindMatrices !== undefined)
                        {
                            mat.fromArray(skinEntry.inverseBindMatrices, j * 16);
                            mat.transpose();
                        }
                        bone.matrixWorld2Bone = mat.elements;
                        bones.push(bone);
                    }
                    else
                    {
                        console.warn('GASEngine.GLTFLoader: Joint "%s" could not be found.', jointId);
                    }
                }
                mesh.addStream('bone', bones);
            }
        }

        // build node hierachy
        parentObject.addChild(node);

        if (nodeDef.children)
        {
            var children = nodeDef.children;

            for (var i = 0, il = children.length; i < il; i++)
            {
                var child = children[i];
                this._buildNodeHierachy(child, node);
            }
        }
    },

    _loadScene: function (sceneIndex)
    {
        var sceneDef = this._json.scenes[sceneIndex];

        var scene = new GASEngine.Scene();
        if (sceneDef.name !== undefined)
            scene.name = sceneDef.name;

        var rootEntity = scene.root;

        //create animator component
        if (this._json.animations && this._json.animations.length > 0)
        {
            var animatorComponent = GASEngine.ComponentFactory.Instance.create('animator', -1);
            rootEntity.addComponent(animatorComponent);

            this._animatorComponent_ = animatorComponent;
        }

        //build node hierachy
        var nodeIds = sceneDef.nodes || [];

        for (var i = 0, il = nodeIds.length; i < il; i++)
        {
            this._buildNodeHierachy(nodeIds[i], rootEntity);
        }
        this._linkScene();
        this._sceneObjects_.set(sceneIndex, scene);
    },

    _loadMesh: function (meshIndex)
    {
        var meshDef = this._json.meshes[meshIndex];

        var meshObject = this._gltfMeshDecoder_.parse(meshDef, this._accessorObjects_);
        if (meshObject)
        {
            var meshEntity = GASEngine.EntityFactory.Instance.create();
            if (meshObject instanceof Array)
            {
                for (var i = 0; i < meshObject.length; i++)
                {
                    var mesh = meshObject[i];
                    var entity = GASEngine.EntityFactory.Instance.create();
                    entity.name = mesh.name;
                    meshEntity.addChild(entity);

                    var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter');
                    meshFilter.uniqueID = GASEngine.generateUUID();
                    var positions = mesh.getStream('position'); //compute AABB
                    if(positions !== undefined) {
                        var minMax = GASEngine.Utilities.getMinMax(positions, 3, 0, positions.length / 3);
                        meshFilter.bbox = new GASEngine.AABB();
                        meshFilter.bbox.min.set(minMax.min[0], minMax.min[1], minMax.min[2]);
                        meshFilter.bbox.max.set(minMax.max[0], minMax.max[1], minMax.max[2]);
                    }
                    entity.addComponent(meshFilter);
                    meshFilter.setMesh(mesh);

                    this._meshFilterComponents_.set(meshFilter, mesh);

                    var materialIndex = meshDef.primitives[i].material;
                    if (materialIndex !== undefined)
                    {
                        var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer');
                        meshRenderer.uniqueID = GASEngine.generateUUID();
                        entity.addComponent(meshRenderer);

                        this._meshRendererComponents_.set(meshRenderer, materialIndex);
                    }
                }
            }
            else
            {
                meshEntity.name = meshObject.name;
                var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter');
                meshFilter.uniqueID = GASEngine.generateUUID();
                var positions = meshObject.getStream('position'); //compute AABB
                if(positions !== undefined) {
                    var minMax = GASEngine.Utilities.getMinMax(positions, 3, 0, positions.length / 3);
                    meshFilter.bbox = new GASEngine.AABB();
                    meshFilter.bbox.min.set(minMax.min[0], minMax.min[1], minMax.min[2]);
                    meshFilter.bbox.max.set(minMax.max[0], minMax.max[1], minMax.max[2]);
                }
                meshEntity.addComponent(meshFilter);
                meshFilter.setMesh(meshObject);

                this._meshFilterComponents_.set(meshFilter, meshObject);

                var materialIndex = meshDef.primitives[0].material;
                if (materialIndex !== undefined)
                {
                    var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer');
                    meshRenderer.uniqueID = GASEngine.generateUUID();
                    meshEntity.addComponent(meshRenderer);

                    this._meshRendererComponents_.set(meshRenderer, materialIndex);
                }
            }
            this._meshObjects_.set(meshIndex, meshEntity);
        }
    },

    _loadNode: function (nodeIndex)
    {
        var json = this._json;

        var nodeDef = json.nodes[nodeIndex];
        var entity;

        if (nodeDef.mesh !== undefined)
        {
            entity = this._meshObjects_.get(nodeDef.mesh);
        }
        else if (nodeDef.camera !== undefined)
        {
            entity = this._cameraObjects_.get(nodeDef.camera);
        }
        else
        {
            entity = GASEngine.EntityFactory.Instance.create();
        }

        if (nodeDef.name !== undefined)
        {
            entity.name = GASEngine.Utilities.sanitizeNodeName(nodeDef.name);
        }

        if (nodeDef.matrix !== undefined)
        {
            var matrix = new GASEngine.Matrix4();
            matrix.fromArray(nodeDef.matrix);
            entity.setLocalMatrix(matrix);
        }
        else
        {
            if (nodeDef.translation !== undefined)
            {
                var localTranslation = new GASEngine.Vector3();
                localTranslation.fromArray(nodeDef.translation);
                entity.setLocalTranslation(localTranslation);
            }

            if (nodeDef.rotation !== undefined)
            {
                var localQuaternion = new GASEngine.Quaternion();
                localQuaternion.fromArray(nodeDef.rotation);
                entity.setLocalQuaternion(localQuaternion);
            }

            if (nodeDef.scale !== undefined)
            {
                var localScale = new GASEngine.Vector3();
                localScale.fromArray(nodeDef.scale);
                entity.setLocalScale(localScale);
            }
        }
        this._nodeObjects_.set(nodeIndex, entity);
    },

    _loadAnimation: function (animationIndex)
    {
        var animationDef = this._json.animations[animationIndex];

        var animationClip = this._gltfAnimationDecoder_.parse(animationDef, this._accessorObjects_, this._nodeObjects_);

        this._animationClips_.set(animationClip.clipID, animationClip);
    },

    _linkScene: function ()
    {
        //link bone
        for (var meshFilter of this._meshFilterComponents_.keys()) 
        {
            var mesh = this._meshFilterComponents_.get(meshFilter);
            if (mesh !== undefined)
            {
                mesh.submitToWebGL();

                if (mesh.isSkinned())
                {
                    var meshParentEntity = meshFilter.getParentEntity();
                    mesh.linkBones(meshParentEntity);
                }

                var bones = mesh.getBones();
                //GASEngine.SkeletonManager.Instance.appendBones(bones);                
            }
        }
        this._meshFilterComponents_.clear();

        //link material
        for (var meshRenderer of this._meshRendererComponents_.keys()) 
        {
            var materialIndex = this._meshRendererComponents_.get(meshRenderer);
            var materialObject = this._materialObjects_.get(materialIndex);
            if (materialObject)
            {
                  meshRenderer.addMaterial(materialObject);
            }
            else
            {
                console.warn('GASEngine.GLTFLoader._linkScene : material ' + materialIndex + 'not exist or not loaded yet.');
            }
        }
        this._meshRendererComponents_.clear();

        //link animation
        if (this._animatorComponent_ !== null && this._animationClips_.size > 0)
        {
            for (var clip of this._animationClips_.values()) 
            {
                this._animatorComponent_.setAnimationClip(clip.name, clip);
                this._animatorComponent_.play(clip.id);
            }
            this._animationClips_.clear();
        }
    },

    _loadTexture: function (textureIndex)
    {
        var textureDef = this._json.textures[textureIndex];
        var source = this._json.images[textureDef.source];

        if (source.bufferView !== undefined) {

            var bufferView = this._bufferViewObjects_.get(source.bufferView);
            if (bufferView) {
                var blob = new Blob( [ bufferView ], { type: source.mimeType } );
                var sourceURI = URL.createObjectURL(blob);

                var textureObject = {
                    "sampler": textureDef.sampler,
                    "uri": sourceURI
                }
                this._textureObjects_.set(textureIndex, textureObject);
            }
            else {
                console.error('GASEngine.GLTFLoader: _loadTexture : failed to get source.bufferView accessor');
            }
        }
        else {
            var textureObject = {
                "sampler": textureDef.sampler,
                "uri": source.uri
            }
            this._textureObjects_.set(textureIndex, textureObject);
        }
    },

    _loadMaterial: function (materialIndex)
    {
        var materialDef = this._json.materials[materialIndex];

        var materialObject = this._gltfMaterialDecoder_.parse(materialDef, this._textureObjects_);
        this._materialObjects_.set(materialIndex, materialObject);

        if (materialObject)
        {
            var maps = [];
            materialObject.getLinkedTextureMaps(maps);
            for (var j = 0; j < maps.length; ++j)
            {
                GASEngine.Resources.Instance.loadTextureOnMaterial(maps[j]);
            }
        }
        else
        {
            console.warn('GASEngine.GLTFLoader._loadMaterial : material ' + materialIndex + 'parse failed.');
        }
    },

    _loadSkin: function (skinIndex)
    {
        if (this._accessorObjects_.size == 0)
            return;

        var skinDef = this._json.skins[skinIndex];

        var skinEntry = { joints: skinDef.joints };

        if (skinDef.inverseBindMatrices !== undefined)
        {
            var accessor = this._accessorObjects_.get(skinDef.inverseBindMatrices);
            if (accessor)
            {
                skinEntry.inverseBindMatrices = accessor;
            }
        }
        this._skinObjects_.set(skinIndex, skinEntry);
    },

    _loadCamera: function (cameraIndex)
    {
        var camera;
        var cameraDef = this._json.cameras[cameraIndex];
        var params = cameraDef[cameraDef.type];

        if ( ! params ) {
			console.warn( 'GASEngine.GLTFLoader: Missing camera parameters.' );
			return;
		}

		if ( cameraDef.type === 'perspective' ) {
			camera = GASEngine.EntityFactory.Instance.create();
		} else if ( cameraDef.type === 'orthographic' ) {
			camera = GASEngine.EntityFactory.Instance.create();
		}

        if ( cameraDef.name !== undefined )
            camera.name = cameraDef.name;
        
        this._cameraObjects_.set(cameraIndex, camera);
    },

    _loadLight: function (lightIndex)
    {

    }
};