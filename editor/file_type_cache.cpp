/*************************************************************************/
/*  file_type_cache.cpp                                                  */
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

#include "file_type_cache.h"

#include "core/os/file_access.h"
#include "core/project_settings.h"
#include "core/class_db.h"
#include "core/property_info.h"

IMPL_GDCLASS(FileTypeCache)

FileTypeCache *FileTypeCache::singleton = nullptr;

bool FileTypeCache::has_file(StringView p_path) const {

    GLOBAL_LOCK_FUNCTION
    return file_type_map.contains_as(p_path);
}

String FileTypeCache::get_file_type(StringView p_path) const {

    GLOBAL_LOCK_FUNCTION
    auto iter = file_type_map.find_as(p_path);
    ERR_FAIL_COND_V(iter==file_type_map.end(), String());
    return iter->second;
}
void FileTypeCache::set_file_type(StringView p_path, StringView p_type) {

    GLOBAL_LOCK_FUNCTION
    file_type_map[String(p_path)] = p_type;
}

void FileTypeCache::load() {

    GLOBAL_LOCK_FUNCTION
    String project = ProjectSettings::get_singleton()->get_resource_path();
    FileAccess *f = FileAccess::open(project + "/file_type_cache.cch", FileAccess::READ);

    if (!f) {

        WARN_PRINT("Can't open file_type_cache.cch.");
        return;
    }

    file_type_map.clear();
    while (!f->eof_reached()) {

        String path = f->get_line();
        if (f->eof_reached())
            break;
        String type = f->get_line();
        set_file_type(path, type);
    }

    memdelete(f);
}

void FileTypeCache::save() {

    GLOBAL_LOCK_FUNCTION
    String project = ProjectSettings::get_singleton()->get_resource_path();
    FileAccess *f = FileAccess::open(project + "/file_type_cache.cch", FileAccess::WRITE);

    ERR_FAIL_COND_MSG(!f, "Can't open file_type_cache.cch for writing, not saving file type cache!"); 

    for(const auto &v : file_type_map) {
        f->store_line(v.first);
        f->store_line(v.second);

    }

    memdelete(f);
}

FileTypeCache::FileTypeCache() {

    ERR_FAIL_COND_MSG(singleton, "FileTypeCache singleton already exist."); 
    singleton = this;
}
