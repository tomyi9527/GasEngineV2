//Author: tomyi
//Date: 2017-06-19

GASEngine.Mesh = function()
{
    this.uniqueID = -1;
    this.name = '';
    this.parentUniqueID = -1;
    this.boundingBox = null;

    this.streams = []; // {type, group, data}
    this._sharedVertexBuffer = false; //TODO: need more smart strategy, this flag just for array buffer test.
    this.drawMode = 'TRIANGLES';
    this.type = null;

    // this.isWireframe = false;
    // this.isWireframeOverlay = false;

    this.bones = [];
    this.matricesWorld2Bone = [];

    this.morphTargets = [];
    this.morphWeights = [];
    this.morphMapping = new Map();

    this._webglDrawMode = undefined;
    this._webglRecords = new Map();
    this._weightReorderArray = [];

    this.vertexShaderElements = [];
    this.fragmentShaderElements = [];

    this.vertexElementCount = 3;
    this.skinIndexItemSize = 4;
};

GASEngine.Mesh.prototype = {

    constructor: GASEngine.Mesh,

    setDrawMode: function(mode)
    {
        this.drawMode = mode;
    },

    setBoundingBox: function(boundingBox)
    {
        this.boundingBox = boundingBox;
    },

    getBoundingBox: function()
    {
        return this.boundingBox;
    },

    getMorphTargetCount: function()
    {
        return this.morphTargets.length;
    },

    getMorphTarget: function(i)
    {
        if(i < this.morphTargets.length && i >=0)
        {
            return this.morphTargets[i];
        }
        else
        {
            return null;
        }
    },

    addMorphTarget: function(target)
    {
        this.morphTargets.push(target);
        this.morphWeights.push(0.0);

        this.morphMapping.set(target.name, this.morphWeights.length - 1);

        this._weightReorderArray.push([0.0, 0]);
    },

    setMorphWeight: function(index, weight)
    {
        this.morphWeights[index] = weight;
    },

    setMorphWeightByName: function(morphName, weight)
    {
        var weightIndex = this.morphMapping.get(morphName);
        this.morphWeights[weightIndex] = weight;
    },

    sortMorphWeights: function(activeMorphWeights, activeMorphTargets)
    {
        for(var i = 0; i < this.morphWeights.length; i++)
        {
            this._weightReorderArray[i][0] = this.morphWeights[i];
            this._weightReorderArray[i][1] = i;
        }

        this._weightReorderArray.sort(function(a, b)
        {
            return Math.abs(b[0]) - Math.abs(a[0]);
        });

        var L = (this._weightReorderArray.length > 4) ? 4 : this._weightReorderArray.length;

        activeMorphTargets[0] = null;
        activeMorphTargets[1] = null;
        activeMorphTargets[2] = null;
        activeMorphTargets[3] = null;

        activeMorphWeights[0] = 0.0;
        activeMorphWeights[1] = 0.0;
        activeMorphWeights[2] = 0.0;
        activeMorphWeights[3] = 0.0;

        for(var j = 0; j < L; j++)
        {
            activeMorphWeights[j] = this._weightReorderArray[j][0] / 100.0;
            
            if(this._weightReorderArray[j][0] > 0.0)
            {
                var index = this._weightReorderArray[j][1];
                activeMorphTargets[j] = this.morphTargets[index];
            }                
        }
    },

    addStream: function(type, data, group)
    {
        var streamData = this.getStream(type);
        if(streamData !== null)
        {
            console.error('GASEngine.Mesh.addStream: the stream with the same type has already exist.');
            return false;
        }

        if(group === undefined)
        {
            group = 0;
        }
        
        this.streams.push({'type':type, 'data':data, 'group':group});
        return true;
    },

    removeStream: function(type)
    {
        for(var i = 0; i < this.streams.length; ++i)
        {
            if(this.streams[i].type === type)
            {
                this.streams.splice(i, 1);
                break;
            }
        }
    },

    getStream: function(type)
    {
        var data = null;
        for(var i = 0; i < this.streams.length; ++i)
        {
            if(this.streams[i].type === type)
            {
                data = this.streams[i].data;
                break;
            }
        }
        return data;
    },

    hashStream: function()
    {
        this.vertexShaderElements = [];

        for(var i = 0; i < this.streams.length; ++i)
        {
            var key = this.streams[i].type;
            this.vertexShaderElements.push(key.toUpperCase());           
        }
    },

    getWebglRecords: function()
    {
        return this._webglRecords;
    },

    isSkinned: function()
    {
        if(this.getStream('bone') !== null && this.getStream('skinIndex') && this.getStream('skinWeight'))
        {
            return true;
        }
        else
        {
            return false;
        }
    },

    isMorphed: function()
    {
        return (this.morphTargets.length > 0);
    },
    
    linkBones: function(node)
    {
        var bonesDataList = this.getStream('bone');
        if(bonesDataList !== null)
        {
            // Find the nearest entity that contain an animator component.
            while(node !== null)
            {
                var animator = node.getComponent('animator');
                if(animator !== null)
                {
                    break;
                }
                else
                {
                    node = node.parent;
                }
            }

            if(node !== null)
            {
                for(var i = 0; i < bonesDataList.length; ++i)
                {
                    //Bone Objects
                    var boneEntity = node.findObjectByID_r(bonesDataList[i].uniqueID);
                    if(boneEntity === null)
                    {
                        this.bones = [];
                        this.matricesWorld2Bone = [];
                        console.error('GASEngine.MeshFilterComponent.linkBones: bone link failed. Cannot find the specified bone entity.');
                        return;
                    }                    
                    this.bones.push(boneEntity);

                    //Matrices
                    var matrixWorldToBone = new GASEngine.Matrix4();

                    var elements = matrixWorldToBone.elements;
                    elements[0] = bonesDataList[i].matrixWorld2Bone[0];
                    elements[1] = bonesDataList[i].matrixWorld2Bone[4];
                    elements[2] = bonesDataList[i].matrixWorld2Bone[8];
                    elements[3] = bonesDataList[i].matrixWorld2Bone[12];

                    elements[4] = bonesDataList[i].matrixWorld2Bone[1];
                    elements[5] = bonesDataList[i].matrixWorld2Bone[5];
                    elements[6] = bonesDataList[i].matrixWorld2Bone[9];
                    elements[7] = bonesDataList[i].matrixWorld2Bone[13];

                    elements[8] = bonesDataList[i].matrixWorld2Bone[2];
                    elements[9] = bonesDataList[i].matrixWorld2Bone[6];
                    elements[10] = bonesDataList[i].matrixWorld2Bone[10];
                    elements[11] = bonesDataList[i].matrixWorld2Bone[14];

                    elements[12] = bonesDataList[i].matrixWorld2Bone[3];
                    elements[13] = bonesDataList[i].matrixWorld2Bone[7];
                    elements[14] = bonesDataList[i].matrixWorld2Bone[11];
                    elements[15] = bonesDataList[i].matrixWorld2Bone[15];

                    this.matricesWorld2Bone.push(matrixWorldToBone);
                }
            }
            else
            {
                console.error('GASEngine.MeshFilterComponent.linkBones: bone link failed. Cannot find a animator component on ancestor entities.');
            }
        }
        else
        {
            console.error('GASEngine.MeshFilterComponent.linkBones: bone link failed. The mesh does not contain any bone information.');
        }
    },

    getBones: function ()
    {
        return this.bones;
    },

    computeSkinningMatrices: (function()
    {
        var matrixBone2NewWorld = new GASEngine.Matrix4();
        var matrixIdentity = new GASEngine.Matrix4();

        return function(outBoneMatrices)
        {
            for(var b = 0; b < this.bones.length; b++)
            {
                var matrix = this.bones[b] ? this.bones[b].matrixWorld : matrixIdentity;

                matrixBone2NewWorld.multiplyMatrices(matrix, this.matricesWorld2Bone[b]);

                if(GASEngine.WebGLDevice.Instance.doesSupportFloatTexture())
                {
                    matrixBone2NewWorld.flattenToArrayOffset(outBoneMatrices, b * 16);
                }
                else
                {
                    matrixBone2NewWorld.encodeMatrix4ToRGBA8(outBoneMatrices, b * 128);
                }
            }
        };
    })(),

    submitToWebGL: function(dynamic)
    {
        this.hashStream();
        
        if(this._sharedVertexBuffer)
        {
            var streamTypes = [];
            var streamData = null;
            for(var i = 0; i < this.streams.length; ++i)
            {
                var stream = this.streams[i];                
                if(stream.type === 'subMesh' || stream.type === 'bone' || stream.type === 'index' || stream.type === 'topology' || stream.type === 'uvtopology')
                {                   
                }
                else
                {
                    streamData = stream.data;
                    streamTypes.push(stream.type);
                }                
            }

            var webglRecords = GASEngine.WebGLBufferManager.Instance.createSharedArrayBuffer(streamData, streamTypes, dynamic);
            
            for(var i = 0; i < webglRecords.length; ++i)
            {
                this._webglRecords.set(streamTypes[i], webglRecords[i]);
            }

            for(var i = 0; i < this.streams.length; ++i)
            {
                var stream = this.streams[i];
                if(stream.type === 'index' || stream.type === 'topology' || stream.type === 'uvtopology')
                {
                    var webglRecord = GASEngine.WebGLBufferManager.Instance.createBuffer(stream.data, stream.type, dynamic);
                    this._webglRecords.set(stream.type, webglRecord);
                }               
            }
        }
        else
        {
            for(var i = 0; i < this.streams.length; ++i)
            {
                var stream = this.streams[i];
                if(stream.type === 'subMesh' || stream.type === 'bone')
                {
                    continue;
                }

                var webglRecord = GASEngine.WebGLBufferManager.Instance.createBuffer(stream.data, stream.type, dynamic);
                this._webglRecords.set(stream.type, webglRecord);
            }
        }

        this._webglDrawMode = GASEngine.WebGLDevice.Instance.gl[this.drawMode];

        for(var i = 0; i < this.morphTargets.length; ++i)
        {
            this.morphTargets[i].submitToWebGL();
        }
    },

    //TODO: vertexElementCount and skinIndexItemSize can be configurable
    getVertexElementCount: function()
    {
        return this.vertexElementCount;
    },

    getSkinIndexItemSize: function()
    {
        return this.skinIndexItemSize;
    },

    getTrianglesCount: function()
    {
        var indices = this.getStream('index');
        var positions = this.getStream('position');

        var count = indices ? indices.length : positions.length;

        if(this.drawMode === 'TRIANGLES')
            return (count / 3);
        else if(this.drawMode === 'TRIANGLE_STRIP' || this.drawMode === 'TRIANGLE_FAN')
            return (count - 2);
        
        return 0;
    },

    createWireframeIndices: function (originalIndices)
    {
        function checkEdge(edges, a, b)
        {
            if (a > b)
            {
                var tmp = a;
                a = b;
                b = tmp;
            }

            var list = edges[a];
            if (list === undefined)
            {
                edges[a] = [b];
                return true;
            }
            else if (list.indexOf(b) === -1)
            {
                list.push(b);
                return true;
            }

            return false;
        };

        var indices = [];

        if (originalIndices !== null)
        {
            var edges = {};

            for (var i = 0, l = originalIndices.length; i < l; i += 3)
            {
                var a = originalIndices[i + 0];
                var b = originalIndices[i + 1];
                var c = originalIndices[i + 2];

                if (checkEdge(edges, a, b)) indices.push(a, b);
                if (checkEdge(edges, b, c)) indices.push(b, c);
                if (checkEdge(edges, c, a)) indices.push(c, a);
            }
        }
        return new Uint32Array(indices);
    },

    createUVTopologicalIndices: function (originalIndices)
    {
        var uvindices = new Uint32Array(originalIndices.length * 2);
        for (var ii = 0; ii < originalIndices.length / 3; ii++)
        {
            uvindices[ii * 6 + 0] = originalIndices[ii * 3 + 0];
            uvindices[ii * 6 + 1] = originalIndices[ii * 3 + 1];
            uvindices[ii * 6 + 2] = originalIndices[ii * 3 + 1];
            uvindices[ii * 6 + 3] = originalIndices[ii * 3 + 2];
            uvindices[ii * 6 + 4] = originalIndices[ii * 3 + 2];
            uvindices[ii * 6 + 5] = originalIndices[ii * 3 + 0];
        }

        return uvindices;
    },

    destroy: function()
    {
        for (var i = 0; i < this.streams.length; ++i) {
            if(this.streams[i].type !== 'subMesh' && this.streams[i].type !== 'bone')
                GASEngine.WebGLBufferManager.Instance.removeWebglBuffer(this.streams[i].data);
        }
        this.streams.length = 0;
        this.bones.length = 0;
        this.matricesWorld2Bone.length = 0;
        this.morphTargets.length = 0;
        this.morphWeights.length = 0;
        this._webglRecords = null;
        this._weightReorderArray.length = 0;
        this.vertexShaderElements.length = 0;
        this.fragmentShaderElements.length = 0;
    }
};

