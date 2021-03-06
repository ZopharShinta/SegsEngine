/*************************************************************************/
/*  image_loader_svg.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "core/plugin_interfaces/PluginDeclarations.h"

using UIString = class QString;

extern "C" {
struct NSVGimage;
struct NSVGrasterizer;
}
/**
    @author Daniel Ramirez <djrmuv@gmail.com>
*/

class SVGRasterizer {

    NSVGrasterizer *rasterizer;

public:
    void rasterize(NSVGimage *p_image, float p_tx, float p_ty, float p_scale, unsigned char *p_dst, int p_w, int p_h, int p_stride);

    SVGRasterizer();
    ~SVGRasterizer();
};

class ImageLoaderSVG : public QObject, public ImageFormatLoader {
    Q_PLUGIN_METADATA(IID "org.segs_engine.ImageLoaderSVG")
    Q_INTERFACES(ImageFormatLoader)
    Q_OBJECT

    static struct ReplaceColors {
        Vector<uint32_t> old_colors;
        Vector<uint32_t> new_colors;
    } replace_colors;
    static SVGRasterizer rasterizer;
    static void _convert_colors(NSVGimage *p_svg_image);
    static Error _create_image(ImageData &p_image, const PoolVector<uint8_t> *p_data, const LoadParams &params);

public:
    void set_convert_colors(Vector<eastl::pair<Color, Color> > *p_replace_color = nullptr);
    static Error create_image_from_string(ImageData &p_image, const char *p_svg_str, float p_scale, bool upsample, bool convert_colors = false);

    Error load_image(ImageData & p_image, FileAccess *f, LoadParams params) override;
    void get_recognized_extensions(Vector<String> &p_extensions) const override;
    void set_loader_option(int option_id,void *option_var) override {
        if(option_id==0)
            set_convert_colors((Vector<eastl::pair<Color,Color>> *)option_var);
    }

    ImageLoaderSVG();
};
