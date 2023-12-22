//Author: tomyi
//Date: 2017-06-21

GASEngine.GAS2KeyframeAnimationDecoder = function()
{
    GASEngine.GAS1KeyframeAnimationDecoder.call(this);
}

GASEngine.GAS2KeyframeAnimationDecoder.prototype = Object.create(GASEngine.GAS1KeyframeAnimationDecoder.prototype);
GASEngine.GAS2KeyframeAnimationDecoder.prototype.constructor = GASEngine.GAS2KeyframeAnimationDecoder;

// GASEngine.GAS2KeyframeAnimationDecoder = function()
// {

// }

// GASEngine.GAS2KeyframeAnimationDecoder.prototype = {

//     constructor: GASEngine.GAS2KeyframeAnimationDecoder,

//     parse: function(dataView)
//     {
//         var clip = GASEngine.KeyframeAnimationFactory.Instance.createKeyframeAnimation();

//         clip.version = dataView.getUint32(12, true);
//         clip.clipID = dataView.getUint32(20, true);

//         var animationFileSize = dataView.getUint32(4, true);
//         if(dataView.byteLength != animationFileSize)
//         {
//             console.error("GASEngine.GAS2KeyframeAnimationDecoder.parse: animation file is corrupted!");
//             return null;
//         }

//         var fps = dataView.getFloat32(40, true);
//         var startFrame = dataView.getFloat32(44, true);
//         var endFrame = dataView.getFloat32(48, true);

//         var sectionCountOffset = dataView.getUint32(16, true);
//         var nodeCount = dataView.getUint32(sectionCountOffset, true);

//         var offset = sectionCountOffset + 4;
//         var clipActualStartFrame, clipActualEndFrame;
//         for(var i = 0; i < nodeCount; ++i)
//         {
//             var sectionType = dataView.getUint32(offset, true);
//             offset += 4;

//             var sectionAttr = dataView.getUint32(offset, true);
//             offset += 4;

//             var sectionOffset = dataView.getUint32(offset, true);
//             offset += 4;

//             var sectionLength = dataView.getUint32(offset, true);
//             offset += 4;

//             var curveCount = dataView.getUint32(offset, true);
//             offset += 4;

//             var elementCount = dataView.getUint32(offset, true);
//             offset += 4;

//             var elementType = dataView.getUint32(offset, true);
//             offset += 4;

//             // var utf8bytes = new Uint8Array(dataView.buffer, sectionOffset, 16);
//             // var objectName = GASEngine.UTF8ArrayToJSString(utf8bytes);

//             var dataOffset = sectionOffset + 16;

//             var objectUniqueID = dataView.getUint32(dataOffset, true);
//             dataOffset += 4;

//             var animationChannels = {};

//             animationChannels.kfs = [];
//             animationChannels.node = null;
//             animationChannels.uniqueID = objectUniqueID;
//             animationChannels.startFrame = 0;
//             animationChannels.endFrame = 0;
//             animationChannels.duration = 0;

//             var minFrame, maxFrame;
//             for(var j = 0; j < curveCount; ++j)
//             {
//                 var targetType = dataView.getUint8(dataOffset, true);
//                 dataOffset += 1;

//                 var keyframeDataType = dataView.getUint8(dataOffset, true);
//                 dataOffset += 1;

//                 var alignmentPlaceholder = dataView.getUint16(dataOffset, true);
//                 dataOffset += 2;

//                 var keyframeCount = dataView.getUint32(dataOffset, true);
//                 dataOffset += 4;

//                 var keyframeDataSize = dataView.getUint32(dataOffset, true);
//                 dataOffset += 4;

//                 var keyframeDataFlag = dataView.getUint32(dataOffset, true);
//                 dataOffset += 4;

//                 var frames = new Float32Array(dataView.buffer, dataOffset, keyframeCount);
//                 var values = new Float32Array(dataView.buffer, dataOffset + keyframeDataSize / 2, keyframeCount);

//                 if(frames.length !== values.length)
//                 {
//                     console.error("GASEngine.GAS2KeyframeAnimationDecoder.parse: animation file is corrupted!");
//                     return null;
//                 }

//                 frames._lastValue = 0;
//                 animationChannels.kfs.push
//                 (
//                     { 
//                         "keyIndexType": 0, //FLOAT TYPE
//                         "keyValueType": 1, //FLOAT TYPE
//                         "t": frames, 
//                         "v": values, 
//                         "target": targetType
//                     }
//                 );
//                 //Find the frame range of a single object
//                 if(minFrame === undefined)
//                 {
//                     minFrame = frames[0];
//                 }
//                 else
//                 {
//                     if(frames[0] < minFrame)
//                     {
//                         minFrame = frames[0];
//                     }
//                 }

//                 if(maxFrame === undefined)
//                 {
//                     maxFrame = frames[frames.length - 1];
//                 }
//                 else
//                 {
//                     if(frames[frames.length - 1] > maxFrame)
//                     {
//                         maxFrame = frames[frames.length - 1];
//                     }
//                 }
//                 //<
//                 dataOffset += keyframeDataSize;
//             }

//             animationChannels.startFrame = minFrame;
//             animationChannels.endFrame = maxFrame;
//             animationChannels.duration = maxFrame - minFrame;

//             clip.animatedNodes.push(animationChannels);

//             //Find the frame range of the whole clip
//             if(clipActualStartFrame === undefined)
//             {
//                 clipActualStartFrame = minFrame;
//             }
//             else
//             {
//                 if(minFrame < clipActualStartFrame)
//                 {
//                     clipActualStartFrame = minFrame;
//                 }
//             }

//             if(clipActualEndFrame === undefined)
//             {
//                 clipActualEndFrame = maxFrame;
//             }
//             else
//             {
//                 if(maxFrame > clipActualEndFrame)
//                 {
//                     clipActualEndFrame = maxFrame;
//                 }
//             }
//         }

//         //
//         clip.fps = fps;
//         clip.localTime = startFrame;
//         clip.startFrame = startFrame;
//         clip.endFrame = endFrame;
//         clip.actualStartFrame = clipActualStartFrame;
//         clip.actualEndFrame = clipActualEndFrame;
//         clip.clipDuration = clipActualEndFrame - clipActualStartFrame;

//         return clip;
//     }
// }