//LineMesh
GASEngine.LineMesh = function(size)
{
    GASEngine.Mesh.call(this);

    size = size || 1;

    this.drawMode = 'LINES';
    this.type = 'line';

    var vertices = [
        0, 0, 0,	
        size, 0, 0
    ];
    var count = vertices.length / 3;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.LineMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.LineMesh.constructor = GASEngine.LineMesh;

//PlaneMesh
GASEngine.PlaneMesh = function(width, height, widthSegments, heightSegments, halfFlag)
{
    GASEngine.Mesh.call(this);

    width = width || 1;
    height = height || 1;

    var width_half = width / 2;
    var height_half = height / 2;

    var gridX = Math.floor(widthSegments) || 1;
    var gridY = Math.floor(heightSegments) || 1;

    var gridX1 = gridX + 1;
    var gridY1 = gridY + 1;

    var segment_width = width / gridX;
    var segment_height = height / gridY;

    var ix, iy;
    this.type = 'plane';
    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];
    // generate vertices, normals and uvs

    for(iy = 0; iy < gridY1; iy++) 
    {
        var y = halfFlag ? (iy * segment_height - height_half) : iy *  segment_height;
        for(ix = 0; ix < gridX1; ix ++) 
        {
            if(halfFlag)
            {
                var x = ix * segment_width - width_half;
                vertices.push(x, - y, 0);   
                uvs.push(ix / gridX);
                uvs.push(1 - (iy / gridY));
            }
            else 
            {
                var x = ix * segment_width;
                vertices.push(x, y, 0);
                uvs.push(ix / gridX);
                uvs.push(iy / gridY);
            }
           
            normals.push(0, 0, 1);
            
        }
    }

    // indices
    for(iy = 0; iy < gridY; iy++) 
    {
        for(ix = 0; ix < gridX; ix++) 
        {
            var a = ix + gridX1 * iy;
            var b = ix + gridX1 * (iy + 1);
            var c = (ix + 1) + gridX1 * (iy + 1);
            var d = (ix + 1) + gridX1 * iy;

            // faces
            indices.push(a, d, b);
            indices.push(b, d, c);
        }
    }

    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': indices.length}]);
    this.setDrawMode('TRIANGLES');
};

