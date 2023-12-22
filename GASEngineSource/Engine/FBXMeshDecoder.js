// parse Geometry data from FBXTree and return map of BufferGeometries
GASEngine.FBXMeshDecoder = function()
{
    this._fbxTree = null;
    this._connectionMap_ = null;
}

GASEngine.FBXMeshDecoder.prototype = {

    constructor: GASEngine.FBXMeshDecoder,

    // Parse nodes in FBXTree.Objects.Geometry
    parse: function (deformers, fbxTree, connections) {
        this._fbxTree = fbxTree;
        this._connectionMap_ = connections;

        var meshObjects = new Map();

        if ('Geometry' in this._fbxTree.Objects) {

            var geoNodes = this._fbxTree.Objects.Geometry;

            for (var nodeID in geoNodes) {

                var relationships = this._connectionMap_.get(parseInt(nodeID));
                var geo = this.parseGeometry(relationships, geoNodes[nodeID], deformers);

                meshObjects.set(parseInt(nodeID), geo);
            }
        }
        return meshObjects;
    },

    // Parse single node in FBXTree.Objects.Geometry
    parseGeometry: function (relationships, geoNode, deformers) {

        switch (geoNode.attrType) {
            case 'Mesh':
                return this.parseMeshGeometry(relationships, geoNode, deformers);
                break;
            case 'NurbsCurve':
                return this.parseNurbsGeometry(geoNode);
                break;
        }
    },

    // Parse single node mesh geometry in FBXTree.Objects.Geometry
    parseMeshGeometry: function (relationships, geoNode, deformers) {
        var skeletons = deformers.skeletons;
        var morphTargets = deformers.morphTargets;
        var modelNodes = relationships.parents.map(function (parent) {
            return this._fbxTree.Objects.Model[parent.ID];
        }.bind(this));

        // don't create geometry if it is not associated with any models
        if (modelNodes.length === 0) return;

        var skeleton = relationships.children.reduce(function (skeleton, child) {
            if (skeletons[child.ID] !== undefined) skeleton = skeletons[child.ID];
            return skeleton;
        }, null);

        var morphTarget = relationships.children.reduce(function (morphTarget, child) {
            if (morphTargets[child.ID] !== undefined) morphTarget = morphTargets[child.ID];
            return morphTarget;
        }, null);

        // TODO: if there is more than one model associated with the geometry, AND the models have
        // different geometric transforms, then this will cause problems
        // if ( modelNodes.length > 1 ) { }

        // For now just assume one model and get the preRotations from that
        var modelNode = modelNodes[0];

        var transformData = {};

        if ('RotationOrder' in modelNode) transformData.eulerOrder = modelNode.RotationOrder.value;
        if ('GeometricTranslation' in modelNode) transformData.translation = modelNode.GeometricTranslation.value;
        if ('GeometricRotation' in modelNode) transformData.rotation = modelNode.GeometricRotation.value;
        if ('GeometricScaling' in modelNode) transformData.scale = modelNode.GeometricScaling.value;

        var transform = GASEngine.Utilities.generateTransform(transformData);
        return this.genGeometry(geoNode, skeleton, morphTarget, transform);
    },

    // Generate a GASEngine.BufferGeometry from a node in FBXTree.Objects.Geometry
    genGeometry: function (geoNode, skeleton, morphTarget, preTransform)
    {
        var geo = GASEngine.MeshFactory.Instance.create();
        geo.uniqueID = GASEngine.generateUUID();
        geo.setDrawMode('TRIANGLES');
        if (geoNode.attrName) geo.name = geoNode.attrName;

        var geoInfo = this.parseGeoNode(geoNode, skeleton);
        var buffers = this.genBuffers(geoInfo);

        //position
        var v1 = new GASEngine.Vector3();
        var il = buffers.vertex.length;
        var outPositions = new Float32Array(il);
        for (var i = 0; i < il; i += 3)
        {
            v1.x = buffers.vertex[i];
            v1.y = buffers.vertex[i + 1];
            v1.z = buffers.vertex[i + 2];
            v1.applyMatrix4(preTransform);
            outPositions[i] = v1.x;
            outPositions[i + 1] = v1.y;
            outPositions[i + 2] = v1.z;
        }
        geo.addStream('position', outPositions);

        //index
        var iil = il / 3;
        var outIndices = new Uint32Array(iil);
        for (var ii = 0; ii < iil; ii++)
        {
            outIndices[ii] = ii;
        }
        geo.addStream('index', outIndices);
        geo.addStream('topology', geo.createWireframeIndices(outIndices));

        //color
        if (buffers.colors.length > 0)
        {
            var ijl = buffers.colors.length;
            var outColors = new Uint8Array(ijl);
            for (var ij = 0; ij < ijl; ij++) {
                outColors[ij] = Math.floor(buffers.colors[ij] * 255);
            }
            geo.addStream('color', outColors);
        }

        if (skeleton)
        {
            var jl = buffers.weightsIndices.length;
            var outSkinIndexs = new Uint16Array(jl);
            for (var j = 0; j < jl; j++)
            {
                outSkinIndexs[j] = buffers.weightsIndices[j];
            }
            geo.addStream('skinIndex', outSkinIndexs);

            var kl = buffers.vertexWeights.length;
            var outSkinWeights = new Float32Array(kl);
            for (var k = 0; k < kl; k++)
            {
                outSkinWeights[k] = buffers.vertexWeights[k];
            }
            geo.addStream('skinWeight', outSkinWeights);
        }

        if (buffers.normal.length > 0)
        {
            var nl = buffers.normal.length;
            var outNormals = new Float32Array(nl);
            var normalMatrix = new GASEngine.Matrix3().getNormalMatrix(preTransform);
            for (var n = 0; n < nl; n += 3)
            {
                v1.x = buffers.normal[n];
                v1.y = buffers.normal[n + 1];
                v1.z = buffers.normal[n + 2];
                v1.applyMatrix3(normalMatrix);
                outNormals[n] = v1.x;
                outNormals[n + 1] = v1.y;
                outNormals[n + 2] = v1.z;
            }
            geo.addStream('normal', outNormals);
        }

        for (var uv = 0, uvl = buffers.uvs.length; uv < uvl; uv++)
        {
            // 'uv' and subsequent uv buffers are called 'uv1', 'uv2', ...
            var name = (uv === 0)? 'uv': 'uv' + (uv + 1).toString();
            var ul = buffers.uvs[uv].length;
            var outUvs = new Float32Array(ul);
            for(var u = 0; u < ul; u++) {
                outUvs[u] = buffers.uvs[uv][u];
            }
            geo.addStream(name, outUvs);
        }

        if (geoInfo.material && geoInfo.material.mappingType !== 'AllSame')
        {
            // Convert the material indices of each vertex into rendering groups on the geometry.
            var prevMaterialIndex = buffers.materialIndex[0];
            var startIndex = 0;
            var subMeshes = [];

            for (var m = 0, ml = buffers.materialIndex.length; m < ml; m++)
            {
                var currentIndex = buffers.materialIndex[m];
                if (currentIndex !== prevMaterialIndex)
                {
                    subMeshes.push({ 'start': startIndex, 'count': m - startIndex, 'materialIndex': prevMaterialIndex });
                    prevMaterialIndex = currentIndex;
                    startIndex = m;
                }
            }

            // the loop above doesn't add the last group, do that here.
            if (subMeshes.length > 0)
            {
                var lastGroup = subMeshes[subMeshes.length - 1];
                var lastIndex = lastGroup.start + lastGroup.count;

                if (lastIndex !== buffers.materialIndex.length)
                {
                    subMeshes.push({ 'start': lastIndex, 'count': buffers.materialIndex.length - lastIndex, 'materialIndex': prevMaterialIndex });
                }
            }

            // case where there are multiple materials but the whole geometry is only
            // using one of them
            if (subMeshes.length === 0)
            {
                subMeshes.push({ 'start': 0, 'count': buffers.materialIndex.length, 'materialIndex': buffers.materialIndex[0] });
            }
        }
        else {
            subMeshes = [{ 'start': 0, 'count': outIndices.length }];
        }
        geo.addStream('subMesh', subMeshes);

        this.addMorphTargets(geo, geoNode, morphTarget, preTransform);
        return geo;
    },

    parseGeoNode: function (geoNode, skeleton) {
        var geoInfo = {};
        geoInfo.vertexPositions = (geoNode.Vertices !== undefined) ? geoNode.Vertices.a : [];
        geoInfo.vertexIndices = (geoNode.PolygonVertexIndex !== undefined) ? geoNode.PolygonVertexIndex.a : [];
        if (geoNode.LayerElementColor) {
            geoInfo.color = this.parseVertexColors(geoNode.LayerElementColor[0]);
        }

        if (geoNode.LayerElementMaterial) {
            geoInfo.material = this.parseMaterialIndices(geoNode.LayerElementMaterial[0]);
        }

        if (geoNode.LayerElementNormal) {
            geoInfo.normal = this.parseNormals(geoNode.LayerElementNormal[0]);
        }

        if (geoNode.LayerElementUV) {
            geoInfo.uv = [];
            var i = 0;
            while (geoNode.LayerElementUV[i]) {
                geoInfo.uv.push(this.parseUVs(geoNode.LayerElementUV[i]));
                i++;
            }
        }

        geoInfo.weightTable = {};
        if (skeleton !== null) {
            geoInfo.skeleton = skeleton;
            skeleton.rawBones.forEach(function (rawBone, i) {

                // loop over the bone's vertex indices and weights
                rawBone.indices.forEach(function (index, j) {
                    if (geoInfo.weightTable[index] === undefined) geoInfo.weightTable[index] = [];

                    geoInfo.weightTable[index].push({
                        id: i,
                        weight: rawBone.weights[j],
                    });
                });
            });
        }
        return geoInfo;
    },

    // extracts the data from the correct position in the FBX array based on indexing type
    getData: function (polygonVertexIndex, polygonIndex, vertexIndex, infoObject)
    {
        var index;
        switch (infoObject.mappingType)
        {
            case 'ByPolygonVertex':
                index = polygonVertexIndex;
                break;
            case 'ByPolygon':
                index = polygonIndex;
                break;
            case 'ByVertice':
                index = vertexIndex;
                break;
            case 'AllSame':
                index = infoObject.indices[0];
                break;
            default:
                console.warn('GASEngine.FBXLoader: unknown attribute mapping type ' + infoObject.mappingType);
        }

        if (infoObject.referenceType === 'IndexToDirect') index = infoObject.indices[index];

        var from = index * infoObject.dataSize;
        var to = from + infoObject.dataSize;

        var dataArray = [];
        return GASEngine.Utilities.slice(dataArray, infoObject.buffer, from, to);
    },

    genBuffers: function (geoInfo) {

        var buffers = {
            vertex: [],
            normal: [],
            colors: [],
            uvs: [],
            materialIndex: [],
            vertexWeights: [],
            weightsIndices: [],
        };

        var polygonIndex = 0;
        var faceLength = 0;
        var displayedWeightsWarning = false;

        // these will hold data for a single face
        var facePositionIndexes = [];
        var faceNormals = [];
        var faceColors = [];
        var faceUVs = [];
        var faceWeights = [];
        var faceWeightIndices = [];

        var polygonVertexIndexLength = geoInfo.vertexIndices.length;
        for(var polygonVertexIndex = 0; polygonVertexIndex < polygonVertexIndexLength; polygonVertexIndex++) {
            var vertexIndex = geoInfo.vertexIndices[polygonVertexIndex];
            var endOfFace = false;
            // Face index and vertex index arrays are combined in a single array
            // A cube with quad faces looks like this:
            // PolygonVertexIndex: *24 {
            //  a: 0, 1, 3, -3, 2, 3, 5, -5, 4, 5, 7, -7, 6, 7, 1, -1, 1, 7, 5, -4, 6, 0, 2, -5
            //  }
            // Negative numbers mark the end of a face - first face here is 0, 1, 3, -3
            // to find index of last vertex bit shift the index: ^ - 1
            if (vertexIndex < 0) {
                vertexIndex = vertexIndex ^ - 1; // equivalent to ( x * -1 ) - 1
                endOfFace = true;
            }
            var weightIndices = [];
            var weights = [];
            facePositionIndexes.push(vertexIndex * 3, vertexIndex * 3 + 1, vertexIndex * 3 + 2);

            if (geoInfo.color) {
                var data = this.getData(polygonVertexIndex, polygonIndex, vertexIndex, geoInfo.color);
                faceColors.push(data[0], data[1], data[2]);
            }

            if (geoInfo.skeleton) {
                if (geoInfo.weightTable[vertexIndex] !== undefined) {
                    for(var wt of geoInfo.weightTable[vertexIndex]) {
                        weights.push(wt.weight);
                        weightIndices.push(wt.id);
                    }
                }

                if (weights.length > 4) {
                    if (!displayedWeightsWarning) {
                        console.warn('GASEngine.FBXLoader: Vertex has more than 4 skinning weights assigned to vertex. Deleting additional weights.');
                        displayedWeightsWarning = true;
                    }
                    var wIndex = [0, 0, 0, 0];
                    var Weight = [0, 0, 0, 0];
                    for(var weightIndex = 0; weightIndex < weights.length; weightIndex++) {
                        var currentWeight = weights[weightIndex];
                        var currentIndex = weightIndices[weightIndex];

                        Weight.forEach(function (comparedWeight, comparedWeightIndex, comparedWeightArray) {
                            if (currentWeight > comparedWeight) {
                                comparedWeightArray[comparedWeightIndex] = currentWeight;
                                currentWeight = comparedWeight;
                                var tmp = wIndex[comparedWeightIndex];
                                wIndex[comparedWeightIndex] = currentIndex;
                                currentIndex = tmp;
                            }
                        });
                    }
                    weightIndices = wIndex;
                    weights = Weight;
                }

                // if the weight array is shorter than 4 pad with 0s
                while (weights.length < 4) {
                    weights.push(0);
                    weightIndices.push(0);
                }

                for (var i = 0; i < 4; ++i) {
                    faceWeights.push(weights[i]);
                    faceWeightIndices.push(weightIndices[i]);
                }
            }

            if (geoInfo.normal) {
                var data = this.getData(polygonVertexIndex, polygonIndex, vertexIndex, geoInfo.normal);
                faceNormals.push(data[0], data[1], data[2]);
            }

            var materialIndex = 0;
            if (geoInfo.material && geoInfo.material.mappingType !== 'AllSame') {
                materialIndex = this.getData(polygonVertexIndex, polygonIndex, vertexIndex, geoInfo.material)[0];
            }

            if (geoInfo.uv) {
                for(var i = 0, il = geoInfo.uv.length; i < il; i++) {
                    var data = this.getData(polygonVertexIndex, polygonIndex, vertexIndex, geoInfo.uv[i]);
                    if (faceUVs[i] === undefined) {
                        faceUVs[i] = [];
                    }
                    faceUVs[i].push(data[0]);
                    faceUVs[i].push(data[1]);
                }
            }
            faceLength++;

            if (endOfFace) {
                this.genFace(buffers, geoInfo, facePositionIndexes, materialIndex, faceNormals, faceColors, faceUVs, faceWeights, faceWeightIndices, faceLength);
                polygonIndex++;
                faceLength = 0;

                // reset arrays for the next face
                facePositionIndexes = [];
                faceNormals = [];
                faceColors = [];
                faceUVs = [];
                faceWeights = [];
                faceWeightIndices = [];
            }
        }
        return buffers;
    },

    // Generate data for a single face in a geometry. If the face is a quad then split it into 2 tris
    genFace: function (buffers, geoInfo, facePositionIndexes, materialIndex, faceNormals, faceColors, faceUVs, faceWeights, faceWeightIndices, faceLength) {

        for (var i = 2; i < faceLength; i++) {

            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[0]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[1]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[2]]);

            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[(i - 1) * 3]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[(i - 1) * 3 + 1]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[(i - 1) * 3 + 2]]);

            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[i * 3]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[i * 3 + 1]]);
            buffers.vertex.push(geoInfo.vertexPositions[facePositionIndexes[i * 3 + 2]]);

            if (geoInfo.skeleton) {

                buffers.vertexWeights.push(faceWeights[0]);
                buffers.vertexWeights.push(faceWeights[1]);
                buffers.vertexWeights.push(faceWeights[2]);
                buffers.vertexWeights.push(faceWeights[3]);

                buffers.vertexWeights.push(faceWeights[(i - 1) * 4]);
                buffers.vertexWeights.push(faceWeights[(i - 1) * 4 + 1]);
                buffers.vertexWeights.push(faceWeights[(i - 1) * 4 + 2]);
                buffers.vertexWeights.push(faceWeights[(i - 1) * 4 + 3]);

                buffers.vertexWeights.push(faceWeights[i * 4]);
                buffers.vertexWeights.push(faceWeights[i * 4 + 1]);
                buffers.vertexWeights.push(faceWeights[i * 4 + 2]);
                buffers.vertexWeights.push(faceWeights[i * 4 + 3]);

                buffers.weightsIndices.push(faceWeightIndices[0]);
                buffers.weightsIndices.push(faceWeightIndices[1]);
                buffers.weightsIndices.push(faceWeightIndices[2]);
                buffers.weightsIndices.push(faceWeightIndices[3]);

                buffers.weightsIndices.push(faceWeightIndices[(i - 1) * 4]);
                buffers.weightsIndices.push(faceWeightIndices[(i - 1) * 4 + 1]);
                buffers.weightsIndices.push(faceWeightIndices[(i - 1) * 4 + 2]);
                buffers.weightsIndices.push(faceWeightIndices[(i - 1) * 4 + 3]);

                buffers.weightsIndices.push(faceWeightIndices[i * 4]);
                buffers.weightsIndices.push(faceWeightIndices[i * 4 + 1]);
                buffers.weightsIndices.push(faceWeightIndices[i * 4 + 2]);
                buffers.weightsIndices.push(faceWeightIndices[i * 4 + 3]);
            }

            if (geoInfo.color) {
                buffers.colors.push(faceColors[0]);
                buffers.colors.push(faceColors[1]);
                buffers.colors.push(faceColors[2]);
                buffers.colors.push(1.0);

                buffers.colors.push(faceColors[(i - 1) * 3]);
                buffers.colors.push(faceColors[(i - 1) * 3 + 1]);
                buffers.colors.push(faceColors[(i - 1) * 3 + 2]);
                buffers.colors.push(1.0);

                buffers.colors.push(faceColors[i * 3]);
                buffers.colors.push(faceColors[i * 3 + 1]);
                buffers.colors.push(faceColors[i * 3 + 2]);
                buffers.colors.push(1.0);
            }

            if (geoInfo.material && geoInfo.material.mappingType !== 'AllSame') {

                buffers.materialIndex.push(materialIndex);
                buffers.materialIndex.push(materialIndex);
                buffers.materialIndex.push(materialIndex);

            }

            if (geoInfo.normal) {
                buffers.normal.push(faceNormals[0]);
                buffers.normal.push(faceNormals[1]);
                buffers.normal.push(faceNormals[2]);

                buffers.normal.push(faceNormals[(i - 1) * 3]);
                buffers.normal.push(faceNormals[(i - 1) * 3 + 1]);
                buffers.normal.push(faceNormals[(i - 1) * 3 + 2]);

                buffers.normal.push(faceNormals[i * 3]);
                buffers.normal.push(faceNormals[i * 3 + 1]);
                buffers.normal.push(faceNormals[i * 3 + 2]);

            }

            if (geoInfo.uv) {
                for(var j = 0, jl = geoInfo.uv.length; j < jl; j++) {
                    if (buffers.uvs[j] === undefined)
                        buffers.uvs[j] = [];

                    buffers.uvs[j].push(faceUVs[j][0]);
                    buffers.uvs[j].push(faceUVs[j][1]);
                    buffers.uvs[j].push(faceUVs[j][(i - 1) * 2]);
                    buffers.uvs[j].push(faceUVs[j][(i - 1) * 2 + 1]);
                    buffers.uvs[j].push(faceUVs[j][i * 2]);
                    buffers.uvs[j].push(faceUVs[j][i * 2 + 1]);
                }
            }
        }
    },

    addMorphTargets: function (parentGeo, parentGeoNode, morphTarget, preTransform) {

        if (morphTarget === null) return;

        //parentGeo.morphAttributes.position = [];
        //parentGeo.morphAttributes.normal = [];

        for(var rawTarget of morphTarget.rawTargets) {
            var morphGeoNode = this._fbxTree.Objects.Geometry[rawTarget.geoID];
            if (morphGeoNode !== undefined) {
                this.genMorphGeometry(parentGeo, parentGeoNode, morphGeoNode, preTransform);
            }
        }
    },

    // a morph geometry node is similar to a standard node, and the node is also contained
    // in FBXTree.Objects.Geometry, however it can only have attributes for position, normal
    // and a special attribute Index defining which vertices of the original geometry are affected
    // Normal and position attributes only have data for the vertices that are affected by the morph
    genMorphGeometry: function (parentGeo, parentGeoNode, morphGeoNode, preTransform)
    {
        var morphGeo = GASEngine.MeshFactory.Instance.create();
        morphGeo.uniqueID = GASEngine.generateUUID();
        if (morphGeoNode.attrName) morphGeo.name = morphGeoNode.attrName;

        var vertexIndices = (parentGeoNode.PolygonVertexIndex !== undefined) ? parentGeoNode.PolygonVertexIndex.a : [];
        // make a copy of the parent's vertex positions
        var vertexPositions = (parentGeoNode.Vertices !== undefined) ? parentGeoNode.Vertices.a.slice() : [];

        var morphPositions = (morphGeoNode.Vertices !== undefined) ? morphGeoNode.Vertices.a : [];
        var indices = (morphGeoNode.Indexes !== undefined) ? morphGeoNode.Indexes.a : [];

        for (var i = 0, il = indices.length; i < il; i++)
        {
            var morphIndex = indices[i] * 3;
            // FBX format uses blend shapes rather than morph targets. This can be converted
            // by additively combining the blend shape positions with the original geometry's positions
            vertexPositions[morphIndex] += morphPositions[i * 3];
            vertexPositions[morphIndex + 1] += morphPositions[i * 3 + 1];
            vertexPositions[morphIndex + 2] += morphPositions[i * 3 + 2];
        }

        // TODO: add morph normal support
        var morphGeoInfo = {
            vertexIndices: vertexIndices,
            vertexPositions: vertexPositions,
        };

        var morphBuffers = this.genBuffers(morphGeoInfo);

        var v1 = new GASEngine.Vector3();
        var jl = morphBuffers.vertex.length;
        var morphPositions = new Float32Array(jl);
        for (var j = 0; j < jl; j += 3)
        {
            v1.x = morphBuffers.vertex[j];
            v1.y = morphBuffers.vertex[j + 1];
            v1.z = morphBuffers.vertex[j + 2];
            v1.applyMatrix4(preTransform);
            morphPositions[j] = v1.x;
            morphPositions[j + 1] = v1.y;
            morphPositions[j + 2] = v1.z;
        }
        morphGeo.addStream('position', morphPositions);

        //add morph target
        parentGeo.addMorphTarget(morphGeo);
    },

    // Parse normal from FBXTree.Objects.Geometry.LayerElementNormal if it exists
    parseNormals: function (NormalNode) {
        var mappingType = NormalNode.MappingInformationType;
        var referenceType = NormalNode.ReferenceInformationType;
        var buffer = NormalNode.Normals.a;
        var indexBuffer = [];
        if (referenceType === 'IndexToDirect') {
            if ('NormalIndex' in NormalNode) {
                indexBuffer = NormalNode.NormalIndex.a;
            } else if ('NormalsIndex' in NormalNode) {
                indexBuffer = NormalNode.NormalsIndex.a;
            }
        }

        return {
            dataSize: 3,
            buffer: buffer,
            indices: indexBuffer,
            mappingType: mappingType,
            referenceType: referenceType
        };
    },

    // Parse UVs from FBXTree.Objects.Geometry.LayerElementUV if it exists
    parseUVs: function (UVNode) {
        var mappingType = UVNode.MappingInformationType;
        var referenceType = UVNode.ReferenceInformationType;
        var buffer = UVNode.UV.a;
        var indexBuffer = [];
        if (referenceType === 'IndexToDirect') {
            indexBuffer = UVNode.UVIndex.a;
        }

        return {
            dataSize: 2,
            buffer: buffer,
            indices: indexBuffer,
            mappingType: mappingType,
            referenceType: referenceType
        };

    },

    // Parse Vertex Colors from FBXTree.Objects.Geometry.LayerElementColor if it exists
    parseVertexColors: function (ColorNode) {
        var mappingType = ColorNode.MappingInformationType;
        var referenceType = ColorNode.ReferenceInformationType;
        var buffer = ColorNode.Colors.a;
        var indexBuffer = [];
        if (referenceType === 'IndexToDirect') {
            indexBuffer = ColorNode.ColorIndex.a;
        }

        return {
            dataSize: 4,
            buffer: buffer,
            indices: indexBuffer,
            mappingType: mappingType,
            referenceType: referenceType
        };
    },

    // Parse mapping and material data in FBXTree.Objects.Geometry.LayerElementMaterial if it exists
    parseMaterialIndices: function (MaterialNode) {
        var mappingType = MaterialNode.MappingInformationType;
        var referenceType = MaterialNode.ReferenceInformationType;

        if (mappingType === 'NoMappingInformation') {
            return {
                dataSize: 1,
                buffer: [0],
                indices: [0],
                mappingType: 'AllSame',
                referenceType: referenceType
            };
        }

        var materialIndexBuffer = MaterialNode.Materials.a;

        // Since materials are stored as indices, there's a bit of a mismatch between FBX and what
        // we expect.So we create an intermediate buffer that points to the index in the buffer,
        // for conforming with the other functions we've written for other data.
        var materialIndices = [];

        for (var i = 0; i < materialIndexBuffer.length; ++i) {
            materialIndices.push(i);
        }

        return {
            dataSize: 1,
            buffer: materialIndexBuffer,
            indices: materialIndices,
            mappingType: mappingType,
            referenceType: referenceType
        };
    },

    // Generate a NurbGeometry from a node in FBXTree.Objects.Geometry
    parseNurbsGeometry: function (geoNode) {
        console.error('GASEngine.FBXLoader: Nurbs will show up as empty geometry.');
        return GASEngine.MeshFactory.Instance.create();
    },

};
