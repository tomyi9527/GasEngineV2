// //Author: tomyi
// //Date: 2017-06-19

GASEngine.GAS2MeshDecoder = function()
{
    GASEngine.GAS1MeshDecoder.call(this);
};

GASEngine.GAS2MeshDecoder.prototype = Object.create(GASEngine.GAS1MeshDecoder.prototype);
GASEngine.GAS2MeshDecoder.prototype.constructor = GASEngine.GAS2MeshDecoder;

// GASEngine.GAS2MeshDecoder = function()
// {
//     this._loadFunctions = new Map();

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_POSITION, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount, bboxMin, bboxMax)
//     {
//         var positions;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             positions = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             positions = this._decodePosition(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount, bboxMin, bboxMax);            
//         }

//         return positions;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_NORMAL0, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var normals;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             normals = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);            
//         }
//         else
//         {
//             normals = this._decodeNormals(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return normals;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_TANGENT0, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var tangents;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             tangents = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             tangents = this._decodeTangents(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return tangents;
//     }.bind(this));


//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_UV0, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount, bboxMin, bboxMax)
//     {
//         var UVs;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             UVs = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             UVs = this._decodeUV(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount, bboxMin, bboxMax);
//         }

//         return UVs;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_VERTEXCOLOR0, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var vertexColors; //MUST BE NORMALIZED!

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             vertexColors = new Uint8Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             vertexColors = this._decodeVertexColors(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return vertexColors;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_BLENDWEIGHT, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var blendWeights;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             blendWeights = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             blendWeights = this._decodeBlendWeights(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return blendWeights;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_BLENDINDEX, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var blendIndices;

//         if(dataAttribute === GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             blendIndices = new Float32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             blendIndices = this._decodeBlendIndices(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }
        
//         return blendIndices;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_INDEX, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var indices;

//         if(dataAttribute == GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             indices = new Uint32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             indices = this._decodeIndices(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return indices;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_SUBMESH, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var subMeshSections = [];
//         var subMeshCount = attributeCount;
//         var subMeshOffset = sectionOffset + 16;

//         for(var i = 0; i < subMeshCount; ++i)
//         {
//             var indexOffset = dataView.getUint32(subMeshOffset, true);
//             subMeshOffset += 4;

//             var triangleCount = dataView.getUint32(subMeshOffset, true);
//             subMeshOffset += 4;

//             subMeshSections.push({ 'start': indexOffset, 'count': triangleCount*3 });
//         }

//         return subMeshSections;
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_BONE, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         boneList = [];

//         var offset = sectionOffset + 16;

//         for(var j = 0; j < attributeCount; ++j)
//         {
//             var bone = {};

//             bone.uniqueID = dataView.getInt32(offset, true);
//             offset += 4;

//             bone.matrixWorld2Bone = [];

//             for(var i = 0; i < 16; ++i)
//             {
//                 var m = dataView.getFloat32(offset, true);
//                 bone.matrixWorld2Bone.push(m);
//                 offset += 4;
//             }

//             boneList.push(bone);
//         }

//         if(attributeCount > 0)
//         {
//             return boneList;
//         }
//         else
//         {
//             return null;
//         }
//     }.bind(this));

//     this._loadFunctions.set(GASEngine.GAS2MeshDecoder.MESH_DATA_TOPOLOGY, function(
//         dataView, sectionOffset, sectionLength, dataAttribute, elementCount, attributeCount)
//     {
//         var topologies;

//         if(dataAttribute == GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE)
//         {
//             topologies = new Uint32Array(dataView.buffer, sectionOffset + 16, elementCount * attributeCount);
//         }
//         else
//         {
//             topologies = this._decodeIndices(dataView, sectionOffset + 16, sectionLength, dataAttribute, elementCount, attributeCount);
//         }

//         return topologies;
//     }.bind(this));
// };

// GASEngine.GAS2MeshDecoder.MESH_DATA_POSITION = 0;
// GASEngine.GAS2MeshDecoder.MESH_DATA_NORMAL0 = 1;
// GASEngine.GAS2MeshDecoder.MESH_DATA_NORMAL1 = 2;
// GASEngine.GAS2MeshDecoder.MESH_DATA_TANGENT0 = 3;
// GASEngine.GAS2MeshDecoder.MESH_DATA_TANGENT1 = 4;
// GASEngine.GAS2MeshDecoder.MESH_DATA_BINORMAL0 = 5;
// GASEngine.GAS2MeshDecoder.MESH_DATA_BINORMAL1 = 6;
// GASEngine.GAS2MeshDecoder.MESH_DATA_UV0 = 7;
// GASEngine.GAS2MeshDecoder.MESH_DATA_UV1 = 8;
// GASEngine.GAS2MeshDecoder.MESH_DATA_VERTEXCOLOR0 = 9;
// GASEngine.GAS2MeshDecoder.MESH_DATA_VERTEXCOLOR1 = 10;
// GASEngine.GAS2MeshDecoder.MESH_DATA_BLENDWEIGHT = 11;
// GASEngine.GAS2MeshDecoder.MESH_DATA_BLENDINDEX = 12;
// GASEngine.GAS2MeshDecoder.MESH_DATA_INDEX = 13;
// GASEngine.GAS2MeshDecoder.MESH_DATA_SUBMESH = 14;
// GASEngine.GAS2MeshDecoder.MESH_DATA_BONE = 15;
// GASEngine.GAS2MeshDecoder.MESH_DATA_MORPHTARGET = 16;
// GASEngine.GAS2MeshDecoder.MESH_DATA_KEYFRAME = 17;
// GASEngine.GAS2MeshDecoder.MESH_DATA_TOPOLOGY = 18;
// GASEngine.GAS2MeshDecoder.MESH_DATA_TOTAL_COUNT = 19;
// GASEngine.GAS2MeshDecoder.MESH_DATA_UNKNOWN = 255;

// GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_LOOSE = 0;
// GASEngine.GAS2MeshDecoder.MESH_ATTRIBUTE_COMPRESSED = 1;

// GASEngine.GAS2MeshDecoder.SHADER_STREAM_ATTRIBUTE_NAMES = [
//         'position',
//         'normal',
//         'normal1',
//         'tangent',
//         'tangent1',
//         'binormal',
//         'binormal1',
//         'uv',
//         'uv1',
//         'color',
//         'color1',
//         'skinWeight',
//         'skinIndex',
//         'index',
//         'subMesh',
//         'bone',
//         'morphTarget',
//         'keyframe',
//         'topology'
// ];

// GASEngine.GAS2MeshDecoder.NORMAL_DECODING_DICTIONARY = [
//         1, 8, 15, 22, 28, 35, 41, 47, 54, 60, 67, 73, 79, 85, 91, 97, 103, 109, 115, 121, 126, 132, 137, 143, 148,
//         153, 158, 163, 168, 173, 178, 182, 186, 191, 195, 199, 203, 206, 210, 213, 216, 219, 222, 225, 228, 230, 233,
//         235, 237, 239, 240, 242, 243, 244, 245, 246, 247, 247, 247, 248, 248, 247, 247, 246, 246, 245, 244, 242, 241,
//         239, 238, 236, 234, 232, 229, 227, 224, 221, 218, 215, 211, 208, 204, 201, 197, 193, 189, 184, 180, 175, 171,
//         166, 161, 156, 151, 145, 140, 135, 129, 124, 118, 112, 106, 100, 94, 88, 82, 76, 70, 63, 57, 51, 44, 38, 31,
//         25, 18, 12, 5, 1];

// GASEngine.GAS2MeshDecoder.NORMAL_ENCODING_PHI_COUNT = 120;

// GASEngine.GAS2MeshDecoder.prototype = {

//     constructor: GASEngine.GAS2MeshDecoder,

//     _decodePosition: function(dataView, offset, sectionLength, encodingFlag, elementCount, vertexCount, bboxMin, bboxMax)
//     {
//         var encodings = [];
//         for(var i = 0; i < elementCount; ++i)
//         {
//             var e = (encodingFlag >> (3 * i)) & 0x7;
//             encodings.push(e);
//         }

//         var precision = [];
//         var lengthes = [];
//         var biases = [];

//         for(var i = 0; i < elementCount; ++i)
//         {
//             var p = 0;
//             if(encodings[i] !== 4)
//             {
//                 p = (bboxMax[i] - bboxMin[i]) / (Math.pow(2.0, encodings[i] * 8.0) - 1.0);
//             }
//             precision.push(p);

//             var len = dataView.getUint32(offset + i * 4, true);

//             var bia = 0;
//             if(i === 0)
//                 bia = offset + elementCount * 4;
//             else
//                 bia = biases[biases.length - 1] + lengthes[lengthes.length - 1];

//             lengthes.push(len);
//             biases.push(bia);
//         }

//         var outputArray = new Float32Array(elementCount * vertexCount);

//         for(var i = 0; i < elementCount; ++i)
//         {
//             if(encodings[i] === 1)
//             {
//                 var bias = 0;
//                 for(var j = 0; j < vertexCount; ++j)
//                 {
//                     var encodedValue = dataView.getUint8(biases[i] + bias, true);
//                     bias += 1;
//                     var decodedValue = bboxMin[i] + encodedValue * precision[i];
//                     outputArray[j * elementCount + i] = decodedValue;
//                 }
//             }
//             else if(encodings[i] === 2)
//             {
//                 var input = new Uint8Array(dataView.buffer, biases[i], lengthes[i]);
//                 var output = new Uint32Array(vertexCount);
//                 this._decodeVarint(input, lengthes[i], vertexCount, output);
//                 this._decodeZigzagAndDelta(output);

//                 for(var j = 0; j < vertexCount; ++j)
//                 {
//                     var encodedValue = output[j];
//                     bias += 2;
//                     var decodedValue = bboxMin[i] + encodedValue * precision[i];
//                     outputArray[j * elementCount + i] = decodedValue;
//                 }
//             }
//             else if(encodings[i] === 4)
//             {
//                 var bias = 0;
//                 for(var j = 0; j < vertexCount; ++j)
//                 {
//                     decodedValue = dataView.getFloat32(biases[i] + bias, true);
//                     bias += 4;
//                     outputArray[j * elementCount + i] = decodedValue;
//                 }
//             }
//             else
//             {
//                 console.error('Unrecognized vertex data encoding!');
//             }
//         }

//         return outputArray;
//     },

//     _decodeVertexColors: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var output = new Uint8Array(elementCount * attributeCount);
//         var unitColor = (dataView.getUint8(offset) === 1) ? true : false;
//         var unitAlpha;
//         var alphaDataOffset;
//         var colorValue;
//         var alphaValue;
//         if(unitColor)
//         {
//             alphaDataOffset = 4;
//             unitAlpha = (dataView.getUint8(offset + alphaDataOffset) === 1) ? true : false;
//             colorValue = [dataView.getUint8(offset + 1), dataView.getUint8(offset + 2), dataView.getUint8(offset + 3)];
//             for(var i = 0; i < attributeCount; ++i)
//             {
//                 output[i * elementCount + 0] = colorValue[0];
//                 output[i * elementCount + 1] = colorValue[1];
//                 output[i * elementCount + 2] = colorValue[2];
//             }
//         }
//         else
//         {
//             alphaDataOffset = 2 + attributeCount * 2;
//             unitAlpha = (dataView.getUint8(offset + alphaDataOffset) === 1) ? true : false;

//             for(var i = 0; i < attributeCount; ++i)
//             {
//                 var s0 = dataView.getUint8(offset + 2 + i * 2);
//                 var s1 = dataView.getUint8(offset + 2 + i * 2 + 1);
//                 var s_color = s0 | (s1 << 8);

//                 var b = (((s_color) & 0x001F) << 3);
//                 var g = (((s_color) & 0x07E0) >> 3);
//                 var r = (((s_color) & 0xF800) >> 8);

//                 output[i * elementCount + 0] = r;
//                 output[i * elementCount + 1] = g;
//                 output[i * elementCount + 2] = b;
//             }
//         }

//         if(unitAlpha)
//         {
//             alphaValue = dataView.getUint8(offset + alphaDataOffset + 1);
//             for(var i = 0; i < attributeCount; ++i)
//             {
//                 output[i * elementCount + 3] = alphaValue;
//             }
//         }
//         else
//         {
//             for(var i = 0; i < attributeCount; ++i)
//             {
//                 output[i * elementCount + 3] = dataView.getUint8(offset + alphaDataOffset + 1 + i);
//             }
//         }

//         return output;
//     },

//     _decodeNormals: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var output = new Float32Array(elementCount * attributeCount);

//         var k = Math.PI / (GASEngine.GAS2MeshDecoder.NORMAL_ENCODING_PHI_COUNT - 1);
//         for(var i = 0; i < attributeCount; ++i)
//         {
//             var encodedPhi = dataView.getUint8(offset + i * 2);
//             var encodedTheta = dataView.getUint8(offset + i * 2 + 1);

//             var phi = encodedPhi * k;
//             var theta = (2.0 * encodedTheta * Math.PI) / GASEngine.GAS2MeshDecoder.NORMAL_DECODING_DICTIONARY[encodedPhi];

//             var m = Math.sin(phi);

//             output[i * elementCount + 0] = m * Math.cos(theta);
//             output[i * elementCount + 1] = m * Math.sin(theta);
//             output[i * elementCount + 2] = Math.cos(phi);
//         }

//         return output;
//     },

//     _decodeTangents: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var output = new Float32Array(elementCount * attributeCount);

//         var k = Math.PI / (GASEngine.GAS2MeshDecoder.NORMAL_ENCODING_PHI_COUNT - 1);
//         for(var i = 0; i < attributeCount; ++i)
//         {
//             var encodedPhi = dataView.getUint8(offset + i * 2);
//             var encodedTheta = dataView.getUint8(offset + i * 2 + 1);

//             output[i * elementCount + 3] = (128 & encodedPhi ? -1 : 1);
//             encodedPhi &= -129;

//             var phi = encodedPhi * k;
//             var theta = (2.0 * encodedTheta * Math.PI) / GASEngine.GAS2MeshDecoder.NORMAL_DECODING_DICTIONARY[encodedPhi];

//             var m = Math.sin(phi);

//             output[i * elementCount + 0] = m * Math.cos(theta);
//             output[i * elementCount + 1] = m * Math.sin(theta);
//             output[i * elementCount + 2] = Math.cos(phi);
//         }

//         return output;
//     },

//     _decodeUV: function(dataView, offset, sectionLength, encodingFlag, elementCount, vertexCount, bboxMin, bboxMax)
//     {
//         var grades = Math.pow(2.0, 12.0) - 1.0;
//         var precisionX = (bboxMax[0] - bboxMin[0]) / grades;
//         var precisionY = (bboxMax[1] - bboxMin[1]) / grades;

//         var outputArray = new Float32Array(elementCount * vertexCount);

//         for(var i = 0; i < vertexCount; ++i)
//         {
//             var k0 = dataView.getUint8(offset + i * 3);
//             var k1 = dataView.getUint8(offset + i * 3 + 1);
//             var k2 = dataView.getUint8(offset + i * 3 + 2);

//             var encoded = k0 | (k1 << 8) | (k2 << 16);

//             var uvX = (encoded & (0x0FFF)) * precisionX + bboxMin[0];
//             var uvY = ((encoded >>> 12) & (0x0FFF)) * precisionY + bboxMin[1];

//             outputArray[i * elementCount] = uvX;
//             outputArray[i * elementCount + 1] = uvY;
//         }

//         return outputArray;
//     },

//     _decodeBlendWeights: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var input = new Uint8Array(dataView.buffer, offset, sectionLength);
//         var output = new Uint32Array(elementCount * attributeCount);
//         this._decodeVarint(input, sectionLength, elementCount * attributeCount, output);

//         var outputArray = new Float32Array(elementCount * attributeCount);
//         for(var i = 0; i < attributeCount; ++i)
//         {
//             outputArray[i * 4 + 0] = output[i * 4 + 0] / 65535.0;
//             outputArray[i * 4 + 1] = output[i * 4 + 1] / 65535.0;
//             outputArray[i * 4 + 2] = output[i * 4 + 2] / 65535.0;
//             outputArray[i * 4 + 3] = output[i * 4 + 3] / 65535.0;
//         }

//         return outputArray;
//     },

//     _decodeBlendIndices: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var input = new Uint8Array(dataView.buffer, offset, sectionLength);
//         var output = new Uint16Array(elementCount * attributeCount);
//         this._decodeVarint(input, sectionLength, elementCount * attributeCount, output);
//         return output;
//     },

//     _decodeIndices: function(dataView, offset, sectionLength, encodingFlag, elementCount, attributeCount)
//     {
//         var output = new Uint32Array(elementCount * attributeCount);
//         var input = new Uint8Array(dataView.buffer, offset, sectionLength);
//         this._decodeVarint(input, sectionLength, elementCount * attributeCount, output);
//         this._decodeZigzagAndDelta(output);

//         return output;
//     },

//     _decodeVarint: function(input, inputSize, elementCount, output)
//     {
//         var s = 0;
//         for(var index = 0; index != elementCount;)
//         {
//             var o = 0;
//             var l = 0;
//             do
//             {
//                 o |= (127 & input[s]) << l;
//                 l += 7;
//             } while(0 != (128 & input[s++]));

//             output[index++] = o;
//         }

//         if(inputSize < s)
//         {
//             console.error('Decode var int failed!');
//         }

//         return s;
//     },

//     _decodeZigzagAndDelta: function(input)
//     {
//         var count = input.length;
//         var lastValue = 0;
//         for(var s = 0; s < count; ++s)
//         {
//             var n = input[s];
//             var i = ((n >> 1) ^ (0 - (n & 1)));
//             var o = lastValue + i;

//             input[s] = o;

//             lastValue = o;
//         }
//     },

//     parseMesh: function(dataView, beginningOffset)
//     {
//         var version = dataView.getUint32(beginningOffset + 12, true);
//         if(version !== 7)
//         {
//             console.error('GASEngine.GAS2MeshDecoder.parseMesh: the old mesh binary format has been not supported anymore!');
//             return null;
//         }

//         var objectSize = dataView.getUint32(beginningOffset + 4, true);
       
//         var sectionCountOffset = beginningOffset + dataView.getUint32(beginningOffset + 16, true);
//         var sectionCount = dataView.getUint32(sectionCountOffset, true);

//         var mesh = GASEngine.MeshFactory.Instance.create();

//         mesh.parentUniqueID = dataView.getInt32(beginningOffset + 20, true);
//         mesh.uniqueID = dataView.getInt32(beginningOffset + 24, true);

//         var posBboxMin = [
//             dataView.getFloat32(beginningOffset + 44, true),
//             dataView.getFloat32(beginningOffset + 48, true),
//             dataView.getFloat32(beginningOffset + 52, true)];

//         var posBboxMax = [
//             dataView.getFloat32(beginningOffset + 56, true),
//             dataView.getFloat32(beginningOffset + 60, true),
//             dataView.getFloat32(beginningOffset + 64, true)];

//         var uvBboxMin = [
//             dataView.getFloat32(beginningOffset + 68, true),
//             dataView.getFloat32(beginningOffset + 72, true),
//             0.0];

//         var uvBboxMax = [
//             dataView.getFloat32(beginningOffset + 76, true),
//             dataView.getFloat32(beginningOffset + 80, true),
//             0.0];

//         mesh.setBoundingBox([posBboxMin, posBboxMax]);

//         var offset = sectionCountOffset;
//         for(var i = 0; i < sectionCount; ++i)
//         {
//             offset += 4;
//             var sectionType = dataView.getUint32(offset, true);
//             offset += 4;
//             var dataAttribute = dataView.getUint32(offset, true);
//             offset += 4;
//             var sectionOffset = beginningOffset + dataView.getUint32(offset, true);
//             offset += 4;
//             var sectionLength = dataView.getUint32(offset, true);
//             offset += 4;
//             var attributeCount = dataView.getUint32(offset, true);
//             offset += 4;
//             var elementCount = dataView.getUint32(offset, true);
//             offset += 4;
//             var elementType = dataView.getUint32(offset, true);

//             if(sectionType === GASEngine.GAS2MeshDecoder.MESH_DATA_POSITION)
//             {
//                 var stream = this._loadFunctions.get(sectionType)(
//                     dataView,
//                     sectionOffset,
//                     sectionLength,
//                     dataAttribute,
//                     elementCount,
//                     attributeCount,
//                     posBboxMin,
//                     posBboxMax);

//                 if(stream !== null)
//                 {
//                     mesh.addStream(GASEngine.GAS2MeshDecoder.SHADER_STREAM_ATTRIBUTE_NAMES[sectionType], stream);
//                 }
//             }
//             else if(sectionType === GASEngine.GAS2MeshDecoder.MESH_DATA_UV0 ||
//                 sectionType === GASEngine.GAS2MeshDecoder.MESH_DATA_UV1)
//             {
//                 var stream = this._loadFunctions.get(sectionType)(
//                     dataView,
//                     sectionOffset,
//                     sectionLength,
//                     dataAttribute,
//                     elementCount,
//                     attributeCount,
//                     uvBboxMin,
//                     uvBboxMax);

//                 if(stream !== null)
//                 {
//                     mesh.addStream(GASEngine.GAS2MeshDecoder.SHADER_STREAM_ATTRIBUTE_NAMES[sectionType], stream);
//                 }
//             }
//             else
//             {
//                 var stream = this._loadFunctions.get(sectionType)(
//                     dataView,
//                     sectionOffset,
//                     sectionLength,
//                     dataAttribute,
//                     elementCount,
//                     attributeCount);

//                 if(stream !== null)
//                 {
//                     mesh.addStream(GASEngine.GAS2MeshDecoder.SHADER_STREAM_ATTRIBUTE_NAMES[sectionType], stream);
//                 }
//             }
//             //<
//         }

//         var normalStream = mesh.getStream('normal');
//         if(normalStream === null)
//         {
//             var indices = mesh.getStream('index');
//             var positions = mesh.getStream('position');
//             var subMeshes = mesh.getStream('subMesh');

//             if(indices !== null && positions !== null && subMeshes !== null)
//             {
//                 var outNormals = new Float32Array(positions.length);
//                 GASEngine.Utilities.computeNormals(indices, positions, subMeshes, outNormals);
//                 mesh.addStream('normal', outNormals);
//             }
//             else
//             {
//                 console.error('GASEngine.GAS2MeshDecoder.parseMesh: unable to compute vertex normals for inadequate source stream!');
//             }
//         }

//         var tangentStream = mesh.getStream('tangent');
//         if(tangentStream === null)
//         {
//             var indices = mesh.getStream('index');
//             var positions = mesh.getStream('position');
//             var normals = mesh.getStream('normal');
//             var UVs = mesh.getStream('uv');
//             var subMeshes = mesh.getStream('subMesh');

//             if(indices !== null && positions !== null && normals !== null && UVs !== null && subMeshes !== null)
//             {
//                 var outTangents = new Float32Array((positions.length / 3) * 4);
//                 GASEngine.Utilities.computeTangents(indices, positions, normals, UVs, subMeshes, outTangents);
//                 mesh.addStream('tangent', outTangents);
//             }
//             else
//             {
//                 console.error('GASEngine.GAS2MeshDecoder.parseMesh: unable to compute vertex tangents for inadequate source stream!');
//             }
//         }

//         mesh.setDrawMode('TRIANGLES');

//         return { 'mesh': mesh, 'objectSize': objectSize };
//     },

//     parse: function(dataView)
//     {
//         var meshTargets = [];

//         var offset = 0;
//         while(offset < dataView.byteLength)
//         {
//             var result = this.parseMesh(dataView, offset);
//             meshTargets.push(result.mesh);

//             offset += result.objectSize;
//         }

//         if(meshTargets.length > 1)
//         {
//             for(var n = 1; n < meshTargets.length; ++n)
//             {
//                 meshTargets[0].addMorphTarget(meshTargets[n]);
//             }
//         }
        
//         return meshTargets[0];
//     }
// };