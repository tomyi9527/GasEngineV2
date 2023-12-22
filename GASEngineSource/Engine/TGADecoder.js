GASEngine.TGADecoder = function()
{
};

GASEngine.TGADecoder.TGA_TYPE_NO_DATA = 0;
GASEngine.TGADecoder.TGA_TYPE_INDEXED = 1;
GASEngine.TGADecoder.TGA_TYPE_RGB = 2;
GASEngine.TGADecoder.TGA_TYPE_GREY = 3;
GASEngine.TGADecoder.TGA_TYPE_RLE_INDEXED = 9;
GASEngine.TGADecoder.TGA_TYPE_RLE_RGB = 10;
GASEngine.TGADecoder.TGA_TYPE_RLE_GREY = 11;
GASEngine.TGADecoder.TGA_ORIGIN_MASK = 0x30;
GASEngine.TGADecoder.TGA_ORIGIN_SHIFT = 0x04;
GASEngine.TGADecoder.TGA_ORIGIN_BL = 0x00;
GASEngine.TGADecoder.TGA_ORIGIN_BR = 0x01;
GASEngine.TGADecoder.TGA_ORIGIN_UL = 0x02;
GASEngine.TGADecoder.TGA_ORIGIN_UR = 0x03;

//https://github.com/vthibault/roBrowser/blob/master/src/Loaders/Targa.js
GASEngine.TGADecoder.prototype =
{
    constructor: GASEngine.TGADecoder,

    tgaCheckHeader: function(header)
    {
        switch(header.image_type)
        {
            case GASEngine.TGADecoder.TGA_TYPE_INDEXED:
            case GASEngine.TGADecoder.TGA_TYPE_RLE_INDEXED:
                if(header.colormap_length > 256 || header.colormap_size !== 24 || header.colormap_type !== 1)
                {
                    console.error('TGA Decoder: Invalid type colormap data for indexed type');
                }
                break;
            case GASEngine.TGADecoder.TGA_TYPE_RGB:
            case GASEngine.TGADecoder.TGA_TYPE_GREY:
            case GASEngine.TGADecoder.TGA_TYPE_RLE_RGB:
            case GASEngine.TGADecoder.TGA_TYPE_RLE_GREY:
                if(header.colormap_type)
                {
                    console.error('TGA Decoder: Invalid type colormap data for colormap type');
                }
                break;
            case GASEngine.TGADecoder.TGA_TYPE_NO_DATA:
                console.error('TGA Decoder: No data');
            default:
                console.error('TGA Decoder: Invalid type " ' + header.image_type + '"');
        }

        if(header.width <= 0 || header.height <= 0)
        {
            console.error('TGA Decoder: Invalid image size');
        }

        if(header.pixel_size !== 8 && header.pixel_size !== 16 && header.pixel_size !== 24 && header.pixel_size !== 32)
        {
            console.error('TGA Decoder: Invalid pixel size "' + header.pixel_size + '"');
        }
    },

    tgaParse: function(use_rle, use_pal, header, offset, data)
    {
        var pixel_size = header.pixel_size >> 3;
        var pixel_total = header.width * header.height * pixel_size;

        // Read palettes
        var palettes;
        if(use_pal)
        {
            palettes = data.subarray(offset, offset += header.colormap_length * (header.colormap_size >> 3));
        }

        // Read RLE
        var pixel_data;
        if(use_rle)
        {
            pixel_data = new Uint8Array(pixel_total);

            var c, count, i;
            var shift = 0;
            var pixels = new Uint8Array(pixel_size);

            while(shift < pixel_total)
            {
                c = data[offset++];
                count = (c & 0x7f) + 1;
                // RLE pixels.
                if(c & 0x80)
                {
                    // Bind pixel tmp array
                    for(i = 0; i < pixel_size; ++i)
                    {
                        pixels[i] = data[offset++];
                    }

                    // Copy pixel array
                    for(i = 0; i < count; ++i)
                    {
                        pixel_data.set(pixels, shift + i * pixel_size);
                    }

                    shift += pixel_size * count;
                }
                else
                {
                    // Raw pixels.
                    count *= pixel_size;
                    for(i = 0; i < count; ++i)
                    {
                        pixel_data[shift + i] = data[offset++];
                    }
                    shift += count;
                }
            }
        }
        else
        {
            // RAW Pixels
            var pixelDataLength = offset + (use_pal ? header.width * header.height : pixel_total);
            pixel_data = data.subarray(offset, pixelDataLength);
        }

        return { 'pixel_data': pixel_data, 'palettes': palettes };
    },

    getTgaRGBA: function(header, use_grey, data, width, height, image, palette)
    {
        function tgaGetImageData8bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image, palettes)
        {
            var colormap = palettes;
            var color, i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i++)
                {
                    color = image[i];
                    imageData[(x + width * y) * 4 + 3] = 255;
                    imageData[(x + width * y) * 4 + 2] = colormap[(color * 3) + 0];
                    imageData[(x + width * y) * 4 + 1] = colormap[(color * 3) + 1];
                    imageData[(x + width * y) * 4 + 0] = colormap[(color * 3) + 2];
                }
            }
            return imageData;
        }

        function tgaGetImageData16bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image)
        {
            var color, i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i += 2)
                {
                    color = image[i + 0] + (image[i + 1] << 8); // Inversed ?
                    imageData[(x + width * y) * 4 + 0] = (color & 0x7C00) >> 7;
                    imageData[(x + width * y) * 4 + 1] = (color & 0x03E0) >> 2;
                    imageData[(x + width * y) * 4 + 2] = (color & 0x001F) >> 3;
                    imageData[(x + width * y) * 4 + 3] = (color & 0x8000) ? 0 : 255;
                }
            }
            return imageData;
        }

        function tgaGetImageData24bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image)
        {
            var i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i += 3)
                {
                    imageData[(x + width * y) * 4 + 3] = 255;
                    imageData[(x + width * y) * 4 + 2] = image[i + 0];
                    imageData[(x + width * y) * 4 + 1] = image[i + 1];
                    imageData[(x + width * y) * 4 + 0] = image[i + 2];
                }
            }
            return imageData;
        }

        function tgaGetImageData32bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image)
        {
            var i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i += 4)
                {
                    imageData[(x + width * y) * 4 + 2] = image[i + 0];
                    imageData[(x + width * y) * 4 + 1] = image[i + 1];
                    imageData[(x + width * y) * 4 + 0] = image[i + 2];
                    imageData[(x + width * y) * 4 + 3] = image[i + 3];
                }
            }
            return imageData;
        }

        function tgaGetImageDataGrey8bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image)
        {
            var color, i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i++)
                {
                    color = image[i];
                    imageData[(x + width * y) * 4 + 0] = color;
                    imageData[(x + width * y) * 4 + 1] = color;
                    imageData[(x + width * y) * 4 + 2] = color;
                    imageData[(x + width * y) * 4 + 3] = 255;
                }
            }
            return imageData;
        }

        function tgaGetImageDataGrey16bits(imageData, width, y_start, y_step, y_end, x_start, x_step, x_end, image)
        {
            var i = 0, x, y;
            for(y = y_start; y !== y_end; y += y_step)
            {
                for(x = x_start; x !== x_end; x += x_step, i += 2)
                {
                    imageData[(x + width * y) * 4 + 0] = image[i + 0];
                    imageData[(x + width * y) * 4 + 1] = image[i + 0];
                    imageData[(x + width * y) * 4 + 2] = image[i + 0];
                    imageData[(x + width * y) * 4 + 3] = image[i + 1];
                }
            }
            return imageData;
        }

        //<
        var x_start, y_start, x_step, y_step, x_end, y_end;
        switch((header.flags & GASEngine.TGADecoder.TGA_ORIGIN_MASK) >> GASEngine.TGADecoder.TGA_ORIGIN_SHIFT)
        {
            default:
            case GASEngine.TGADecoder.TGA_ORIGIN_UL:
                x_start = 0; x_step = 1; x_end = width; y_start = 0; y_step = 1; y_end = height;
                break;
            case GASEngine.TGADecoder.TGA_ORIGIN_BL:
                x_start = 0; x_step = 1; x_end = width; y_start = height - 1; y_step = -1; y_end = -1;
                break;
            case GASEngine.TGADecoder.TGA_ORIGIN_UR:
                x_start = width - 1; x_step = -1; x_end = -1; y_start = 0; y_step = 1; y_end = height;
                break;
            case GASEngine.TGADecoder.TGA_ORIGIN_BR:
                x_start = width - 1; x_step = -1; x_end = -1; y_start = height - 1; y_step = -1; y_end = -1;
                break;
        }

        if(use_grey)
        {
            switch(header.pixel_size)
            {
                case 8:
                    tgaGetImageDataGrey8bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image);
                    break;
                case 16:
                    tgaGetImageDataGrey16bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image);
                    break;
                default:
                    console.error('TGA Decoder: not support this format');
                    break;
            }
        }
        else
        {
            switch(header.pixel_size)
            {
                case 8:
                    tgaGetImageData8bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image, palette);
                    break;
                case 16:
                    tgaGetImageData16bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image);
                    break;
                case 24:
                    tgaGetImageData24bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image);
                    break;
                case 32:
                    tgaGetImageData32bits(data, header.width, y_start, y_step, y_end, x_start, x_step, x_end, image);
                    break;
                default:
                    console.error('TGA Decoder: not support this format');
                    break;
            }
        }

        // Load image data according to specific method
        // var func = 'tgaGetImageData' + (use_grey ? 'Grey' : '') + (header.pixel_size) + 'bits';
        // func(data, y_start, y_step, y_end, x_start, x_step, x_end, width, image, palette );
        return data;
    },

    decode: function(buffer)
    {
        //if(buffer.length < 19)
        //{
        //    console.error('TGA Decoder: Not enough data to contain header!');
        //}
        var content = new Uint8Array(buffer);
        var offset = 0;
        var header =
            {
                id_length: content[offset++],
                colormap_type: content[offset++],
                image_type: content[offset++],
                colormap_index: content[offset++] | content[offset++] << 8,
                colormap_length: content[offset++] | content[offset++] << 8,
                colormap_size: content[offset++],

                origin:
                [
                    content[offset++] | content[offset++] << 8,
                    content[offset++] | content[offset++] << 8
                ],
                width: content[offset++] | content[offset++] << 8,
                height: content[offset++] | content[offset++] << 8,
                pixel_size: content[offset++],
                flags: content[offset++]
            };


        this.tgaCheckHeader(header);
        if(header.id_length + offset > buffer.length)
        {
            console.error('TGA Decoder: No data');
        }

        //trivial data
        offset += header.id_length;
        var use_rle = false, use_pal = false, use_grey = false;

        switch(header.image_type)
        {

            case GASEngine.TGADecoder.TGA_TYPE_RLE_INDEXED:
                use_rle = true; use_pal = true;
                break;
            case GASEngine.TGADecoder.TGA_TYPE_INDEXED:
                use_pal = true;
                break;
            case GASEngine.TGADecoder.TGA_TYPE_RLE_RGB:
                use_rle = true;
                break;
            case GASEngine.TGADecoder.TGA_TYPE_RGB:
                break;
            case GASEngine.TGADecoder.TGA_TYPE_RLE_GREY:
                use_rle = true; use_grey = true;
                break;
            case GASEngine.TGADecoder.TGA_TYPE_GREY:
                use_grey = true;
                break;
        }

        var canvas = document.createElement('canvas');
        canvas.width = header.width;
        canvas.height = header.height;

        var context = canvas.getContext('2d');
        var imageData = context.createImageData(header.width, header.height);

        var result = this.tgaParse(use_rle, use_pal, header, offset, content);
        var rgbaData = this.getTgaRGBA(header, use_grey, imageData.data, header.width, header.height, result.pixel_data, result.palettes);

        context.putImageData(imageData, 0, 0);

        return canvas;
    }
};
