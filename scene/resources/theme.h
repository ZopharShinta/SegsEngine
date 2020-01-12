/*************************************************************************/
/*  theme.h                                                              */
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

#include "core/io/resource_loader.h"
#include "core/resource.h"
#include "scene/resources/shader.h"
#include "scene/resources/style_box.h"
#include "scene/resources/texture.h"
#include "core/hash_map.h"

class Font;

class Theme : public Resource {

    GDCLASS(Theme,Resource)

    RES_BASE_EXTENSION("theme")

    void _emit_theme_changed();

    DefHashMap<StringName, DefHashMap<StringName, Ref<Texture> > > icon_map;
    DefHashMap<StringName, DefHashMap<StringName, Ref<StyleBox> > > style_map;
    DefHashMap<StringName, DefHashMap<StringName, Ref<Font> > > font_map;
    DefHashMap<StringName, DefHashMap<StringName, Ref<Shader> > > shader_map;
    DefHashMap<StringName, DefHashMap<StringName, Color> > color_map;
    DefHashMap<StringName, DefHashMap<StringName, int> > constant_map;

protected:
    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(ListPOD<PropertyInfo> *p_list) const;

    static Ref<Theme> project_default_theme;
    static Ref<Theme> default_theme;
    static Ref<Texture> default_icon;
    static Ref<StyleBox> default_style;
    static Ref<Font> default_font;

    Ref<Font> default_theme_font;

    PoolVector<se_string> _get_icon_list(const se_string &p_type) const;
    PoolVector<se_string> _get_stylebox_list(const se_string &p_type) const;
    PoolVector<se_string> _get_stylebox_types() const;
    PoolVector<se_string> _get_font_list(const se_string &p_type) const;
    PoolVector<se_string> _get_color_list(const se_string &p_type) const;
    PoolVector<se_string> _get_constant_list(const se_string &p_type) const;
    PoolVector<se_string> _get_type_list(se_string_view p_type) const;

    static void _bind_methods();

public:
    struct ThemeConstant {
        const char *name;
        const char *type;
        int value;
    };
    struct ThemeIcon {
        const char *name;
        const char *icon_name;
        const char *icon_type;
    };
    struct ThemeColor {
        const char *name;
        const char *type;
        Color color;
    };
    static Ref<Theme> get_default();
    static void set_default(const Ref<Theme> &p_default);

    static Ref<Theme> get_project_default();
    static void set_project_default(const Ref<Theme> &p_project_default);

    static void set_default_icon(const Ref<Texture> &p_icon);
    static void set_default_style(const Ref<StyleBox> &p_style);
    static void set_default_font(const Ref<Font> &p_font);

    void set_default_theme_font(const Ref<Font> &p_default_font);
    Ref<Font> get_default_theme_font() const;

    void set_icons(Span<const ThemeIcon> icon_defs, const StringName &p_type);
    void set_icon(const StringName &p_name, const StringName &p_type, const Ref<Texture> &p_icon);
    Ref<Texture> get_icon(const StringName &p_name, const StringName &p_type) const;
    bool has_icon(const StringName &p_name, const StringName &p_type) const;
    void clear_icon(const StringName &p_name, const StringName &p_type);
    void get_icon_list(const StringName& p_type, PODVector<StringName> *p_list) const;

    void set_shader(const StringName &p_name, const StringName &p_type, const Ref<Shader> &p_shader);
    Ref<Shader> get_shader(const StringName &p_name, const StringName &p_type) const;
    bool has_shader(const StringName &p_name, const StringName &p_type) const;
    void clear_shader(const StringName &p_name, const StringName &p_type);
    void get_shader_list(const StringName &p_type, PODVector<StringName> *p_list) const;

    void set_stylebox(const StringName &p_name, const StringName &p_type, const Ref<StyleBox> &p_style);
    Ref<StyleBox> get_stylebox(const StringName &p_name, const StringName &p_type) const;
    bool has_stylebox(const StringName &p_name, const StringName &p_type) const;
    void clear_stylebox(const StringName &p_name, const StringName &p_type);
    void get_stylebox_list(const StringName& p_type, PODVector<StringName> *p_list) const;
    void get_stylebox_types(PODVector<StringName> *p_list) const;

    void set_font(const StringName &p_name, const StringName &p_type, const Ref<Font> &p_font);
    Ref<Font> get_font(const StringName &p_name, const StringName &p_type) const;
    bool has_font(const StringName &p_name, const StringName &p_type) const;
    void clear_font(const StringName &p_name, const StringName &p_type);
    void get_font_list(const StringName& p_type, PODVector<StringName> *p_list) const;

    void set_colors(Span<const ThemeColor> colors);
    void set_color(const StringName &p_name, const StringName &p_type, const Color &p_color);
    Color get_color(const StringName &p_name, const StringName &p_type) const;
    bool has_color(const StringName &p_name, const StringName &p_type) const;
    void clear_color(const StringName &p_name, const StringName &p_type);
    void get_color_list(const StringName& p_type, PODVector<StringName> *p_list) const;


    void set_constants(Span<const ThemeConstant> vals);
    void set_constant(const StringName &p_name, const StringName &p_type, int p_constant);
    int get_constant(const StringName &p_name, const StringName &p_type) const;
    bool has_constant(const StringName &p_name, const StringName &p_type) const;
    void clear_constant(const StringName &p_name, const StringName &p_type);
    void get_constant_list(const StringName& p_type, PODVector<StringName> *p_list) const;

    void get_type_list(PODVector<StringName> *p_list) const;

    void copy_default_theme();
    void copy_theme(const Ref<Theme> &p_other);
    void clear();

    Theme();
    ~Theme() override;
};