GASEngine.PlaneMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.PlaneMesh.constructor = GASEngine.PlaneMesh;

GASEngine.Box3Mesh = function(min, max)
{
    GASEngine.Mesh.call(this);
    this.type = 'box3';

    var positions = new Float32Array([
        max.x, max.y, max.z,
        min.x, max.y, max.z,
        min.x, min.y, max.z,
        max.x, min.y, max.z,
        max.x, max.y, min.z,
        min.x, max.y, min.z,
        min.x, min.y, min.z,
        max.x, min.y, min.z
    ]);

    var outIndices = new Uint32Array([0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7]);

    this.addStream('index', outIndices);
    this.addStream('position', positions);
    this.addStream('subMesh', [{ 'start': 0, 'count': outIndices.length }]);
    this.setDrawMode('LINES');
};

GASEngine.Box3Mesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.Box3Mesh.constructor = GASEngine.Box3Mesh;

GASEngine.GridMesh = function(width, height, widthSegments, heightSegments, halfFlag)
{
    GASEngine.Mesh.call(this);
    this.type = 'grid';

    let width_half = width / 2;
    let height_half = height / 2;

    let gridX = widthSegments === undefined ? 10 : widthSegments;
    let gridY = heightSegments === undefined ? 10 : heightSegments;

    let gridX1 = gridX + 1;
    let gridY1 = gridY + 1;

    let segment_width = width / gridX;
    let segment_height = height / gridY;

    // buffers
    let indices = [];
    let vertices = [];

    let ix, iy;
    // generate vertices, normals and uvs
    for (iy = 0; iy < gridY1; iy++) 
    {
        let y = iy * segment_height;
        if (halfFlag)
            y = height_half - y;

        for (ix = 0; ix < gridX1; ix++) 
        {
            let x = ix * segment_width;
            if (halfFlag)
                x -= width_half;

            vertices.push(x, 0, y);
        }
    }

    // horizonal
    for (iy = 0; iy <= gridY; iy++) 
    {
        for (ix = 0; ix < gridX; ix++) 
        {
            indices.push(iy * gridX1 + ix, iy * gridX1 + ix + 1);
        }
    }
    //vertical
    for (ix = 0; ix <= gridX; ix++)
    {
        for (iy = 0; iy < gridY; iy++)
        {
            indices.push(iy * gridX1 + ix, (iy + 1) * gridX1 + ix);
        }
    }

    this.addStream('position', new Float32Array(vertices));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': indices.length }]);
    this.setDrawMode('LINES');
};

