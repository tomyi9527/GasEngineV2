GASEngine.GAS1KeyframeAnimationDecoder = function()
{

}

GASEngine.GAS1KeyframeAnimationDecoder.prototype = {

    constructor: GASEngine.GAS1KeyframeAnimationDecoder,

    parse: function(dataView)
    {
        var clip = GASEngine.KeyframeAnimationFactory.Instance.createKeyframeAnimation();

        clip.version = dataView.getUint32(12, true);
        clip.clipID = dataView.getUint32(20, true);
        clip.guid = GASEngine.generateUUID();

        var animationFileSize = dataView.getUint32(4, true);
        if(dataView.byteLength != animationFileSize)
        {
            console.error("GASEngine.GAS1KeyframeAnimationDecoder.parse: animation file is corrupted!");
            return null;
        }

        var animationNameIndex = dataView.getInt32(36, true);
        var fps = dataView.getFloat32(40, true);
        var startFrame = dataView.getFloat32(44, true);
        var endFrame = dataView.getFloat32(48, true);

        var stringTableOffset = dataView.getInt32(84, true);
        if(stringTableOffset > 0)
        {
            this._constructAnimationStringTable(dataView, stringTableOffset);
        }

        var animationName;
        if(this.animationStringTable)
        {
            animationName = this.animationStringTable[animationNameIndex];;
        }

        var sectionCountOffset = dataView.getUint32(16, true);
        var nodeCount = dataView.getUint32(sectionCountOffset, true);

        var offset = sectionCountOffset + 4;
        var clipActualStartFrame, clipActualEndFrame;
        for(var i = 0; i < nodeCount; ++i)
        {
            var sectionType = dataView.getUint32(offset, true);
            offset += 4;

            var sectionAttr = dataView.getUint32(offset, true);
            offset += 4;

            var sectionOffset = dataView.getUint32(offset, true);
            offset += 4;

            var sectionLength = dataView.getUint32(offset, true);
            offset += 4;

            var curveCount = dataView.getUint32(offset, true);
            offset += 4;

            var elementCount = dataView.getUint32(offset, true);
            offset += 4;

            var elementType = dataView.getUint32(offset, true);
            offset += 4;

            var dataOffset = sectionOffset + 16;

            var objectNameIndex = dataView.getUint32(sectionOffset, true);
            var objectName;
            if(this.animationStringTable)
            {
                objectName = this.animationStringTable[objectNameIndex];
            }

            var objectUniqueID = dataView.getUint32(dataOffset, true);
            dataOffset += 4;

            var animationChannels = {};

            animationChannels.kfs = [];
            animationChannels.node = null;
            animationChannels.nodeName = objectName;
            animationChannels.uniqueID = objectUniqueID;
            animationChannels.startFrame = 0;
            animationChannels.endFrame = 0;
            animationChannels.duration = 0;

            var minFrame, maxFrame;
            for(var j = 0; j < curveCount; ++j)
            {
                var targetType = dataView.getUint8(dataOffset, true);
                dataOffset += 1;

                var keyValueType = dataView.getUint8(dataOffset, true);
                dataOffset += 1;

                var keyIndexType = dataView.getUint8(dataOffset, true);
                dataOffset += 1;

                var keySize = dataView.getUint8(dataOffset, true);
                dataOffset += 1;

                var keyCount = dataView.getUint32(dataOffset, true);
                dataOffset += 4;

                var animationDataSize = dataView.getUint32(dataOffset, true);
                dataOffset += 4;

                var propertyNameIndex = dataView.getUint32(dataOffset, true);
                dataOffset += 4;

                var targetName = '';
                //12:ANIMATION_CUSTOMIZED_PROPERTY
                if((targetType === 9 || targetType === 12) && this.animationStringTable)
                {
                    targetName = this.animationStringTable[propertyNameIndex];
                }

                if(keyIndexType === 0) //old float data
                {
                    var frames = new Float32Array(dataView.buffer, dataOffset, keyCount);
                }
                else if(keyIndexType === 1) //mmd data
                {
                    var keyframeIndexDataSize = dataView.getUint32(dataOffset + 0, true);
                    var keyframeIndexCount = dataView.getUint32(dataOffset + 4, true);

                    var frames = new Int32Array(dataView.buffer, dataOffset + 16, keyCount);                                       
                }

                if(keyValueType === 1)//old float data
                {
                    var values = new Float32Array(dataView.buffer, dataOffset + animationDataSize / 2, keyCount);
                }
                else if(keyValueType === 2) //KEY_FLOAT_BEZIER_MMD
                {
                    var offset4 = dataOffset + 16 + keyframeIndexDataSize * keyframeIndexCount;
                    var keyframeDataSize = dataView.getUint32(offset4 + 0, true);        
                    var keyframeDataCount = dataView.getUint32(offset4 + 4, true);
                   
                    var values = [];
                    offset4 += 16;
                    for(var dataIndex = 0; dataIndex < keyframeDataCount; ++dataIndex)
                    {
                        var value = dataView.getFloat32(offset4, true);
                        offset4 += 4;

                        var x1 = dataView.getUint8(offset4, true);
                        offset4 += 1;

                        var y1 = dataView.getUint8(offset4, true);
                        offset4 += 1;

                        var x2 = dataView.getUint8(offset4, true);
                        offset4 += 1;

                        var y2 = dataView.getUint8(offset4, true);
                        offset4 += 1;

                        var frameData = 
                        {
                            "value": value,
                            "mmd_bezier_x1": x1,
                            "mmd_bezier_y1": y1,
                            "mmd_bezier_x2": x2,
                            "mmd_bezier_y2": y2,
                        };

                        values.push(frameData);
                    }
                }
                else if(keyValueType === 10) //KEY_QUATERNION_BEZIER_MMD
                {
                    var offset5 = dataOffset + 16 + keyframeIndexDataSize * keyframeIndexCount;

                    var keyframeDataSize = dataView.getUint32(offset5, true);
                    offset5 += 4;

                    var keyframeDataCount = dataView.getUint32(offset5, true);
                    offset5 += 4;

                    offset5 += 8; //ununsed

                    var values = [];

                    for(var dataIndex = 0; dataIndex < keyframeDataCount; ++dataIndex)
                    {
                        var qx = dataView.getFloat32(offset5, true);
                        offset5 += 4;

                        var qy = dataView.getFloat32(offset5, true);
                        offset5 += 4;

                        var qz = dataView.getFloat32(offset5, true);
                        offset5 += 4;

                        var qw = dataView.getFloat32(offset5, true);
                        offset5 += 4;

                        var x1 = dataView.getUint8(offset5, true);
                        offset5 += 1;

                        var y1 = dataView.getUint8(offset5, true);
                        offset5 += 1;

                        var x2 = dataView.getUint8(offset5, true);
                        offset5 += 1;

                        var y2 = dataView.getUint8(offset5, true);
                        offset5 += 1;

                        var frameData = 
                        {
                            "value": new GASEngine.Quaternion(qx, qy, qz, qw),
                            "mmd_bezier_x1": x1,
                            "mmd_bezier_y1": y1,
                            "mmd_bezier_x2": x2,
                            "mmd_bezier_y2": y2,
                        };

                        values.push(frameData);
                    }
                }

                frames._lastValue = 0;
                animationChannels.kfs.push
                (
                    {
                        "keyIndexType": keyIndexType,
                        "keyValueType":keyValueType,
                        "t": frames,
                        "v": values,
                        "target": targetType,
                        "targetName": targetName
                    }
                );

                //Find the frame range of a single object
                if(minFrame === undefined)
                {
                    minFrame = frames[0];
                }
                else
                {
                    if(frames[0] < minFrame)
                    {
                        minFrame = frames[0];
                    }
                }

                if(maxFrame === undefined)
                {
                    maxFrame = frames[frames.length - 1];
                }
                else
                {
                    if(frames[frames.length - 1] > maxFrame)
                    {
                        maxFrame = frames[frames.length - 1];
                    }
                }
                //<
                dataOffset += animationDataSize;
            }

            animationChannels.startFrame = minFrame;
            animationChannels.endFrame = maxFrame;
            animationChannels.duration = maxFrame - minFrame;

            clip.animatedNodes.push(animationChannels);

            //Find the frame range of the whole clip
            if(clipActualStartFrame === undefined)
            {
                clipActualStartFrame = minFrame;
            }
            else
            {
                if(minFrame < clipActualStartFrame)
                {
                    clipActualStartFrame = minFrame;
                }
            }

            if(clipActualEndFrame === undefined)
            {
                clipActualEndFrame = maxFrame;
            }
            else
            {
                if(maxFrame > clipActualEndFrame)
                {
                    clipActualEndFrame = maxFrame;
                }
            }
        }

        //
        clip.fps = fps;

        if(animationName !== undefined && animationName !== null)
        {
            var indexDot = animationName.lastIndexOf(".");
            if(indexDot !== -1)
            {
                clip.clipName = animationName.substr(0, indexDot);;
            }
            else
            {
                clip.clipName = animationName;
            }
        }

        clip.startFrame = startFrame >= 0 ? startFrame : 0;
        clip.clampedSF = clip.startFrame;

        clip.localTime = clip.startFrame / clip.fps;

        clip.endFrame = endFrame;
        clip.clampedEF = clip.endFrame;

        clip.actualStartFrame = clipActualStartFrame >= 0 ? clipActualStartFrame : 0;
        clip.actualEndFrame = clipActualEndFrame > clip.actualStartFrame ? clipActualEndFrame : clip.actualStartFrame;
        clip.clipDuration = clipActualEndFrame - clipActualStartFrame;

        return clip;
    },

    _constructAnimationStringTable: function(dataView, offset)
    {
        var entryCount = dataView.getInt32(offset, true);
        offset += 4;

        this.animationStringTable = [];
        for(var i = 0; i < entryCount; ++i)
        {
            var stringSize = dataView.getInt32(offset, true);
            offset += 4;
            var utf8bytes = new Uint8Array(dataView.buffer, offset, stringSize);
            offset += (stringSize + 1);
            var jsString = GASEngine.UTF8ArrayToJSString(utf8bytes);
            this.animationStringTable.push(jsString);
        }
    }
}