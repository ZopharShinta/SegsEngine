/*************************************************************************/
/*  physics_material.cpp                                                 */
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

#include "physics_material.h"
#include "core/method_bind.h"

IMPL_GDCLASS(PhysicsMaterial)
RES_BASE_EXTENSION_IMPL(PhysicsMaterial,"phymat")

void PhysicsMaterial::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_friction", {"friction"}), &PhysicsMaterial::set_friction);
    MethodBinder::bind_method(D_METHOD("get_friction"), &PhysicsMaterial::get_friction);

    MethodBinder::bind_method(D_METHOD("set_rough", {"rough"}), &PhysicsMaterial::set_rough);
    MethodBinder::bind_method(D_METHOD("is_rough"), &PhysicsMaterial::is_rough);

    MethodBinder::bind_method(D_METHOD("set_bounce", {"bounce"}), &PhysicsMaterial::set_bounce);
    MethodBinder::bind_method(D_METHOD("get_bounce"), &PhysicsMaterial::get_bounce);

    MethodBinder::bind_method(D_METHOD("set_absorbent", {"absorbent"}), &PhysicsMaterial::set_absorbent);
    MethodBinder::bind_method(D_METHOD("is_absorbent"), &PhysicsMaterial::is_absorbent);

    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "friction"), "set_friction", "get_friction");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "rough"), "set_rough", "is_rough");
    ADD_PROPERTY(PropertyInfo(VariantType::FLOAT, "bounce"), "set_bounce", "get_bounce");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "absorbent"), "set_absorbent", "is_absorbent");
}

void PhysicsMaterial::set_friction(real_t p_val) {
    friction = p_val;
    emit_changed();
}

void PhysicsMaterial::set_rough(bool p_val) {
    rough = p_val;
    emit_changed();
}

void PhysicsMaterial::set_bounce(real_t p_val) {
    bounce = p_val;
    emit_changed();
}

void PhysicsMaterial::set_absorbent(bool p_val) {
    absorbent = p_val;
    emit_changed();
}

PhysicsMaterial::PhysicsMaterial() :
        friction(1),
        rough(false),
        bounce(0),
        absorbent(false) {}
