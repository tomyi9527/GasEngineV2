// GL STATE CONSTANTS
GASEngine.CullFaceNone = 0;
GASEngine.CullFaceBack = 1;
GASEngine.CullFaceFront = 2;
GASEngine.CullFaceFrontBack = 3;

GASEngine.FrontFaceDirectionCW = 0;
GASEngine.FrontFaceDirectionCCW = 1;

// SHADOWING TYPES

GASEngine.BasicShadowMap = 0;
GASEngine.PCFShadowMap = 1;
GASEngine.PCFSoftShadowMap = 2;

// MATERIAL CONSTANTS

// side

GASEngine.FrontSide = 0;
GASEngine.BackSide = 1;
GASEngine.DoubleSide = 2;

// shading

GASEngine.FlatShading = 1;
GASEngine.SmoothShading = 2;

// colors

GASEngine.NoColors = 0;
GASEngine.FaceColors = 1;
GASEngine.VertexColors = 2;

// blending modes

GASEngine.NoBlending = 0;
GASEngine.NormalBlending = 1;
GASEngine.AdditiveBlending = 2;
GASEngine.SubtractiveBlending = 3;
GASEngine.MultiplyBlending = 4;
GASEngine.CustomBlending = 5;

// custom blending equations
// (numbers start from 100 not to clash with other
// mappings to OpenGL constants defined in Texture.js)
GASEngine.AddEquation = 100;
GASEngine.SubtractEquation = 101;
GASEngine.ReverseSubtractEquation = 102;
GASEngine.MinEquation = 103;
GASEngine.MaxEquation = 104;

// custom blending destination factors

GASEngine.ZeroFactor = 200;
GASEngine.OneFactor = 201;
GASEngine.SrcColorFactor = 202;
GASEngine.OneMinusSrcColorFactor = 203;
GASEngine.SrcAlphaFactor = 204;
GASEngine.OneMinusSrcAlphaFactor = 205;
GASEngine.DstAlphaFactor = 206;
GASEngine.OneMinusDstAlphaFactor = 207;

// custom blending source factors

//GASEngine.ZeroFactor = 200;
//GASEngine.OneFactor = 201;
//GASEngine.SrcAlphaFactor = 204;
//GASEngine.OneMinusSrcAlphaFactor = 205;
//GASEngine.DstAlphaFactor = 206;
//GASEngine.OneMinusDstAlphaFactor = 207;
GASEngine.DstColorFactor = 208;
GASEngine.OneMinusDstColorFactor = 209;
GASEngine.SrcAlphaSaturateFactor = 210;

// depth modes

GASEngine.NeverDepth = 0;
GASEngine.AlwaysDepth = 1;
GASEngine.LessDepth = 2;
GASEngine.LessEqualDepth = 3;
GASEngine.EqualDepth = 4;
GASEngine.GreaterEqualDepth = 5;
GASEngine.GreaterDepth = 6;
GASEngine.NotEqualDepth = 7;


// TEXTURE CONSTANTS

GASEngine.MultiplyOperation = 0;
GASEngine.MixOperation = 1;
GASEngine.AddOperation = 2;

// Mapping modes

GASEngine.UVMapping = 300;

GASEngine.CubeReflectionMapping = 301;
GASEngine.CubeRefractionMapping = 302;

GASEngine.EquirectangularReflectionMapping = 303;
GASEngine.EquirectangularRefractionMapping = 304;

GASEngine.SphericalReflectionMapping = 305;

// Data types
GASEngine.UnsignedByteType = 1009;
GASEngine.ByteType = 1010;
GASEngine.ShortType = 1011;
GASEngine.UnsignedShortType = 1012;
GASEngine.IntType = 1013;
GASEngine.UnsignedIntType = 1014;
GASEngine.FloatType = 1015;
GASEngine.HalfFloatType = 1025;

// Pixel types

//GASEngine.UnsignedByteType = 1009;
GASEngine.UnsignedShort4444Type = 1016;
GASEngine.UnsignedShort5551Type = 1017;
GASEngine.UnsignedShort565Type = 1018;

// Pixel formats

GASEngine.AlphaFormat = 1019;
GASEngine.RGBFormat = 1020;
GASEngine.RGBAFormat = 1021;
GASEngine.LuminanceFormat = 1022;
GASEngine.LuminanceAlphaFormat = 1023;
// GASEngine.RGBEFormat handled as GASEngine.RGBAFormat in shaders
GASEngine.RGBEFormat = GASEngine.RGBAFormat; //1024;

// DDS / ST3C Compressed texture formats

GASEngine.RGB_S3TC_DXT1_Format = 2001;
GASEngine.RGBA_S3TC_DXT1_Format = 2002;
GASEngine.RGBA_S3TC_DXT3_Format = 2003;
GASEngine.RGBA_S3TC_DXT5_Format = 2004;


// PVRTC compressed texture formats

GASEngine.RGB_PVRTC_4BPPV1_Format = 2100;
GASEngine.RGB_PVRTC_2BPPV1_Format = 2101;
GASEngine.RGBA_PVRTC_4BPPV1_Format = 2102;
GASEngine.RGBA_PVRTC_2BPPV1_Format = 2103;

// ETC compressed texture formats

GASEngine.RGB_ETC1_Format = 2151;

// Loop styles for AnimationAction

GASEngine.LoopOnce = 2200;
GASEngine.LoopRepeat = 2201;
GASEngine.LoopPingPong = 2202;

// Interpolation

GASEngine.InterpolateDiscrete = 2300;
GASEngine.InterpolateLinear = 2301;
GASEngine.InterpolateSmooth = 2302;

// Interpolant ending modes

GASEngine.ZeroCurvatureEnding = 2400;
GASEngine.ZeroSlopeEnding = 2401;
GASEngine.WrapAroundEnding = 2402;

// Triangle Draw modes
GASEngine.TrianglesDrawMode = 0;
GASEngine.TriangleStripDrawMode = 1;
GASEngine.TriangleFanDrawMode = 2;