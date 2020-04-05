/*************************************************************************/
/*  particles_2d.cpp                                                     */
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

#include "particles_2d.h"

#include "core/method_bind.h"
#include "core/os/os.h"
#include "core/object_tooling.h"
#include "core/translation_helpers.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/particles_material.h"
#include "scene/scene_string_names.h"

#ifdef TOOLS_ENABLED
#include "core/engine.h"
#endif

IMPL_GDCLASS(Particles2D)
VARIANT_ENUM_CAST(Particles2D::DrawOrder)

void Particles2D::set_emitting(bool p_emitting) {

    VisualServer::get_singleton()->particles_set_emitting(particles, p_emitting);

    if (p_emitting && one_shot) {
        set_process_internal(true);
    } else if (!p_emitting) {
        set_process_internal(false);
    }
}

void Particles2D::set_amount(int p_amount) {

    ERR_FAIL_COND_MSG(p_amount < 1, "Amount of particles cannot be smaller than 1.");
    amount = p_amount;
    VisualServer::get_singleton()->particles_set_amount(particles, amount);
}
void Particles2D::set_lifetime(float p_lifetime) {

    ERR_FAIL_COND_MSG(p_lifetime <= 0, "Particles lifetime must be greater than 0.");
    lifetime = p_lifetime;
    VisualServer::get_singleton()->particles_set_lifetime(particles, lifetime);
}

void Particles2D::set_one_shot(bool p_enable) {

    one_shot = p_enable;
    VisualServer::get_singleton()->particles_set_one_shot(particles, one_shot);

    if (is_emitting()) {

        set_process_internal(true);
        if (!one_shot)
            VisualServer::get_singleton()->particles_restart(particles);
    }

    if (!one_shot)
        set_process_internal(false);
}
void Particles2D::set_pre_process_time(float p_time) {

    pre_process_time = p_time;
    VisualServer::get_singleton()->particles_set_pre_process_time(particles, pre_process_time);
}
void Particles2D::set_explosiveness_ratio(float p_ratio) {

    explosiveness_ratio = p_ratio;
    VisualServer::get_singleton()->particles_set_explosiveness_ratio(particles, explosiveness_ratio);
}
void Particles2D::set_randomness_ratio(float p_ratio) {

    randomness_ratio = p_ratio;
    VisualServer::get_singleton()->particles_set_randomness_ratio(particles, randomness_ratio);
}
void Particles2D::set_visibility_rect(const Rect2 &p_visibility_rect) {

    visibility_rect = p_visibility_rect;
    AABB aabb;
    aabb.position.x = p_visibility_rect.position.x;
    aabb.position.y = p_visibility_rect.position.y;
    aabb.size.x = p_visibility_rect.size.x;
    aabb.size.y = p_visibility_rect.size.y;

    VisualServer::get_singleton()->particles_set_custom_aabb(particles, aabb);

    Object_change_notify(this,"visibility_rect");
    update();
}
void Particles2D::set_use_local_coordinates(bool p_enable) {

    local_coords = p_enable;
    VisualServer::get_singleton()->particles_set_use_local_coordinates(particles, local_coords);
    set_notify_transform(!p_enable);
    if (!p_enable && is_inside_tree()) {
        _update_particle_emission_transform();
    }
}

void Particles2D::_update_particle_emission_transform() {

    Transform2D xf2d = get_global_transform();
    Transform xf;
    xf.basis.set_axis(0, Vector3(xf2d.get_axis(0).x, xf2d.get_axis(0).y, 0));
    xf.basis.set_axis(1, Vector3(xf2d.get_axis(1).x, xf2d.get_axis(1).y, 0));
    xf.set_origin(Vector3(xf2d.get_origin().x, xf2d.get_origin().y, 0));

    VisualServer::get_singleton()->particles_set_emission_transform(particles, xf);
}

void Particles2D::set_process_material(const Ref<Material> &p_material) {

    process_material = p_material;
    Ref<ParticlesMaterial> pm = dynamic_ref_cast<ParticlesMaterial>(p_material);
    if (pm && !pm->get_flag(ParticlesMaterial::FLAG_DISABLE_Z) && pm->get_gravity() == Vector3(0, -9.8, 0)) {
        // Likely a new (3D) material, modify it to match 2D space
        pm->set_flag(ParticlesMaterial::FLAG_DISABLE_Z, true);
        pm->set_gravity(Vector3(0, 98, 0));
    }
    RID material_rid;
    if (process_material)
        material_rid = process_material->get_rid();
    VisualServer::get_singleton()->particles_set_process_material(particles, material_rid);

    update_configuration_warning();
}

