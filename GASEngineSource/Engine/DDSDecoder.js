GASEngine.DDSDecoder = function()
{
};

GASEngine.DDSDecoder.prototype =
{
    constructor: GASEngine.DDSDecoder,

    decode: function(buffer, loadMipmaps)
    {
        var dds = { mipmaps: [], width: 0, height: 0, format: null, mipmapCount: 1 };

        // Adapted from @toji's DDS utils
        // https://github.com/toji/webgl-texture-utils/blob/master/texture-util/dds.js

        // All values and structures referenced from:
        // http://msdn.microsoft.com/en-us/library/bb943991.aspx/

        var DDS_MAGIC = 0x20534444;

        var DDSD_CAPS = 0x1,
            DDSD_HEIGHT = 0x2,
            DDSD_WIDTH = 0x4,
            DDSD_PITCH = 0x8,
            DDSD_PIXELFORMAT = 0x1000,
            DDSD_MIPMAPCOUNT = 0x20000,
            DDSD_LINEARSIZE = 0x80000,
            DDSD_DEPTH = 0x800000;

        var DDSCAPS_COMPLEX = 0x8,
            DDSCAPS_MIPMAP = 0x400000,
            DDSCAPS_TEXTURE = 0x1000;

        var DDSCAPS2_CUBEMAP = 0x200,
            DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
            DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
            DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
            DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
            DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
            DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
            DDSCAPS2_VOLUME = 0x200000;

        var DDPF_ALPHAPIXELS = 0x1,
            DDPF_ALPHA = 0x2,
            DDPF_FOURCC = 0x4,
            DDPF_RGB = 0x40,
            DDPF_YUV = 0x200,
            DDPF_LUMINANCE = 0x20000;

        function fourCCToInt32(value)
        {
            return value.charCodeAt( 0 ) +
                ( value.charCodeAt( 1 ) << 8 ) +
                ( value.charCodeAt( 2 ) << 16 ) +
                ( value.charCodeAt( 3 ) << 24 );
        }

        function int32ToFourCC(value)
        {
            return String.fromCharCode(
                value & 0xff,
                ( value >> 8 ) & 0xff,
                ( value >> 16 ) & 0xff,
                ( value >> 24 ) & 0xff
            );
        }

        function loadARGBMip(buffer, outBuffer, dataOffset, width, height)
        {
            var dataLength = width * height * 4;
            var srcBuffer = new Uint8Array( buffer, dataOffset, dataLength );
            var dst = 0;
            var src = 0;
            for(var y = 0; y < height; y++)
            {
                for(var x = 0; x < width; x++)
                {
                    var b = srcBuffer[ src ]; src ++;
                    var g = srcBuffer[ src ]; src ++;
                    var r = srcBuffer[ src ]; src ++;
                    var a = srcBuffer[ src ]; src ++;
                    outBuffer[dst] = r; dst++;	//r
                    outBuffer[dst] = g; dst++;	//g
                    outBuffer[dst] = b; dst++;	//b
                    outBuffer[dst] = a; dst++;	//a
                }

            }
        }

        var FOURCC_DXT1 = fourCCToInt32( "DXT1" );
        var FOURCC_DXT3 = fourCCToInt32( "DXT3" );
        var FOURCC_DXT5 = fourCCToInt32( "DXT5" );
        var FOURCC_ETC1 = fourCCToInt32( "ETC1" );

        var headerLengthInt = 31; // The header length in 32 bit ints

        // Offsets into the header array

        var off_magic = 0;

        var off_size = 1;
        var off_flags = 2;
        var off_height = 3;
        var off_width = 4;

        var off_mipmapCount = 7;

        var off_pfFlags = 20;
        var off_pfFourCC = 21;
        var off_RGBBitCount = 22;
        var off_RBitMask = 23;
        var off_GBitMask = 24;
        var off_BBitMask = 25;
        var off_ABitMask = 26;

        var off_caps = 27;
        var off_caps2 = 28;
        var off_caps3 = 29;
        var off_caps4 = 30;

        // Parse header
        var header = new Int32Array( buffer, 0, headerLengthInt );

        if(header[off_magic] !== DDS_MAGIC)
        {
            console.error('GASEngine.DDSDecoder.decode: Invalid magic number in DDS header.');
            return dds;
        }

        if(!header[off_pfFlags] & DDPF_FOURCC)
        {
            console.error('GASEngine.DDSDecoder.decode: Unsupported format, must contain a FourCC code.');
            return dds;
        }

        var blockBytes;

        var fourCC = header[ off_pfFourCC ];

        var isRGBAUncompressed = false;

        var s3tcExtension = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_s3tc');
        var gl = GASEngine.WebGLDevice.Instance.gl;

        switch(fourCC)
        {
            case FOURCC_DXT1:
                blockBytes = 8;
                dds.format = s3tcExtension.COMPRESSED_RGB_S3TC_DXT1_EXT;
                break;
            case FOURCC_DXT3:
                blockBytes = 16;
                dds.format = s3tcExtension.COMPRESSED_RGBA_S3TC_DXT3_EXT;
                break;
            case FOURCC_DXT5:
                blockBytes = 16;
                dds.format = s3tcExtension.COMPRESSED_RGBA_S3TC_DXT5_EXT;
                break;
            //case FOURCC_ETC1:
            //    blockBytes = 8;
            //    dds.format = etc1Extension.COMPRESSED_RGB_ETC1_WEBGL;
            //    break;
            default:

                if ( header[ off_RGBBitCount ] === 32
                    && header[ off_RBitMask ] & 0xff0000
                    && header[ off_GBitMask ] & 0xff00
                    && header[ off_BBitMask ] & 0xff
                    && header[off_ABitMask] & 0xff000000)
                {
                    isRGBAUncompressed = true;
                    blockBytes = 64;
                    dds.format = gl.RGBA;
                }
                else
                {
                    console.error('GASEngine.DDSDecoder.decode: Unsupported FourCC code ', int32ToFourCC(fourCC));
                    return dds;
                }
        }

        dds.mipmapCount = 1;

        if(header[off_flags] & DDSD_MIPMAPCOUNT && loadMipmaps !== false)
        {
            dds.mipmapCount = Math.max( 1, header[ off_mipmapCount ] );
        }

        var caps2 = header[ off_caps2 ];
        dds.isCubemap = caps2 & DDSCAPS2_CUBEMAP ? true : false;
        if ( dds.isCubemap && (
            ! ( caps2 & DDSCAPS2_CUBEMAP_POSITIVEX ) ||
            ! ( caps2 & DDSCAPS2_CUBEMAP_NEGATIVEX ) ||
            ! ( caps2 & DDSCAPS2_CUBEMAP_POSITIVEY ) ||
            ! ( caps2 & DDSCAPS2_CUBEMAP_NEGATIVEY ) ||
            ! ( caps2 & DDSCAPS2_CUBEMAP_POSITIVEZ ) ||
            ! ( caps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ )
            ))
        {
            console.error('GASEngine.DDSDecoder.decode: Incomplete cubemap faces');
            return dds;
        }

        dds.width = header[ off_width ];
        dds.height = header[ off_height ];

        var dataOffset = header[ off_size ] + 4;

        // Extract mipmaps buffers
        var faces = dds.isCubemap ? 6 : 1;

        for(var face = 0; face < faces; face++)
        {
            var width = dds.width;
            var height = dds.height;

            for(var i = 0; i < dds.mipmapCount; i++)
            {
                var rgbaData = null;
                var dataLength = 0;

                if(isRGBAUncompressed)
                {
                    var canvas = document.createElement('canvas');
                    canvas.width = width;
                    canvas.height = height;
                    var context = canvas.getContext('2d');
                    var imageData = context.createImageData(width, height);
                    loadARGBMip(buffer, imageData.data, dataOffset, width, height);
                    dataLength = width * height * 4;

                    context.putImageData(imageData, 0, 0);
                    rgbaData = canvas;
                }
                else
                {
                    var blockCountWidth = Math.max(4, width) / 4;
                    var blockCountHeight = Math.max(4, height) / 4;
                    dataLength = blockCountWidth * blockCountHeight * blockBytes;
                    var byteArray = new Uint8Array(buffer, dataOffset, dataLength);

                    //var byteArray = this.flipY(byteArray, blockBytes, blockCountWidth, blockCountHeight, dds.format);

                    var canvas = document.createElement('canvas');
                    canvas.width = blockCountWidth * 4;
                    canvas.height = blockCountHeight * 4;
                    var context = canvas.getContext('2d');
                    var imageData = context.createImageData(blockCountWidth * 4, blockCountHeight * 4);
                    this.BC13_RGBA(byteArray, imageData.data, blockBytes, blockCountWidth, blockCountHeight, dds.format);
                    context.putImageData(imageData, 0, 0);

                    rgbaData = canvas;
                }

                var mipmap = { "data": byteArray, "rgbaData": rgbaData, "width": width, "height": height };
                dds.mipmaps.push( mipmap );

                dataOffset += dataLength;

                width = Math.max( width >> 1, 1 );
                height = Math.max( height >> 1, 1 );
            }
        }

        return dds;
    },

    BC13_RGBA: function(input, output, blockBytes, blockCountWidth, blockCountHeight, format)
    {
        function rgb565_8888(color)
        {
            var b = (((color) & 0x001F) << 3);
            var g = (((color) & 0x07E0) >> 3);
            var r = (((color) & 0xF800) >> 8);
            return [r, g, b, 255];
        }

        var colorPalette = [];
        colorPalette.length = 4;

        var alphaPalette = [];
        alphaPalette.length = 8;

        var colorIndices = [];
        colorIndices.length = 16;

        var alphaIndices = [];
        alphaIndices.length = 16;

        var rgbaImageWidth = blockCountWidth * 4;
        var rgbaImageHeight = blockCountHeight * 4;

        function decodeBC1(input, offset, output, x, y)
        {
            var c0_565 = (input[offset + 0]) | (input[offset + 1] << 8);
            var c1_565 = (input[offset + 2]) | (input[offset + 3] << 8);

            colorPalette[0] = rgb565_8888(c0_565);
            colorPalette[1] = rgb565_8888(c1_565);

            if(c0_565 <= c1_565)
            {
                colorPalette[2] = [
                    Math.floor((1.0 / 2.0) * colorPalette[0][0] + (1.0 / 2.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][1] + (1.0 / 2.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][2] + (1.0 / 2.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [0, 0, 0, 0];
            }
            else
            {
                colorPalette[2] = [
                    Math.floor((2.0 / 3.0) * colorPalette[0][0] + (1.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][1] + (1.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][2] + (1.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [
                    Math.floor((1.0 / 3.0) * colorPalette[0][0] + (2.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][1] + (2.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][2] + (2.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];
            }            

            colorIndices[0] = ((input[offset + 4] >> 0) & 0x03);
            colorIndices[1] = ((input[offset + 4] >> 2) & 0x03);
            colorIndices[2] = ((input[offset + 4] >> 4) & 0x03);
            colorIndices[3] = ((input[offset + 4] >> 6) & 0x03);

            colorIndices[4] = ((input[offset + 5] >> 0) & 0x03);
            colorIndices[5] = ((input[offset + 5] >> 2) & 0x03);
            colorIndices[6] = ((input[offset + 5] >> 4) & 0x03);
            colorIndices[7] = ((input[offset + 5] >> 6) & 0x03);

            colorIndices[8] = ((input[offset + 6] >> 0) & 0x03);
            colorIndices[9] = ((input[offset + 6] >> 2) & 0x03);
            colorIndices[10] = ((input[offset + 6] >> 4) & 0x03);
            colorIndices[11] = ((input[offset + 6] >> 6) & 0x03);

            colorIndices[12] = ((input[offset + 7] >> 0) & 0x03);
            colorIndices[13] = ((input[offset + 7] >> 2) & 0x03);
            colorIndices[14] = ((input[offset + 7] >> 4) & 0x03);
            colorIndices[15] = ((input[offset + 7] >> 6) & 0x03);

            var c;
            var _x, _y;
            var _of;

            for(var i = 0; i < colorIndices.length; ++i)
            {
                c = colorPalette[colorIndices[i]];
                _x = i % 4;
                _y = Math.floor(i / 4);

                _of = ((y + _y) * rgbaImageWidth + x + _x) * 4;
                output[_of + 0] = c[0];
                output[_of + 1] = c[1];
                output[_of + 2] = c[2];
                output[_of + 3] = c[3];
            }
        };

        var alphaPalette = [];
        alphaPalette.length = 8;

        var alphaIndices = [];
        alphaIndices.length = 16;

        function decodeBC3(input, offset, output, x, y)
        {
            //Alpha
            alphaPalette[0] = input[offset + 0];
            alphaPalette[1] = input[offset + 1];
            if(alphaPalette[0] > alphaPalette[1])
            {
                alphaPalette[2] = Math.floor((6.0 / 7.0) * alphaPalette[0] + (1.0 / 7.0) * alphaPalette[1] + 0.5);
                alphaPalette[3] = Math.floor((5.0 / 7.0) * alphaPalette[0] + (2.0 / 7.0) * alphaPalette[1] + 0.5);
                alphaPalette[4] = Math.floor((4.0 / 7.0) * alphaPalette[0] + (3.0 / 7.0) * alphaPalette[1] + 0.5);
                alphaPalette[5] = Math.floor((3.0 / 7.0) * alphaPalette[0] + (4.0 / 7.0) * alphaPalette[1] + 0.5);
                alphaPalette[6] = Math.floor((2.0 / 7.0) * alphaPalette[0] + (5.0 / 7.0) * alphaPalette[1] + 0.5);
                alphaPalette[7] = Math.floor((1.0 / 7.0) * alphaPalette[0] + (6.0 / 7.0) * alphaPalette[1] + 0.5);
            }
            else
            {
                alphaPalette[2] = Math.floor((4.0 / 5.0) * alphaPalette[0] + (1.0 / 5.0) * alphaPalette[1] + 0.5);
                alphaPalette[3] = Math.floor((3.0 / 5.0) * alphaPalette[0] + (2.0 / 5.0) * alphaPalette[1] + 0.5);
                alphaPalette[4] = Math.floor((2.0 / 5.0) * alphaPalette[0] + (3.0 / 5.0) * alphaPalette[1] + 0.5);
                alphaPalette[5] = Math.floor((1.0 / 5.0) * alphaPalette[0] + (4.0 / 5.0) * alphaPalette[1] + 0.5);
                alphaPalette[6] = 0;
                alphaPalette[7] = 255;
            }

            var ai0 = (input[offset + 2]) | (input[offset + 3] << 8) | (input[offset + 4] << 16);
            var ai1 = (input[offset + 5]) | (input[offset + 6] << 8) | (input[offset + 7] << 16);

            alphaIndices[0] = ((ai0 >> 0) & 0x07);
            alphaIndices[1] = ((ai0 >> 3) & 0x07);
            alphaIndices[2] = ((ai0 >> 6) & 0x07);
            alphaIndices[3] = ((ai0 >> 9) & 0x07);

            alphaIndices[4] = ((ai0 >> 12) & 0x07);
            alphaIndices[5] = ((ai0 >> 15) & 0x07);
            alphaIndices[6] = ((ai0 >> 18) & 0x07);
            alphaIndices[7] = ((ai0 >> 21) & 0x07);

            alphaIndices[8] = ((ai1 >> 0) & 0x07);
            alphaIndices[9] = ((ai1 >> 3) & 0x07);
            alphaIndices[10] = ((ai1 >> 6) & 0x07);
            alphaIndices[11] = ((ai1 >> 9) & 0x07);

            alphaIndices[12] = ((ai1 >> 12) & 0x07);
            alphaIndices[13] = ((ai1 >> 15) & 0x07);
            alphaIndices[14] = ((ai1 >> 18) & 0x07);
            alphaIndices[15] = ((ai1 >> 21) & 0x07);

            //Color
            var c0_565 = (input[offset + 8]) | (input[offset + 9] << 8);
            var c1_565 = (input[offset + 10]) | (input[offset + 11] << 8);

            colorPalette[0] = rgb565_8888(c0_565);
            colorPalette[1] = rgb565_8888(c1_565);

            if(c0_565 <= c1_565)
            {
                colorPalette[2] = [
                    Math.floor((1.0 / 2.0) * colorPalette[0][0] + (1.0 / 2.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][1] + (1.0 / 2.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][2] + (1.0 / 2.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [0, 0, 0, 0];
            }
            else
            {
                colorPalette[2] = [
                    Math.floor((2.0 / 3.0) * colorPalette[0][0] + (1.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][1] + (1.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][2] + (1.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [
                    Math.floor((1.0 / 3.0) * colorPalette[0][0] + (2.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][1] + (2.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][2] + (2.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];
            }

            colorIndices[0] = ((input[offset + 12] >> 0) & 0x03);
            colorIndices[1] = ((input[offset + 12] >> 2) & 0x03);
            colorIndices[2] = ((input[offset + 12] >> 4) & 0x03);
            colorIndices[3] = ((input[offset + 12] >> 6) & 0x03);

            colorIndices[4] = ((input[offset + 13] >> 0) & 0x03);
            colorIndices[5] = ((input[offset + 13] >> 2) & 0x03);
            colorIndices[6] = ((input[offset + 13] >> 4) & 0x03);
            colorIndices[7] = ((input[offset + 13] >> 6) & 0x03);

            colorIndices[8] = ((input[offset + 14] >> 0) & 0x03);
            colorIndices[9] = ((input[offset + 14] >> 2) & 0x03);
            colorIndices[10] = ((input[offset + 14] >> 4) & 0x03);
            colorIndices[11] = ((input[offset + 14] >> 6) & 0x03);

            colorIndices[12] = ((input[offset + 15] >> 0) & 0x03);
            colorIndices[13] = ((input[offset + 15] >> 2) & 0x03);
            colorIndices[14] = ((input[offset + 15] >> 4) & 0x03);
            colorIndices[15] = ((input[offset + 15] >> 6) & 0x03);

            var c;
            var _x, _y;
            var _of;
            var a;

            for(var i = 0; i < colorIndices.length; ++i)
            {
                c = colorPalette[colorIndices[i]];
                a = alphaPalette[alphaIndices[i]];
                _x = i % 4;
                _y = Math.floor(i / 4);

                _of = ((y + _y) * rgbaImageWidth + x + _x) * 4;
                output[_of + 0] = c[0];
                output[_of + 1] = c[1];
                output[_of + 2] = c[2];
                output[_of + 3] = a;
            }
        };

        var alphaValues = [];
        alphaValues.length = 8;

        function decodeBC2(input, offset, output, x, y)
        {
            //Alpha
            alphaValues[0] = ((input[offset + 0] >> 0) & 0x0F) * 15;
            alphaValues[1] = ((input[offset + 0] >> 4) & 0x0F) * 15;
            alphaValues[2] = ((input[offset + 1] >> 0) & 0x0F) * 15;
            alphaValues[3] = ((input[offset + 1] >> 4) & 0x0F) * 15;
            alphaValues[4] = ((input[offset + 2] >> 0) & 0x0F) * 15;
            alphaValues[5] = ((input[offset + 2] >> 4) & 0x0F) * 15;
            alphaValues[6] = ((input[offset + 3] >> 0) & 0x0F) * 15;
            alphaValues[7] = ((input[offset + 3] >> 4) & 0x0F) * 15;
            alphaValues[8] = ((input[offset + 4] >> 0) & 0x0F) * 15;
            alphaValues[9] = ((input[offset + 4] >> 4) & 0x0F) * 15;
            alphaValues[10] = ((input[offset + 5] >> 0) & 0x0F) * 15;
            alphaValues[11] = ((input[offset + 5] >> 4) & 0x0F) * 15;
            alphaValues[12] = ((input[offset + 6] >> 0) & 0x0F) * 15;
            alphaValues[13] = ((input[offset + 6] >> 4) & 0x0F) * 15;
            alphaValues[14] = ((input[offset + 7] >> 0) & 0x0F) * 15;
            alphaValues[15] = ((input[offset + 7] >> 4) & 0x0F) * 15;


            //Color
            var c0_565 = (input[offset + 8]) | (input[offset + 9] << 8);
            var c1_565 = (input[offset + 10]) | (input[offset + 11] << 8);

            colorPalette[0] = rgb565_8888(c0_565);
            colorPalette[1] = rgb565_8888(c1_565);

            if(c0_565 <= c1_565)
            {
                colorPalette[2] = [
                    Math.floor((1.0 / 2.0) * colorPalette[0][0] + (1.0 / 2.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][1] + (1.0 / 2.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 2.0) * colorPalette[0][2] + (1.0 / 2.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [0, 0, 0, 0];
            }
            else
            {
                colorPalette[2] = [
                    Math.floor((2.0 / 3.0) * colorPalette[0][0] + (1.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][1] + (1.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((2.0 / 3.0) * colorPalette[0][2] + (1.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];

                colorPalette[3] = [
                    Math.floor((1.0 / 3.0) * colorPalette[0][0] + (2.0 / 3.0) * colorPalette[1][0] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][1] + (2.0 / 3.0) * colorPalette[1][1] + 0.5),
                    Math.floor((1.0 / 3.0) * colorPalette[0][2] + (2.0 / 3.0) * colorPalette[1][2] + 0.5),
                    255
                ];
            }

            colorIndices[0] = ((input[offset + 12] >> 0) & 0x03);
            colorIndices[1] = ((input[offset + 12] >> 2) & 0x03);
            colorIndices[2] = ((input[offset + 12] >> 4) & 0x03);
            colorIndices[3] = ((input[offset + 12] >> 6) & 0x03);

            colorIndices[4] = ((input[offset + 13] >> 0) & 0x03);
            colorIndices[5] = ((input[offset + 13] >> 2) & 0x03);
            colorIndices[6] = ((input[offset + 13] >> 4) & 0x03);
            colorIndices[7] = ((input[offset + 13] >> 6) & 0x03);

            colorIndices[8] = ((input[offset + 14] >> 0) & 0x03);
            colorIndices[9] = ((input[offset + 14] >> 2) & 0x03);
            colorIndices[10] = ((input[offset + 14] >> 4) & 0x03);
            colorIndices[11] = ((input[offset + 14] >> 6) & 0x03);

            colorIndices[12] = ((input[offset + 15] >> 0) & 0x03);
            colorIndices[13] = ((input[offset + 15] >> 2) & 0x03);
            colorIndices[14] = ((input[offset + 15] >> 4) & 0x03);
            colorIndices[15] = ((input[offset + 15] >> 6) & 0x03);

            var c;
            var _x, _y;
            var _of;
            var a;

            for(var i = 0; i < colorIndices.length; ++i)
            {
                c = colorPalette[colorIndices[i]];
                a = alphaValues[i];
                _x = i % 4;
                _y = Math.floor(i / 4);

                _of = ((y + _y) * rgbaImageWidth + x + _x) * 4;
                output[_of + 0] = c[0];
                output[_of + 1] = c[1];
                output[_of + 2] = c[2];
                output[_of + 3] = a;
            }
        };


        var s3tcExtension = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_s3tc');
        if(format === s3tcExtension.COMPRESSED_RGB_S3TC_DXT1_EXT)
        {
            for(var y = 0; y < blockCountHeight; ++y)
            {
                for(var x = 0; x < blockCountWidth; ++x)
                {
                    var p = (y * blockCountWidth + x) * blockBytes;

                    decodeBC1(input, p, output, x*4, y*4);
                }
            }
        }
        else if(format === s3tcExtension.COMPRESSED_RGBA_S3TC_DXT3_EXT)
        {
            for(var y = 0; y < blockCountHeight; ++y)
            {
                for(var x = 0; x < blockCountWidth; ++x)
                {
                    var p = (y * blockCountWidth + x) * blockBytes;

                    decodeBC2(input, p, output, x * 4, y * 4);
                }
            }
        }
        else if(format === s3tcExtension.COMPRESSED_RGBA_S3TC_DXT5_EXT)
        {
            for(var y = 0; y < blockCountHeight; ++y)
            {
                for(var x = 0; x < blockCountWidth; ++x)
                {
                    var p = (y * blockCountWidth + x) * blockBytes;

                    decodeBC3(input, p, output, x * 4, y * 4);
                }
            }
        }
        else
        {
            console.error('Cannot support this dds format. Engine only support bc1, bc3 and rgba uncompressed format.')
        }
    },

    flipY: function(input, blockBytes, blockCountWidth, blockCountHeight, format)
    {
        var s3tcExtension = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_s3tc');

        if(format === s3tcExtension.COMPRESSED_RGB_S3TC_DXT1_EXT)
        {
            var output = new Uint8Array(blockCountWidth * blockCountHeight * blockBytes);
            for(var y = 0; y < blockCountHeight; ++y)
            {
                for(var x = 0; x < blockCountWidth; ++x)
                {
                    var p = (y * blockCountWidth + x) * blockBytes;
                    var pd = ((blockCountHeight - y - 1) * blockCountWidth + x) * blockBytes;

                    output[pd + 0] = input[p + 0];
                    output[pd + 1] = input[p + 1];
                    output[pd + 2] = input[p + 2];
                    output[pd + 3] = input[p + 3];

                    output[pd + 4] = input[p + 7];
                    output[pd + 5] = input[p + 6];
                    output[pd + 6] = input[p + 5];
                    output[pd + 7] = input[p + 4];
                }
            }
            //<
        }
        else if(format === s3tcExtension.COMPRESSED_RGBA_S3TC_DXT5_EXT)
        {
            var output = new Uint8Array(blockCountWidth * blockCountHeight * blockBytes);
            for(var y = 0; y < blockCountHeight; ++y)
            {
                for(var x = 0; x < blockCountWidth; ++x)
                {
                    var p = (y * blockCountWidth + x) * blockBytes;
                    var pd = ((blockCountHeight - y - 1) * blockCountWidth + x) * blockBytes;

                    output[pd + 0] = input[p + 0];//Alpha
                    output[pd + 1] = input[p + 1];                

                    var line0 = input[p + 2] | (input[p + 3] << 8) | (input[p + 4] << 16);
                    var invLine0 = ((line0 << 12) & 0x00fff000) | (line0 >>> 12);

                    var line1 = input[p + 5] | (input[p + 6] << 8) | (input[p + 7] << 16);
                    var invLine1 = ((line1 << 12) & 0x00fff000) | (line1 >>> 12);

                    output[pd + 2] = (invLine1 >>> 0) & 0x000000ff; //Alpha indices
                    output[pd + 3] = (invLine1 >>> 8) & 0x000000ff;
                    output[pd + 4] = (invLine1 >>> 16) & 0x000000ff;
                    output[pd + 5] = (invLine0 >>> 0) & 0x000000ff;
                    output[pd + 6] = (invLine0 >>> 8) & 0x000000ff;
                    output[pd + 7] = (invLine0 >>> 16) & 0x000000ff;

                    output[pd +  8] = input[p +  8];//Color
                    output[pd +  9] = input[p +  9];
                    output[pd + 10] = input[p + 10];
                    output[pd + 11] = input[p + 11];

                    output[pd + 12] = input[p + 15];//Color index
                    output[pd + 13] = input[p + 14];
                    output[pd + 14] = input[p + 13];
                    output[pd + 15] = input[p + 12];
                }
            }
        }
        else
        {
            output = input;
        }

        return output;
    }
};