//Version 2 PVR Texture Header
//typedef struct _PVRTexHeader
//{
//    uint32_t headerLength;
//    uint32_t height;
//    uint32_t width;
//    uint32_t numMipmaps;
//    uint32_t flags;
//    uint32_t dataLength;
//    uint32_t bpp;
//    uint32_t bitmaskRed;
//    uint32_t bitmaskGreen;
//    uint32_t bitmaskBlue;
//    uint32_t bitmaskAlpha;
//    uint32_t pvrTag;
//    uint32_t numSurfs;
//} PVRTexHeader;

//Version 3 PVR Texture Header
//typedef struct _PVRTexHeaderV3
//{
//    uint32_t    version;
//    uint32_t    flags;
//    uint64_t    pixelFormat;
//    uint32_t    colourSpace;
//    uint32_t    channelType;
//    uint32_t    height;
//    uint32_t    width;
//    uint32_t    depth;
//    uint32_t    numSurfaces;
//    uint32_t    numFaces;
//    uint32_t    numMipmaps;
//    uint32_t    metaDataSize;
//}

//PVRTC 2bpp RGB 0
//PVRTC 2bpp RGBA 1
//PVRTC 4bpp RGB 2
//PVRTC 4bpp RGBA 3
//PVRTC-II 2bpp 4
//PVRTC-II 4bpp 5
//ETC1 6
//DXT1 7
//DXT2 8
//DXT3 9
//DXT4 10
//DXT5 11
//BC1 7
//BC2 9
//BC3 11
//BC4 12
//BC5 13
//BC6 14
//BC7 15
//UYVY 16
//YUY2 17
//BW1bpp 18
//R9G9B9E5 Shared Exponent 19
//RGBG8888 20
//GRGB8888 21
//ETC2 RGB 22
//ETC2 RGBA 23
//ETC2 RGB A1 24
//EAC R11 Unsigned 25
//EAC R11 Signed 26
//EAC RG11 Unsigned 27
//EAC RG11 Signed 28

//If you are looking to parse a Version 3 Texture header, go grab the PowerVR SDK from:
//http://www.imgtec.com/powervr/insider/sdkdownloads/index.asp

GASEngine.PVRDecoder = function()
{
};

