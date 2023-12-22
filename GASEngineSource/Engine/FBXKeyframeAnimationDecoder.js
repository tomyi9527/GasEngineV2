// parse animation data from FBXTree
GASEngine.FBXKeyframeAnimationDecoder = function ()
{
    this._fbxTree = null;
    this._connectionMap_ = null;
    this._nodeObjects_ = null;
}

GASEngine.FBXKeyframeAnimationDecoder.prototype = {

    constructor: GASEngine.FBXKeyframeAnimationDecoder,

    // take raw animation clips and turn them into GASEngine.js animation clips
    parse: function (fbxTree, connections, nodeObjects)
    {
        this._fbxTree = fbxTree;
        this._connectionMap_ = connections;
        this._nodeObjects_ = nodeObjects;

        var rawClips = this.parseClips();
        if (rawClips === undefined)
            return null;

        var animationClips = new Map();

        for (var key in rawClips)
        {
            var rawClip = rawClips[key];
            var animationClip = this.addClip(rawClip);

            animationClips.set(animationClip.clipID, animationClip);
        }
        return animationClips;
    },

    parseClips: function ()
    {
        // since the actual transformation data is stored in FBXTree.Objects.AnimationCurve,
        // if this is undefined we can safely assume there are no animations
        if (this._fbxTree.Objects.AnimationCurve === undefined) return undefined;

        var curveNodesMap = this.parseAnimationCurveNodes();

        this.parseAnimationCurves(curveNodesMap);

        var layersMap = this.parseAnimationLayers(curveNodesMap);
        var rawClips = this.parseAnimStacks(layersMap);

        return rawClips;
    },

    // parse nodes in FBXTree.Objects.AnimationCurveNode
    // each AnimationCurveNode holds data for an animation transform for a model (e.g. left arm rotation )
    // and is referenced by an AnimationLayer
    parseAnimationCurveNodes: function ()
    {
        var rawCurveNodes = this._fbxTree.Objects.AnimationCurveNode;
        var curveNodesMap = new Map();
        for (var nodeID in rawCurveNodes)
        {
            var rawCurveNode = rawCurveNodes[nodeID];
            if (rawCurveNode.attrName.match(/S|R|T|DeformPercent/) !== null)
            {
                var curveNode = {
                    id: rawCurveNode.id,
                    attr: rawCurveNode.attrName,
                    curves: {},
                };
                curveNodesMap.set(curveNode.id, curveNode);
            }
        }
        return curveNodesMap;
    },

    // parse nodes in FBXTree.Objects.AnimationCurve and connect them up to
    // previously parsed AnimationCurveNodes. Each AnimationCurve holds data for a single animated
    // axis ( e.g. times and values of x rotation)
    parseAnimationCurves: function (curveNodesMap)
    {
        var rawCurves = this._fbxTree.Objects.AnimationCurve;
        // TODO: Many values are identical up to roundoff error, but won't be optimised
        // e.g. position times: [0, 0.4, 0. 8]
        // position values: [7.23538335023477e-7, 93.67518615722656, -0.9982695579528809, 7.23538335023477e-7, 93.67518615722656, -0.9982695579528809, 7.235384487103147e-7, 93.67520904541016, -0.9982695579528809]
        // clearly, this should be optimised to
        // times: [0], positions [7.23538335023477e-7, 93.67518615722656, -0.9982695579528809]
        // this shows up in nearly every FBX file, and generally time array is length > 100

        for (var nodeID in rawCurves)
        {
            var animationCurve = {
                id: rawCurves[nodeID].id,
                times: rawCurves[nodeID].KeyTime.a.map(GASEngine.FBXKeyframeAnimationDecoder.convertFBXTimeToSeconds),
                values: rawCurves[nodeID].KeyValueFloat.a,
            };

            var relationships = this._connectionMap_.get(animationCurve.id);

            if (relationships !== undefined)
            {
                var animationCurveID = relationships.parents[0].ID;
                var animationCurveRelationship = relationships.parents[0].relationship;

                if (animationCurveRelationship.match(/X/))
                {
                    curveNodesMap.get(animationCurveID).curves['x'] = animationCurve;
                } else if (animationCurveRelationship.match(/Y/))
                {
                    curveNodesMap.get(animationCurveID).curves['y'] = animationCurve;
                } else if (animationCurveRelationship.match(/Z/))
                {
                    curveNodesMap.get(animationCurveID).curves['z'] = animationCurve;
                } else if (animationCurveRelationship.match(/d|DeformPercent/) && curveNodesMap.has(animationCurveID))
                {
                    curveNodesMap.get(animationCurveID).curves['morph'] = animationCurve;
                }
            }
        }
    },

    // parse nodes in FBXTree.Objects.AnimationLayer. Each layers holds references
    // to various AnimationCurveNodes and is referenced by an AnimationStack node
    // note: theoretically a stack can have multiple layers, however in practice there always seems to be one per stack
    parseAnimationLayers: function (curveNodesMap)
    {
        var rawLayers = this._fbxTree.Objects.AnimationLayer;
        var layersMap = new Map();
        for (var nodeID in rawLayers)
        {
            var layerCurveNodes = [];
            var connection = this._connectionMap_.get(parseInt(nodeID));
            if (connection !== undefined)
            {
                // all the animationCurveNodes used in the layer
                var children = connection.children;

                for (var i = 0, il = children.length; i < il; i++)
                {
                    var child = children[i];
                    if (curveNodesMap.has(child.ID))
                    {
                        var curveNode = curveNodesMap.get(child.ID);
                        // check that the curves are defined for at least one axis, otherwise ignore the curveNode
                        if (curveNode.curves.x !== undefined || curveNode.curves.y !== undefined || curveNode.curves.z !== undefined)
                        {
                            if (layerCurveNodes[i] === undefined)
                            {
                                var modelID;
                                this._connectionMap_.get(child.ID).parents.forEach(function (parent)
                                {
                                    if (parent.relationship !== undefined) modelID = parent.ID;
                                });
                                var rawModel = this._fbxTree.Objects.Model[modelID.toString()];
                                var node = {
                                    modelName: GASEngine.Utilities.sanitizeNodeName(rawModel.attrName),
                                    initialPosition: [0, 0, 0],
                                    initialRotation: [0, 0, 0],
                                    initialScale: [1, 1, 1],
                                    transform: this.getModelAnimTransform(rawModel),
                                };

                                // if the animated model is pre rotated, we'll have to apply the pre rotations to every
                                // animation value as well
                                if ('PreRotation' in rawModel) node.preRotations = rawModel.PreRotation.value;
                                if ('PostRotation' in rawModel) node.postRotations = rawModel.PostRotation.value;

                                layerCurveNodes[i] = node;
                            }

                            layerCurveNodes[i][curveNode.attr] = curveNode;
                        } else if (curveNode.curves.morph !== undefined)
                        {

                            if (layerCurveNodes[i] === undefined)
                            {
                                var deformerID;
                                this._connectionMap_.get(child.ID).parents.forEach(function (parent)
                                {
                                    if (parent.relationship !== undefined) deformerID = parent.ID;
                                });

                                var morpherID = this._connectionMap_.get(deformerID).parents[0].ID;
                                var geoID = this._connectionMap_.get(morpherID).parents[0].ID;

                                // assuming geometry is not used in more than one model
                                var modelID = this._connectionMap_.get(geoID).parents[0].ID;
                                var rawModel = this._fbxTree.Objects.Model[modelID];

                                var node = {
                                    modelName: GASEngine.Utilities.sanitizeNodeName(rawModel.attrName),
                                    morphName: this._fbxTree.Objects.Deformer[deformerID].attrName,
                                };
                                layerCurveNodes[i] = node;
                            }
                            layerCurveNodes[i][curveNode.attr] = curveNode;
                        }
                    }
                }
                layersMap.set(parseInt(nodeID), layerCurveNodes);
            }
        }
        return layersMap;
    },

    getModelAnimTransform: function (modelNode)
    {
        var transformData = {};

        if ('RotationOrder' in modelNode) transformData.eulerOrder = parseInt(modelNode.RotationOrder.value);
        if ('Lcl_Translation' in modelNode) transformData.translation = modelNode.Lcl_Translation.value;
        if ('RotationOffset' in modelNode) transformData.rotationOffset = modelNode.RotationOffset.value;
        if ('Lcl_Rotation' in modelNode) transformData.rotation = modelNode.Lcl_Rotation.value;
        if ('PreRotation' in modelNode) transformData.preRotation = modelNode.PreRotation.value;
        if ('PostRotation' in modelNode) transformData.postRotation = modelNode.PostRotation.value;
        if ('Lcl_Scaling' in modelNode) transformData.scale = modelNode.Lcl_Scaling.value;

        return GASEngine.Utilities.generateTransform(transformData);
    },

    // parse nodes in FBXTree.Objects.AnimationStack. These are the top level node in the animation
    // hierarchy. Each Stack node will be used to create a GASEngine.AnimationClip
    parseAnimStacks: function (layersMap)
    {
        var rawStacks = this._fbxTree.Objects.AnimationStack;
        // connect the stacks (clips) up to the layers
        var rawClips = {};
        for (var nodeID in rawStacks)
        {
            var children = this._connectionMap_.get(parseInt(nodeID)).children;
            if (children.length > 1)
            {
                // it seems like stacks will always be associated with a single layer. But just in case there are files
                // where there are multiple layers per stack, we'll display a warning
                console.warn('GASEngine.FBXLoader: Encountered an animation stack with multiple layers, this is currently not supported. Ignoring subsequent layers.');
            }
            var layer = layersMap.get(children[0].ID);
            rawClips[nodeID] = {
                name: rawStacks[nodeID].attrName,
                layer: layer,
            };
        }
        return rawClips;
    },

    addClip: function (rawClip)
    {
        var clip = GASEngine.KeyframeAnimationFactory.Instance.createKeyframeAnimation();

        clip.version = 1;
        clip.clipID = GASEngine.generateUUID();

        var fps = 30;
        var clipActualStartFrame, clipActualEndFrame;

        var initialPosition = new GASEngine.Vector3();
        var initialRotation = new GASEngine.Quaternion();
        var initialScale = new GASEngine.Vector3();

        for (var rawTracks of rawClip.layer)
        {
            var node = this._nodeObjects_.get(rawTracks.modelName);
            if(node === null)
                continue;

            //parse tracks
            var tracks = [];
            if (rawTracks.transform) {
                rawTracks.transform.decompose(initialPosition, initialRotation, initialScale);
            }

            if (rawTracks.T !== undefined && Object.keys(rawTracks.T.curves).length > 0)
            {
                var positionTrack = this.generateVectorTrack(rawTracks.T.curves, initialPosition, 'translation');
                if (positionTrack !== undefined) tracks.push(positionTrack);
            }
            if (rawTracks.R !== undefined && Object.keys(rawTracks.R.curves).length > 0)
            {
                var rotationTrack = this.generateRotationTrack(rawTracks.R.curves, initialRotation, rawTracks.preRotations, rawTracks.postRotations);
                if (rotationTrack !== undefined) tracks.push(rotationTrack);
            }

            if (rawTracks.S !== undefined && Object.keys(rawTracks.S.curves).length > 0)
            {
                var scaleTrack = this.generateVectorTrack(rawTracks.S.curves, initialScale, 'scale');
                if (scaleTrack !== undefined) tracks.push(scaleTrack);
            }

            if (rawTracks.DeformPercent !== undefined)
            {
                var morphTrack = this.generateMorphTrack(rawTracks);
                if (morphTrack !== undefined) tracks.push(morphTrack);
            }

            var animationChannels = {};
            animationChannels.kfs = [];
            animationChannels.node = null;

            var meshFilter = node.getComponent('meshFilter');
            animationChannels.nodeName = node.name;
            animationChannels.uniqueID = node.uniqueID;
            animationChannels.startFrame = 0;
            animationChannels.endFrame = 0;
            animationChannels.duration = 0;
           
            // compute minFrame, maxFrame
            var minFrame, maxFrame;
            var curveCount = 0;
            var target = 0;

            //Convert tracks to kfs
            for (var track of tracks)
            {
                switch (track.path)
                {
                    case 'weights':
                        target = GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT;
                        curveCount = 1;
                        if(meshFilter) {
                            animationChannels.nodeName = undefined;
                            animationChannels.uniqueID = meshFilter.uniqueID;
                        }
                        break;
                    case 'rotation':
                        target = GASEngine.KeyframeAnimation.TARGET_RQ;
                        curveCount = 4;
                        break;
                    case 'translation':
                        target = GASEngine.KeyframeAnimation.TARGET_TX;
                        curveCount = 3;
                        break;
                    case 'scale':
                        target = GASEngine.KeyframeAnimation.TARGET_SX;
                        curveCount = 3;
                        break;
                    default:
                        break;
                }
                if (curveCount === 0)
                    continue;

                var keyframeCount = track.times.length;
                var frames = new Float32Array(keyframeCount);
                for (var ii = 0; ii < keyframeCount; ii++)
                {
                    frames[ii] = Math.floor(track.times[ii] * fps);
                }
                frames._lastValue = 0;

                //Inteplation mode: Linerar
                if (target === GASEngine.KeyframeAnimation.TARGET_RQ)
                {
                    var values = [];
                    for (var ii = 0; ii < keyframeCount; ii++)
                    {
                        var qx = track.values[ii * curveCount];
                        var qy = track.values[ii * curveCount + 1];
                        var qz = track.values[ii * curveCount + 2];
                        var qw = track.values[ii * curveCount + 3];

                        values.push(new GASEngine.Quaternion(qx, qy, qz, qw));
                    }
                    animationChannels.kfs.push
                        (
                            {
                                "keyIndexType": 0, //Float32 Type
                                "keyValueType": 9, //GASEngine.Quaternation Type
                                "t": frames,
                                "v": values,
                                "target": target
                            }
                        );
                }
                else if (target === GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT)
                {
                    for (var j = 0; j < curveCount; j++)
                    {
                        var values = new Float32Array(keyframeCount);

                        for (var ii = 0; ii < keyframeCount; ii++)
                        {
                            values[ii] = track.values[ii * curveCount + j] * 100;
                        }

                        animationChannels.kfs.push
                            (
                                {
                                    "keyIndexType": 0, //Float32 Type
                                    "keyValueType": 1, //Float32 Type
                                    "t": frames,
                                    "v": values,
                                    "target": target
                                }
                            );
                    }
                }
                else
                {
                    for (var j = 0; j < curveCount; j++)
                    {
                        var values = new Float32Array(keyframeCount);

                        for (var ii = 0; ii < keyframeCount; ii++)
                        {
                            values[ii] = track.values[ii * curveCount + j];
                        }
                        animationChannels.kfs.push
                            (
                                {
                                    "keyIndexType": 0, //Float32 Type
                                    "keyValueType": 1, //Float32 Type
                                    "t": frames,
                                    "v": values,
                                    "target": target + j
                                }
                            );
                    }
                }
            }

            //Find the frame range of a single object
            if (minFrame === undefined)
            {
                minFrame = frames[0];
            }
            else
            {
                if (frames[0] < minFrame)
                {
                    minFrame = frames[0];
                }
            }

            if (maxFrame === undefined)
            {
                maxFrame = frames[frames.length - 1];
            }
            else
            {
                if (frames[frames.length - 1] > maxFrame)
                {
                    maxFrame = frames[frames.length - 1];
                }
            }

            animationChannels.startFrame = minFrame;
            animationChannels.endFrame = maxFrame;
            animationChannels.duration = maxFrame - minFrame;

            clip.animatedNodes.push(animationChannels);

            //Find the frame range of the whole clip
            if (clipActualStartFrame === undefined)
            {
                clipActualStartFrame = minFrame;
            }
            else
            {
                if (minFrame < clipActualStartFrame)
                {
                    clipActualStartFrame = minFrame;
                }
            }

            if (clipActualEndFrame === undefined)
            {
                clipActualEndFrame = maxFrame;
            }
            else
            {
                if (maxFrame > clipActualEndFrame)
                {
                    clipActualEndFrame = maxFrame;
                }
            }
        }

        var animationName = rawClip.name;
        clip.fps = fps;
        var indexDot = animationName.lastIndexOf(".");
        if (indexDot !== -1)
        {
            clip.clipName = animationName.substr(0, indexDot);;
        }
        else
        {
            clip.clipName = animationName;
        }

        clip.startFrame = 0;
        clip.clampedSF = clip.startFrame;
        clip.localTime = clip.startFrame / clip.fps;

        clip.endFrame = clipActualEndFrame;
        clip.clampedEF = clip.endFrame;

        clip.actualStartFrame = clipActualStartFrame >= 0 ? clipActualStartFrame : 0;
        clip.actualEndFrame = clipActualEndFrame > clip.actualStartFrame ? clipActualEndFrame : clip.actualStartFrame;
        clip.clipDuration = clipActualEndFrame - clipActualStartFrame;
        return clip;
    },

    generateVectorTrack: function (curves, initialValue, type)
    {
        var times = this.getTimesForAllAxes(curves);
        var values = this.getKeyframeTrackValues(times, curves, initialValue);
        return {
            path: type,
            times: times,
            values: values
        };
    },

    generateRotationTrack: function (curves, initialValue, preRotations, postRotations)
    {
        if (curves.x !== undefined)
        {
            this.interpolateRotations(curves.x);
            curves.x.values = curves.x.values.map(GASEngine.degToRad);
        }
        if (curves.y !== undefined)
        {
            this.interpolateRotations(curves.y);
            curves.y.values = curves.y.values.map(GASEngine.degToRad);
        }
        if (curves.z !== undefined)
        {
            this.interpolateRotations(curves.z);
            curves.z.values = curves.z.values.map(GASEngine.degToRad);
        }
        var times = this.getTimesForAllAxes(curves);
        var values = this.getKeyframeTrackValues(times, curves, initialValue);

        if (preRotations !== undefined)
        {
            preRotations = preRotations.map(GASEngine.degToRad);
            preRotations.push('ZYX');
            preRotations = new GASEngine.Euler().fromArray(preRotations);
            preRotations = new GASEngine.Quaternion().setFromEuler(preRotations);
        }

        if (postRotations !== undefined)
        {
            postRotations = postRotations.map(GASEngine.degToRad);
            postRotations.push('ZYX');
            postRotations = new GASEngine.Euler().fromArray(postRotations);
            postRotations = new GASEngine.Quaternion().setFromEuler(postRotations).inverse();
        }

        var quaternion = new GASEngine.Quaternion();
        var euler = new GASEngine.Euler();
        var quaternionValues = [];

        for (var i = 0; i < values.length; i += 3)
        {
            euler.set(values[i], values[i + 1], values[i + 2], 'ZYX');
            quaternion.setFromEuler(euler);
            if (preRotations !== undefined) quaternion.premultiply(preRotations);
            if (postRotations !== undefined) quaternion.multiply(postRotations);

            quaternion.toArray(quaternionValues, (i / 3) * 4);
        }
        return {
            path: 'rotation',
            times: times,
            values: quaternionValues
        };
    },

    generateMorphTrack: function (rawTracks)
    {
        var curves = rawTracks.DeformPercent.curves.morph;
        var values = curves.values.map(function (val)
        {
            return val / 100;
        });

        //var morphNum = sceneGraph.getObjectByName(rawTracks.modelName).morphTargetDictionary[rawTracks.morphName];
        return {
            path: 'weights',
            times: curves.times,
            values: values
        }
    },

    // For all animated objects, times are defined separately for each axis
    // Here we'll combine the times into one sorted array without duplicates
    getTimesForAllAxes: function (curves)
    {
        var times = [];
        // first join together the times for each axis, if defined
        if (curves.x !== undefined) times = times.concat(curves.x.times);
        if (curves.y !== undefined) times = times.concat(curves.y.times);
        if (curves.z !== undefined) times = times.concat(curves.z.times);

        // then sort them and remove duplicates
        times = times.sort(function (a, b)
        {
            return a - b;
        }).filter(function (elem, index, array)
        {
            return array.indexOf(elem) == index;
        });
        return times;
    },

    getKeyframeTrackValues: function (times, curves, initialValue)
    {
        var prevValue = initialValue;
        var values = [];

        var xIndex = - 1;
        var yIndex = - 1;
        var zIndex = - 1;

        times.forEach(function (time)
        {
            if (curves.x) xIndex = curves.x.times.indexOf(time);
            if (curves.y) yIndex = curves.y.times.indexOf(time);
            if (curves.z) zIndex = curves.z.times.indexOf(time);

            // if there is an x value defined for this frame, use that
            if (xIndex !== - 1)
            {
                var xValue = curves.x.values[xIndex];
                values.push(xValue);
                prevValue[0] = xValue;
            } else
            {
                // otherwise use the x value from the previous frame
                values.push(prevValue[0]);
            }

            if (yIndex !== - 1)
            {
                var yValue = curves.y.values[yIndex];
                values.push(yValue);
                prevValue[1] = yValue;

            } else
            {
                values.push(prevValue[1]);
            }

            if (zIndex !== - 1)
            {
                var zValue = curves.z.values[zIndex];
                values.push(zValue);
                prevValue[2] = zValue;

            } else
            {
                values.push(prevValue[2]);
            }
        });
        return values;
    },

    // Rotations are defined as Euler angles which can have values  of any size
    // These will be converted to quaternions which don't support values greater than
    // PI, so we'll interpolate large rotations
    interpolateRotations: function (curve)
    {
        for (var i = 1; i < curve.values.length; i++)
        {
            var initialValue = curve.values[i - 1];
            var valuesSpan = curve.values[i] - initialValue;

            var absoluteSpan = Math.abs(valuesSpan);

            if (absoluteSpan >= 180)
            {
                var numSubIntervals = absoluteSpan / 180;
                var step = valuesSpan / numSubIntervals;
                var nextValue = initialValue + step;

                var initialTime = curve.times[i - 1];
                var timeSpan = curve.times[i] - initialTime;
                var interval = timeSpan / numSubIntervals;
                var nextTime = initialTime + interval;

                var interpolatedTimes = [];
                var interpolatedValues = [];

                while (nextTime < curve.times[i])
                {
                    interpolatedTimes.push(nextTime);
                    nextTime += interval;

                    interpolatedValues.push(nextValue);
                    nextValue += step;
                }
                curve.times = inject(curve.times, i, interpolatedTimes);
                curve.values = inject(curve.values, i, interpolatedValues);
            }
        }
    }
};

// Converts FBX ticks into real time seconds.
GASEngine.FBXKeyframeAnimationDecoder.convertFBXTimeToSeconds = function (time)
{
    return time / 46186158000;
};
