GASEngine.GLTFMeshDecoder = function ()
{
};

GASEngine.GLTFMeshDecoder.prototype = {

    constructor: GASEngine.GLTFMeshDecoder,
    parse: function (meshDef, accessorObjects)
    {
        var primitives = meshDef.primitives;
        var meshes = new Array(0);

        for (var i = 0, il = primitives.length; i < il; i++)
        {
            var primitive = primitives[i];

            var mesh = this._parseMesh(primitive, accessorObjects);
            if (!mesh)
            {
                console.error('GLTFMeshDecoder : parse() : failed to parse ' + meshDef.name + 'mesh primitive' + i);
                continue;
            }

            mesh.name = meshDef.name || ('meshname');
            meshes.push(mesh);

            //add Morph Targets
            if (primitive.targets !== undefined)
            {
                var meshTargets = this._parseMorphTargets(mesh, primitive.targets, accessorObjects);
                if (meshTargets.length > 0)
                {

                    for (var n = 0; n < meshTargets.length; ++n)
                    {
                        mesh.addMorphTarget(meshTargets[n]);
                    }
                }
            }
        }

        if (meshes.length === 1)
        {
            return meshes[0];
        }
        return meshes;
    },

    _parseMesh: function (primitive, accessorObjects)
    {
        // 1. create Mesh
        var mesh = GASEngine.MeshFactory.Instance.create();

        var attributes = primitive.attributes;

        for (var gltfAttributeName in attributes)
        {
            var attributeName = GASEngine.GLTFConsts.ATTRIBUTES[gltfAttributeName];
            var accessor = accessorObjects.get(attributes[gltfAttributeName]);

            // skip attributeName existed.
            if (!attributeName) continue;
            if (mesh.getStream(attributeName)) continue;

            //FBX is likely constructed with the assumption that (0, 0) is bottom left, whereas glTF has (0, 0) as top left. 
            //As GASEngine is like FBX, we must flip the texcoords v.
            if (attributeName === 'uv')
            { //flip v
                var outUVs = new Float32Array(accessor.length);

                for (var ii = 0; ii < accessor.length; ii += 2)
                {
                    outUVs[ii] = accessor[ii];
                    outUVs[ii + 1] = 1.0 - accessor[ii + 1];
                }
                mesh.addStream(attributeName, outUVs);
            }
            else if(attributeName === 'color') {
                var outColors = new Uint8Array(accessor.length);
                for (var ij = 0; ij < accessor.length; ij++) {
                    outColors[ij] = Math.floor(accessor[ij] * 255);
                }
                mesh.addStream('color', outColors);
            }
            else {
                mesh.addStream(attributeName, accessor);
            }
        }

        // assign index
        if (primitive.indices !== undefined)
        {
            var accessor = accessorObjects.get(primitive.indices);
            mesh.addStream('index', accessor)
            mesh.addStream('topology', mesh.createWireframeIndices(accessor));
            mesh.addStream('uvtopology', mesh.createUVTopologicalIndices(accessor));
            mesh.addStream('subMesh', [{ 'start': 0, 'count': accessor.length }]);
        }
        else
        {
            var positions = mesh.getStream('position');
            var iil = positions.length / 3;
            var outIndices = new Uint32Array(iil);
            for (var ii = 0; ii < iil; ii++)
            {
                outIndices[ii] = ii;
            }
            mesh.addStream('index', outIndices);
            mesh.addStream('topology', mesh.createWireframeIndices(outIndices));
            mesh.addStream('subMesh', [{ 'start': 0, 'count': outIndices.length }]);
        }

        var normalStream = mesh.getStream('normal');
        if (normalStream === null)
        {
            var indices = mesh.getStream('index');
            var positions = mesh.getStream('position');
            var subMeshes = mesh.getStream('subMesh');

            if (indices !== null && positions !== null && subMeshes !== null)
            {
                var outNormals = new Float32Array(positions.length);
                GASEngine.Utilities.computeNormals(indices, positions, subMeshes, outNormals);
                mesh.addStream('normal', outNormals);
            }
            else
            {
                console.error('GASEngine.GLTFMeshDecoder.parseMesh: unable to compute vertex normals for inadequate source stream!');
            }
        }

        var tangentStream = mesh.getStream('tangent');
        var UVs = mesh.getStream('uv');
        if (tangentStream === null && UVs !== null)
        {
            var indices = mesh.getStream('index');
            var positions = mesh.getStream('position');
            var normals = mesh.getStream('normal');
            var subMeshes = mesh.getStream('subMesh');

            if (indices !== null && positions !== null && normals !== null && UVs !== null && subMeshes !== null)
            {
                var outTangents = new Float32Array((positions.length / 3) * 4);
                GASEngine.Utilities.computeTangents(indices, positions, normals, UVs, subMeshes, outTangents);
                mesh.addStream('tangent', outTangents);
            }
            else
            {
                console.error('GASEngine.GAS1MeshDecoder.parseMesh: unable to compute vertex tangents for inadequate source stream!');
            }
        }

        if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLES ||
            primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLE_STRIP ||
            primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLE_FAN ||
            primitive.mode === undefined)
        {
            if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLE_STRIP)
            {
                mesh.setDrawMode('TRIANGLE_STRIP');
            } else if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.TRIANGLE_FAN)
            {
                mesh.setDrawMode('TRIANGLE_FAN');
            }
            else
            {
                mesh.setDrawMode('TRIANGLES');
            }

        }
        else if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.LINES)
        {
            mesh.setDrawMode('LINES');
        }
        else if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.LINE_STRIP)
        {
            mesh.setDrawMode('LINE');
        }
        else if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.LINE_LOOP)
        {
            mesh.setDrawMode('LINELOOP');
        }
        else if (primitive.mode === GASEngine.GLTFConsts.WEBGL_CONSTANTS.POINTS)
        {
            mesh.setDrawMode('POINTS');
        } else
        {
            console.error('GASEngine.GLTFMeshDecoder: Primitive mode unsupported: ' + primitive.mode);
        }
        return mesh;
    },

    _parseMorphTargets: function (mesh, targets, accessorObjects)
    {
        var meshTargets = [];
        var hasMorphPosition = false;
        var hasMorphNormal = false;

        for (var i = 0, il = targets.length; i < il; i++)
        {
            var target = targets[i];

            if (target.POSITION !== undefined) hasMorphPosition = true;
            if (target.NORMAL !== undefined) hasMorphNormal = true;

            if (hasMorphPosition && hasMorphNormal) break;
        }

        if (!hasMorphPosition && !hasMorphNormal)
            return meshTargets;

        var positions = mesh.getStream('position');
        var normals = mesh.getStream('normal');

        if(hasMorphPosition && positions === null) {
            console.warn('GASEngine.GAS1MeshDecoder._parseMorphTargets: original mesh is missing position stream!!');
            return meshTargets;
        }

        for (var i = 0, il = targets.length; i < il; i++)
        {
            var target = targets[i];

            var meshTarget = GASEngine.MeshFactory.Instance.create();
            meshTarget.name = 'morphTarget' + i;
            meshTarget.uniqueID = GASEngine.generateUUID();

            if (hasMorphPosition)
            {
                // GASEngine morph position is absolute value. The formula is
                //   basePosition
                //     + weight0 * ( morphPosition0 - basePosition )
                //     + weight1 * ( morphPosition1 - basePosition )
                //     ...
                // while the glTF one is relative
                //   basePosition
                //     + weight0 * glTFmorphPosition0
                //     + weight1 * glTFmorphPosition1
                //     ...
                // then we need to convert from relative to absolute here.
                if (target.POSITION !== undefined)
                {
                    var targetPositions = accessorObjects.get(target.POSITION);
                    if(targetPositions !== null && targetPositions.length === positions.length) {
                        var outPositions = new Float32Array(positions.length);

                        for (var j = 0, jl = positions.length; j < jl; j++)
                        {
                            outPositions[j] = positions[j] + targetPositions[j];
                        }
                        meshTarget.addStream('position', outPositions);
                    }
                    else {
                        console.error('GASEngine.GAS1MeshDecoder._parseMorphTargets: The length of target.POSITION accessor is not equal to position stream!');
                    }
                }
                else
                {
                    var outPositions = new Float32Array(positions.length);

                    for (var j = 0, jl = positions.length; j < jl; j++)
                    {
                        outPositions[j] = positions[j];
                    }
                    meshTarget.addStream('position', outPositions);
                }
            }

            if (hasMorphNormal && normals !== null)
            {
                // see target.POSITION's comment
                if (target.NORMAL !== undefined)
                {
                    var targetNormals = accessorObjects.get(target.NORMAL);
                    if(targetNormals !== null && targetNormals.length === normals.length) {
                        var outNormals = new Float32Array(normals.length);

                        for (var j = 0, jl = normals.length; j < jl; j++)
                        {
                            outNormals[j] = normals[j] + targetNormals[j];
                        }
                        meshTarget.addStream('normal', outNormals);
                    }
                    else {
                        console.error('GASEngine.GAS1MeshDecoder._parseMorphTargets: The length of target.NORMAL accessor is not equal to normal stream!');
                    }
                } else
                {
                    var outNormals = new Float32Array(normals.length);

                    for (var j = 0, jl = normals.count; j < jl; j++)
                    {
                        outNormals[j] = normals[j];
                    }
                    meshTarget.addStream('normal', outNormals);
                }
            }

            meshTargets.push(meshTarget);
        }
        return meshTargets;
    }
};