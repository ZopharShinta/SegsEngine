/*************************************************************************/
/*  navigation_mesh_instance.cpp                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "navigation_mesh_instance.h"
#include "mesh_instance_3d.h"
#include "navigation_3d.h"

#include "core/class_db.h"
#include "core/method_bind.h"
#include "core/method_bind_interface.h"
#include "core/object_tooling.h"
#include "core/os/thread.h"
#include "core/translation_helpers.h"
#include "scene/main/scene_tree.h"
#include "servers/navigation_server.h"

IMPL_GDCLASS(NavigationMeshInstance)

void NavigationMeshInstance::set_enabled(bool p_enabled) {

    if (enabled == p_enabled)
        return;
    enabled = p_enabled;

    if (!is_inside_tree())
        return;

    if (!enabled) {

        NavigationServer::get_singleton()->region_set_map(region, RID());
    } else {

        if (navigation) {

            NavigationServer::get_singleton()->region_set_map(region, navigation->get_rid());
        }
    }

    if (debug_view) {
        MeshInstance *dm = object_cast<MeshInstance>(debug_view);
        if (is_enabled()) {
            dm->set_material_override(get_tree()->get_debug_navigation_material());
        } else {
            dm->set_material_override(get_tree()->get_debug_navigation_disabled_material());
        }
    }

    update_gizmo();
}

bool NavigationMeshInstance::is_enabled() const {

    return enabled;
}

/////////////////////////////

void NavigationMeshInstance::_notification(int p_what) {

    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {

            Node3D *c = this;
            while (c) {

                navigation = object_cast<Navigation3D>(c);
                if (navigation) {

                    if (enabled) {

                        NavigationServer::get_singleton()->region_set_map(region, navigation->get_rid());
                    }
                    break;
                }

                c = c->get_parent_spatial();
            }

            if (navmesh && get_tree()->is_debugging_navigation_hint()) {

                MeshInstance *dm = memnew(MeshInstance);
                dm->set_mesh(navmesh->get_debug_mesh());
                if (is_enabled()) {
                    dm->set_material_override(get_tree()->get_debug_navigation_material());
                } else {
                    dm->set_material_override(get_tree()->get_debug_navigation_disabled_material());
                }
                add_child(dm);
                debug_view = dm;
            }

        } break;
        case NOTIFICATION_TRANSFORM_CHANGED: {

            NavigationServer::get_singleton()->region_set_transform(region, get_global_transform());

        } break;
        case NOTIFICATION_EXIT_TREE: {

            if (navigation) {

                NavigationServer::get_singleton()->region_set_map(region, RID());
            }

            if (debug_view) {
                debug_view->queue_delete();
                debug_view = nullptr;
            }
            navigation = nullptr;
        } break;
    }
}

void NavigationMeshInstance::set_navigation_mesh(const Ref<NavigationMesh> &p_navmesh) {

    if (p_navmesh == navmesh)
        return;

    if (navmesh) {
        Object_remove_change_receptor(navmesh.get(),this);
    }

    navmesh = p_navmesh;

    if (navmesh) {
        Object_remove_change_receptor(navmesh.get(),this);
    }

    NavigationServer::get_singleton()->region_set_navmesh(region, p_navmesh);

    if (debug_view && navmesh) {
        object_cast<MeshInstance>(debug_view)->set_mesh(navmesh->get_debug_mesh());
    }

    emit_signal("navigation_mesh_changed");

    update_gizmo();
    update_configuration_warning();
}

Ref<NavigationMesh> NavigationMeshInstance::get_navigation_mesh() const {

    return navmesh;
}

struct BakeThreadsArgs {
    NavigationMeshInstance *nav_mesh_instance;
};

void _bake_navigation_mesh(void *p_user_data) {
    BakeThreadsArgs *args = static_cast<BakeThreadsArgs *>(p_user_data);

    if (args->nav_mesh_instance->get_navigation_mesh()) {
        Ref<NavigationMesh> nav_mesh((NavigationMesh *)args->nav_mesh_instance->get_navigation_mesh()->duplicate().get());

        NavigationServer::get_singleton()->region_bake_navmesh(nav_mesh, args->nav_mesh_instance);
        args->nav_mesh_instance->call_deferred("_bake_finished", nav_mesh);
        memdelete(args);
    } else {

        ERR_PRINT("Can't bake the navigation mesh if the `NavigationMesh` resource doesn't exist");
        args->nav_mesh_instance->call_deferred("_bake_finished", Ref<NavigationMesh>());
        memdelete(args);
    }
}

void NavigationMeshInstance::bake_navigation_mesh() {
    ERR_FAIL_COND(bake_thread != nullptr);

    BakeThreadsArgs *args = memnew(BakeThreadsArgs);
    args->nav_mesh_instance = this;

    bake_thread = Thread::create(_bake_navigation_mesh, args);
    ERR_FAIL_COND(bake_thread == nullptr);
}

void NavigationMeshInstance::_bake_finished(Ref<NavigationMesh> p_nav_mesh) {
    set_navigation_mesh(p_nav_mesh);
    bake_thread = nullptr;
}

StringName NavigationMeshInstance::get_configuration_warning() const {

    if (!is_visible_in_tree() || !is_inside_tree())
        return StringName();

    if (!navmesh) {
        return TTR("A NavigationMesh resource must be set or created for this node to work.");
    }
    const Node3D *c = this;
    while (c) {

        if (object_cast<Navigation3D>(c))
            return StringName();

        c = object_cast<Node3D>(c->get_parent());
    }

    return TTR("NavigationMeshInstance must be a child or grandchild to a Navigation node. It only provides navigation data.");
}

void NavigationMeshInstance::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_navigation_mesh", {"navmesh"}),&NavigationMeshInstance::set_navigation_mesh);
    MethodBinder::bind_method(D_METHOD("get_navigation_mesh"), &NavigationMeshInstance::get_navigation_mesh);

    MethodBinder::bind_method(D_METHOD("set_enabled", {"enabled"}),&NavigationMeshInstance::set_enabled);
    MethodBinder::bind_method(D_METHOD("is_enabled"), &NavigationMeshInstance::is_enabled);

    MethodBinder::bind_method(D_METHOD("bake_navigation_mesh"), &NavigationMeshInstance::bake_navigation_mesh);
    MethodBinder::bind_method(D_METHOD("_bake_finished", {"nav_mesh"}),&NavigationMeshInstance::_bake_finished);

    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "navmesh", PropertyHint::ResourceType, "NavigationMesh"), "set_navigation_mesh", "get_navigation_mesh");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "enabled"), "set_enabled", "is_enabled");

    ADD_SIGNAL(MethodInfo("navigation_mesh_changed"));
    ADD_SIGNAL(MethodInfo("bake_finished"));
}

void NavigationMeshInstance::_changed_callback(Object *p_changed, StringName p_prop) {
    update_gizmo();
    update_configuration_warning();
}

NavigationMeshInstance::NavigationMeshInstance() {

    enabled = true;
    set_notify_transform(true);
    region = NavigationServer::get_singleton()->region_create();

    navigation = nullptr;
    debug_view = nullptr;
    bake_thread = nullptr;
}

NavigationMeshInstance::~NavigationMeshInstance() {
    if (navmesh)
        Object_remove_change_receptor(navmesh.get(),this);
    NavigationServer::get_singleton()->free(region);
}
