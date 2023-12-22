
GASEngine.Utilities =
{
    hashString: function (str)
    {
        var hash = 0, i, chr;

        if (str.length === 0)
            return hash;

        for (i = 0; i < str.length; i++)
        {
            chr = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + chr;
            hash |= 0; // Convert to 32bit integer
        }
        return hash;
    },

    sanitizeNodeName: function ()
    {
        var RESERVED_CHARS_RE = '\\[\\]\\.:\\/';
        var reservedRe = new RegExp('[' + RESERVED_CHARS_RE + ']', 'g');
        return function sanitizeNodeName(name)
        {
            return name.replace(/\s/g, '_').replace(reservedRe, '');
        };
    }(),

    computeNormals: function(indices, positions, subMeshes, outNormals)
    {
        if(outNormals.length !== positions.length)
        {
            console.error('GASEngine.Utilities.computeVertexNormals: the number of normal must be equal with of position.');
            return;
        }

        var pA = new GASEngine.Vector3();
        var pB = new GASEngine.Vector3();
        var pC = new GASEngine.Vector3();

        var cb = new GASEngine.Vector3();
		var ab = new GASEngine.Vector3();

		for(var j = 0; j < subMeshes.length; ++j)
        {
		    var endIndex = subMeshes[j].start + subMeshes[j].count;

		    for(var i = subMeshes[j].start; i < endIndex; i += 3)
            {
                var vA = indices[i + 0] * 3;
                var vB = indices[i + 1] * 3;
                var vC = indices[i + 2] * 3;

                pA.fromArray(positions, vA);
                pB.fromArray(positions, vB);
                pC.fromArray(positions, vC);

                cb.subVectors(pC, pB);
                ab.subVectors(pA, pB);
                cb.cross(ab);

                outNormals[vA] += cb.x;
                outNormals[vA + 1] += cb.y;
                outNormals[vA + 2] += cb.z;

                outNormals[vB] += cb.x;
                outNormals[vB + 1] += cb.y;
                outNormals[vB + 2] += cb.z;

                outNormals[vC] += cb.x;
                outNormals[vC + 1] += cb.y;
                outNormals[vC + 2] += cb.z;
            }
        }        

        GASEngine.Utilities.normalizeNormals(outNormals);
    },

    normalizeNormals: function(normals)
    {
        for(var i = 0; i < normals.length; i += 3)
        {
            var x = normals[i];
            var y = normals[i + 1];
            var z = normals[i + 2];

            var n = 1.0 / Math.sqrt(x * x + y * y + z * z);

            normals[i] *= n;
            normals[i + 1] *= n;
            normals[i + 2] *= n;
        }
    },

    validateVector : function(vector)
    {
        var flag = true;
        var epsilon = 0.0001;
        var abs = Math.abs(vector.x);
        if (abs <= epsilon) 
        {
            vector.x = epsilon;
            flag = false;
        }
        var abs = Math.abs(vector.y);
        if (abs <= epsilon) 
        {
            vector.y = epsilon;
            flag = false;
        }
        var abs = Math.abs(vector.z);
        if (abs <= epsilon) 
        {
            vector.z = epsilon;
            flag = false;
        }
        return flag;
    },

    computeTangents: function(indices, positions, normals, uvs, subMeshes, outTangents)
    {
        // based on http://www.terathon.com/code/tangent.html
        // (per vertex tangents)
        if((positions.length / 3) !== (outTangents.length / 4))
        {
            console.error('GASEngine.Utilities.computeTangents: the number of tangent must be equal with of position.');
            return;
        }

        var vertexCount = positions.length / 3;
        var tan1 = [], tan2 = [];

        for(var k = 0; k < vertexCount; k++)
        {
            tan1[k] = new GASEngine.Vector3();
            tan2[k] = new GASEngine.Vector3();
        }

        var vA = new GASEngine.Vector3(),
			vB = new GASEngine.Vector3(),
			vC = new GASEngine.Vector3(),

			uvA = new GASEngine.Vector2(),
			uvB = new GASEngine.Vector2(),
			uvC = new GASEngine.Vector2(),

			sdir = new GASEngine.Vector3(),
			tdir = new GASEngine.Vector3();

        function handleTriangle(a, b, c)
        {
            vA.fromArray(positions, a * 3);
            vB.fromArray(positions, b * 3);
            vC.fromArray(positions, c * 3);

            uvA.fromArray(uvs, a * 2);
            uvB.fromArray(uvs, b * 2);
            uvC.fromArray(uvs, c * 2);

            var x1 = vB.x - vA.x;
            var x2 = vC.x - vA.x;

            var y1 = vB.y - vA.y;
            var y2 = vC.y - vA.y;

            var z1 = vB.z - vA.z;
            var z2 = vC.z - vA.z;

            var s1 = uvB.x - uvA.x;
            var s2 = uvC.x - uvA.x;

            var t1 = uvB.y - uvA.y;
            var t2 = uvC.y - uvA.y;

            var r = 1.0 / Math.max((s1 * t2 - s2 * t1), 1e-6);

            sdir.set(
				(t2 * x1 - t1 * x2) * r,
				(t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r
			);

            tdir.set(
				(s1 * x2 - s2 * x1) * r,
				(s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r
			);

            tan1[a].add(sdir);
            tan1[b].add(sdir);
            tan1[c].add(sdir);

            tan2[a].add(tdir);
            tan2[b].add(tdir);
            tan2[c].add(tdir);

        }

        for(var j = 0, jl = subMeshes.length; j < jl; ++j)
        {
            var group = subMeshes[j];
            var start = group.start;
            var count = group.count;

            for(var i = start, il = start + count; i < il; i += 3)
            {
                handleTriangle(
					indices[i + 0],
					indices[i + 1],
					indices[i + 2]
				);
            }
        }

        var tmp = new GASEngine.Vector3(), tmp2 = new GASEngine.Vector3();
        var n = new GASEngine.Vector3(), n2 = new GASEngine.Vector3();
        var w, t, test;

        function handleVertex(v)
        {
            n.fromArray(normals, v * 3);
            n2.copy(n);
            t = tan1[v];

            // Gram-Schmidt orthogonalize
            tmp.copy(t);
            tmp.sub(n.multiplyScalar(n.dot(t))).normalize();

            // Calculate handedness
            tmp2.crossVectors(n2, t);
            test = tmp2.dot(tan2[v]);
            w = (test < 0.0) ? -1.0 : 1.0;

            outTangents[v * 4] = tmp.x;
            outTangents[v * 4 + 1] = tmp.y;
            outTangents[v * 4 + 2] = tmp.z;
            outTangents[v * 4 + 3] = w;
        }

        for(var j = 0, jl = subMeshes.length; j < jl; ++j)
        {
            var group = subMeshes[j];
            var start = group.start;
            var count = group.count;

            for(var i = start, il = start + count; i < il; i += 3)
            {
                handleVertex(indices[i + 0]);
                handleVertex(indices[i + 1]);
                handleVertex(indices[i + 2]);
            }
        }
    },

    cpuSkinning: function(mesh) 
    {
        var bones = mesh.bones;
        var invMatrices = mesh.matricesWorld2Bone;

        //refresh bone matrices
        var boneMatrices = [];
        for(var j = 0; j < bones.length; ++j) 
        {
            boneMatrices.push(new GASEngine.Matrix4());
        }

        for(var j = 0; j < bones.length; ++j) 
        {
            var matrix = bones[j].matrixWorld;
            boneMatrices[j].multiplyMatrices(matrix, invMatrices[j]);
        }

        var positionData = mesh.getStream('position'); //Float32Array
        var positionItemSize =  mesh.getVertexElementCount(); 

        var skinWeightData = mesh.getStream('skinWeight'); //Float32Array

        var skinIndexData = mesh.getStream('skinIndex'); //Float32Array
        var skinIndexItemSize = mesh.getSkinIndexItemSize();

        if(!positionData || !skinWeightData || !skinIndexData)
        {
            console.warn('GASEngine.Utilities.cpuskinning: the mesh does not have necessary streams!');
            return;
        }

        var skinnedPosition = mesh.getStream('skinnedPosition');
        if(skinnedPosition === null)
        {
            skinnedPosition = new Float32Array(positionData.length);
            mesh.addStream('skinnedPosition', skinnedPosition); 
        }

        var tmp_0_0 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_1 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_2 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_3 = new GASEngine.Vector4(0, 0, 0, 1);
        var tmp_0_4 = new GASEngine.Vector4(0, 0, 0, 1);

        var tmp_1 = new GASEngine.Vector4(0, 0, 0, 0);
        var tmp_2 = new GASEngine.Vector4(0, 0, 0, 0);

        var mat4MulVec4 = function (mat4, vec4, factor) 
        {
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

        for(var i = 0, k = 0; i < positionData.length; i += positionItemSize, k += skinIndexItemSize) 
        {
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

            //tmp_0_0.applyMatrix4(boneMatrices[tmp_2.x]).multiplyScalar(tmp_1.x);
            //tmp_0_1.applyMatrix4(boneMatrices[tmp_2.y]).multiplyScalar(tmp_1.y);
            //tmp_0_2.applyMatrix4(boneMatrices[tmp_2.z]).multiplyScalar(tmp_1.z);
            //tmp_0_3.applyMatrix4(boneMatrices[tmp_2.w]).multiplyScalar(tmp_1.w);

            tmp_0_4.x = tmp_0_0.x + tmp_0_1.x + tmp_0_2.x + tmp_0_3.x;
            tmp_0_4.y = tmp_0_0.y + tmp_0_1.y + tmp_0_2.y + tmp_0_3.y;
            tmp_0_4.z = tmp_0_0.z + tmp_0_1.z + tmp_0_2.z + tmp_0_3.z;
            tmp_0_4.w = 1.0;

            skinnedPosition[i + 0] = tmp_0_4.x;
            skinnedPosition[i + 1] = tmp_0_4.y;
            skinnedPosition[i + 2] = tmp_0_4.z;
        }
    },

    cpuMorph: function(mesh, matrixWorld)
    {
        var activeMorphWeights = new Float32Array(4);
        var activeMorphTargets = [null, null, null, null];
        var activeMorphTarget = null;
        var morphTarget = new GASEngine.Vector3();
        var objectPosition = new GASEngine.Vector3();
        var position = new GASEngine.Vector3();
        var morphTargetPosition, positionData, skinnedPosition;
        var morphWeight = 0, positionItemSize = 3;

        mesh.sortMorphWeights(activeMorphWeights, activeMorphTargets);
    
        positionData = mesh.getStream('position');
        positionItemSize =  mesh.getVertexElementCount();

        skinnedPosition = mesh.getStream('skinnedPosition');
        if(skinnedPosition === null)
        {
            skinnedPosition = new Float32Array(positionData.length);
            mesh.addStream('skinnedPosition', skinnedPosition); 
        }

        for(var i = 0, l = positionData.length; i < l; i += positionItemSize)
        {
            position.x = positionData[i+0];
            position.y = positionData[i+1];
            position.z = positionData[i+2];

            objectPosition.x = positionData[i+0];
            objectPosition.y = positionData[i+1];
            objectPosition.z = positionData[i+2];

            for(var j = 0; j < activeMorphTargets.length; ++j)
            {
                activeMorphTarget = activeMorphTargets[j];
                morphWeight = activeMorphWeights[j];
                if(activeMorphTarget)
                {
                    morphTargetPosition = activeMorphTarget.getStream('position');

                    morphTarget.x = morphTargetPosition[i+0];
                    morphTarget.y = morphTargetPosition[i+1];
                    morphTarget.z = morphTargetPosition[i+2];

                    objectPosition.x += (morphTarget.x - position.x) * morphWeight;
                    objectPosition.y += (morphTarget.y - position.y) * morphWeight;
                    objectPosition.z += (morphTarget.z - position.z) * morphWeight;
                }
            }

            objectPosition.applyMatrix4(matrixWorld);   

            skinnedPosition[i+0] = objectPosition.x;
            skinnedPosition[i+1] = objectPosition.y;
            skinnedPosition[i+2] = objectPosition.z;
        }
    },
    
    calcMousePositionCameraSpace: function(pos, camera, mouseX, mouseY, canvasWidth, canvasHeight, delta)
    {
        var ymax = camera.near * Math.tan(GASEngine.degToRad(camera.fov * 0.5));
        var xmax = ymax * camera.aspect;

        var x = (mouseX / canvasWidth) * 2 - 1;
        var y = -(mouseY / canvasHeight) * 2 + 1;

        pos.set(xmax * x, ymax * y, -(camera.near + delta));
    },

    // used for editorMode
    getPointerPosition: function(currentTarget)
    {
        var rect = currentTarget.getBoundingClientRect();
        var offsetX = event.clientX - rect.left;
        var offsetY = event.clientY - rect.top;

        var position = new GASEngine.Vector2();
        var canvasWidth = currentTarget ? currentTarget.width : event.currentTarget.width;
        var canvasHeight = currentTarget ? currentTarget.height : event.currentTarget.height;
        var ratioX = (offsetX / canvasWidth) * 2 - 1;
        var ratioY = -(offsetY / canvasHeight) * 2 + 1;   
        position.set(ratioX, ratioY);
        return position;
    },

    getPointerPosition1: function(mousePos, currentTarget)
    {
        var rect = currentTarget.getBoundingClientRect();
        var offsetX = mousePos.x - rect.left;
        var offsetY = mousePos.y - rect.top;

        var position = new GASEngine.Vector2();
        var canvasWidth = currentTarget.width;
        var canvasHeight = currentTarget.height;
        var ratioX = (offsetX / canvasWidth) * 2 - 1;
        var ratioY = -(offsetY / canvasHeight) * 2 + 1;   
        position.set(ratioX, ratioY);
        return position;
    },
    
    gizmoTransformIntersection: function(ray, picker)
    {
        var pickInfo = null;

        if(picker.type === 'line')
        {
            pickInfo = this.linesIntersection(ray, picker);
        }
        else if(picker.type === 'plane')
        {
            pickInfo = this.planeIntersection(ray, picker);
        }
        else if(picker.type === 'circle')
        {
            pickInfo = this.circleIntersection(ray, picker);
        }
        return pickInfo;
    },

    gizmoPickerIntersection: function(ray, pickers)
    {
        var picker = null, pickInfo = null, intersects = [];
        for(var i = 0, l = pickers.length; i < l; i++ )
        {
            picker = pickers[i];
            if(picker.type === 'line')
            {
                pickInfo = this.linesIntersection(ray, picker);
            }
            else if(picker.type === 'plane')
            {
                pickInfo = this.planeIntersection(ray, picker);
            }
            else if(picker.type === 'circle')
            {
                pickInfo = this.circleIntersection(ray, picker);
            }
            if(pickInfo)
            {
                intersects.push(pickInfo);
            }
        }
        intersects.sort((a, b)=>{return a.distance -b.distance;});
        // console.log(intersects);
        return intersects.length === 0 ? null : intersects[0];
    },

    myRaySegmentIntersection: function(ray, pOrigin, pEnd, pointOnRay, pointOnSegment)
    {
        //ray 1
        //p 2
        var pDir = new GASEngine.Vector3();
        pDir.subVectors(pOrigin, pEnd);
        var pDirNeg = pDir.clone().negate();

        var rayDir = ray.direction.clone();
        var rayDirNeg = rayDir.clone().negate();

        var n = new GASEngine.Vector3();
        n.crossVectors(pDir, rayDir).normalize();
        var nNeg = new GASEngine.Vector3();
        nNeg.crossVectors(pDirNeg, rayDirNeg);

        var ab = new GASEngine.Vector3();
        ab.subVectors(ray.origin, pOrigin);
        var abNeg = ab.clone().negate();

        var length = Math.abs(n.dot(ab));

        var tmp1 = new GASEngine.Vector3();
        var tmp2 = new GASEngine.Vector3();
        tmp1.crossVectors(abNeg, pDirNeg);
        tmp2.crossVectors(abNeg, rayDirNeg);

        var t1 = tmp1.dot(nNeg);
        var t2 = tmp2.dot(nNeg);
        var nNegLength = nNeg.length();
        t1 /= nNegLength * nNegLength;
        t2 /= nNegLength * nNegLength;

        pointOnRay.addVectors(ray.origin, rayDir.multiplyScalar(t1));
        pointOnSegment.addVectors(pOrigin, pDir.multiplyScalar(t2));

        return length;
    },

    

    myRayPlaneIntersection: function(ray, planeNorml, planePosition, pointVec)
    {
        var denominator = planeNorml.dot(ray.direction);
        //ray与平面平行
        if(denominator === 0)
        {
            var ab = new GASEngine.Vector3();
            ab.subVectors(ray.origin, planePosition);
            var flag = ab.dot(planeNorml);
            //ray在平面上
            if(flag === 0)
            {
                pointVec = planePosition.clone();
                return 0;
            }
            return -1;
        }
        var tmp = planeNorml.dot(planePosition); 
        var tmp2 = planeNorml.dot(ray.origin);
        var t = (tmp - tmp2) / denominator;
        pointVec.addVectors(ray.origin, ray.direction.clone().multiplyScalar(t));
        var pp = new GASEngine.Vector3();
        pp.subVectors(ray.origin, pointVec);
        var distance = pp.dot(planeNorml) / planeNorml.length();
        distance = Math.abs(distance);
        return distance;
    },

    planeIntersection: function(ray, picker)
    {
        var planeObj = {};
        planeObj.normal = new GASEngine.Vector3();
        planeObj.normal.fromArray(picker.normal, 0);
        planeObj.position = new GASEngine.Vector3();
        planeObj.position.fromArray(picker.position, 0);
        planeObj.constant = 0;
        var pointVec = new GASEngine.Vector3();
        // ray.intersectPlane(planeObj, pointVec);
        var distance = this.myRayPlaneIntersection(ray,  planeObj.normal,  planeObj.position, pointVec);
        if(distance < 0)
        {
            return null;
        }

        if(!(pointVec.x <= picker.size && pointVec.x >= 0 
            && pointVec.y <= picker.size && pointVec.y >= 0
            && pointVec.z <=  picker.size && pointVec.z >= 0 ))
        {
            return null;
        }

        var distance2 = pointVec.distanceTo(ray.origin);
        // console.log('plane name: ' + picker.name + '; distance: '+ distance + '; distance2: ' + distance2);
        return {
            'type': picker.type,
            'picker': picker,
            'rayPoint': pointVec,
            'distance': distance
        };
    },

    myRayCircleIntersection: function(ray, normal, position, radius, pointVec)
    {
        var center = new GASEngine.Vector3(0, 0, 0);
        var rayDir = ray.direction.clone().normalize();
        var denominator = normal.dot(rayDir);
        if(denominator === 0)
        {
            var ab = new GASEngine.Vector3();
            ab.subVectors(ray.origin, center);
            var flag = ab.dot(normal);
            //ray在平面上
            if(flag === 0)
            {
                var dd = ab.length();
                dd *= dd;
                var bb = ab.dot(rayDir);
                bb *= bb;
                var flag = dd - bb - radius * radius;
                if(flag > 0)
                {
                    return -1;
                }
                else if(flag === 0)
                {
                    var t = center.dot(rayDir) - ray.origin.dot(rayDir);
                    pointVec.addVectors(ray.origin, rayDir.multiplyScalar(t));
                    return 0;
                }
                else 
                {
                    var t = center.dot(rayDir) - ray.origin.dot(rayDir);
                    pointVec.addVectors(ray.origin, rayDir.multiplyScalar(t));
                    var pCenter = new GASEngine.Vector3();
                    pCenter.subVectors(pointVec, center);
                    var len = pCenter.length();
                    var diff = Math.sqrt(radius * radius - len * len);
                    var t1 =  t - diff;
                    // var t2 = t + diff;
                    pointVec.addVectors(ray.origin, rayDir.multiplyScalar(t1));
                    return 0;
                }
            }
            return -1;
        }

        var tmp = normal.dot(position); 
        var tmp2 = normal.dot(ray.origin);
        var t = (tmp - tmp2) / denominator;
        pointVec.addVectors(ray.origin, rayDir.multiplyScalar(t));
        var pp = new GASEngine.Vector3();
        pp.subVectors(ray.origin, pointVec);
        var distance = pp.dot(normal) / normal.length();
        distance = Math.abs(distance);
        return distance;
    },

    circleIntersection: function(ray, picker)
    {
        var normal = new GASEngine.Vector3();
        normal.fromArray(picker.normal, 0);
        var center = new GASEngine.Vector3();
        center.fromArray(picker.position, 0);
        var radius = picker.size;
        var pointVec = new GASEngine.Vector3();
        var distance = this.myRayCircleIntersection(ray, normal, center, radius, pointVec);
        if(distance < 0)
        {
            return null;
        }

        var center = new GASEngine.Vector3(0, 0, 0);
        var dis = pointVec.distanceTo(center);
        var distance2 = Math.abs(dis - radius);
        return {
            'type': picker.type,
            'picker': picker,
            'rayPoint': pointVec,
            'distance': distance2
        };
    },

    /////////////////////////////////// plane 相关 ///////////////////////////////////
    getRayPlane: function(rayOrigin, rayDir)
    {
        var plane = {};
        plane.normal = new GASEngine.Vector3();
        plane.normal.copy(rayDir).normalize();

        var dot = rayOrigin.dot(plane.normal);
        plane.offset = dot;

        return plane;
    },

    getFaceToPlane: function(beginPos, endPos)
    {
        var plane = {};
        plane.normal = new GASEngine.Vector3();
        plane.normal.copy(endPos).sub(beginPos).normalize();

        var dot = beginPos.dot(plane.normal);
        plane.offset = dot;

        return plane;
    },

    getRayFacePointPlane: function()
    {
        var cross = new GASEngine.Vector3();

        return function(rayOrigin, rayDir, point)
        {
            var plane = {};
            plane.normal = new GASEngine.Vector3();
 
            cross.copy( point ).sub( rayOrigin ).cross( rayDir );
            plane.normal.copy( rayDir ).cross( cross ).normalize();
    
            var dot = rayOrigin.dot(plane.normal);
            plane.offset = dot;
            
            return plane;
        };
    }(),

    getRayIntersectPlanePoint: function()
    {
        var dir = new GASEngine.Vector3();
        

        return function(rayOrigin, rayDir, plane)
        {
            var point = new GASEngine.Vector3(0, 0, 0); 

            dir.copy(rayDir);
            dir.normalize();
    
            var originDot = rayOrigin.dot(plane.normal);
            if (originDot === plane.offset)
            {
                point.copy(originDot);
                return point;
            }

            var dot = dir.dot(plane.normal);
            if (originDot > plane.offset)
            {
                if (dot >= 0)
                {
                    return null;
                }
            }
            else
            {
                if (dot <= 0)
                {
                    return null;
                }
            }
  
            // if (Math.abs(dot) <= this._epsilon)
            // {
            //     return point;
            // }
    
            var dot2 = rayOrigin.dot(plane.normal);
            var lamda = (plane.offset - dot2) / dot;
    
            point.copy(rayOrigin);
            point.add(dir.multiplyScalar(lamda));
    
            return point;
        };
    }(),

    getRayIntersectSpherePoint: function()
    {
        var cross = new GASEngine.Vector3();
        var rayOriginToSphereOrigin = new GASEngine.Vector3();
        var normal = new GASEngine.Vector3();
        var mapToNormal = new GASEngine.Vector3();
        var mapToRay = new GASEngine.Vector3();

        return function(rayOrigin, rayDir, sphereOrigin, sphereRadius)
        {
            var point = new GASEngine.Vector3();

            rayOriginToSphereOrigin.copy(sphereOrigin).sub(rayOrigin);
            var dis = rayOriginToSphereOrigin.length();

            cross.copy(rayDir).cross(rayOriginToSphereOrigin);

            normal.copy(cross).cross(rayDir).normalize();

            var dot = rayOriginToSphereOrigin.dot(normal);
            var absDot = Math.abs(dot);
            if (absDot > sphereRadius)
            {
                point.copy(normal).multiplyScalar(-dot).add(sphereOrigin);
                return point;
            }

            mapToNormal.copy(normal).multiplyScalar(dot);
            mapToRay.copy(rayOriginToSphereOrigin).sub(mapToNormal);

            if (mapToRay.dot(rayDir) < 0)
            {
                return point;
            }

            var mapToRayLength = mapToRay.length();
            mapToRay.normalize();

            var delta = Math.sqrt(sphereRadius * sphereRadius - absDot * absDot);

            if (dis === sphereRadius)
            {
                point.copy(rayOrigin);
                return point;
            }
            else if (dis > sphereRadius)
            {
                var length = mapToRayLength - delta;
                point.copy(rayOrigin).add(mapToRay.multiplyScalar(length));
                return point;
            }
            else
            {
                var length = mapToRayLength + delta;
                point.copy(rayOrigin).add(mapToRay.multiplyScalar(length));
                return point;
            }
        };
    }(),

    getPointMapInPlane: function()
    {
        var scaleV = new GASEngine.Vector3();

        return function(point, plane)
        {
            // mapPoint = point + (plane.offset - point * plane.normal) * plane.normal;
            var mapPoint = new GASEngine.Vector3(); 
            mapPoint.copy(point);
            var dot = point.dot(plane.normal);
            
            scaleV.copy(plane.normal).multiplyScalar(plane.offset - dot);
            mapPoint.add(scaleV);
            return mapPoint;
        };
    }(),

    // // 闭包test
    // testtest : function () 
    // {
	// 	var cross = new GASEngine.Vector3(); 

	// 	return function ( v1, v2 ) {

	// 		cross.copy(v1).cross(v2);

	// 		return cross;
	// 	};
	// }(),

    /////////////////////////////////// 拾取相关 ///////////////////////////////////
    
    /////////////////////////////////// Math Utilities ///////////////////////////////////
    //  return value: (y<0) ? -x : x
    chgsign: function()
    {
        var idxes = ['x', 'y', 'z', 'w'];
        return function(x, y, out)
        {
            for (var i = 0;i < idxes.length;i ++)
            {
                var idx = idxes[i];
                if (x[idx] && y[idx])
                {
                    out[idx] = y[idx] < 0 ? -x[idx] : x[idx];
                }
            }
        };
    }(),

    scaleMulQuat: function()
    {
        var vecUniform = new GASEngine.Vector4(1, 1, 1, 1);
        var vecTmp = new GASEngine.Vector4();
        var vecSign = new GASEngine.Vector4();

        return function(scale, quat, outQuat)
        {
            vecTmp.set(scale.x, scale.y, scale.z, 1);
            this.chgsign(vecUniform, vecTmp, vecSign);
            vecTmp.set(vecSign.y * vecSign.z, vecSign.x * vecSign.z, vecSign.x * vecSign.y, 1);
            this.chgsign(quat, vecTmp, outQuat);
        };
    }(),

    // 
    inverseScale: function()
    {
        var epsilon = 0.0001;

        return function(scale, outInvScale)
        {
            outInvScale.x = Math.abs(scale.x) < epsilon ? 0 : 1 / scale.x; 
            outInvScale.y = Math.abs(scale.y) < epsilon ? 0 : 1 / scale.y; 
            outInvScale.z = Math.abs(scale.z) < epsilon ? 0 : 1 / scale.z; 
        };
    }(),

    // vec-trs.h: (p - t) * r.reverse() * s.reverse()
    invMulTRS_Position: function()
    {
        var invQuat = new GASEngine.Quaternion();
        var invScale = new GASEngine.Vector3();

        return function(t, r, s, p)
        {
            // (p - t)
            p.sub(t);

            // * r.reverse()
            invQuat.setFromEuler(r).inverse();
            p.applyQuaternion(invQuat);

            // * s.reverse()
            this.inverseScale(s, invScale);
            p.multiply(invScale);
        };
    }(),

    InverseTransformPosition: function()
    {
        var tmpEuler = new GASEngine.Euler();
        var tmpTVec = new GASEngine.Vector3();
        var tmpSVec = new GASEngine.Vector3();

        return function(entity, p)
        {
            if (entity.parent)
            {
                this.InverseTransformPosition(entity.parent, p);
            }
            // this.invMulTRS_Position(entity.translation, entity.rotation, entity.scale, p);

            tmpTVec.copy(entity.getLocalTranslation());
            tmpEuler.copy(entity.getLocalRotation());
            tmpSVec.copy(entity.getLocalScale());
            this.invMulTRS_Position(tmpTVec, tmpEuler, tmpSVec, p);
            
        };
    }(),

    // vec-quat.h: 
    invMulRS_Rotation: function()
    {
        var invQuat = new GASEngine.Quaternion();

        return function(r, s, q)
        {
            // * r.reverse()
            invQuat.setFromEuler(r).inverse();
            invQuat.multiply(q);

            // scaleMulQuat
            this.scaleMulQuat(s, invQuat, q);
        };
    }(),

    InverseTransformRotation: function()
    {
        var tmpEuler = new GASEngine.Euler();
        var tmpSVec = new GASEngine.Vector3();

        return function(entity, q)
        {
            if (entity.parent)
            {
                this.InverseTransformRotation(entity.parent, q);
            }

            // this.invMulRS_Rotation(entity.rotation, entity.scale, q);
            
            tmpEuler.copy(entity.getLocalRotation());
            tmpSVec.copy(entity.getLocalScale());
            this.invMulRS_Rotation(tmpEuler, tmpSVec, q);

        };
    }(),

    engineDecodeURIComponent: function(textString)
    {
        try {
            var text = decodeURIComponent(textString);
            return text;
        } catch(e) {
            console.log(e);
            return textString;
        }
    },

    //------------------------------------------------------------------------------
    decodeText: function (array)
    {
        // Avoid the String.fromCharCode.apply(null, array) shortcut, which
        // throws a "maximum call stack size exceeded" error for large arrays.

        var s = '';

        for (var i = 0, il = array.length; i < il; i++)
        {

            // Implicitly assumes little-endian.
            s += String.fromCharCode(array[i]);

        }

        // Merges multi-byte utf-8 characters.
        return decodeURIComponent(escape(s));
    },

    stringToArrayBuffer: function (text)
    {
        var array = new Uint8Array(new ArrayBuffer(text.length));
        for (var i = 0, il = text.length; i < il; i++)
        {
            var value = text.charCodeAt(i);
            // Replacing multi-byte character with space(0x20).
            array[i] = value > 0xFF ? 0x20 : value;
        }
        return array.buffer;
    },

    arrayBufferToString: function (buffer, from, to)
    {
        if ( from === undefined ) from = 0;
        if ( to === undefined ) to = buffer.byteLength;
        return GASEngine.Utilities.decodeText(new Uint8Array(buffer, from, to));
    },

    generateTransform: function ()
    {
        var tempMat = new GASEngine.Matrix4();
        var tempEuler = new GASEngine.Euler();
        var tempVec = new GASEngine.Vector3();
        var translation = new GASEngine.Vector3();
        var rotation = new GASEngine.Matrix4();
        var eulerOrders = [
            'ZYX', // -> XYZ extrinsic
            'YZX', // -> XZY extrinsic
            'XZY', // -> YZX extrinsic
            'ZXY', // -> YXZ extrinsic
            'YXZ', // -> ZXY extrinsic
            'XYZ', // -> ZYX extrinsic
        ];

        // generate transformation from FBX transform data
        // ref: https://help.autodesk.com/view/FBX/2017/ENU/?guid=__files_GUID_10CDD63C_79C1_4F2D_BB28_AD2BE65A02ED_htm
        // transformData = {
        //	 eulerOrder: int,
        //	 translation: [],
        //   rotationOffset: [],
        //	 preRotation
        //	 rotation
        //	 postRotation
        //   scale
        // }
        // all entries are optional
        return function (transformData)
        {
            var transform = new GASEngine.Matrix4();
            translation.set(0, 0, 0);
            rotation.identity();

            var order = (transformData.eulerOrder) ? eulerOrders[transformData.eulerOrder] : eulerOrders[0];

            if (transformData.translation) translation.fromArray(transformData.translation);
            if (transformData.rotationOffset) translation.add(tempVec.fromArray(transformData.rotationOffset));

            if (transformData.rotation)
            {
                var array = transformData.rotation.map(GASEngine.degToRad);
                array.push(order);
                rotation.makeRotationFromEuler(tempEuler.fromArray(array));

            }

            if (transformData.preRotation)
            {
                var array = transformData.preRotation.map(GASEngine.degToRad);
                array.push(order);
                tempMat.makeRotationFromEuler(tempEuler.fromArray(array));
                rotation.premultiply(tempMat);
            }

            if (transformData.postRotation)
            {
                var array = transformData.postRotation.map(GASEngine.degToRad);
                array.push(order);
                tempMat.makeRotationFromEuler(tempEuler.fromArray(array));
                tempMat.getInverse(tempMat);
                rotation.multiply(tempMat);
            }

            if (transformData.scale) transform.scale(tempVec.fromArray(transformData.scale));
            transform.setPosition(translation);
            transform.multiply(rotation);
            return transform;
        };
    }(),

    // Parses comma separated list of numbers and returns them an array.
	// Used internally by the TextParser
	parseNumberArray: function( value ) {
		var array = value.split( ',' ).map( function ( val ) {
			return parseFloat( val );
		} );
		return array;
    },
    
    append: function (a, b ) {
		for ( var i = 0, j = a.length, l = b.length; i < l; i ++, j ++ ) {
			a[ j ] = b[ i ];
		}
	},

	slice: function( a, b, from, to ) {
		for ( var i = from, j = 0; i < to; i ++, j ++ ) {
			a[ j ] = b[ i ];
		}
		return a;
    },
    
	// inject array a2 into array a1 at index
    inject: function (a1, index, a2 ) {
		return a1.slice( 0, index ).concat( a2 ).concat( a1.slice( index));
    },
    
    getMinMax: function (array, itemSize, start, count) {
        var output = {
            min: new Array(itemSize).fill(Number.POSITIVE_INFINITY),
            max: new Array(itemSize).fill(Number.NEGATIVE_INFINITY)
        };

        for (var i = start; i < start + count; i++)
        {
            for (var a = 0; a < itemSize; a++)
            {
                var value = array[i * itemSize + a];
                output.min[a] = Math.min(output.min[a], value);
                output.max[a] = Math.max(output.max[a], value);
            }
        }
        return output;
    },

    getPaddedBufferSize: function (bufferSize)
    {
        return Math.ceil(bufferSize / 4) * 4;
    },

    getPaddedArrayBuffer: function (arrayBuffer, paddingByte)
    {
        paddingByte = paddingByte || 0;

        var paddedLength = GASEngine.Utilities.getPaddedBufferSize(arrayBuffer.byteLength);

        if (paddedLength !== arrayBuffer.byteLength)
        {
            var array = new Uint8Array(paddedLength);
            array.set(new Uint8Array(arrayBuffer));

            if (paddingByte !== 0)
            {
                for (var i = arrayBuffer.byteLength; i < paddedLength; i++)
                {
                    array[i] = paddingByte;
                }
            }
            return array.buffer;
        }
        return arrayBuffer;
    },

    equalArray: function (array1, array2)
    {
        return (array1.length === array2.length) && array1.every(function (element, index)
        {
            return element === array2[index];
        });

    }
};