GASEngine.GridMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.GridMesh.constructor = GASEngine.GridMesh;

//ArrowMesh
GASEngine.ArrowMesh = function(radiusTop, radiusBottom, height, radialSegments, heightSegments, openEnded, thetaStart, thetaLength)
{
    GASEngine.Mesh.call(this);

    radiusTop = radiusTop !== undefined ? radiusTop : 1;
    radiusBottom = radiusBottom !== undefined ? radiusBottom : 1;
    height = height || 1;

    radialSegments = Math.floor(radialSegments) || 8;
    heightSegments = Math.floor(heightSegments) || 1;

    openEnded = openEnded !== undefined ? openEnded : false;
    thetaStart = thetaStart !== undefined ? thetaStart : 0.0;
    thetaLength = thetaLength !== undefined ? thetaLength : Math.PI * 2;

    this.type = 'arrow';

    // buffers
    let indices = [];
    let vertices = [];
    let normals = [];
    let uvs = [];

    // helper variables
    let index = 0;
    let indexArray = [];
    let halfHeight = height / 2;
    generateTorso();

    if (openEnded === false) 
    {

        if (radiusTop > 0) generateCap(true);
        if (radiusBottom > 0) generateCap(false);

    }
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('subMesh', [{ 'start': 0, 'count': indices.length}]);
    
    function generateTorso() 
    {
        let x, y;
        let normal = new GASEngine.Vector3();
        let vertex = new GASEngine.Vector3();

        // this will be used to calculate the normal
        let slope = (radiusBottom - radiusTop) / height;

        // generate vertices, normals and uvs
        for (y = 0; y <= heightSegments; y++) 
        {
            let indexRow = [];
            let v = y / heightSegments;
            // calculate the radius of the current row
            let radius = v * (radiusBottom - radiusTop) + radiusTop;
            for (x = 0; x <= radialSegments; x ++) 
            {
                let u = x / radialSegments;
                let theta = u * thetaLength + thetaStart;
                let sinTheta = Math.sin(theta);
                let cosTheta = Math.cos(theta);
                // vertex
                vertex.x = radius * sinTheta;
                vertex.y = - v * height + halfHeight;
                vertex.z = radius * cosTheta;
                vertices.push(vertex.x, vertex.y, vertex.z);
                // normal
                normal.set(sinTheta, slope, cosTheta).normalize();
                normals.push(normal.x, normal.y, normal.z);
                // uv
                uvs.push(u, 1 - v);
                // save index of vertex in respective row
                indexRow.push(index++);
            }
            // now save vertices of the row in our index array
            indexArray.push(indexRow);
        }

        // generate indices
        for(x = 0; x < radialSegments; x ++) 
        {
            for(y = 0; y < heightSegments; y++) 
            {
                // we use the index array to access the correct indices
                let a = indexArray[y][x];
                let b = indexArray[y + 1][x];
                let c = indexArray[y + 1][x + 1];
                let d = indexArray[y][x + 1];
                // faces
                indices.push(a, b, d);
                indices.push(b, c, d);
            }
        }
    }

    function generateCap(top) 
    {
        let x, centerIndexStart, centerIndexEnd;
        let uv = new GASEngine.Vector2();
        let vertex = new GASEngine.Vector3();
        let groupCount = 0;
        let radius = (top === true) ? radiusTop : radiusBottom;
        let sign = (top === true) ? 1 : - 1;
        // save the index of the first center vertex
        centerIndexStart = index;
        // first we generate the center vertex data of the cap.
        // because the geometry needs one set of uvs per face,
        // we must generate a center vertex per face/segment
        for (x = 1; x <= radialSegments; x ++) 
        {
            // vertex
            vertices.push(0, halfHeight * sign, 0);
            // normal
            normals.push(0, sign, 0);
            // uv
            uvs.push(0.5, 0.5);
            // increase index
            index ++;
        }
        // save the index of the last center vertex
        centerIndexEnd = index;
        // now we generate the surrounding vertices, normals and uvs
        for(x = 0; x <= radialSegments; x ++) 
        {
            var u = x / radialSegments;
            var theta = u * thetaLength + thetaStart;

            var cosTheta = Math.cos(theta);
            var sinTheta = Math.sin(theta);
            // vertex
            vertex.x = radius * sinTheta;
            vertex.y = halfHeight * sign;
            vertex.z = radius * cosTheta;
            vertices.push(vertex.x, vertex.y, vertex.z);

            // normal
            normals.push(0, sign, 0);

            // uv
            uv.x = (cosTheta * 0.5) + 0.5;
            uv.y = (sinTheta * 0.5 * sign) + 0.5;
            uvs.push(uv.x, uv.y);
            // increase index
            index ++;
        }

        // generate indices
        for (x = 0; x < radialSegments; x ++) 
        {
            let c = centerIndexStart + x;
            let i = centerIndexEnd + x;
            if (top === true) 
            {
                // face top
                indices.push(i, i + 1, c);
            } 
            else 
            {
                // face bottom
                indices.push(i + 1, i, c);
            }
        }
    }
};