GASEngine.PVRDecoder.prototype =
{
    constructor: GASEngine.PVRDecoder,

    decode: function(buffer, loadMipmaps)
    {
        var headerLengthInt = 13;
        var header = new Uint32Array(buffer, 0, headerLengthInt);
        var pvrDatas =
        {
            'buffer': buffer,
            'header': header,
            'loadMipmaps': loadMipmaps
        };
        
        if(header[0] === 0x03525650)
        {
            //PVR Version 3
            return this._parseV3(pvrDatas);
        }
        else if(header[11] === 0x21525650)
        {
            //PVR Version 2
            return this._parseV2(pvrDatas);
        }
        else
        {
            console.error('GASEngine.PVRDecoder.decode: Unsupported PVR texture format.');
            return null;
        }
    },

    _getPixelInfo: function(pixelFormat)
    {
        var extPVRTC = null;
        if(pixelFormat >= 0 && pixelFormat <= 3)
        {
            extPVRTC = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_pvrtc');
            if(extPVRTC === null)
            {
                console.error("GASEngine.PVRDecoder._getPixelInfo: Do not support PVRTC webgl extension.");
                return null;
            }
        }

        var extETC1 = null;
        if(pixelFormat == 6)
        {
            extETC1 = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_etc1');

            if(extETC1 === null)
            {
                console.error("GASEngine.PVRDecoder._getPixelInfo: Do not support ETC1 webgl extension.");
                return null;
            }
        }

        var extS3TC = null;
        if(pixelFormat == 7 || pixelFormat == 9 || pixelFormat == 11)
        {
            extS3TC = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_s3tc');

            if(extS3TC === null)
            {
                console.error("GASEngine.PVRDecoder._getPixelInfo: Do not support S3TC webgl extension.");
                return null;
            }
        }

        var bpp, format;
        switch(pixelFormat)
        {
            case 0:
                bpp = 2;
                format = extPVRTC.COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
                break;
            case 1:
                bpp = 2;
                format = extPVRTC.COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
                break;
            case 2:
                bpp = 4;
                format = extPVRTC.COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
                break;
            case 3:
                bpp = 4;
                format = extPVRTC.COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
                break;
            case 6: 
                bpp = 4;
                format = extETC1.COMPRESSED_RGB_ETC1_WEBGL;
                break;
            case 7: //DXT1
                bpp = 4;
                format = extS3TC.COMPRESSED_RGB_S3TC_DXT1_EXT;
                break;
            case 9: //DXT3
                bpp = 8;
                format = extS3TC.COMPRESSED_RGBA_S3TC_DXT3_EXT;
                break;
            case 11: //DXT5
                bpp = 8;
                format = extS3TC.COMPRESSED_RGBA_S3TC_DXT5_EXT;
                break;
            default:
                console.error("GASEngine.PVRDecoder._getPixelInfo: Unsupported PVRTC pixel format:" + pixelFormat);
                return null;
        }

        return { 'bpp': bpp, 'format': format };
    },

    _parseV3: function(pvrDatas)
    {
        var header = pvrDatas.header;
        
        var metaLen     = header[12];
        var pixelFormat = header[2];
        var height      = header[6];
        var width       = header[7];
        var numSurfs    = header[9];
        var numFaces    = header[10];
        var numMipmaps  = header[11];

        var result = this._getPixelInfo(pixelFormat);
        if(result === null)
        {
            return null;
        }

        pvrDatas.dataPtr        = 52 + metaLen;
        pvrDatas.bpp            = result.bpp;
        pvrDatas.format         = result.format;
        pvrDatas.width          = width;
        pvrDatas.height         = height;
        pvrDatas.numSurfaces    = numFaces;
        pvrDatas.numMipmaps     = numMipmaps;
        pvrDatas.isCubemap      = (numFaces === 6);

        if(pixelFormat === 6)
        {
            return this._extractETC1(pvrDatas);
        }
        else if(pixelFormat === 7 || pixelFormat === 9 || pixelFormat === 11)
        {
            return this._extractDXTC(pvrDatas);
        }
        else if(pixelFormat >= 0 && pixelFormat <= 3)
        {
            return this._extractPVRTC(pvrDatas);
        }
    },

    _parseV2: function(pvrDatas)
    {
        var header = pvrDatas.header;
        var headerLength  =  header[0];
        height        =  header[1];
        width         =  header[2];
        numMipmaps    =  header[3];
        flags         =  header[4];
        dataLength    =  header[5];
        bpp           =  header[6];
        bitmaskRed    =  header[7];
        bitmaskGreen  =  header[8];
        bitmaskBlue   =  header[9];
        bitmaskAlpha  =  header[10];
        pvrTag        =  header[11];
        numSurfs      =  header[12];

        var TYPE_MASK = 0xff;
        var PVRTC_2 = 24;
        var PVRTC_4 = 25;
        var formatFlags = flags & TYPE_MASK;
        var bpp, format;
        var _hasAlpha = bitmaskAlpha > 0;

        var extPVRTC = GASEngine.WebGLDevice.Instance.getWebglExtensions('WEBGL_compressed_texture_pvrtc');
       
        if(extPVRTC === null)
        {
            console.error("GASEngine.PVRDecoder._parseV2: Do not support PVRTC webgl extension.");
            return null;
        }

        if(formatFlags === PVRTC_4)
        {
            format = _hasAlpha ? extPVRTC.COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : extPVRTC.COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            bpp = 4;
        }
        else if(formatFlags === PVRTC_2)
        {
            format = _hasAlpha ? extPVRTC.COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : extPVRTC.COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            bpp = 2;
        }
        else
        {
            console.error("GASEngine.PVRDecoder._parseV2: Unsupported PVRTC pixel format:" + pixelFormat);
            return null;
        }

        pvrDatas.dataPtr        = headerLength;
        pvrDatas.bpp            = bpp;
        pvrDatas.format         = format;
        pvrDatas.width          = width;
        pvrDatas.height         = height;
        pvrDatas.numSurfaces    = numSurfs;
        pvrDatas.numMipmaps     = numMipmaps + 1;
        pvrDatas.isCubemap      = (numSurfs === 6);

        return this._extractPVRTC(pvrDatas);
    },

    _extractPVRTC: function(pvrDatas)
    {
        //There isn't any max size limit via format itself[citation needed], but GPU's max texture size limit does apply to PVRTC textures. For this reason, 
        //on a mobile device, a single PVRTC texture may be capped to 4096x4096 resolution.
        //Since most PVRTC and PVRTC2 texture creation tools are targeting iPhones, iPads and iPod touches, additional limitations[4] set by Apple apply:
        //Height and width must be a power of 2.
        //Height and width must be at least 8.
        //Must be square (height==width)

        var pvr = 
        {
            'mipmaps': [],
            'width': pvrDatas.width,
            'height': pvrDatas.height,
            'format': pvrDatas.format,
            'mipmapCount': pvrDatas.numMipmaps,
            'isCubemap': pvrDatas.isCubemap
        };

        var buffer = pvrDatas.buffer;
        var dataOffset = pvrDatas.dataPtr;
        var bpp = pvrDatas.bpp;
        var numSurfs = pvrDatas.numSurfaces;

        var blockWidth = 0;
        var blockHeight = 0;
        if(bpp === 2)
        {
            blockWidth = 8;
            blockHeight = 4;
        }
        else
        {
            blockWidth = 4;
            blockHeight = 4;
        }

        var blockSize = (blockWidth * blockHeight) * bpp / 8;
        pvr.mipmaps.length = pvrDatas.numMipmaps * numSurfs;

        var dataSize = 0;
        var widthBlocks = 0;
        var heightBlocks = 0;
        var mipLevel = 0;

        while(mipLevel < pvrDatas.numMipmaps)
        {
            var sWidth = pvrDatas.width >> mipLevel;
            var sHeight = pvrDatas.height >> mipLevel;

            widthBlocks = sWidth / blockWidth;
            heightBlocks = sHeight / blockHeight;

            if(widthBlocks < 2)
            {
                widthBlocks = 2;
            }

            if(heightBlocks < 2)
            {
                heightBlocks = 2;
            }

            dataSize = widthBlocks * heightBlocks * blockSize;

            for(var surfIndex = 0; surfIndex < numSurfs; surfIndex++)
            {
                var byteArray = new Uint8Array(buffer, dataOffset, dataSize);
                var mipmap =
                {
                    'data': byteArray,
                    'width': sWidth,
                    'height': sHeight
                };

                pvr.mipmaps[surfIndex * pvrDatas.numMipmaps + mipLevel] = mipmap;
                dataOffset += dataSize;
            }

            mipLevel++;
        }

        return pvr;
    },

    _extractETC1: function(data)
    {
        var etc1 =
        {
            'mipmaps': [],
            'width': data.width,
            'height': data.height,
            'format': data.format,
            'mipmapCount': data.numMipmaps,
            'isCubemap': data.isCubemap
        };

        var buffer = data.buffer;
        var dataOffset = data.dataPtr;
        var bpp = data.bpp;
        var numSurfs = data.numSurfaces;

        var blockWidth = 4;
        var blockHeight = 4;
        
        var blockSize = 8; //64bit

        etc1.mipmaps.length = data.numMipmaps * numSurfs;

        var dataSize = 0;
       
        for(var face = 0; face < numSurfs; face++)
        {
            for(var mipLevel = 0; mipLevel < data.numMipmaps; mipLevel++)
            {
                var sWidth = data.width >> mipLevel;
                var sHeight = data.height >> mipLevel;

                var widthBlocks = Math.max(4, sWidth) / 4;
                var heightBlocks = Math.max(4, sHeight) / 4;

                dataSize = widthBlocks * heightBlocks * blockSize;

                var byteArray = new Uint8Array(buffer, dataOffset, dataSize);

                var mipmap =
                {
                    'data': byteArray,
                    'width': sWidth,
                    'height': sHeight
                };
                etc1.mipmaps[face * data.numMipmaps + mipLevel] = mipmap;

                dataOffset += dataSize;
            }
        }

        return etc1;
    },

    _extractDXTC: function(data)
    {
        var dxtc =
        {
            'mipmaps': [],
            'width': data.width,
            'height': data.height,
            'format': data.format,
            'mipmapCount': data.numMipmaps,
            'isCubemap': data.isCubemap
        };

        var buffer = data.buffer;
        var dataOffset = data.dataPtr;
        var numSurfs = data.numSurfaces;

        var blockWidth = 4;
        var blockHeight = 4;

        var blockSize = 2 * data.bpp;

        dxtc.mipmaps.length = data.numMipmaps * numSurfs;

        var dataSize = 0;

        for(var face = 0; face < numSurfs; face++)
        {
            for(var mipLevel = 0; mipLevel < data.numMipmaps; mipLevel++)
            {
                var sWidth = data.width >> mipLevel;
                var sHeight = data.height >> mipLevel;

                var widthBlocks = Math.max(4, sWidth) / 4;
                var heightBlocks = Math.max(4, sHeight) / 4;

                dataSize = widthBlocks * heightBlocks * blockSize;

                var byteArray = new Uint8Array(buffer, dataOffset, dataSize);

                var mipmap =
                {
                    'data': byteArray,
                    'width': sWidth,
                    'height': sHeight
                };
                dxtc.mipmaps[face * data.numMipmaps + mipLevel] = mipmap;

                dataOffset += dataSize;
            }
        }

        return dxtc;
    }
};