#pragma once

#include "core/vector.h"

class ImageLoader;
class ImageFormatSaver;
class FileAccess;
class Image;
template<class T>
class Ref;
//TODO: SEGS- convert ImageSaver to singelton, so we can have a single initialization point at which to register plugin resolver
class GODOT_EXPORT ImageSaver {
public:
    static void register_plugin_resolver();
    static Error save_image(se_string_view p_file, const Ref<Image> &p_image, FileAccess *p_custom = nullptr, float p_quality = 1.0);
    static Error save_image(se_string_view ext, const Ref<Image> & p_image, PODVector<uint8_t> &tgt, float p_quality = 1.0);

    static void get_recognized_extensions(PODVector<se_string> &p_extensions);
    static ImageFormatSaver *recognize(se_string_view p_extension);

    static void add_image_format_saver(ImageFormatSaver *p_loader);
    static void remove_image_format_saver(ImageFormatSaver *p_loader);

    static const PODVector<ImageFormatSaver *> &get_image_format_savers();

    static void cleanup();
};