void Particles2D::set_speed_scale(float p_scale) {

    speed_scale = p_scale;
    VisualServer::get_singleton()->particles_set_speed_scale(particles, p_scale);
}

bool Particles2D::is_emitting() const {

    return VisualServer::get_singleton()->particles_get_emitting(particles);
}
int Particles2D::get_amount() const {

    return amount;
}
float Particles2D::get_lifetime() const {

    return lifetime;
}

bool Particles2D::get_one_shot() const {

    return one_shot;
}
float Particles2D::get_pre_process_time() const {

    return pre_process_time;
}
float Particles2D::get_explosiveness_ratio() const {

    return explosiveness_ratio;
}
float Particles2D::get_randomness_ratio() const {

    return randomness_ratio;
}
Rect2 Particles2D::get_visibility_rect() const {

    return visibility_rect;
}
bool Particles2D::get_use_local_coordinates() const {

    return local_coords;
}
Ref<Material> Particles2D::get_process_material() const {

    return process_material;
}

float Particles2D::get_speed_scale() const {

    return speed_scale;
}

void Particles2D::set_draw_order(DrawOrder p_order) {

    draw_order = p_order;
    VisualServer::get_singleton()->particles_set_draw_order(particles, VS::ParticlesDrawOrder(p_order));
}

Particles2D::DrawOrder Particles2D::get_draw_order() const {

    return draw_order;
}

void Particles2D::set_fixed_fps(int p_count) {
    fixed_fps = p_count;
    VisualServer::get_singleton()->particles_set_fixed_fps(particles, p_count);
}

int Particles2D::get_fixed_fps() const {
    return fixed_fps;
}

void Particles2D::set_fractional_delta(bool p_enable) {
    fractional_delta = p_enable;
    VisualServer::get_singleton()->particles_set_fractional_delta(particles, p_enable);
}

bool Particles2D::get_fractional_delta() const {
    return fractional_delta;
}

StringName Particles2D::get_configuration_warning() const {

    String warnings;

    if (not process_material) {
        if (!warnings.empty())
            warnings += '\n';
        warnings += "- " + TTR("A material to process the particles is not assigned, so no behavior is imprinted.");
    } else {

        CanvasItemMaterial *mat = object_cast<CanvasItemMaterial>(get_material().get());

        if (not get_material() || (mat && !mat->get_particles_animation())) {
            const ParticlesMaterial *process = object_cast<ParticlesMaterial>(process_material.get());
            if (process &&
                    (process->get_param(ParticlesMaterial::PARAM_ANIM_SPEED) != 0.0f || process->get_param(ParticlesMaterial::PARAM_ANIM_OFFSET) != 0.0f ||
                            process->get_param_texture(ParticlesMaterial::PARAM_ANIM_SPEED) || process->get_param_texture(ParticlesMaterial::PARAM_ANIM_OFFSET))) {
                if (!warnings.empty())
                    warnings += '\n';
                warnings += "- " + TTR("Particles2D animation requires the usage of a CanvasItemMaterial with \"Particles Animation\" enabled.");
            }
        }
    }

    return StringName(warnings);
}

Rect2 Particles2D::capture_rect() const {

    AABB aabb = VisualServer::get_singleton()->particles_get_current_aabb(particles);
    Rect2 r;
    r.position.x = aabb.position.x;
    r.position.y = aabb.position.y;
    r.size.x = aabb.size.x;
    r.size.y = aabb.size.y;
    return r;
}

void Particles2D::set_texture(const Ref<Texture> &p_texture) {
    texture = p_texture;
    update();
}

Ref<Texture> Particles2D::get_texture() const {
    return texture;
}

void Particles2D::set_normal_map(const Ref<Texture> &p_normal_map) {

    normal_map = p_normal_map;
    update();
}

Ref<Texture> Particles2D::get_normal_map() const {
    return normal_map;
}

