GASEngine.GLTFExporter = function ()
{
    GASEngine.Events.attach(this);

    this.cachedCanvas = null;
    this.byteOffset = 0;
    this._skinsObjects_ = [];
    this._bufferObjects_ = [];
    this._nodeObjects_ = new Map();
    this._materialObjects_ = new Map();
    this._textureObjects = new Map();
    this._imageObjects = new Map();

    this._onFinished_ = null;
    //for binary
    this._binaryImages = new Map();

    GASEngine.GLTFExporter.Instance = this;
};

GASEngine.GLTFExporter.prototype = {
    constructor: GASEngine.GLTFExporter,

    parse: function (input, onFinished, options)
    {
        var DEFAULT_OPTIONS = {
            binary: false,
            embedImages: true,
            forcePowerOfTwoTextures: false,
            trs: true
        };

        this.byteOffset = 0;
        this._onFinished_ = onFinished;
        this.options = Object.assign({}, DEFAULT_OPTIONS, options);

        this.outputJSON = {
            asset: {
                version: "2.0",
                generator: "GASEngine.GLTFExporter"
            }
        };

        if (input instanceof GASEngine.Scene)
        {
            this.processScene(input);

            for (var i = 0; i < this._skinsObjects_.length; ++i) {
                this.processSkin(this._skinsObjects_[i]);
            }

            for (var clip of input._activeAnimationClips) {
                this.processAnimation(clip);
            }

            this.onFinishedParse();
        }
    },

    onFinishedParse() {
        if(this._binaryImages.size > 0) //wait binary images
            return;

        if(this._onFinished_ === null) {
            console.error('GASEngine.GLTFExporter.onFinishedParse: onFinished is necessary.');
            return;
        }

        // Merge buffers.
        var blob = new Blob(this._bufferObjects_, { type: 'application/octet-stream' });

        // Buffer exist
        if (this.outputJSON.buffers && this.outputJSON.buffers.length > 0)
        {
            // Update bytelength of the single buffer.
            this.outputJSON.buffers[0].byteLength = blob.size;
            var reader = new window.FileReader();

            if (this.options.binary)
            {
                var GLB_HEADER_BYTES = 12;
                var GLB_HEADER_MAGIC = 0x46546C67;
                var GLB_VERSION = 2;

                var GLB_CHUNK_PREFIX_BYTES = 8;
                var GLB_CHUNK_TYPE_JSON = 0x4E4F534A;
                var GLB_CHUNK_TYPE_BIN = 0x004E4942;

                reader.readAsArrayBuffer(blob);
                reader.onloadend = function ()
                {
                    // Binary chunk.
                    var binaryChunk = GASEngine.Utilities.getPaddedArrayBuffer(reader.result);
                    var binaryChunkPrefix = new DataView(new ArrayBuffer(GLB_CHUNK_PREFIX_BYTES));
                    binaryChunkPrefix.setUint32(0, binaryChunk.byteLength, true);
                    binaryChunkPrefix.setUint32(4, GLB_CHUNK_TYPE_BIN, true);

                    // JSON chunk.
                    var jsonChunk = GASEngine.Utilities.getPaddedArrayBuffer(GASEngine.Utilities.stringToArrayBuffer(JSON.stringify(this.outputJSON)), 0x20);
                    var jsonChunkPrefix = new DataView(new ArrayBuffer(GLB_CHUNK_PREFIX_BYTES));
                    jsonChunkPrefix.setUint32(0, jsonChunk.byteLength, true);
                    jsonChunkPrefix.setUint32(4, GLB_CHUNK_TYPE_JSON, true);

                    // GLB header.
                    var header = new ArrayBuffer(GLB_HEADER_BYTES);
                    var headerView = new DataView(header);
                    headerView.setUint32(0, GLB_HEADER_MAGIC, true);
                    headerView.setUint32(4, GLB_VERSION, true);
                    var totalByteLength = GLB_HEADER_BYTES
                        + jsonChunkPrefix.byteLength + jsonChunk.byteLength
                        + binaryChunkPrefix.byteLength + binaryChunk.byteLength;
                    headerView.setUint32(8, totalByteLength, true);

                    var glbBlob = new Blob([
                        header,
                        jsonChunkPrefix,
                        jsonChunk,
                        binaryChunkPrefix,
                        binaryChunk
                    ], { type: 'application/octet-stream' });

                    var glbReader = new window.FileReader();
                    glbReader.readAsArrayBuffer(glbBlob);
                    glbReader.onloadend = function ()
                    {
                        this._onFinished_(glbReader.result);
                        this._onFinished_ = null;
                    }.bind(this);
                }.bind(this);
            }
            else
            {
                reader.readAsDataURL(blob);
                reader.onloadend = function ()
                {
                    var base64data = reader.result;
                    this.outputJSON.buffers[0].uri = base64data;
                    this._onFinished_(this.outputJSON);
                    this._onFinished_ = null;
                }.bind(this);
            }
        }
        else
        {
            this._onFinished_(this.outputJSON);
            this._onFinished_ = null;
        }
    },

    processBuffer: function (buffer)
    {

        if (!this.outputJSON.buffers)
        {
            this.outputJSON.buffers = [{ byteLength: 0 }];
        }
        this._bufferObjects_.push(buffer);
        return 0;
    },

    processBufferViewImage: function(blob, onSuccess) {
        if ( ! this.outputJSON.bufferViews ) {
            this.outputJSON.bufferViews = [];
        }

        var reader = new window.FileReader();
        reader.readAsArrayBuffer(blob);
        reader.onloadend = function() {
            var buffer = GASEngine.Utilities.getPaddedArrayBuffer(reader.result);

            var bufferView = {
                buffer: this.processBuffer(buffer),
                byteOffset : this.byteOffset,
                byteLength : buffer.byteLength
            };

            this.byteOffset += buffer.byteLength;
            this.outputJSON.bufferViews.push(bufferView);
            onSuccess(this.outputJSON.bufferViews.length - 1);
        }.bind(this);
    },

    processBufferView: function (array, itemSize, componentType, start, count, target)
    {
        if (!this.outputJSON.bufferViews)
        {
            this.outputJSON.bufferViews = [];
        }

        // Create a new dataview and dump the attribute's array into it
        var componentSize;
        if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_BYTE)
        {
            componentSize = 1;
        } else if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_SHORT)
        {
            componentSize = 2;
        } else
        {
            componentSize = 4;
        }

        var byteLength = GASEngine.Utilities.getPaddedBufferSize(count * itemSize * componentSize);
        var dataView = new DataView(new ArrayBuffer(byteLength));
        var offset = 0;

        for (var i = start; i < start + count; i++)
        {
            for (var a = 0; a < itemSize; a++)
            {
                var value = array[i * itemSize + a];

                if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.FLOAT)
                {
                    dataView.setFloat32(offset, value, true);
                } else if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_INT)
                {
                    dataView.setUint32(offset, value, true);
                } else if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_SHORT)
                {
                    dataView.setUint16(offset, value, true);
                } else if (componentType === GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_BYTE)
                {
                    dataView.setUint8(offset, value);
                }
                offset += componentSize;
            }
        }

        var gltfBufferView = {

            buffer: this.processBuffer(dataView.buffer),
            byteOffset: this.byteOffset,
            byteLength: byteLength
        };

        if (target !== undefined) gltfBufferView.target = target;

        if (target === GASEngine.GLTFConsts.WEBGL_CONSTANTS.ARRAY_BUFFER)
        {
            // Only define byteStride for vertex attributes.
            gltfBufferView.byteStride = itemSize * componentSize;

        }
        this.byteOffset += byteLength;
        this.outputJSON.bufferViews.push(gltfBufferView);

        var output = {
            id: this.outputJSON.bufferViews.length - 1,
            byteOffset: 0
        };
        return output;
    },

    processAccessor: function (array, itemSize, start, count)
    {
        var types = {
            1: 'SCALAR',
            2: 'VEC2',
            3: 'VEC3',
            4: 'VEC4',
            16: 'MAT4'
        };
        var componentType;

        // Detect the component type of the attribute array (float, uint or ushort)
        if (array instanceof Float32Array)
        {
            componentType = GASEngine.GLTFConsts.WEBGL_CONSTANTS.FLOAT;
        } else if (array instanceof Uint32Array)
        {
            componentType = GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_INT;
        } else if (array instanceof Uint16Array)
        {
            componentType = GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_SHORT;
        } else if (array instanceof Uint8Array)
        {
            componentType = GASEngine.GLTFConsts.WEBGL_CONSTANTS.UNSIGNED_BYTE;
        } else
        {
            console.error('GASEngine.GLTFExporter: Unsupported bufferAttribute component type.');
            return;
        }

        if (start === undefined) start = 0;
        if (count === undefined)
        {
            count = array.length / itemSize;
        }

        // Skip creating an accessor if the attribute doesn't have data to export
        if (count === 0)
            return null;

        var minMax = GASEngine.Utilities.getMinMax(array, itemSize, start, count);
        var bufferView = this.processBufferView(array, itemSize, componentType, start, count);

        var gltfAccessor = {

            bufferView: bufferView.id,
            byteOffset: bufferView.byteOffset,
            componentType: componentType,
            count: count,
            max: minMax.max,
            min: minMax.min,
            type: types[itemSize]
        };

        if (!this.outputJSON.accessors)
        {
            this.outputJSON.accessors = [];
        }

        this.outputJSON.accessors.push(gltfAccessor);
        return this.outputJSON.accessors.length - 1;
    },

    processImage: function (image, flipY)
    {
        var cachedImage = this._imageObjects.get(image);

        if (cachedImage !== undefined)
        {
            return cachedImage;
        }

        if (!this.outputJSON.images)
        {
            this.outputJSON.images = [];
        }

        var dataUriRegex = /^data:(.*?)(;base64)?,(.*)$/;
        var dataUriRegexResult = image.match(dataUriRegex);

        var mimeType = 'image/png';
        var isBlobImage = /^blob:.*$/i.test(image);

        if (dataUriRegexResult)
        {   //data: URI
            mimeType = dataUriRegexResult[1];
        }
        else {
            if (image.lastIndexOf('.jpg') >= 0 || image.lastIndexOf('.jpeg') >= 0)
                mimeType = 'image/jpeg';
            else if(image.lastIndexOf('.png') >= 0)
                mimeType = 'image/png';
        }

        var gltfImage = { mimeType: mimeType };

        if (dataUriRegexResult) {
            gltfImage.uri = image; //directly assign to uri
        }
        else {
            if(!isBlobImage) {
                var lastIndex = image.lastIndexOf('/');
                if (lastIndex > 0)
                    gltfImage.name = image.substring(lastIndex);
                else
                    gltfImage.name = image;
            }
    
            if (this.options.embedImages || isBlobImage)
            {
                var imageObject = GASEngine.Resources.Instance.imageCache.get(image);
                if (imageObject !== null)
                {
                    var canvas = this.cachedCanvas = this.cachedCanvas || document.createElement('canvas');
                    canvas.width = imageObject.width;
                    canvas.height = imageObject.height;

                    if (this.options.forcePowerOfTwoTextures && !GASEngine.Utilities.isPowerOfTwo(record))
                    {
                        console.warn('GLTFExporter: Resized non-power-of-two image.', image);
                        canvas.width = GASEngine.floorPowerOfTwo(canvas.width);
                        canvas.height = GASEngine.floorPowerOfTwo(canvas.height);
                    }
                    var ctx = canvas.getContext('2d');
                    if (flipY === true)
                    {
                        ctx.translate(0, canvas.height);
                        ctx.scale(1, - 1);
                    }
                    ctx.drawImage(imageObject, 0, 0, canvas.width, canvas.height);
                    if (this.options.binary === true)
                    {
                        this._binaryImages.set(image, imageObject);

                        canvas.toBlob(function (blob)
                        {
                            this.processBufferViewImage(blob, function (bufferViewIndex)
                            {
                                gltfImage.bufferView = bufferViewIndex;
                                this._binaryImages.delete(image);
                                this.onFinishedParse();
                            }.bind(this));
                        }.bind(this), mimeType);
                    } else
                    {
                        gltfImage.uri = canvas.toDataURL(mimeType);
                    }
                }
                else {
                    gltfImage.uri = image;
                }
            } else
            {
                gltfImage.uri = image;
            }
        }
        this.outputJSON.images.push(gltfImage);

        var index = this.outputJSON.images.length - 1;
        this._imageObjects.set(image, index);

        return index;
    },

    processSampler: function (map)
    {

        if (!this.outputJSON.samplers)
        {
            this.outputJSON.samplers = [];
        }

        var gltfSampler = {
            magFilter: GASEngine.GLTFConsts.TO_WEBGL_FILTERS[map.maxFilter],
            minFilter: GASEngine.GLTFConsts.TO_WEBGL_FILTERS[map.minFilter],
            wrapS: GASEngine.GLTFConsts.TO_WEBGL_WRAPPINGS[map.wrapModeU],
            wrapT: GASEngine.GLTFConsts.TO_WEBGL_WRAPPINGS[map.wrapModeV]
        };

        this.outputJSON.samplers.push(gltfSampler);
        return this.outputJSON.samplers.length - 1;
    },

    processTexture: function (map)
    {
        if (this._textureObjects.has(map))
            return this._textureObjects.get(map);

        if (!this.outputJSON.textures)
        {
            this.outputJSON.textures = [];
        }

        var gltfTexture = {
            sampler: this.processSampler(map),
            source: this.processImage(map.texture)
        };

        this.outputJSON.textures.push(gltfTexture);

        var index = this.outputJSON.textures.length - 1;
        this._textureObjects.set(map, index);
        return index;
    },

    processMaterial: function (material)
    {
        if (!this.outputJSON.materials)
        {
            this.outputJSON.materials = [];
        }

        var gltfMaterial = {

            pbrMetallicRoughness: {}
        };

        // pbrMetallicRoughness.baseColorFactor
        if(material.albedoColor !== undefined) {
            var color = Array.prototype.slice.call(material.albedoColor);
            color.push(1.0);

            if (!GASEngine.Utilities.equalArray(color, [1, 1, 1, 1]))
            {
                gltfMaterial.pbrMetallicRoughness.baseColorFactor = color;
            }
        }

        gltfMaterial.pbrMetallicRoughness.metallicFactor = material.metalnessFactor ? material.metalnessFactor[0] : 0.0;
        gltfMaterial.pbrMetallicRoughness.roughnessFactor = material.roughnessFactor ? material.roughnessFactor[0] : 0.0;
        gltfMaterial.alphaMode = 'OPAQUE';

        // pbrMetallicRoughness.metallicRoughnessTexture
        if (material.metalnessMap || material.roughnessMap)
        {
            if (material.metalnessMap && material.metalnessMap.texture !== '')
            {
                gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture = {
                    index: this.processTexture(material.metalnessMap)
                };
            }
            else if(material.roughnessMap && material.roughnessMap.texture !== '')
            {
                gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture = {
                    index: this.processTexture(material.roughnessMap)
                };
            }
        }

        // pbrMetallicRoughness.baseColorTexture
        if (material.albedoEnable && material.albedoMap && material.albedoMap.texture !== '')
        {
            gltfMaterial.pbrMetallicRoughness.baseColorTexture = {
                index: this.processTexture(material.albedoMap)
            };
        }  

        // emissiveFactor
        if (material.emissiveColor !== undefined)
        {
            var emissive = Array.prototype.slice.call(material.emissiveColor);

            if (!GASEngine.Utilities.equalArray(emissive, [0, 0, 0]))
            {
                gltfMaterial.emissiveFactor = emissive;
            }
        }

        // emissiveTexture
        if (material.emissiveEnable && material.emissiveMap && material.emissiveMap.texture !== '')
        {
            gltfMaterial.emissiveTexture = {
                index: this.processTexture(material.emissiveMap)
            };
        }

        // normalTexture
        if (material.normalEnable && material.normalMap && material.normalMap.texture !== '')
        {
            gltfMaterial.normalTexture = {
                index: this.processTexture(material.normalMap)
            };

            // if ( material.normalScale.x !== - 1 ) {

            // 	if ( material.normalScale.x !== material.normalScale.y ) {

            // 		console.warn( 'GLTFExporter: Normal scale components are different, ignoring Y and exporting X.' );
            // 	}
            // 	gltfMaterial.normalTexture.scale = material.normalScale.x;
            // }
        }

        // occlusionTexture
        if (material.aoEnable && material.aoMap && material.aoMap.texture !== '')
        {
            gltfMaterial.occlusionTexture = {
                index: this.processTexture(material.aoMap)
            };

            if (material.aoMapIntensity !== 1.0)
            {
                gltfMaterial.occlusionTexture.strength = material.aoMapIntensity;
            }

        }

        // alphaMode
        // if (material.transparent || material.alphaTest > 0.0)
        // {
        //     gltfMaterial.alphaMode = material.opacity < 1.0 ? 'BLEND' : 'MASK';

        //     // Write alphaCutoff if it's non-zero and different from the default (0.5).
        //     if (material.alphaTest > 0.0 && material.alphaTest !== 0.5)
        //     {

        //         gltfMaterial.alphaCutoff = material.alphaTest;
        //     }
        // }

        // doubleSided
        gltfMaterial.doubleSided = (material.culling === GASEngine.Material.CullingOff);

        if (material.name !== '')
        {
            gltfMaterial.name = material.name;
        }

        this.outputJSON.materials.push(gltfMaterial);

        var index = this.outputJSON.materials.length - 1;
        this._materialObjects_.set(material, index);
        return index;
    },

    processMesh: function (mesh, materials)
    {
        var mode = GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLES;

        var gltfMesh = {
            name: mesh.name
        };

        var attributes = {};
        var indices;
        var primitives = [];
        var targets = [];

        // Conversion between attributes names in threejs and gltf spec
        var nameConversion = {
            uv: 'TEXCOORD_0',
            uv1: 'TEXCOORD_1',
            uv2: 'TEXCOORD_2',
            position: 'POSITION',
            normal: 'NORMAL',
            color: 'COLOR_0',
            skinWeight: 'WEIGHTS_0',
            skinIndex: 'JOINTS_0'
        };

        var itemSizeMap = {
            uv: 2,
            uv1: 2,
            uv2: 2,
            position: 3,
            normal: 3,
            color: 4,
            skinWeight: 4,
            skinIndex: 4,
            index: 1
        }

        for (var stream of mesh.streams)
        {
            if (stream.type === "bone" || stream.type.startsWith("tangent") || stream.type === "topology" || stream.type === "subMesh")
                continue;

            var attributeName = nameConversion[stream.type] || stream.type.toUpperCase();

            var array = stream.data;
            //JOINTS_0 Convert to Uint16Array;
            if (attributeName === 'JOINTS_0' && !(array instanceof Uint16Array) && !(array instanceof Uint8Array))
            {
                console.warn('GLTFExporter: Attribute "skinIndex" converted to type UNSIGNED_SHORT.');
                var outArray = new Uint16Array(array.length);
                for (var ii = 0, il = array.length; ii < il; ii++)
                {
                    outArray[ii] = array[ii];
                }
                array = outArray;
            }

            if(stream.type === 'uv') {
                var outArray = new Float32Array(array.length);
                for (var ii = 0, il = array.length; ii < il; ii += 2)
                {
                    outArray[ii] = array[ii];
                    outArray[ii + 1] = 1.0 - array[ii + 1];
                }
                array = outArray;
            }
            else if(stream.type === 'color') {
                var outArray = new Float32Array(array.length);
                for (var ii = 0, il = array.length; ii < il; ii++)
                {
                    outArray[ii] = parseFloat(array[ii] / 255.0);
                }
                array = outArray;
            }

            var accessor = this.processAccessor(array, itemSizeMap[stream.type]);
            if (accessor !== null)
            {
                if(stream.type === 'index')
                    indices = accessor;
                else
                    attributes[attributeName] = accessor;
            }
        }

        //Add Morph MeshTarget
        var meshTargetCount = mesh.getMorphTargetCount();
        if (meshTargetCount > 0)
        {
            var weights = [];
            var targetNames = [];

            for (var targetIndex = 0; targetIndex < meshTargetCount; targetIndex++)
            {
                var meshTarget = mesh.getMorphTarget(targetIndex);
                if (meshTarget === null)
                    continue;

                var target = {};

                for (var ss of meshTarget.streams)
                {

                    var baseArray = mesh.getStream(ss.type);
                    var inArray = ss.data;
                    var outArray = new Float32Array(inArray.length);

                    for (var j = 0, jl = inArray.length; j < jl; j++)
                    {
                        outArray[j] = inArray[j] - baseArray[j];
                    }
                    target[ss.type.toUpperCase()] = this.processAccessor(outArray, itemSizeMap[ss.type]);
                }
                targets.push(target);
                weights.push(0.0);
                targetNames.push(meshTarget.name);
            }
            gltfMesh.weights = weights;
            if (targetNames.length > 0)
            {
                gltfMesh.extras = {};
                gltfMesh.extras.targetNames = targetNames;
            }
        }

        var groups = [{ materialIndex: 0, start: undefined, count: undefined }];

        for (var i = 0, il = groups.length; i < il; i++)
        {
            var primitive = {
                mode: mode,
                attributes: attributes
            };

            if(indices !== undefined) {
                primitive.indices = indices;
            }

            if (targets.length > 0)
                primitive.targets = targets;

            var material = materials[groups[i].materialIndex];
            if(material instanceof GASEngine.CompoundMaterial) {
                material = material.materials.get('dielectric');
            }

            if(material !== undefined) {
                var materialIndex = this.processMaterial(material);
                if (materialIndex !== null)
                {
                    primitive.material = materialIndex;
                }
            }
            primitives.push(primitive);
        }

        gltfMesh.primitives = primitives;

        if (!this.outputJSON.meshes)
        {
            this.outputJSON.meshes = [];
        }
        this.outputJSON.meshes.push(gltfMesh);

        return this.outputJSON.meshes.length - 1;
    },

    processNode: function (object) // Object is entity
    {
        var environmentalLight = object.getComponent('environmentalLight');
        var punctualLight = object.getComponent('punctualLight');
        if(environmentalLight || punctualLight) {
            return null;
        }

        if (!this.outputJSON.nodes)
        {
            this.outputJSON.nodes = [];
        }
        var gltfNode = {};

        //Only TRS properties, and not matrices, may be targeted by animatio
        if (this.options.trs)
        {
            var localTranslation = object.getLocalTranslation();
            var localScale = object.getLocalScale();
            var localQuaterion = object.getLocalQuaternion();

            if(!!object.MB_PROPS)
            {
                var localMatrix = new GASEngine.Matrix4();
                localMatrix.composeMBMatrixQuaternion(object.MB_PROPS,localTranslation , localQuaterion, localScale);
                localTranslation = new GASEngine.Vector3();
                localQuaterion = new GASEngine.Quaternion();
                localScale = new GASEngine.Vector3();
                localMatrix.decompose(localTranslation, localQuaterion, localScale);
            }
    
            var rotation = localQuaterion.toArray();
            var position = localTranslation.toArray();
            var scale = localScale.toArray();

            if (!GASEngine.Utilities.equalArray(rotation, [0, 0, 0, 1]))
            {
                gltfNode.rotation = rotation;
            }

            if (!GASEngine.Utilities.equalArray(position, [0, 0, 0]))
            {

                gltfNode.translation = position;
            }

            if (!GASEngine.Utilities.equalArray(scale, [1, 1, 1]))
            {
                gltfNode.scale = scale;
            }
        }
        else
        {
            object.updateLocalMatrix();
            var matArray = object.matrixLocal.toArray();
            if (!GASEngine.Utilities.equalArray(matArray, [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]))
            {
                gltfNode.matrix = matArray;
            }
        }

        // We don't export empty strings name because it represents no-name.
        if (object.name !== '')
        {
            gltfNode.name = String(object.name);
        }

        var meshFilter, camera;
        if (meshFilter = object.getComponent('meshFilter'))
        {
            var meshObject = meshFilter.getMesh();
            if (meshObject)
            {
                var meshRender = object.getComponent('meshRenderer');
                var materials = meshRender ? meshRender.getMaterials() : [];
                var mesh = this.processMesh(meshObject, materials);

                if (mesh !== null)
                {
                    gltfNode.mesh = mesh;
                }

                if (meshObject.isSkinned())
                {
                    this._skinsObjects_.push(object);
                }
            }
        }
        else if (camera = object.getComponent('camera'))
        {
            gltfNode.camera = this.processCamera(camera);
        }

        if (object.children.length > 0)
        {
            var children = [];

            for (var i = 0, l = object.children.length; i < l; i++)
            {
                var child = object.children[i];
                var node = this.processNode(child);
                if (node !== null)
                {
                    children.push(node);
                }
            }

            if (children.length > 0)
            {
                gltfNode.children = children;
            }
        }
        this.outputJSON.nodes.push(gltfNode);

        var nodeIndex = this.outputJSON.nodes.length - 1;
        this._nodeObjects_.set(object, nodeIndex);

        return nodeIndex;
    },

    processScene: function (scene)
    {
        if (!this.outputJSON.scenes)
        {
            this.outputJSON.scenes = [];
            this.outputJSON.scene = 0;
        }

        var gltfScene = {
            nodes: []
        };

        if (scene.name !== '')
        {
            gltfScene.name = scene.name;
        }

        this.outputJSON.scenes.push(gltfScene);
        var nodes = [];

        for (var i = 0, l = scene.root.children.length; i < l; i++)
        {
            var child = scene.root.children[i];
            if(child.type === 'helper')
                continue;

            var node = this.processNode(child);
            if (node !== null)
            {
                nodes.push(node);
            }
        }

        if (nodes.length > 0)
        {
            gltfScene.nodes = nodes;
        }
    },

    processCamera: function (camera) {
        if ( ! this.outputJSON.cameras ) {
            this.outputJSON.cameras = [];
        }

        var gltfCamera = {
            type: camera.type
        };

        if (camera.type === 'orthographic') {
            gltfCamera.orthographic = {
                xmag: camera.right * 2,
                ymag: camera.top * 2,
                zfar: camera.far <= 0 ? 0.001 : camera.far,
                znear: camera.near < 0 ? 0 : camera.near
            };
        } else {

            gltfCamera.perspective = {
                aspectRatio: camera.aspect,
                yfov: GASEngine.degToRad( camera.fov ) / camera.aspect,
                zfar: camera.far <= 0 ? 0.001 : camera.far,
                znear: camera.near < 0 ? 0 : camera.near
            };
        }

        if ( camera.name !== '' ) {
            gltfCamera.name = camera.type;
        }

        this.outputJSON.cameras.push( gltfCamera );
        return this.outputJSON.cameras.length - 1;
    },

    processSkin: function (object) {
        var nodeIndex = this._nodeObjects_.get(object);
        var meshFilter = object.getComponent('meshFilter');
        var mesh = meshFilter.getMesh();

        var node = this.outputJSON.nodes[nodeIndex];

        var rootJoint = mesh.bones[ 0 ];

        if ( rootJoint === undefined ) return null;

        var joints = [];
        var inverseBindMatrices = new Float32Array(mesh.bones.length * 16 );

        for ( var i = 0; i < mesh.bones.length; ++ i ) {
            var boneEntity = mesh.bones[i];
            joints.push(this._nodeObjects_.get(boneEntity));
            
            mesh.matricesWorld2Bone[i].toArray( inverseBindMatrices, i * 16);
        }

        if (this.outputJSON.skins === undefined ) {
            this.outputJSON.skins = [];
        }

        this.outputJSON.skins.push( {
            inverseBindMatrices: this.processAccessor(inverseBindMatrices, 16),
            joints: joints,
            skeleton: this._nodeObjects_.get(rootJoint )
        } );

        var skinIndex = node.skin = this.outputJSON.skins.length - 1;
        return skinIndex;
    },

    processAnimation: function (clip)
    {
        if (!this.outputJSON.animations)
        {
            this.outputJSON.animations = [];
        }

        var channels = [];
        var samplers = [];
        var endFrame = clip.actualEndFrame;
        var keyframeCount = endFrame + 1;

        //Temp for MB_PROPS
        var localTranslation = new GASEngine.Vector3();
        var localScale = new GASEngine.Vector3();
        var localQuaterion = new GASEngine.Quaternion();
        var localMatrix = new GASEngine.Matrix4();

        //times
        var times = new Float32Array(keyframeCount);
        for (var ii = 0; ii < keyframeCount; ii++)
        {
            times[ii] = ii * (1.0 / clip.fps);
        }
        var inputSampler = this.processAccessor(times, 1);

        //loop
        for (var i = 0; i < clip.animatedNodes.length; i++)
        {
            var animationChannels = clip.animatedNodes[i];
            var trackNode = animationChannels.node;
            if(trackNode instanceof GASEngine.MeshFilterComponent) {
                trackNode = trackNode.getParentEntity();
            }

            if (!trackNode)
            {
                console.warn('GLTFExporter: Could not export animation about "%s".', trackNode.name);
                return null;
            }

            var valuesMap = {};
            var curveCount = animationChannels.kfs.length;

            for (var j = 0; j < curveCount; j++)
            {
                var keyframe = animationChannels.kfs[j];

                if (keyframe.target === GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT)
                {
                    values = valuesMap['weights'];
                    if (values === undefined)
                    {
                        values = valuesMap['weights'] = new Float32Array(keyframeCount * curveCount);
                    }
                    offset = j;

                    if (values !== undefined)
                    {
                        var lastValue;
                        var lastFrame;

                        for (var k = 0; k < keyframe.v.length; k++)
                        {
                            var t = keyframe.t[k];
                            var v = keyframe.v[k]; //Float
                            t = (t < 0) ? 0: t; //t must be positive

                            if (lastFrame === undefined || lastValue === undefined || t <= lastFrame)
                            {
                                for (var frame = 0; frame <= t; frame++)
                                {
                                    values[frame * curveCount + offset] = v / 100.0;
                                }
                            }
                            else
                            {
                                for (var frame = lastFrame + 1; frame <= t; frame++)
                                {
                                    var rate = (frame - lastFrame) / (t - lastFrame);
                                    values[frame * curveCount + offset] = Math.lerp(lastValue, v, rate) / 100.0;
                                }
                            }
                            lastFrame = t, lastValue = v;
                        }

                        for(var frame = lastFrame + 1; frame <= endFrame; frame++) //...end
                        {
                            values[frame * curveCount + offset] = lastValue / 100.0;
                        }
                    }
                }
                else if ((keyframe.target >= GASEngine.KeyframeAnimation.TARGET_TX) && (keyframe.target <= GASEngine.KeyframeAnimation.TARGET_SZ))
                {
                    var offset = 0;
                    var values;
                    if (keyframe.target >= GASEngine.KeyframeAnimation.TARGET_TX && keyframe.target <= GASEngine.KeyframeAnimation.TARGET_TZ)
                    {
                        values = valuesMap['translation'];
                        if (values === undefined)
                        {
                            values = valuesMap['translation'] = new Float32Array(keyframeCount * 3);
                        }
                        offset = keyframe.target - GASEngine.KeyframeAnimation.TARGET_TX;
                    }
                    else if (keyframe.target >= GASEngine.KeyframeAnimation.TARGET_SX && keyframe.target <= GASEngine.KeyframeAnimation.TARGET_SZ)
                    {
                        values = valuesMap['scale'];
                        if (values === undefined)
                        {
                            values = valuesMap['scale'] = new Float32Array(keyframeCount * 3);
                        }
                        offset = keyframe.target - GASEngine.KeyframeAnimation.TARGET_SX;
                    }

                    if (values !== undefined)
                    {
                        var lastValue;
                        var lastFrame;

                        for (var k = 0; k < keyframe.v.length; k++)
                        {
                            var t = keyframe.t[k];
                            var v = keyframe.v[k]; //Float
                            t = (t < 0) ? 0: t; //t must be positive

                            if (lastFrame === undefined || lastValue === undefined || t <= lastFrame)
                            {
                                for (var frame = 0; frame <= t; frame++)
                                {
                                    values[frame * 3 + offset] = v;
                                }
                            }
                            else
                            {
                                for (var frame = lastFrame + 1; frame <= t; frame++)
                                {
                                    var rate = (frame - lastFrame) / (t - lastFrame);
                                    values[frame * 3 + offset] = Math.lerp(lastValue, v, rate);
                                }
                            }
                            lastFrame = t, lastValue = v;
                        }

                        for(var frame = lastFrame + 1; frame <= endFrame; frame++) //...end
                        {
                            values[frame * 3 + offset] = lastValue;
                        }
                    }
                }
                else if (keyframe.target === GASEngine.KeyframeAnimation.TARGET_RQ)
                {
                    var values = valuesMap['rotation'] = new Float32Array(keyframeCount * 4);

                    if (values !== undefined)
                    {
                        var lastValue;
                        var lastFrame;

                        for (var k = 0; k < keyframe.v.length; k++)
                        {
                            var t = keyframe.t[k];
                            var v = keyframe.v[k]; //Quaternion
                            t = (t < 0) ? 0: t; //t must be positive

                            if (lastFrame === undefined || lastValue === undefined || t <= lastFrame)
                            {
                                for (var frame = 0; frame <= t; frame++)
                                {
                                    values[frame * 4] = v.x;
                                    values[frame * 4 + 1] = v.y;
                                    values[frame * 4 + 2] = v.z;
                                    values[frame * 4 + 3] = v.w;
                                }
                            }
                            else
                            {
                                for (var frame = lastFrame + 1; frame <= t; frame++)
                                {
                                    var rate = (frame - lastFrame) / (t - lastFrame);
                                    var newValue = lastValue.clone();
                                    newValue.slerp(v, rate);
                                    values[frame * 4] = newValue.x;
                                    values[frame * 4 + 1] = newValue.y;
                                    values[frame * 4 + 2] = newValue.z;
                                    values[frame * 4 + 3] = newValue.w;
                                }
                            }
                            lastFrame = t, lastValue = v;
                        }

                        for(var frame = lastFrame + 1; frame <= endFrame; frame++) //...end
                        {
                            values[frame * 4] = lastValue.x;
                            values[frame * 4 + 1] = lastValue.y;
                            values[frame * 4 + 2] = lastValue.z;
                            values[frame * 4 + 3] = lastValue.w;
                        }
                    }
                }
            }

            //compute MB_PROPS
            if(!!trackNode.MB_PROPS)
            {
                var translationValues = valuesMap['translation'];
                var rotationValues = valuesMap['rotation'];
                var scaleValues = valuesMap['scale'];

                if(translationValues !== undefined || rotationValues !== undefined || scaleValues !== undefined) {
                    localTranslation.copy(trackNode.getLocalTranslation());
                    localQuaterion.copy(trackNode.getLocalQuaternion());
                    localScale.copy(trackNode.getLocalScale());

                    for (var frame = 0; frame <= endFrame; frame++)
                    {
                        if(translationValues !== undefined) {
                            localTranslation.set(translationValues[frame*3], translationValues[frame*3 + 1], translationValues[frame*3 + 2]);
                        }

                        if(rotationValues !== undefined) {
                            localQuaterion.set(rotationValues[frame*4],rotationValues[frame*4+1],rotationValues[frame*4+2],rotationValues[frame*4+3])
                        }

                        if(scaleValues !== undefined) {
                            localScale.set(scaleValues[frame*3], scaleValues[frame*3 + 1], scaleValues[frame*3 + 2]);
                        }
                        localMatrix.composeMBMatrixQuaternion(trackNode.MB_PROPS, localTranslation, localQuaterion, localScale);
                        localMatrix.decompose(localTranslation, localQuaterion, localScale);

                        if(translationValues !== undefined) {
                            translationValues[frame*3] = localTranslation.x;
                            translationValues[frame*3 + 1] = localTranslation.y;
                            translationValues[frame*3 + 2] = localTranslation.z;
                        }

                        if(rotationValues !== undefined) {
                            rotationValues[frame*4] = localQuaterion.x;
                            rotationValues[frame*4 + 1] = localQuaterion.y;
                            rotationValues[frame*4 + 2] = localQuaterion.z;
                            rotationValues[frame*4 + 3] = localQuaterion.w;
                        }

                        if(scaleValues !== undefined) {
                            scaleValues[frame*3] = localScale.x;
                            scaleValues[frame*3 + 1] = localScale.y;
                            scaleValues[frame*3 + 2] = localScale.z;
                        }
                    }
                }
            }

            for (var trackProperty in valuesMap)
            {
                var values = valuesMap[trackProperty];
                var outputItemSize = trackProperty === 'weights' ? 1: values.length / times.length;

                samplers.push({

                    input: inputSampler,
                    output: this.processAccessor(values, outputItemSize),
                    interpolation: 'LINEAR'
                });

                channels.push({
                    sampler: samplers.length - 1,
                    target: {
                        node: this._nodeObjects_.get(trackNode),
                        path: trackProperty
                    }
                });
            }
        }
        this.outputJSON.animations.push({
            name: clip.name || 'clip_' + this.outputJSON.animations.length,
            samplers: samplers,
            channels: channels
        });
        return this.outputJSON.animations.length - 1;
    }
};