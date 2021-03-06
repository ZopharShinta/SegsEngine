/*************************************************************************/
/*  string_builder.h                                                     */
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

#include "core/string.h"

#include "core/vector.h"
#include <cassert>

class GODOT_EXPORT StringBuilder {

    uint32_t string_length;

    Vector<String> strings;
    Vector<StringView> c_strings;

    // -1 means it's a Godot String
    // a natural number means C string.
    Vector<int32_t> appended_strings;

    int current_indent_level=0;
    void add_indent() {
        static const char *spaces = "                                                                ";
        assert(current_indent_level<16);
        int32_t len = 4*current_indent_level;

        c_strings.push_back(StringView(spaces,current_indent_level*4));
        appended_strings.push_back(len);

        string_length += len;
    }
public:
    StringBuilder &append(StringView p_string);
    StringBuilder &append(const char *p_cstring);

    _FORCE_INLINE_ StringBuilder &operator+(const String &p_string) {
        return append(p_string);
    }

    _FORCE_INLINE_ StringBuilder &operator+(const char *p_cstring) {
        return append(p_cstring);
    }

    _FORCE_INLINE_ void operator+=(const String &p_string) {
        append(p_string);
    }

    _FORCE_INLINE_ void operator+=(const char *p_cstring) {
        append(p_cstring);
    }

    _FORCE_INLINE_ int num_strings_appended() const {
        return appended_strings.size();
    }

    _FORCE_INLINE_ uint32_t get_string_length() const {
        return string_length;
    }
    StringBuilder &append_indented(const String &p_string) {
        add_indent();
        return append(p_string);
    }
    StringBuilder &append_indented(const char *p_string) {
        add_indent();
        return append(p_string);
    }
    StringBuilder& append_indented_multiline(StringView p_string) {
        Vector<StringView> lines;
        String::split_ref(lines,p_string,'\n');
        for(StringView line : lines) {
            add_indent();
            append(line);
            append("\n");
        }
        return *this;
    }
    void indent(int level=1) { current_indent_level += level; }
    void dedent(int level=1) { current_indent_level -= level; if(current_indent_level<0) current_indent_level=0; }

    String as_string() const;

    _FORCE_INLINE_ operator String() const {
        return as_string();
    }

    StringBuilder() {
        string_length = 0;
    }
};