void Particles2D::_validate_property(PropertyInfo &property) const {
}

void Particles2D::restart() {
    VisualServer::get_singleton()->particles_restart(particles);
    VisualServer::get_singleton()->particles_set_emitting(particles, true);
}

void Particles2D::_notification(int p_what) {

    if (p_what == NOTIFICATION_DRAW) {

        RID texture_rid;
        if (texture)
            texture_rid = texture->get_rid();
        RID normal_rid;
        if (normal_map)
            normal_rid = normal_map->get_rid();

        VisualServer::get_singleton()->canvas_item_add_particles(get_canvas_item(), particles, texture_rid, normal_rid);

#ifdef TOOLS_ENABLED
        if (Engine::get_singleton()->is_editor_hint() && (this == get_tree()->get_edited_scene_root() || get_tree()->get_edited_scene_root()->is_a_parent_of(this))) {

            draw_rect(visibility_rect, Color(0, 0.7, 0.9, 0.4), false);
        }
#endif
    }

    if (p_what == NOTIFICATION_PAUSED || p_what == NOTIFICATION_UNPAUSED) {
        if (can_process()) {
            VisualServer::get_singleton()->particles_set_speed_scale(particles, speed_scale);
        } else {

            VisualServer::get_singleton()->particles_set_speed_scale(particles, 0);
        }
    }

    if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
        _update_particle_emission_transform();
    }

    if (p_what == NOTIFICATION_INTERNAL_PROCESS) {

        if (one_shot && !is_emitting()) {
            Object_change_notify(this);
            set_process_internal(false);
        }
    }
}

