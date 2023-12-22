GASEngine.GLTFKeyframeAnimationDecoder = function ()
{
}

GASEngine.GLTFKeyframeAnimationDecoder.prototype = {

    constructor: GASEngine.GLTFKeyframeAnimationDecoder,

    parse: function (animationDef, accessorObjects, nodeObjects)
    {
        var clip = GASEngine.KeyframeAnimationFactory.Instance.createKeyframeAnimation();

        clip.version = 1;
        clip.clipID = GASEngine.generateUUID();

        var fps;
        var clipActualStartFrame, clipActualEndFrame;

        for (var i = 0, il = animationDef.channels.length; i < il; i++)
        {
            var channel = animationDef.channels[i];
            var sampler = animationDef.samplers[channel.sampler];
            if (sampler === 0)
                continue;

            var targetDef = channel.target;
            var node = nodeObjects.get(targetDef.node);
            if (node === null)
                continue;

            var input = animationDef.parameters !== undefined ? animationDef.parameters[sampler.input] : sampler.input;
            var output = animationDef.parameters !== undefined ? animationDef.parameters[sampler.output] : sampler.output;

            var inputAccessor = accessorObjects.get(input);
            var outputAccessor = accessorObjects.get(output);

            if (inputAccessor === null || outputAccessor === null)
                continue;

            if(fps === undefined && inputAccessor.length > 2) {
                fps = Math.floor((inputAccessor.length - 1)  / inputAccessor[inputAccessor.length - 1]);
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

            switch (targetDef.path)
            {
                case 'weights':
                    target = GASEngine.KeyframeAnimation.TARGET_MORPH_WEIGHT;
                    curveCount = 0;
                    if(meshFilter) {
                        animationChannels.nodeName = undefined;
                        animationChannels.uniqueID = meshFilter.uniqueID;
                        var mesh = meshFilter.getMesh();
                        if(mesh) {
                            curveCount = mesh.getMorphTargetCount();
                        }
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

            var keyframeCount = inputAccessor.length;
            var frames = new Float32Array(keyframeCount);
            for (var ii = 0; ii < keyframeCount; ii++)
            {
                frames[ii] = ii;
            }
            frames._lastValue = 0;

            //Inteplation mode: Linerar
            if (target === GASEngine.KeyframeAnimation.TARGET_RQ)
            {
                var values = [];
                for (var ii = 0; ii < keyframeCount; ii++)
                {
                    var qx = outputAccessor[ii * curveCount];
                    var qy = outputAccessor[ii * curveCount + 1];
                    var qz = outputAccessor[ii * curveCount + 2];
                    var qw = outputAccessor[ii * curveCount + 3];

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
                        values[ii] = outputAccessor[ii * curveCount + j] * 100;
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
                        values[ii] = outputAccessor[ii * curveCount + j];
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

        var animationName = animationDef.name;

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
    }
}