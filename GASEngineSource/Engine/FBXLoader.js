/*
* Loader loads FBX file and generates Group representing FBX scene.
* Requires FBX file to be >= 7.0 and in ASCII or >= 6400 in Binary format
* Versions lower than this may load but will probably have errors
*
* Needs Support:
*  Morph normals / blend shape normals
*
* FBX format references:
* 	https://wiki.blender.org/index.php/User:Mont29/Foundation/FBX_File_Structure
* 	http://help.autodesk.com/view/FBX/2017/ENU/?guid=__cpp_ref_index_html (C++ SDK reference)
*
* 	Binary format specification:
*		https://code.blender.org/2013/08/fbx-binary-file-format-specification/
*/
GASEngine.FBXLoader = function () {
    GASEngine.Events.attach(this);

    this._fbxMaterialDecoder_ = new GASEngine.FBXMaterialDecoder();
    this._fbxMeshDecoder_ = new GASEngine.FBXMeshDecoder();
    this._fbxAnimationDecoder_ = new GASEngine.FBXKeyframeAnimationDecoder();

    this._onSuccess_ = null;

    //temp for parseTree
    this._fbxTree = null;
    this._scene_ = null;
    this._connectionMap_ = new Map();
    this._imageObjects_ = new Map();
    this._textureObjects_ = new Map();
    this._materialObjects_ = new Map();
    this._meshObjects_ = null;
    this._modelObjects_ = new Map();
    this._deformers_ = {};

    this._nodeObjects_ = new Map();
    this._meshFilterComponents_ = new Map();
    this._animatorComponent_ = null;
    this._animationClips_ = null;
};