GASEngine.ArrowMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.ArrowMesh.constructor = GASEngine.ArrowMesh;


GASEngine.CircleMesh = function(radius, segments, halfFlag, thetaStart, thetaLength)
{
    GASEngine.Mesh.call(this);

    radius = radius || 1;
    segments = segments !== undefined ? Math.max( 3, segments ) : 8;

    var arc = halfFlag ? 0.5 : 1;

    thetaStart = thetaStart !== undefined ? thetaStart : 0;
    thetaLength = thetaLength !== undefined ? thetaLength : Math.PI * 2;

    this.setDrawMode('LINES');
    this.type = 'circle';

    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

	// helper variables
    var i, s;
    var vertex = new GASEngine.Vector3();
    var uv = new GASEngine.Vector2();

    for ( s = 0, i = 0; s <= segments * arc; s ++, i += 3 ) {

        var segment = thetaStart + s / segments * thetaLength;

        vertex.y = radius * Math.cos( segment );
        vertex.z = radius * Math.sin( segment );

        vertices.push( vertex.x, vertex.y, vertex.z );

        // normal
        normals.push( 1, 0, 0 );

        // uvs
        uv.x = ( vertices[ i ] / radius + 1 ) / 2;
        uv.y = ( vertices[ i + 1 ] / radius + 1 ) / 2;

        uvs.push( uv.x, uv.y );
    }

    // indices
    for ( i = 0; i < segments * arc; i ++ ) {
        indices.push( i, i + 1);
    }
    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.CircleMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.CircleMesh.constructor = GASEngine.CircleMesh;

GASEngine.CircleMesh_v2 = function(radius, segments, halfFlag, thetaStart, thetaLength)
{
    GASEngine.Mesh.call(this);

    radius = radius || 1;
    segments = segments !== undefined ? Math.max( 3, segments ) : 8;

    var tube = 10;

    var arc = halfFlag ? 0.5 : 1;

    thetaStart = thetaStart !== undefined ? thetaStart : 0;
    thetaLength = thetaLength !== undefined ? thetaLength : Math.PI * 2;

    this.type = 'circle';

    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

	// helper variables
    var i, s;
    var vertex = new GASEngine.Vector3();
    var uv = new GASEngine.Vector2();

    for ( s = 0, i = 0; s <= segments; s ++, i += 12 ) 
    {
        // vertex
        // vertex.x = radius * Math.cos( segment );
        // vertex.y = radius * Math.sin( segment );

        var segment = thetaStart + s / segments * thetaLength;
        vertex.y = radius * Math.cos( segment );
        vertex.z = radius * Math.sin( segment );
        vertices.push( vertex.x, vertex.y, vertex.z );

        vertex.y = (radius + tube) * Math.cos( segment );
        vertex.z = (radius + tube)* Math.sin( segment );
        vertices.push( vertex.x, vertex.y, vertex.z );

        segment = thetaStart + ( s + 1) / segments * thetaLength;
        vertex.y = radius * Math.cos( segment );
        vertex.z = radius * Math.sin( segment );
        vertices.push( vertex.x, vertex.y, vertex.z );

        vertex.y = (radius + tube) * Math.cos( segment );
        vertex.z = (radius + tube)* Math.sin( segment );
        vertices.push( vertex.x, vertex.y, vertex.z );

        // normal
        if(halfFlag)
        {
            normals.push( 1, 0, 0 );
            normals.push( 1, 0, 0 );
            normals.push( 1, 0, 0 );
            normals.push( 1, 0, 0 );
        }
        else 
        {    
            normals.push( -1, 0, 0 );
            normals.push( -1, 0, 0 );
            normals.push( -1, 0, 0 );
            normals.push( -1, 0, 0 );
        }

        // uvs

        uv.x = ( vertices[ i ] / radius + 1 ) / 2;
        uv.y = ( vertices[ i + 1 ] / radius + 1 ) / 2;
        uvs.push( uv.x, uv.y );

        uv.x = ( vertices[ i + 3] / radius + 1 ) / 2;
        uv.y = ( vertices[ i + 4 ] / radius + 1 ) / 2;
        uvs.push( uv.x, uv.y );

        uv.x = ( vertices[ i + 6] / radius + 1 ) / 2;
        uv.y = ( vertices[ i + 7 ] / radius + 1 ) / 2;
        uvs.push( uv.x, uv.y );

        uv.x = ( vertices[ i + 9] / radius + 1 ) / 2;
        uv.y = ( vertices[ i + 10 ] / radius + 1 ) / 2;
        uvs.push( uv.x, uv.y );
    }

    // indices
    var len = vertices.length / 3;
    for ( i = 0; i < len; i += 4 ) 
    {
        indices.push( i, i + 2, i + 1);
        indices.push( i+1, i + 2, i + 3);
    }

    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.CircleMesh_v2.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.CircleMesh_v2.constructor = GASEngine.CircleMesh_v2;

GASEngine.BoxMesh = function(size)
{
    GASEngine.Mesh.call(this);

    size = size || 1;
    this.type = 'box';

    var max = new GASEngine.Vector3(), min = new GASEngine.Vector3();
    max.set(size / 2, size / 2, size / 2);
    min.set(- size / 2, - size / 2, - size / 2);
    // buffers
    var indices = [];

    var vertices = [
        max.x, max.y, max.z,
        min.x, max.y, max.z,
        min.x, min.y, max.z,
        max.x, min.y, max.z,
        max.x, max.y, min.z,
        min.x, max.y, min.z,
        min.x, min.y, min.z,
        max.x, min.y, min.z
    ];

    var indices = [
        0, 1, 2, 0, 2, 3,
        4, 0, 3, 4, 3, 7,
        5, 4, 7, 5, 7, 6,
        1, 5, 6, 1, 6, 2,
        0, 4, 5, 0, 5, 1,
        3, 7, 6, 3, 6, 2
    ];
    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.BoxMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.BoxMesh.constructor = GASEngine.BoxMesh;

GASEngine.EmptyMesh = function()
{
    GASEngine.Mesh.call(this);

    this.setDrawMode('LINES');
    this.type = 'circle';

    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

    // indices

    for ( i = 1; i <= segments; i ++ ) {

        indices.push( i, i + 1);

    }
    indices.push( segments - 1, 1);

    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.EmptyMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.EmptyMesh.constructor = GASEngine.EmptyMesh;


// Polyhedron多面缓冲几何体
GASEngine.PolyhedronMesh = function()
{
    GASEngine.Mesh.call(this);

    this.setDrawMode('LINES');
    this.type = 'circle';

    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

    // indices

    for ( i = 1; i <= segments; i ++ ) {

        indices.push( i, i + 1);

    }
    indices.push( segments - 1, 1);

    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.PolyhedronMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.PolyhedronMesh.constructor = GASEngine.PolyhedronMesh;


// Torus圆环
GASEngine.TorusMesh = function(radius, tube, radialSegments, tubularSegments, arc)
{
    GASEngine.Mesh.call(this);

    radius = radius || 1;
    tube = tube || 0.4;
    radialSegments = Math.floor( radialSegments ) || 8;
    tubularSegments = Math.floor( tubularSegments ) || 6;
    arc = arc || Math.PI * 2;

    this.type = 'torus';

    // buffers
    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

    // helper variables
    var center = new GASEngine.Vector3();
    var vertex = new GASEngine.Vector3();
    var normal = new GASEngine.Vector3();

    var j, i;

    // generate vertices, normals and uvs
    for (j = 0; j <= radialSegments; j ++) 
    {
        for (i = 0; i <= tubularSegments; i ++) 
        {
            var u = i / tubularSegments * arc;
            var v = j / radialSegments * Math.PI * 2;
            // vertex
            vertex.x = (radius + tube * Math.cos(v)) * Math.cos(u);
            vertex.y = (radius + tube * Math.cos(v)) * Math.sin(u);
            vertex.z = tube * Math.sin( v );

            vertices.push(vertex.x, vertex.y, vertex.z);

            // normal
            center.x = radius * Math.cos( u );
            center.y = radius * Math.sin( u );
            normal.subVectors(vertex, center).normalize();

            normals.push(normal.x, normal.y, normal.z);

            // uv
            uvs.push(i / tubularSegments);
            uvs.push(j / radialSegments);
        }
    }

    // generate indices
    for ( j = 1; j <= radialSegments; j ++ ) 
    {
        for ( i = 1; i <= tubularSegments; i ++ ) 
        {
            // indices
            var a = ( tubularSegments + 1 ) * j + i - 1;
            var b = ( tubularSegments + 1 ) * ( j - 1 ) + i - 1;
            var c = ( tubularSegments + 1 ) * ( j - 1 ) + i;
            var d = ( tubularSegments + 1 ) * j + i;

            // faces
            indices.push( a, b, d );
            // indices.push( b, c, d );
            indices.push( b, d, c );
        }
    }
    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.TorusMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.TorusMesh.constructor = GASEngine.TorusMesh;


GASEngine.SphereMesh = function(radius, widthSegments, heightSegments, phiStart, phiLength, thetaStart, thetaLength)
{
    GASEngine.Mesh.call(this);

    radius = radius || 1;

    widthSegments = Math.max( 3, Math.floor( widthSegments ) || 100 );
    heightSegments = Math.max( 2, Math.floor( heightSegments ) || 100 );

    phiStart = phiStart !== undefined ? phiStart : 0;
    phiLength = phiLength !== undefined ? phiLength : Math.PI * 2;

    thetaStart = thetaStart !== undefined ? thetaStart : 0;
    thetaLength = thetaLength !== undefined ? thetaLength : Math.PI;

    var thetaEnd = thetaStart + thetaLength;
    var ix, iy;

    var index = 0;
    var grid = [];

    var vertex = new GASEngine.Vector3();
    var normal = new GASEngine.Vector3();

    // buffers

    var indices = [];
    var vertices = [];
    var normals = [];
    var uvs = [];

    // generate vertices, normals and uvs

    for ( iy = 0; iy <= heightSegments; iy ++ ) {

        var verticesRow = [];

        var v = iy / heightSegments;

        for ( ix = 0; ix <= widthSegments; ix ++ ) {

            var u = ix / widthSegments;

            // vertex

            vertex.x = - radius * Math.cos( phiStart + u * phiLength ) * Math.sin( thetaStart + v * thetaLength );
            vertex.y = radius * Math.cos( thetaStart + v * thetaLength );
            vertex.z = radius * Math.sin( phiStart + u * phiLength ) * Math.sin( thetaStart + v * thetaLength );

            vertices.push( vertex.x, vertex.y, vertex.z );

            // normal

            normal.set( vertex.x, vertex.y, vertex.z ).normalize();
            normals.push( normal.x, normal.y, normal.z );

            // uv

            uvs.push( u, 1 - v );

            verticesRow.push( index ++ );

        }

        grid.push( verticesRow );

    }

    // indices
    for ( iy = 0; iy < heightSegments; iy ++ ) {

        for ( ix = 0; ix < widthSegments; ix ++ ) {

            var a = grid[ iy ][ ix + 1 ];
            var b = grid[ iy ][ ix ];
            var c = grid[ iy + 1 ][ ix ];
            var d = grid[ iy + 1 ][ ix + 1 ];

            if ( iy !== 0 || thetaStart > 0 ) indices.push( a, b, d );
            if ( iy !== heightSegments - 1 || thetaEnd < Math.PI ) indices.push( b, c, d );

        }
    }

    this.type = 'sphere';
    var count = indices.length;
    this.addStream('position', new Float32Array(vertices));
    this.addStream('normal', new Float32Array(normals));
    this.addStream('uv', new Float32Array(uvs));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('subMesh', [{ 'start': 0, 'count': count}]);
};

GASEngine.SphereMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.SphereMesh.constructor = GASEngine.SphereMesh;


GASEngine.OctahedronMesh = function(pos, size)
{
    GASEngine.Mesh.call(this);

    size = size || 1;
    this.type = 'octahedron';

    var norm = pos.clone().normalize();
    
    var midpos = new GASEngine.Vector3(0, 0, 0);
    var rv = new GASEngine.Vector3(norm.x, norm.y - size, norm.z);
    //console.log(rv);

    var offset1 = new GASEngine.Vector3();
    var offset2 = new GASEngine.Vector3();
    offset1.crossVectors(norm, rv);
    offset2.crossVectors(norm, offset1);

    // var max = new GASEngine.Vector3(), min = new GASEngine.Vector3();
    // max.set(size / 2, size / 2, size / 2);
    // min.set(- size / 2, - size / 2, - size / 2);
    // buffers
    var indices = [];

    var vertices = [
        midpos.x - offset1.x, midpos.y - offset1.y, midpos.z - offset1.z,
        midpos.x - offset1.x, midpos.y - offset1.y, midpos.z - offset1.z,
        midpos.x - offset1.x, midpos.y - offset1.y, midpos.z - offset1.z,
        midpos.x - offset1.x, midpos.y - offset1.y, midpos.z - offset1.z,
        midpos.x + offset1.x, midpos.y + offset1.y, midpos.z + offset1.z,
        midpos.x + offset1.x, midpos.y + offset1.y, midpos.z + offset1.z,
        midpos.x + offset1.x, midpos.y + offset1.y, midpos.z + offset1.z,
        midpos.x + offset1.x, midpos.y + offset1.y, midpos.z + offset1.z,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        midpos.x - offset2.x, midpos.y - offset2.y, midpos.z - offset2.z,
        midpos.x - offset2.x, midpos.y - offset2.y, midpos.z - offset2.z,
        midpos.x - offset2.x, midpos.y - offset2.y, midpos.z - offset2.z,
        midpos.x - offset2.x, midpos.y - offset2.y, midpos.z - offset2.z,
        midpos.x + offset2.x, midpos.y + offset2.y, midpos.z + offset2.z,
        midpos.x + offset2.x, midpos.y + offset2.y, midpos.z + offset2.z,
        midpos.x + offset2.x, midpos.y + offset2.y, midpos.z + offset2.z,
        midpos.x + offset2.x, midpos.y + offset2.y, midpos.z + offset2.z,
        pos.x, pos.y, pos.z,
        pos.x, pos.y, pos.z,
        pos.x, pos.y, pos.z,
        pos.x, pos.y, pos.z,
    ];

    var indices = [
        0, 12, 20, 21, 13, 4, 5, 16, 22, 23, 17, 1,
        2, 8, 14, 15, 9, 6, 7, 10, 18, 19, 11, 3
    ];
    var count = indices.length;

    var outNormals = new Float32Array(vertices.length);
    var subMeshes = [{ 'start': 0, 'count': count }];
    GASEngine.Utilities.computeNormals(indices, vertices, subMeshes, outNormals);


    var colors = [
        0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
        0.1, 0.1, 0.1, 0.1, 0.1, 0.1
    ];

    this.addStream('position', new Float32Array(vertices));
    this.addStream('index', new Uint32Array(indices));
    this.addStream('normal', outNormals);
    this.addStream('subMesh', subMeshes);
    this.addStream('color', new Float32Array(colors));
};

GASEngine.OctahedronMesh.prototype = Object.create(GASEngine.Mesh.prototype);
GASEngine.OctahedronMesh.constructor = GASEngine.OctahedronMesh;


