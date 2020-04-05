/*************************************************************************/
/*  register_types.cpp                                                   */
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

#include "register_types.h"

#include "core/engine.h"
#include "core/class_db.h"
#include "core/resource/resource_manager.h"

#include "csharp_script.h"
#include "signal_awaiter_utils.h"
CSharpLanguage *script_language_cs = nullptr;
Ref<ResourceFormatLoaderCSharpScript> resource_loader_cs;
Ref<ResourceFormatSaverCSharpScript> resource_saver_cs;

_GodotSharp *_godotsharp = nullptr;
void register_mono_types() {
    ClassDB::register_class<CSharpScript>();
    ClassDB::register_class<_GodotSharp>();
    MonoGCHandle::initialize_class();
    SignalAwaiterHandle::initialize_class();

    _godotsharp = memnew(_GodotSharp);
    Engine::get_singleton()->add_singleton(Engine::Singleton("GodotSharp", _GodotSharp::get_singleton()));

    script_language_cs = memnew(CSharpLanguage);
    script_language_cs->set_language_index(ScriptServer::get_language_count());
    ScriptServer::register_language(script_language_cs);

    resource_loader_cs= make_ref_counted<ResourceFormatLoaderCSharpScript>();
    gResourceManager().add_resource_format_loader(resource_loader_cs);

    resource_saver_cs=make_ref_counted<ResourceFormatSaverCSharpScript>();
    gResourceManager().add_resource_format_saver(resource_saver_cs);
#if 0
#endif
}

void unregister_mono_types() {
    ScriptServer::unregister_language(script_language_cs);

    if (script_language_cs)
        memdelete(script_language_cs);

    gResourceManager().remove_resource_format_loader(resource_loader_cs);
    resource_loader_cs.unref();

    gResourceManager().remove_resource_format_saver(resource_saver_cs);
    resource_saver_cs.unref();

    if (_godotsharp)
        memdelete(_godotsharp);
}