void Particles2D::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_emitting", {"emitting"}), &Particles2D::set_emitting);
    MethodBinder::bind_method(D_METHOD("set_amount", {"amount"}), &Particles2D::set_amount);
    MethodBinder::bind_method(D_METHOD("set_lifetime", {"secs"}), &Particles2D::set_lifetime);
    MethodBinder::bind_method(D_METHOD("set_one_shot", {"secs"}), &Particles2D::set_one_shot);
    MethodBinder::bind_method(D_METHOD("set_pre_process_time", {"secs"}), &Particles2D::set_pre_process_time);
    MethodBinder::bind_method(D_METHOD("set_explosiveness_ratio", {"ratio"}), &Particles2D::set_explosiveness_ratio);
    MethodBinder::bind_method(D_METHOD("set_randomness_ratio", {"ratio"}), &Particles2D::set_randomness_ratio);
    MethodBinder::bind_method(D_METHOD("set_visibility_rect", {"visibility_rect"}), &Particles2D::set_visibility_rect);
    MethodBinder::bind_method(D_METHOD("set_use_local_coordinates", {"enable"}), &Particles2D::set_use_local_coordinates);
    MethodBinder::bind_method(D_METHOD("set_fixed_fps", {"fps"}), &Particles2D::set_fixed_fps);
    MethodBinder::bind_method(D_METHOD("set_fractional_delta", {"enable"}), &Particles2D::set_fractional_delta);
    MethodBinder::bind_method(D_METHOD("set_process_material", {"material"}), &Particles2D::set_process_material);
    MethodBinder::bind_method(D_METHOD("set_speed_scale", {"scale"}), &Particles2D::set_speed_scale);

    MethodBinder::bind_method(D_METHOD("is_emitting"), &Particles2D::is_emitting);
    MethodBinder::bind_method(D_METHOD("get_amount"), &Particles2D::get_amount);
    MethodBinder::bind_method(D_METHOD("get_lifetime"), &Particles2D::get_lifetime);
    MethodBinder::bind_method(D_METHOD("get_one_shot"), &Particles2D::get_one_shot);
    MethodBinder::bind_method(D_METHOD("get_pre_process_time"), &Particles2D::get_pre_process_time);
    MethodBinder::bind_method(D_METHOD("get_explosiveness_ratio"), &Particles2D::get_explosiveness_ratio);
    MethodBinder::bind_method(D_METHOD("get_randomness_ratio"), &Particles2D::get_randomness_ratio);
    MethodBinder::bind_method(D_METHOD("get_visibility_rect"), &Particles2D::get_visibility_rect);
    MethodBinder::bind_method(D_METHOD("get_use_local_coordinates"), &Particles2D::get_use_local_coordinates);
    MethodBinder::bind_method(D_METHOD("get_fixed_fps"), &Particles2D::get_fixed_fps);
    MethodBinder::bind_method(D_METHOD("get_fractional_delta"), &Particles2D::get_fractional_delta);
    MethodBinder::bind_method(D_METHOD("get_process_material"), &Particles2D::get_process_material);
    MethodBinder::bind_method(D_METHOD("get_speed_scale"), &Particles2D::get_speed_scale);

    MethodBinder::bind_method(D_METHOD("set_draw_order", {"order"}), &Particles2D::set_draw_order);
    MethodBinder::bind_method(D_METHOD("get_draw_order"), &Particles2D::get_draw_order);

    MethodBinder::bind_method(D_METHOD("set_texture", {"texture"}), &Particles2D::set_texture);
    MethodBinder::bind_method(D_METHOD("get_texture"), &Particles2D::get_texture);

    MethodBinder::bind_method(D_METHOD("set_normal_map", {"texture"}), &Particles2D::set_normal_map);
    MethodBinder::bind_method(D_METHOD("get_normal_map"), &Particles2D::get_normal_map);

    MethodBinder::bind_method(D_METHOD("capture_rect"), &Particles2D::capture_rect);

    MethodBinder::bind_method(D_METHOD("restart"), &Particles2D::restart);

    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "emitting"), "set_emitting", "is_emitting");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "amount", PropertyHint::ExpRange, "1,1000000,1"), "set_amount", "get_amount");
    ADD_GROUP("Time", "");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "lifetime", PropertyHint::Range, "0.01,600.0,0.01,or_greater"), "set_lifetime", "get_lifetime");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "one_shot"), "set_one_shot", "get_one_shot");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "preprocess", PropertyHint::Range, "0.00,600.0,0.01"), "set_pre_process_time", "get_pre_process_time");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "speed_scale", PropertyHint::Range, "0,64,0.01"), "set_speed_scale", "get_speed_scale");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "explosiveness", PropertyHint::Range, "0,1,0.01"), "set_explosiveness_ratio", "get_explosiveness_ratio");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "randomness", PropertyHint::Range, "0,1,0.01"), "set_randomness_ratio", "get_randomness_ratio");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "fixed_fps", PropertyHint::Range, "0,1000,1"), "set_fixed_fps", "get_fixed_fps");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "fract_delta"), "set_fractional_delta", "get_fractional_delta");
    ADD_GROUP("Drawing", "");
    ADD_PROPERTY(PropertyInfo(VariantType::RECT2, "visibility_rect"), "set_visibility_rect", "get_visibility_rect");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "local_coords"), "set_use_local_coordinates", "get_use_local_coordinates");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "draw_order", PropertyHint::Enum, "Index,Lifetime"), "set_draw_order", "get_draw_order");
    ADD_GROUP("Process Material", "process_");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "process_material", PropertyHint::ResourceType, "ShaderMaterial,ParticlesMaterial"), "set_process_material", "get_process_material");
    ADD_GROUP("Textures", "");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "texture", PropertyHint::ResourceType, "Texture"), "set_texture", "get_texture");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "normal_map", PropertyHint::ResourceType, "Texture"), "set_normal_map", "get_normal_map");

    BIND_ENUM_CONSTANT(DRAW_ORDER_INDEX)
    BIND_ENUM_CONSTANT(DRAW_ORDER_LIFETIME)
}

Particles2D::Particles2D() {

    particles = VisualServer::get_singleton()->particles_create();

    one_shot = false; // Needed so that set_emitting doesn't access uninitialized values
    set_emitting(true);
    set_one_shot(false);
    set_amount(8);
    set_lifetime(1);
    set_fixed_fps(0);
    set_fractional_delta(true);
    set_pre_process_time(0);
    set_explosiveness_ratio(0);
    set_randomness_ratio(0);
    set_visibility_rect(Rect2(Vector2(-100, -100), Vector2(200, 200)));
    set_use_local_coordinates(true);
    set_draw_order(DRAW_ORDER_INDEX);
    set_speed_scale(1);
}

Particles2D::~Particles2D() {

    VisualServer::get_singleton()->free_rid(particles);
}
