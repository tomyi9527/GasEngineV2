
GASEngine.GLTFConsts =
{
    WEBGL_CONSTANTS :  {
		FLOAT: 5126,
		//FLOAT_MAT2: 35674,
		FLOAT_MAT3: 35675,
		FLOAT_MAT4: 35676,
		FLOAT_VEC2: 35664,
		FLOAT_VEC3: 35665,
		FLOAT_VEC4: 35666,
		LINEAR: 9729,
		REPEAT: 10497,
		SAMPLER_2D: 35678,
		POINTS: 0,
		LINES: 1,
		LINE_LOOP: 2,
		LINE_STRIP: 3,
		TRIANGLES: 4,
		TRIANGLE_STRIP: 5,
		TRIANGLE_FAN: 6,

		UNSIGNED_BYTE: 5121,
		UNSIGNED_SHORT: 5123,
		UNSIGNED_INT: 5125,

		ARRAY_BUFFER: 0x8892,
		ELEMENT_ARRAY_BUFFER: 0x8893,
    },
    
	WEBGL_TYPE : {
		5126: Number,
		//35674: GASEngine.Matrix2,
		35675: GASEngine.Matrix3,
		35676: GASEngine.Matrix4,
		35664: GASEngine.Vector2,
		35665: GASEngine.Vector3,
		35666: GASEngine.Vector4,
		//35678: GASEngine.Texture
    },
    
	WEBGL_COMPONENT_TYPES : {
		5120: Int8Array,
		5121: Uint8Array,
		5122: Int16Array,
		5123: Uint16Array,
		5125: Uint32Array,
		5126: Float32Array
    },
    
	WEBGL_FILTERS : {
		9728: 'NEAREST',
		9729: 'LINEAR',
		9984: 'NEAREST_MIPMAP_NEAREST',
		9985: 'LINEAR_MIPMAP_NEAREST',
		9986: 'NEAREST_MIPMAP_LINEAR',
		9987: 'LINEAR_MIPMAP_LINEAR'
	},

	TO_WEBGL_FILTERS: {
		'NEAREST': 9728,
		'LINEAR': 9729,
		'NEAREST_MIPMAP_NEAREST': 9728,
		'LINEAR_MIPMAP_NEAREST': 9985,
		'NEAREST_MIPMAP_LINEAR': 9986,
		'LINEAR_MIPMAP_LINEAR': 9987
	},

	WEBGL_WRAPPINGS : {
		33071: 'CLAMP_TO_EDGE',
		33648: 'MIRRORED_REPEAT',
		10497: 'REPEAT'
	},

	TO_WEBGL_WRAPPINGS : {
		'CLAMP_TO_EDGE': 33071,
		'MIRRORED_REPEAT': 33648,
		'REPEAT': 10497
	},

	WEBGL_SIDES : {
		1028: GASEngine.BackSide, // Culling front
		1029: GASEngine.FrontSide // Culling back
		//1032: GASEngine.NoSide   // Culling front and back, what to do?
	},

	WEBGL_DEPTH_FUNCS : {
		512: GASEngine.NeverDepth,
		513: GASEngine.LessDepth,
		514: GASEngine.EqualDepth,
		515: GASEngine.LessEqualDepth,
		516: GASEngine.GreaterEqualDepth,
		517: GASEngine.NotEqualDepth,
		518: GASEngine.GreaterEqualDepth,
		519: GASEngine.AlwaysDepth
	},

	WEBGL_BLEND_EQUATIONS : {
		32774: GASEngine.AddEquation,
		32778: GASEngine.SubtractEquation,
		32779: GASEngine.ReverseSubtractEquation
	},

	WEBGL_BLEND_FUNCS : {
		0: GASEngine.ZeroFactor,
		1: GASEngine.OneFactor,
		768: GASEngine.SrcColorFactor,
		769: GASEngine.OneMinusSrcColorFactor,
		770: GASEngine.SrcAlphaFactor,
		771: GASEngine.OneMinusSrcAlphaFactor,
		772: GASEngine.DstAlphaFactor,
		773: GASEngine.OneMinusDstAlphaFactor,
		774: GASEngine.DstColorFactor,
		775: GASEngine.OneMinusDstColorFactor,
		776: GASEngine.SrcAlphaSaturateFactor
		// The followings are not supported by Three.js yet
		//32769: CONSTANT_COLOR,
		//32770: ONE_MINUS_CONSTANT_COLOR,
		//32771: CONSTANT_ALPHA,
		//32772: ONE_MINUS_CONSTANT_COLOR
	},

	WEBGL_TYPE_SIZES : {
		'SCALAR': 1,
		'VEC2': 2,
		'VEC3': 3,
		'VEC4': 4,
		'MAT2': 4,
		'MAT3': 9,
		'MAT4': 16
	},

	ATTRIBUTES : {
		POSITION: 'position',
		NORMAL: 'normal',
		TEXCOORD_0: 'uv',
		TEXCOORD0: 'uv', // deprecated
		TEXCOORD: 'uv', // deprecated
		TEXCOORD_1: 'uv2',
		COLOR_0: 'color',
		COLOR0: 'color', // deprecated
		COLOR: 'color', // deprecated
		WEIGHTS_0: 'skinWeight',
		WEIGHT: 'skinWeight', // deprecated
		JOINTS_0: 'skinIndex',
		JOINT: 'skinIndex' // deprecated
	},

	STATES_ENABLES : {
		2884: 'CULL_FACE',
		2929: 'DEPTH_TEST',
		3042: 'BLEND',
		3089: 'SCISSOR_TEST',
		32823: 'POLYGON_OFFSET_FILL',
		32926: 'SAMPLE_ALPHA_TO_COVERAGE'
	},

	ALPHA_MODES : {
		OPAQUE: 'OPAQUE',
		MASK: 'MASK',
		BLEND: 'BLEND'
	},

	/* BINARY EXTENSION */
	BINARY_EXTENSION_BUFFER_NAME: 'binary_glTF',

	KHR_BINARY_GLTF: 'KHR_binary_glTF',

	BINARY_EXTENSION_HEADER_MAGIC: 'glTF',
	BINARY_EXTENSION_HEADER_LENGTH: 12,
	BINARY_EXTENSION_CHUNK_TYPES:
	{
		JSON: 0x4E4F534A, BIN: 0x004E4942
	}
};