GASEngine.FBXLoader.prototype = {
    constructor: GASEngine.FBXLoader,

    load: function (modelName, onSuccess) {
        this._onSuccess_ = onSuccess;

        GASEngine.FileSystem.Instance.read
            (
                modelName,
                function (data) {
                    this.parse(data, onSuccess);
                }.bind(this),
                null,
                function (path, ext) {
                }.bind(this)
            );
    },

    parse: function (data, onSuccess) {
        this._onSuccess_ = onSuccess;

        var fbxTree = null;
        if (this._isFbxFormatBinary(data)) {
            fbxTree = this._parseBinary(data);
        }
        else {

            var FBXText = GASEngine.Utilities.arrayBufferToString(data);
            if (!this._isFbxFormatASCII(FBXText)) {
                console.error('GASEngine.FBXLoader: Unknown format.')
                return;
            }

            if (this._getFbxVersion(FBXText) < 7000) {
                console.error('GASEngine.FBXLoader: FBX version not supported, FileVersion: ' + getFbxVersion(FBXText));
                return;
            }

            fbxTree = this._parseASCII(FBXText);
        }
        if (fbxTree) {
            this._parseTree(fbxTree);
        }
    },

    _isFbxFormatBinary: function (buffer) {
        var CORRECT = 'Kaydara FBX Binary  \0';
        return buffer.byteLength >= CORRECT.length && CORRECT === GASEngine.Utilities.arrayBufferToString(buffer, 0, CORRECT.length);
    },

    _isFbxFormatASCII: function (text) {
        var CORRECT = ['K', 'a', 'y', 'd', 'a', 'r', 'a', '\\', 'F', 'B', 'X', '\\', 'B', 'i', 'n', 'a', 'r', 'y', '\\', '\\'];
        var cursor = 0;
        function read(offset) {
            var result = text[offset - 1];
            text = text.slice(cursor + offset);
            cursor++;
            return result;
        }

        for (var i = 0; i < CORRECT.length; ++i) {
            var num = read(1);
            if (num === CORRECT[i]) {
                return false;
            }
        }
        return true;
    },

    _getFbxVersion: function (text) {
        var versionRegExp = /FBXVersion: (\d+)/;
        var match = text.match(versionRegExp);
        if (match) {
            var version = parseInt(match[1]);
            return version;
        }
        console.error('GASEngine.FBXLoader: Cannot find the version number for the file given.');
    },

    //Binary=====================================================================================
    _parseBinary: function (buffer) {
        var reader = new GASEngine.FBXLoader.BinaryReader(buffer);
        reader.skip(23); // skip magic 23 bytes

        var version = reader.getUint32();

        console.log('GASEngine.FBXLoader: FBX binary version: ' + version);

        var allNodes = new GASEngine.FBXLoader.FBXTree();

        while (!this._endOfContent(reader)) {
            var node = this._parseNode(reader, version);
            if (node !== null) allNodes.add(node.name, node);
        }
        return allNodes;
    },

    // Check if reader has reached the end of content.
    _endOfContent: function (reader) {
        // footer size: 160bytes + 16-byte alignment padding
        // - 16bytes: magic
        // - padding til 16-byte alignment (at least 1byte?)
        //	(seems like some exporters embed fixed 15 or 16bytes?)
        // - 4bytes: magic
        // - 4bytes: version
        // - 120bytes: zero
        // - 16bytes: magic
        if (reader.size() % 16 === 0) {
            return ((reader.getOffset() + 160 + 16) & ~0xf) >= reader.size();
        } else {
            return reader.getOffset() + 160 + 16 >= reader.size();
        }
    },

    // recursively parse nodes until the end of the file is reached
    _parseNode: function (reader, version) {
        var node = {};
        // The first GASEngine data sizes depends on version.
        var endOffset = (version >= 7500) ? reader.getUint64() : reader.getUint32();
        var numProperties = (version >= 7500) ? reader.getUint64() : reader.getUint32();

        // note: do not remove this even if you get a linter warning as it moves the buffer forward
        var propertyListLen = (version >= 7500) ? reader.getUint64() : reader.getUint32();

        var nameLen = reader.getUint8();
        var name = reader.getString(nameLen);

        // Regards this node as NULL-record if endOffset is zero
        if (endOffset === 0) return null;
        var propertyList = [];

        for (var i = 0; i < numProperties; i++) {
            propertyList.push(this._parseProperty(reader));
        }

        // Regards the first GASEngine elements in propertyList as id, attrName, and attrType
        var id = propertyList.length > 0 ? propertyList[0] : '';
        var attrName = propertyList.length > 1 ? propertyList[1] : '';
        var attrType = propertyList.length > 2 ? propertyList[2] : '';

        // check if this node represents just a single property
        // like (name, 0) set or (name2, [0, 1, 2]) set of {name: 0, name2: [0, 1, 2]}
        node.singleProperty = (numProperties === 1 && reader.getOffset() === endOffset) ? true : false;

        while (endOffset > reader.getOffset()) {
            var subNode = this._parseNode(reader, version);
            if (subNode !== null) this._parseSubNode(name, node, subNode);
        }

        node.propertyList = propertyList; // raw property list used by parent

        if (typeof id === 'number') node.id = id;
        if (attrName !== '') node.attrName = attrName;
        if (attrType !== '') node.attrType = attrType;
        if (name !== '') node.name = name;

        return node;
    },

    _parseSubNode: function (name, node, subNode) {
        // special case: child node is single property
        if (subNode.singleProperty === true) {
            var value = subNode.propertyList[0];
            if (Array.isArray(value)) {
                node[subNode.name] = subNode;
                subNode.a = value;
            } else {
                node[subNode.name] = value;
            }
        } else if (name === 'Connections' && subNode.name === 'C') {
            var array = [];
            subNode.propertyList.forEach(function (property, i) {
                // first Connection is FBX type (OO, OP, etc.). We'll discard these
                if (i !== 0) array.push(property);
            });

            if (node.connections === undefined) {
                node.connections = [];
            }
            node.connections.push(array);
        } else if (subNode.name === 'Properties70') {
            var keys = Object.keys(subNode);
            keys.forEach(function (key) {
                node[key] = subNode[key];
            });
        } else if (name === 'Properties70' && subNode.name === 'P') {
            var innerPropName = subNode.propertyList[0];
            var innerPropType1 = subNode.propertyList[1];
            var innerPropType2 = subNode.propertyList[2];
            var innerPropFlag = subNode.propertyList[3];
            var innerPropValue;

            if (innerPropName.indexOf('Lcl ') === 0) innerPropName = innerPropName.replace('Lcl ', 'Lcl_');
            if (innerPropType1.indexOf('Lcl ') === 0) innerPropType1 = innerPropType1.replace('Lcl ', 'Lcl_');

            if (innerPropType1 === 'Color' || innerPropType1 === 'ColorRGB' || innerPropType1 === 'Vector' || innerPropType1 === 'Vector3D' || innerPropType1.indexOf('Lcl_') === 0) {

                innerPropValue = [
                    subNode.propertyList[4],
                    subNode.propertyList[5],
                    subNode.propertyList[6]
                ];

            } else {
                innerPropValue = subNode.propertyList[4];
            }

            // this will be copied to parent, see above
            node[innerPropName] = {
                'type': innerPropType1,
                'type2': innerPropType2,
                'flag': innerPropFlag,
                'value': innerPropValue
            };

        } else if (node[subNode.name] === undefined) {
            if (typeof subNode.id === 'number') {
                node[subNode.name] = {};
                node[subNode.name][subNode.id] = subNode;
            } else {
                node[subNode.name] = subNode;
            }
        } else {
            if (subNode.name === 'PoseNode') {
                if (!Array.isArray(node[subNode.name])) {
                    node[subNode.name] = [node[subNode.name]];
                }
                node[subNode.name].push(subNode);
            } else if (node[subNode.name][subNode.id] === undefined) {
                node[subNode.name][subNode.id] = subNode;
            }
        }
    },

    _parseProperty: function (reader) {
        var type = reader.getString(1);
        switch (type) {
            case 'C':
                return reader.getBoolean();
            case 'D':
                return reader.getFloat64();
            case 'F':
                return reader.getFloat32();
            case 'I':
                return reader.getInt32();
            case 'L':
                return reader.getInt64();
            case 'R':
                var length = reader.getUint32();
                return reader.getArrayBuffer(length);
            case 'S':
                var length = reader.getUint32();
                return reader.getString(length);
            case 'Y':
                return reader.getInt16();
            case 'b':
            case 'c':
            case 'd':
            case 'f':
            case 'i':
            case 'l':
                var arrayLength = reader.getUint32();
                var encoding = reader.getUint32(); // 0: non-compressed, 1: compressed
                var compressedLength = reader.getUint32();

                if (encoding === 0) {
                    switch (type) {
                        case 'b':
                        case 'c':
                            return reader.getBooleanArray(arrayLength);
                        case 'd':
                            return reader.getFloat64Array(arrayLength);
                        case 'f':
                            return reader.getFloat32Array(arrayLength);
                        case 'i':
                            return reader.getInt32Array(arrayLength);
                        case 'l':
                            return reader.getInt64Array(arrayLength);
                    }
                }

                if (typeof Zlib === 'undefined') {
                    console.error('GASEngine.FBXLoader: External library Inflate.min.js required, obtain or import from https://github.com/imaya/zlib.js');
                }

                var inflate = new Zlib.Inflate(new Uint8Array(reader.getArrayBuffer(compressedLength))); // eslint-disable-line no-undef
                var reader2 = new GASEngine.FBXLoader.BinaryReader(inflate.decompress().buffer);

                switch (type) {
                    case 'b':
                    case 'c':
                        return reader2.getBooleanArray(arrayLength);
                    case 'd':
                        return reader2.getFloat64Array(arrayLength);
                    case 'f':
                        return reader2.getFloat32Array(arrayLength);
                    case 'i':
                        return reader2.getInt32Array(arrayLength);
                    case 'l':
                        return reader2.getInt64Array(arrayLength);
                }
            default:
                throw new Error('GASEngine.FBXLoader: Unknown property type ' + type);
        }
    },

    //parseASCII===============================================================================
    _parseASCII: function (text) {
        this.currentIndent = 0;

        this.allNodes = new GASEngine.FBXLoader.FBXTree();
        this.nodeStack = [];
        this.currentProp = [];
        this.currentPropName = '';

        var split = text.split(/[\r\n]+/);

        split.forEach(function (line, i) {

            var matchComment = line.match(/^[\s\t]*;/);
            var matchEmpty = line.match(/^[\s\t]*$/);

            if (matchComment || matchEmpty) return;

            var matchBeginning = line.match('^\\t{' + this.currentIndent + '}(\\w+):(.*){', '');
            var matchProperty = line.match('^\\t{' + (this.currentIndent) + '}(\\w+):[\\s\\t\\r\\n](.*)');
            var matchEnd = line.match('^\\t{' + (this.currentIndent - 1) + '}}');

            if (matchBeginning) {
                this.parseNodeBegin(line, matchBeginning);
            } else if (matchProperty) {
                this.parseNodeProperty(line, matchProperty, split[++i]);
            } else if (matchEnd) {
                this.popStack();
            } else if (line.match(/^[^\s\t}]/)) {
                // large arrays are split over multiple lines terminated with a ',' character
                // if this is encountered the line needs to be joined to the previous line
                this.parseNodePropertyContinued(line);
            }
        }.bind(this));
        return this.allNodes;
    },

    getPrevNode: function () {
        return this.nodeStack[this.currentIndent - 2];
    },

    getCurrentNode: function () {
        return this.nodeStack[this.currentIndent - 1];
    },

    getCurrentProp: function () {
        return this.currentProp;
    },

    pushStack: function (node) {
        this.nodeStack.push(node);
        this.currentIndent += 1;

    },

    popStack: function () {
        this.nodeStack.pop();
        this.currentIndent -= 1;
    },

    setCurrentProp: function (val, name) {
        this.currentProp = val;
        this.currentPropName = name;
    },

    parseNodeBegin: function (line, property) {

        var nodeName = property[1].trim().replace(/^"/, '').replace(/"$/, '');
        var nodeAttrs = property[2].split(',').map(function (attr) {
            return attr.trim().replace(/^"/, '').replace(/"$/, '');
        });

        var node = { name: nodeName };
        var attrs = this.parseNodeAttr(nodeAttrs);

        var currentNode = this.getCurrentNode();

        // a top node
        if (this.currentIndent === 0) {
            this.allNodes.add(nodeName, node);
        } else { // a subnode

            // if the subnode already exists, append it
            if (nodeName in currentNode) {

                // special case Pose needs PoseNodes as an array
                if (nodeName === 'PoseNode') {
                    currentNode.PoseNode.push(node);
                } else if (currentNode[nodeName].id !== undefined) {
                    currentNode[nodeName] = {};
                    currentNode[nodeName][currentNode[nodeName].id] = currentNode[nodeName];
                }
                if (attrs.id !== '') currentNode[nodeName][attrs.id] = node;

            } else if (typeof attrs.id === 'number') {
                currentNode[nodeName] = {};
                currentNode[nodeName][attrs.id] = node;

            } else if (nodeName !== 'Properties70') {
                if (nodeName === 'PoseNode') currentNode[nodeName] = [node];
                else currentNode[nodeName] = node;
            }
        }

        if (typeof attrs.id === 'number') node.id = attrs.id;
        if (attrs.name !== '') node.attrName = attrs.name;
        if (attrs.type !== '') node.attrType = attrs.type;

        this.pushStack(node);

    },

    parseNodeAttr: function (attrs) {
        var id = attrs[0];
        if (attrs[0] !== '') {
            id = parseInt(attrs[0]);
            if (isNaN(id)) {
                id = attrs[0];
            }
        }
        var name = '', type = '';

        if (attrs.length > 1) {
            name = attrs[1].replace(/^(\w+)::/, '');
            type = attrs[2];
        }
        return { id: id, name: name, type: type };
    },

    parseNodeProperty: function (line, property, contentLine) {

        var propName = property[1].replace(/^"/, '').replace(/"$/, '').trim();
        var propValue = property[2].replace(/^"/, '').replace(/"$/, '').trim();

        // for special case: base64 image data follows "Content: ," line
        //	Content: ,
        //	 "/9j/4RDaRXhpZgAATU0A..."
        if (propName === 'Content' && propValue === ',') {
            propValue = contentLine.replace(/"/g, '').replace(/,$/, '').trim();
        }

        var currentNode = this.getCurrentNode();
        var parentName = currentNode.name;

        if (parentName === 'Properties70') {
            this.parseNodeSpecialProperty(line, propName, propValue);
            return;
        }

        // Connections
        if (propName === 'C') {
            var connProps = propValue.split(',').slice(1);
            var from = parseInt(connProps[0]);
            var to = parseInt(connProps[1]);

            var rest = propValue.split(',').slice(3);

            rest = rest.map(function (elem) {
                return elem.trim().replace(/^"/, '');
            });

            propName = 'connections';
            propValue = [from, to];
            GASEngine.Utilities.append(propValue, rest);

            if (currentNode[propName] === undefined) {
                currentNode[propName] = [];
            }
        }

        // Node
        if (propName === 'Node') currentNode.id = propValue;

        // connections
        if (propName in currentNode && Array.isArray(currentNode[propName])) {
            currentNode[propName].push(propValue);
        } else {
            if (propName !== 'a') currentNode[propName] = propValue;
            else currentNode.a = propValue;
        }
        this.setCurrentProp(currentNode, propName);

        // convert string to array, unless it ends in ',' in which case more will be added to it
        if (propName === 'a' && propValue.slice(- 1) !== ',') {
            currentNode.a = GASEngine.Utilities.parseNumberArray(propValue);
        }
    },

    parseNodePropertyContinued: function (line) {

        var currentNode = this.getCurrentNode();
        currentNode.a += line;

        // if the line doesn't end in ',' we have reached the end of the property value
        // so convert the string to an array
        if (line.slice(- 1) !== ',') {
            currentNode.a = GASEngine.Utilities.parseNumberArray(currentNode.a);
        }
    },

    // parse "Property70"
    parseNodeSpecialProperty: function (line, propName, propValue) {

        // split this
        // P: "Lcl Scaling", "Lcl Scaling", "", "A",1,1,1
        // into array like below
        // ["Lcl Scaling", "Lcl Scaling", "", "A", "1,1,1" ]
        var props = propValue.split('",').map(function (prop) {

            return prop.trim().replace(/^\"/, '').replace(/\s/, '_');

        });

        var innerPropName = props[0];
        var innerPropType1 = props[1];
        var innerPropType2 = props[2];
        var innerPropFlag = props[3];
        var innerPropValue = props[4];

        // cast values where needed, otherwise leave as strings
        switch (innerPropType1) {

            case 'int':
            case 'enum':
            case 'bool':
            case 'ULongLong':
            case 'double':
            case 'Number':
            case 'FieldOfView':
                innerPropValue = parseFloat(innerPropValue);
                break;

            case 'Color':
            case 'ColorRGB':
            case 'Vector3D':
            case 'Lcl_Translation':
            case 'Lcl_Rotation':
            case 'Lcl_Scaling':
                innerPropValue = GASEngine.Utilities.parseNumberArray(innerPropValue);
                break;

        }

        // CAUTION: these props must append to parent's parent
        this.getPrevNode()[innerPropName] = {

            'type': innerPropType1,
            'type2': innerPropType2,
            'flag': innerPropFlag,
            'value': innerPropValue

        };

        this.setCurrentProp(this.getPrevNode(), innerPropName);
    },

    //Tree =====================================================================================
    _parseTree: function (fbxTree) {
        if (fbxTree === null)
            return;

        this._fbxTree = fbxTree;

        this._parseConnections();

        this._parseImages();
        this._parseTextures();
        this._parseMaterials();
        this._parse_deformers_();

        this._meshObjects_ = this._fbxMeshDecoder_.parse(this._deformers_, this._fbxTree, this._connectionMap_);

        this._parseScene();
    },

    // Parses FBXTree.Connections which holds parent-child connections between objects (e.g. material -> texture, model->geometry )
    // and details the connection type
    _parseConnections: function () {
        this._connectionMap_.clear();

        if ('Connections' in this._fbxTree) {
            var rawConnections = this._fbxTree.Connections.connections;

            rawConnections.forEach(function (rawConnection) {
                var fromID = rawConnection[0];
                var toID = rawConnection[1];
                var relationship = rawConnection[2];

                if (!this._connectionMap_.has(fromID)) {
                    this._connectionMap_.set(fromID, {
                        parents: [],
                        children: []
                    });
                }
                var parentRelationship = { ID: toID, relationship: relationship };
                this._connectionMap_.get(fromID).parents.push(parentRelationship);

                if (!this._connectionMap_.has(toID)) {
                    this._connectionMap_.set(toID, {
                        parents: [],
                        children: []
                    });
                }

                var childRelationship = { ID: fromID, relationship: relationship };
                this._connectionMap_.get(toID).children.push(childRelationship);
            }.bind(this));
        }
    },

    // Parse FBXTree.Objects.Video for embedded image data
    // These images are connected to textures in FBXTree.Objects.Textures
    // via FBXTree.Connections.
    _parseImages: function () {
        this._imageObjects_.clear();

        var blobs = {};

        if ('Video' in this._fbxTree.Objects) {
            var videoNodes = this._fbxTree.Objects.Video;
            for (var nodeID in videoNodes) {
                var videoNode = videoNodes[nodeID];
                var id = parseInt(nodeID);

                this._imageObjects_.set(id, videoNode.RelativeFilename || videoNode.Filename);

                // raw image data is in videoNode.Content
                if ('Content' in videoNode) {
                    var arrayBufferContent = (videoNode.Content instanceof ArrayBuffer) && (videoNode.Content.byteLength > 0);
                    var base64Content = (typeof videoNode.Content === 'string') && (videoNode.Content !== '');

                    if (arrayBufferContent || base64Content) {
                        var image = this._parseImage(videoNodes[nodeID]);
                        blobs[videoNode.RelativeFilename || videoNode.Filename] = image;
                    }
                }
            }
        }

        for (var id of this._imageObjects_.keys()) {
            var filename = this._imageObjects_.get(id);

            if (blobs[filename] !== undefined)
                this._imageObjects_.set(id, blobs[filename]);
            else {
                this._imageObjects_.set(id, filename.split('\\').pop());
            }
        }
    },

    // Parse embedded image data in FBXTree.Video.Content
    _parseImage: function (videoNode) {
        var content = videoNode.Content;
        var fileName = videoNode.RelativeFilename || videoNode.Filename;
        var extension = fileName.slice(fileName.lastIndexOf('.') + 1).toLowerCase();

        var type;
        switch (extension) {
            case 'bmp':
                type = 'image/bmp';
                break;
            case 'jpg':
            case 'jpeg':
                type = 'image/jpeg';
                break;
            case 'png':
                type = 'image/png';
                break;
            case 'tif':
                type = 'image/tiff';
                break;
            case 'tga':
                type = 'image/tga';
                break;
            default:
                console.warn('FBXLoader: Image type "' + extension + '" is not supported.');
                return;
        }

        if (typeof content === 'string') { // ASCII format
            return 'data:' + type + ';base64,' + content;
        } else { // Binary Format
            var array = new Uint8Array(content);
            return URL.createObjectURL(new Blob([array], { type: type }));
        }
    },

    // Parse nodes in FBXTree.Objects.Texture
    // These contain details such as UV scaling, cropping, rotation etc and are connected
    // to images in FBXTree.Objects.Video
    _parseTextures: function () {
        this._textureObjects_.clear();

        if ('Texture' in this._fbxTree.Objects) {
            var textureNodes = this._fbxTree.Objects.Texture;
            for (var nodeID in textureNodes) {
                var texture = this._parseTexture(textureNodes[nodeID]);
                this._textureObjects_.set(parseInt(nodeID), texture);
            }
        }
    },

    // Parse individual node in FBXTree.Objects.Texture
    _parseTexture: function (textureNode) {
        var fileName;
        var children = this._connectionMap_.get(textureNode.id).children;

        if (children !== undefined && children.length > 0) {
            fileName = this._imageObjects_.get(children[0].ID);
        }

        var textureMap = GASEngine.MaterialMapFactory.Instance.create();
        textureMap.texture = fileName;
        textureMap.ID = textureNode.id;
        textureMap.name = textureNode.attrName;

        var wrapModeU = textureNode.WrapModeU;
        var wrapModeV = textureNode.WrapModeV;
        var valueU = wrapModeU !== undefined ? wrapModeU.value : 0;
        var valueV = wrapModeV !== undefined ? wrapModeV.value : 0;

        textureMap.wrapModeU = valueU === 0 ? 'REPEAT' : 'CLAMP_TO_EDGE';
        textureMap.wrapModeV = valueV === 0 ? 'REPEAT' : 'CLAMP_TO_EDGE';

        textureMap.minFilter = 'LINEAR_MIPMAP_LINEAR';
        textureMap.maxFilter = 'LINEAR';

        if ('Scaling' in textureNode) {
            var values = textureNode.Scaling.value;
            for(var ii = 0; ii < 3 && ii < values.length; ii++) {
                textureMap.scaling[ii] = values[ii];
            }
        }

        var pixelChannel = 0; //default
        textureMap.pixelChannels = new Float32Array([0.0, 0.0, 0.0, 0.0]);
        if (pixelChannel < 0 || pixelChannel > 3)
            pixelChannel = 0;
        textureMap.pixelChannels[pixelChannel] = 1.0;

        return textureMap;
    },

    // Parse nodes in FBXTree.Objects.Material
    _parseMaterials: function () {
        this._materialObjects_.clear();

        if ('Material' in this._fbxTree.Objects) {
            var materialNodes = this._fbxTree.Objects.Material;
            for (var nodeID in materialNodes) {
                var material = this._parseMaterial(materialNodes[nodeID]);

                if (material !== null) {
                    this._materialObjects_.set(parseInt(nodeID), material);
                }
            }
        }
    },

    // Parse single node in FBXTree.Objects.Material
    // Materials are connected to texture maps in FBXTree.Objects.Textures
    _parseMaterial: function (materialNode) {
        // Ignore unused materials which don't have any connections.
        if (this._connectionMap_.has(materialNode.id)) {
            var materialObject = this._fbxMaterialDecoder_.parse(materialNode, this._fbxTree, this._connectionMap_, this._textureObjects_);
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
            return materialObject;
        }
        return null;
    },

    // Parse nodes in FBXTree.Objects.Deformer
    // Deformer node can contain skinning or Vertex Cache animation data, however only skinning is supported here
    // Generates map of Skeleton-like objects for use later when generating and binding skeletons.
    _parse_deformers_: function () {
        var skeletons = {};
        var morphTargets = {};

        if ('Deformer' in this._fbxTree.Objects) {
            var DeformerNodes = this._fbxTree.Objects.Deformer;
            for (var nodeID in DeformerNodes) {
                var deformerNode = DeformerNodes[nodeID];
                var relationships = this._connectionMap_.get(parseInt(nodeID));
                if (deformerNode.attrType === 'Skin') {
                    var skeleton = this.parseSkeleton(relationships, DeformerNodes);
                    skeleton.ID = nodeID;
                    if (relationships.parents.length > 1)
                        console.warn('GASEngine.FBXLoader: skeleton attached to more than one geometry is not supported.');
                    skeleton.geometryID = relationships.parents[0].ID;
                    skeletons[nodeID] = skeleton;
                } else if (deformerNode.attrType === 'BlendShape') {
                    var morphTarget = {
                        id: nodeID,
                    };
                    morphTarget.rawTargets = this.parseMorphTargets(relationships, DeformerNodes);
                    morphTarget.id = nodeID;

                    if (relationships.parents.length > 1)
                        console.warn('GASEngine.FBXLoader: morph target attached to more than one geometry is not supported.');
                    morphTargets[nodeID] = morphTarget;
                }
            }
        }
        this._deformers_ =  {
            skeletons: skeletons,
            morphTargets: morphTargets,
        };
    },

    // Parse single nodes in FBXTree.Objects.Deformer
    // The top level skeleton node has type 'Skin' and sub nodes have type 'Cluster'
    // Each skin node represents a skeleton and each cluster node represents a bone
    parseSkeleton: function (relationships, deformerNodes) {
        var rawBones = [];
        for(var child of relationships.children) {
            var boneNode = deformerNodes[child.ID];
            if (boneNode.attrType !== 'Cluster') return;

            var rawBone = {
                ID: child.ID,
                indices: [],
                weights: [],
                transform: new GASEngine.Matrix4().fromArray(boneNode.Transform.a),
                transformLink: new GASEngine.Matrix4().fromArray(boneNode.TransformLink.a),
                linkMode: boneNode.Mode,
            };

            if ('Indexes' in boneNode) {

                rawBone.indices = boneNode.Indexes.a;
                rawBone.weights = boneNode.Weights.a;

            }
            rawBones.push(rawBone);
        }

        return {
            rawBones: rawBones,
            bones: []

        };

    },

    // The top level morph deformer node has type "BlendShape" and sub nodes have type "BlendShapeChannel"
    parseMorphTargets: function (relationships,deformerNodes) {
        var rawMorphTargets = [];
        for (var i = 0; i < relationships.children.length; i++) {
            if (i === 8) {
                console.warn('FBXLoader: maximum of 8 morph targets supported. Ignoring additional targets.');
                break;
            }
            var child = relationships.children[i];
            var morphTargetNode = deformerNodes[child.ID];
            var rawMorphTarget = {
                name: morphTargetNode.attrName,
                initialWeight: morphTargetNode.DeformPercent,
                id: morphTargetNode.id,
                fullWeights: morphTargetNode.FullWeights.a

            };

            if (morphTargetNode.attrType !== 'BlendShapeChannel')
                return;
            var targetRelationships = this._connectionMap_.get(parseInt(child.ID));
            targetRelationships.children.forEach(function (child) {
                if (child.relationship === undefined) rawMorphTarget.geoID = child.ID;
            });
            rawMorphTargets.push(rawMorphTarget);
        }
        return rawMorphTargets;
    },


    // create the main GASEngine.Scene() to be returned by the loader
    _parseScene: function () {
        var scene = new GASEngine.Scene();
        this._scene_ = scene;

        this._parseModels();

        var modelNodes = this._fbxTree.Objects.Model;
        for (var id of this._modelObjects_.keys()) {
            var model = this._modelObjects_.get(id);
            var modelNode = modelNodes[model.ID];
            this.setLookAtProperties(model, modelNode);

            var parentConnections = this._connectionMap_.get(model.ID).parents;
            for (var connection of parentConnections) {
                var parent = this._modelObjects_.get(connection.ID);
                if (parent !== undefined)
                    parent.addChild(model);
            }

            if (model.parent === null) {
                scene.appendEntityOnRoot(model);
            }
        }

        this.bindSkeleton();

        //parse animaion
        var rootEntity = scene.root;
        this._animationClips_ = this._fbxAnimationDecoder_.parse(this._fbxTree, this._connectionMap_, this._nodeObjects_);
        if(this._animationClips_ !== null && this._animationClips_.size > 0) {
            var animatorComponent = GASEngine.ComponentFactory.Instance.create('animator', -1);
            rootEntity.addComponent(animatorComponent);
            this._animatorComponent_ = animatorComponent;
        }
        this._linkScene();
        this._onSuccess_(scene);
    },

    // parse nodes in FBXTree.Objects.Model
    _parseModels: function () {
        this._modelObjects_.clear();
        this._nodeObjects_.clear();

        var modelNodes = this._fbxTree.Objects.Model;
        for (var nodeID in modelNodes) {
            var id = parseInt(nodeID);
            var node = modelNodes[nodeID];
            var relationships = this._connectionMap_.get(id);

            var model = this.buildSkeleton(relationships, id, node.attrName);
            if (model === null) {
                switch (node.attrType) {
                    case 'Camera':
                        model = this._createCamera(relationships);
                        break;
                    case 'Light':
                        model = this._createLight(relationships);
                        break;
                    case 'Mesh':
                        model = this._createMesh(relationships);
                        break;
                    case 'NurbsCurve':
                        model = this._createCurve(relationships);
                        break;
                    case 'LimbNode': // usually associated with a Bone, however if a Bone was not created we'll make a Group instead
                    case 'Null':
                    default:
                        model = GASEngine.EntityFactory.Instance.create();
                        break;
                }
                if(model !== null) {
                    model.name = GASEngine.Utilities.sanitizeNodeName(node.attrName);
                    model.ID = id;
                }
            }
            if(model !== null) {
                this.setModelTransforms(model, node);
                this._modelObjects_.set(id, model);
                this._nodeObjects_.set(model.name, model);
            }
        }
    },

    buildSkeleton: function ( relationships, id, name) {
        var bone = null;
        var skeletons = this._deformers_.skeletons;
        for(var parent of relationships.parents) {
            for ( var ID in skeletons ) {
                var skeleton = skeletons[ ID ];

                skeleton.rawBones.forEach( function ( rawBone, i ) {
                    if ( rawBone.ID === parent.ID ) {
                        var subBone = bone;
                        bone = GASEngine.EntityFactory.Instance.create();
                        bone.matrixWorld.copy(rawBone.transformLink);
                        bone.name = GASEngine.Utilities.sanitizeNodeName(name);
                        bone.ID = id;
                        skeleton.bones[ i ] = bone;

                        // In cases where a bone is shared between multiple meshes
                        // duplicate the bone here and add it as a child of the first bone
                        if ( subBone !== null ) {
                            bone.addChild(subBone);
                        }
                    }
                });
            }
        }
        return bone;
    },

    // create a CameraComponent
    _createCamera: function (relationships) {
        var model = GASEngine.EntityFactory.Instance.create();
        var cameraAttribute;
        relationships.children.forEach(function (child) {
            var attr = this._fbxTree.Objects.NodeAttribute[child.ID];
            if (attr !== undefined) {
                cameraAttribute = attr;
            }
        });

        if (cameraAttribute !== undefined) {
            var type = 0;
            if (cameraAttribute.CameraProjectionType !== undefined && cameraAttribute.CameraProjectionType.value === 1) {
                type = 1;
            }
            var nearClippingPlane = 1;
            if (cameraAttribute.NearPlane !== undefined) {
                nearClippingPlane = cameraAttribute.NearPlane.value / 1000;
            }

            var farClippingPlane = 1000;
            if (cameraAttribute.FarPlane !== undefined) {
                farClippingPlane = cameraAttribute.FarPlane.value / 1000;
            }

            var width = window.innerWidth;
            var height = window.innerHeight;

            if (cameraAttribute.AspectWidth !== undefined && cameraAttribute.AspectHeight !== undefined) {
                width = cameraAttribute.AspectWidth.value;
                height = cameraAttribute.AspectHeight.value;
            }
            var aspect = width / height;
            var fov = 45;
            if (cameraAttribute.FieldOfView !== undefined) {
                fov = cameraAttribute.FieldOfView.value;
            }

            if(type === 0) { //Perspective
                component = GASEngine.ComponentFactory.Instance.create('camera');
                model.addComponent(component);
                component.fov = fov;
                component.aspect = aspect;
                component.far = farClippingPlane;
                component.near = nearClippingPlane;
            }
            else if(type === 1) {
                component = GASEngine.ComponentFactory.Instance.create('camera');
                model.addComponent(component);
                component.left = -width / 2;
                component.right = width / 2;
                component.top = height / 2;
                component.bottom = -height / 2;
                component.far = farClippingPlane;
                component.near = nearClippingPlane;
            }
        }
        return model;
    },

    // Create a Light
    _createLight: function (relationships) {
        var model = GASEngine.EntityFactory.Instance.create();
        return model;
    },

    _createMesh: function (relationships)
    {
        var model = GASEngine.EntityFactory.Instance.create();
        var materials = [];

        // get meshes and materials(s) from connections
        for (var child of relationships.children)
        {
            if (this._meshObjects_.has(child.ID) && model.getComponent('meshFilter') === null)
            { //just one
                var mesh = this._meshObjects_.get(child.ID);
                var meshFilter = GASEngine.ComponentFactory.Instance.create('meshFilter');
                model.addComponent(meshFilter);
                meshFilter.setMesh(mesh);

                this._meshFilterComponents_.set(meshFilter, mesh);
            }

            if (this._materialObjects_.has(child.ID)) {
                materials.push(this._materialObjects_.get(child.ID));
            }
        }

        if (materials.length === 0)
        {
            var material = GASEngine.MaterialFactory.Instance.create('pureColor');
            material.setPureColorBase([0.2, 0.2, 0.2, 1.0]);
            materials.push(material);
        }

        //add meshRenderer
        var meshRenderer = GASEngine.ComponentFactory.Instance.create('meshRenderer');
        model.addComponent(meshRenderer);
        for(var ii = 0, il = materials.length; ii < il; ii++) {
            meshRenderer.addMaterial(materials[ii]);
        }
        return model;
    },

    _createCurve: function (relationships) {
    },

    // parse the model node for transform details and apply them to the model
    setModelTransforms: function (model, modelNode) {
        var transformData = {};

        if ('RotationOrder' in modelNode) transformData.eulerOrder = parseInt(modelNode.RotationOrder.value);
        if ('Lcl_Translation' in modelNode) transformData.translation = modelNode.Lcl_Translation.value;
        if ('RotationOffset' in modelNode) transformData.rotationOffset = modelNode.RotationOffset.value;
        if ('Lcl_Rotation' in modelNode) transformData.rotation = modelNode.Lcl_Rotation.value;
        if ('PreRotation' in modelNode) transformData.preRotation = modelNode.PreRotation.value;
        if ('PostRotation' in modelNode) transformData.postRotation = modelNode.PostRotation.value;
        if ('Lcl_Scaling' in modelNode) transformData.scale = modelNode.Lcl_Scaling.value;

        var transform = GASEngine.Utilities.generateTransform(transformData);
        model.setLocalMatrix(transform);
    },

    setLookAtProperties: function (model, modelNode) {
        if ('LookAtProperty' in modelNode) {
            var children = this._connectionMap_.get(model.ID).children;
            for(var child of children) {
                if (child.relationship === 'LookAtProperty') {
                    var lookAtTarget = this._fbxTree.Objects.Model[child.ID];
                    if ('Lcl_Translation' in lookAtTarget) {
                        var pos = lookAtTarget.Lcl_Translation.value;

                        // DirectionalLight, SpotLight
                        if (model.target !== undefined) {
                            model.target.position.fromArray(pos);
                            this._scene_.appendEntityOnRoot(model.target);
                        } else { // Cameras and other Object3Ds
                            model.lookAt(new GASEngine.Vector3().fromArray(pos));
                        }
                    }
                }
            }
        }
    },

    bindSkeleton: function ()
    {
        var skeletons = this._deformers_.skeletons;
        var bindMatrices = this.parsePoseNodes();
        for (var ID in skeletons)
        {
            var skeleton = skeletons[ID];
            var parents = this._connectionMap_.get(parseInt(skeleton.ID)).parents;
            for (var parent of parents)
            {
                if (this._meshObjects_.has(parent.ID))
                {
                    var geoID = parent.ID;
                    var mesh = this._meshObjects_.get(parent.ID);
                    var bones = [];
                    
                    var geoRelationships = this._connectionMap_.get(geoID);
                    for (var geoConnParent of geoRelationships.parents)
                    {
                        if (this._modelObjects_.has(geoConnParent.ID))
                        {
                            for (var jointNode of skeleton.bones) {
                                var bone = {
                                    "uniqueID": jointNode.uniqueID,
                                    "name": jointNode.name
                                };
                                var mat = new GASEngine.Matrix4();
                                mat.getInverse(jointNode.matrixWorld);
                                mat.multiply(bindMatrices[geoConnParent.ID]);
                                mat.transpose();
                                bone.matrixWorld2Bone = mat.elements;
                                bones.push(bone);
                            }
                        }
                    }
                    mesh.addStream('bone', bones);
                }
            }
        }
    },

    parsePoseNodes: function ()
    {
        var bindMatrices = {};
        if ('Pose' in this._fbxTree.Objects)
        {
            var BindPoseNode = this._fbxTree.Objects.Pose;
            for (var nodeID in BindPoseNode)
            {
                if (BindPoseNode[nodeID].attrType === 'BindPose')
                {
                    var poseNodes = BindPoseNode[nodeID].PoseNode;
                    if (Array.isArray(poseNodes))
                    {
                        poseNodes.forEach(function (poseNode)
                        {
                            bindMatrices[poseNode.Node] = new GASEngine.Matrix4().fromArray(poseNode.Matrix.a);
                        });
                    } else
                    {
                        bindMatrices[poseNodes.Node] = new GASEngine.Matrix4().fromArray(poseNodes.Matrix.a);
                    }
                }
            }
        }
        return bindMatrices;
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
    }
};

// FBXTree holds a representation of the FBX data, 
//returned by the parseASCII ( FBX ASCII format) and parseBinary( FBX Binary format)
GASEngine.FBXLoader.FBXTree = function () {
};

GASEngine.FBXLoader.FBXTree.prototype = {
    constructor: GASEngine.FBXLoader.FBXTree,

    add: function (key, val) {
        this[key] = val;
    }
};

//BinaryReader
GASEngine.FBXLoader.BinaryReader = function (buffer, littleEndian) {
    this.dv = new DataView(buffer);
    this.offset = 0;
    this.littleEndian = (littleEndian !== undefined) ? littleEndian : true;
};

GASEngine.FBXLoader.BinaryReader.prototype = {
    constructor: GASEngine.FBXLoader.BinaryReader,
    getOffset: function () {
        return this.offset;
    },

    size: function () {
        return this.dv.buffer.byteLength;
    },

    skip: function (length) {
        this.offset += length;
    },

    // seems like true/false representation depends on exporter.
    // true: 1 or 'Y'(=0x59), false: 0 or 'T'(=0x54)
    // then sees LSB.
    getBoolean: function () {
        return (this.getUint8() & 1) === 1;
    },

    getBooleanArray: function (size) {
        var a = [];
        for (var i = 0; i < size; i++) {
            a.push(this.getBoolean());
        }
        return a;
    },

    getUint8: function () {
        var value = this.dv.getUint8(this.offset);
        this.offset += 1;
        return value;
    },

    getInt16: function () {
        var value = this.dv.getInt16(this.offset, this.littleEndian);
        this.offset += 2;
        return value;
    },

    getInt32: function () {
        var value = this.dv.getInt32(this.offset, this.littleEndian);
        this.offset += 4;
        return value;
    },

    getInt32Array: function (size) {
        var a = [];
        for (var i = 0; i < size; i++) {
            a.push(this.getInt32());
        }
        return a;
    },

    getUint32: function () {
        var value = this.dv.getUint32(this.offset, this.littleEndian);
        this.offset += 4;
        return value;
    },

    // JavaScript doesn't support 64-bit integer so calculate this here
    // 1 << 32 will return 1 so using multiply operation instead here.
    // There's a possibility that this method returns wrong value if the value
    // is out of the range between Number.MAX_SAFE_INTEGER and Number.MIN_SAFE_INTEGER.
    // TODO: safely handle 64-bit integer
    getInt64: function () {
        var low, high;
        if (this.littleEndian) {
            low = this.getUint32();
            high = this.getUint32();
        } else {
            high = this.getUint32();
            low = this.getUint32();
        }

        // calculate negative value
        if (high & 0x80000000) {
            high = ~high & 0xFFFFFFFF;
            low = ~low & 0xFFFFFFFF;

            if (low === 0xFFFFFFFF) high = (high + 1) & 0xFFFFFFFF;
            low = (low + 1) & 0xFFFFFFFF;
            return - (high * 0x100000000 + low);
        }
        return high * 0x100000000 + low;
    },

    getInt64Array: function (size) {
        var a = [];
        for (var i = 0; i < size; i++) {
            a.push(this.getInt64());
        }
        return a;
    },

    // Note: see getInt64() comment
    getUint64: function () {
        var low, high;
        if (this.littleEndian) {
            low = this.getUint32();
            high = this.getUint32();
        } else {
            high = this.getUint32();
            low = this.getUint32();
        }
        return high * 0x100000000 + low;
    },

    getFloat32: function () {
        var value = this.dv.getFloat32(this.offset, this.littleEndian);
        this.offset += 4;
        return value;
    },

    getFloat32Array: function (size) {
        var a = [];
        for (var i = 0; i < size; i++) {
            a.push(this.getFloat32());
        }
        return a;
    },

    getFloat64: function () {
        var value = this.dv.getFloat64(this.offset, this.littleEndian);
        this.offset += 8;
        return value;
    },

    getFloat64Array: function (size) {
        var a = [];
        for (var i = 0; i < size; i++) {
            a.push(this.getFloat64());
        }
        return a;
    },

    getArrayBuffer: function (size) {
        var value = this.dv.buffer.slice(this.offset, this.offset + size);
        this.offset += size;
        return value;
    },

    getString: function (size) {
        // note: safari 9 doesn't support Uint8Array.indexOf; create intermediate array instead
        var a = [];

        for (var i = 0; i < size; i++) {
            a[i] = this.getUint8();
        }

        var nullByte = a.indexOf(0);
        if (nullByte >= 0) a = a.slice(0, nullByte);

        return GASEngine.Utilities.decodeText(new Uint8Array(a));
    }
};