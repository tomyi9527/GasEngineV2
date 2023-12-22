#include "build_scene.h"
#include <cctype>
#include <filesystem>
#include <memory>
#include <string>
#include "httplib.h"
#include "FreeImage.h"
#include "url.h"

void UploadScreenShot(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                      const BasicConfig& config) {
    // output screen shot
    std::string rgba_pixels =
        glit::GetFrameBufferRGBAContent(context->width, context->height, context->fb);
    FIBITMAP* bitmap = FreeImage_Allocate(context->width, context->height, 24);
    auto bytes = (RGBTRIPLE*)FreeImage_GetBits(bitmap);
    for (int idx = 0; idx < context->width * context->height; ++idx) {
        bytes[idx].rgbtRed = rgba_pixels[idx * 4 + 0];    // b
        bytes[idx].rgbtGreen = rgba_pixels[idx * 4 + 1];  // g
        bytes[idx].rgbtBlue = rgba_pixels[idx * 4 + 2];   // r
    }
    if (context->width != config.window_width) {
        auto old_bitmap = bitmap;
        bitmap = FreeImage_Rescale(old_bitmap, config.window_width, config.window_height,
                                   FILTER_BILINEAR);
        FreeImage_Unload(old_bitmap);
    }
    FIMEMORY* dest_stream = FreeImage_OpenMemory();
    FreeImage_SaveToMemory(FIF_PNG, bitmap, dest_stream, 0);

    FreeImage_Unload(bitmap);

    BYTE* mem_buffer = nullptr;
    DWORD size_in_bytes = 0;
    FreeImage_AcquireMemory(dest_stream, &mem_buffer,
                            &size_in_bytes);  // mem_buffer 指向 dest_memory 内
    std::string content;
    content.assign(mem_buffer, mem_buffer + size_in_bytes);
    mem_buffer = nullptr;
    FreeImage_CloseMemory(dest_stream);

    std::string content_type = "image/png";
    Url url(config.output);
    std::string domain = url.protocol_ + "://" + url.host_;
    httplib::Client download(domain.c_str());
    auto res = download.Put(url.path_with_param_.c_str(), content, content_type.c_str());
    if (!res || res->status >= 300) LOG_ERROR("upload to %s failed", config.output.c_str());
}