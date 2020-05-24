/*************************************************************************/
/*  object.h                                                             */
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

#pragma once

#include "core/os/memory.h"
#include "core/vector.h"
#include "core/object_id.h"
#include "core/variant.h"
#include "core/string_name.h"
#include "core/forward_decls.h"
#include "core/dictionary.h"

//#include <QObject>

class IObjectTooling;

#ifdef DEBUG_ENABLED
#include <atomic> // For ObjectRC*
#endif
class GODOT_EXPORT TypeInfo
{
public:
    constexpr TypeInfo(const char *typeName, const TypeInfo *baseTypeInfo) :
        /*type_(typeName),*/
        typeName_(typeName),
        baseTypeInfo_(baseTypeInfo) {}

    //bool IsTypeOf(se_string_hash type) const;
    constexpr bool IsTypeOf(const TypeInfo* typeInfo) const
    {
        const TypeInfo* current = this;
        while (current)
        {
            if (current == typeInfo)
                return true;

            current = current->getBaseTypeInfo();
        }
        return false;
    }
    /// Check current type is type of specified class type.
    template <typename T> bool isTypeOf() const { return IsTypeOf(T::get_type_info_static()); }

    //se_string_hash GetType() const { return type_; }
    constexpr const char *getTypeName() const { return typeName_; }
    /// Return base type info.
    constexpr const TypeInfo *getBaseTypeInfo() const { return baseTypeInfo_; }

private:
    //StringHash    type_;         //!< Type.
    const char *    typeName_;     //!< Type name.
    const TypeInfo *baseTypeInfo_; //!< Base class type info.
};


#define VARIANT_ARG_LIST const Variant &p_arg1 = Variant::null_variant, const Variant &p_arg2 = Variant::null_variant, const Variant &p_arg3 = Variant::null_variant, const Variant &p_arg4 = Variant::null_variant, const Variant &p_arg5 = Variant::null_variant
#define VARIANT_ARG_PASS p_arg1, p_arg2, p_arg3, p_arg4, p_arg5
#define VARIANT_ARG_DECLARE const Variant &p_arg1, const Variant &p_arg2, const Variant &p_arg3, const Variant &p_arg4, const Variant &p_arg5
#define VARIANT_ARG_MAX 5
#define VARIANT_ARGPTRS const Variant *argptr[5] = { &p_arg1, &p_arg2, &p_arg3, &p_arg4, &p_arg5 };
#define VARIANT_ARGPTRS_PASS *argptr[0], *argptr[1], *argptr[2], *argptr[3], *argptr[4]
#define VARIANT_ARGS_FROM_ARRAY(m_arr) m_arr[0], m_arr[1], m_arr[2], m_arr[3], m_arr[4]

#define ADD_SIGNAL(m_signal) ClassDB::add_signal(get_class_static_name(), eastl::move(m_signal))
#define ADD_PROPERTY(m_property, m_setter, m_getter) ClassDB::add_property(get_class_static_name(), m_property, m_setter, m_getter)
#define ADD_PROPERTYI(m_property, m_setter, m_getter, m_index) ClassDB::add_property(get_class_static_name(), m_property, m_setter, m_getter, m_index)
#define ADD_PROPERTY_DEFAULT(m_property, m_default) ClassDB::set_property_default_value(get_class_static_name(), m_property, m_default)
#define ADD_GROUP(m_name, m_prefix) ClassDB::add_property_group(get_class_static_name(), m_name, m_prefix)

Array convert_property_list(const List<PropertyInfo> *p_list);
Array convert_property_vector(Span<const PropertyInfo> p_list);

/*
   the following is an incomprehensible blob of hacks and workarounds to compensate for many of the fallencies in C++. As a plus, this macro pretty much alone defines the object model.
*/

#define REVERSE_GET_PROPERTY_LIST                                  \
public:                                                            \
    constexpr bool _is_gpl_reversed() const { return true; }; \
                                                                   \
private:

#define UNREVERSE_GET_PROPERTY_LIST                                 \
public:                                                             \
    constexpr bool _is_gpl_reversed() const { return false; }; \
                                                                    \
private:

#define GDCLASS(m_class,m_inherits)                                                                                    \
    using ClassName = m_class;                                                                                         \
    using BaseClassName = m_inherits;                                                                                  \
                                                                                                                       \
private:                                                                                                               \
    mutable StringName _class_name;                                                                                    \
    friend class ClassDB;                                                                                              \
    static constexpr TypeInfo typeInfoStatic {#m_class, BaseClassName::get_type_info_static()};              \
                                                                                                                       \
public:                                                                                                                \
    static constexpr const TypeInfo *get_type_info_static() { return &typeInfoStatic; }                                \
    static constexpr const char *get_type_name_static() { return typeInfoStatic.getTypeName(); }                       \
    const TypeInfo *get_type_info() const override { return &typeInfoStatic; }                                         \
    static bool initialize_class();                                                                                    \
    const StringName *_get_class_namev() const override {                                                              \
        if (!_class_name)                                                                                              \
            _class_name = StringName(#m_class);		                                                                   \
        return &_class_name;                                                                                           \
    }                                                                                                                  \
    static const void *get_class_ptr_static() {                                                                        \
        return &typeInfoStatic;                                                                                        \
    }                                                                                                                  \
    static constexpr const char *get_class_static() {                                                   \
        return #m_class;				                                                                               \
    }                                                                                                                  \
    static StringName get_class_static_name() {                                                         \
        return StringName(#m_class);	                                                                               \
    }                                                                                                                  \
    static const char *get_parent_class_static() {                                                      \
        return BaseClassName::get_class_static();                                                                        \
    }                                                                                                                  \
    virtual bool is_class(StringView p_class) const override {                                                        \
            return (p_class==#m_class) ? true : BaseClassName::is_class(p_class); }                            \
    virtual bool is_class_ptr(void *p_ptr) const override {                                                            \
            return (p_ptr == get_class_ptr_static()) ? true : BaseClassName::is_class_ptr(p_ptr); }                      \
                                                                                                                       \
protected:                                                                                                             \
    static void (*_get_bind_methods())() {                                                              \
        return &m_class::_bind_methods;                                                                                \
    }                                                                                                                  \
                                                                                                                       \
protected:                                                                                                             \
    bool _initialize_classv() override { return initialize_class(); }                                                  \
    static constexpr bool (Object::*_get_get() )(const StringName &p_name, Variant &r_ret) const {      \
        return (bool (Object::*)(const StringName &, Variant &) const) & m_class::_get;                                \
    }                                                                                                                  \
    bool _getv(const StringName &p_name, Variant &r_ret) const override {                                              \
        if (m_class::_get_get() != BaseClassName::_get_get()) {                                                          \
            if (_get(p_name, r_ret)) return true;                                                                      \
        }                                                                                                              \
        return BaseClassName::_getv(p_name, r_ret);                                                                      \
    }                                                                                                                  \
    static constexpr bool (Object::*_get_set() )(const StringName &p_name, const Variant &p_property) { \
        return (bool (Object::*)(const StringName &, const Variant &)) & m_class::_set;                                \
    }                                                                                                                  \
    bool _setv(const StringName &p_name, const Variant &p_property) override {                                         \
        if (BaseClassName::_setv(p_name, p_property)) return true;                                                       \
        if (m_class::_get_set() != BaseClassName::_get_set()) {                                                          \
            return _set(p_name, p_property);                                                                           \
        }                                                                                                              \
        return false;                                                                                                  \
    }                                                                                                                  \
    void (Object::*_get_get_property_list() const)(Vector<PropertyInfo> * p_list) const {             \
        return (void (Object::*)(Vector<PropertyInfo> *) const) & m_class::_get_property_list;                        \
    }                                                                                                                  \
    void _get_property_listv(Vector<PropertyInfo> *p_list, bool p_reversed) const override;                           \
    void (Object::*_get_notification() const)(int) {                                                    \
        return (void (Object::*)(int)) & m_class::_notification;                                                       \
    }                                                                                                                  \
    void _notificationv(int p_notification, bool p_reversed) override;                                                 \
                                                                                                                       \
private:

#define IMPL_GDCLASS(m_class)                                                                                          \
    bool m_class::initialize_class() {                                                                                 \
        static bool initialized = false;                                                                               \
        if (initialized) return false;                                                                                 \
        BaseClassName::initialize_class();                                                                               \
        ClassDB::_add_class<m_class, BaseClassName>();                                                                   \
        ClassDB::_set_class_header(get_class_static_name(), __FILE__);                                                 \
        if (m_class::_get_bind_methods() != BaseClassName::_get_bind_methods()) _bind_methods();                         \
        initialized = true;                                                                                            \
        return true;                                                                                                   \
    }                                                                                                                  \
    void m_class::_notificationv(int p_notification, bool p_reversed) {                                                \
        if (!p_reversed) BaseClassName::_notificationv(p_notification, p_reversed);                                      \
        if (m_class::_get_notification() != BaseClassName::_get_notification()) {                                        \
            _notification(p_notification);                                                                             \
        }                                                                                                              \
        if (p_reversed) BaseClassName::_notificationv(p_notification, p_reversed);                                       \
    }                                                                                                                  \
    void m_class::_get_property_listv(Vector<PropertyInfo> *p_list, bool p_reversed) const {                        \
        if (!p_reversed) {                                                                                             \
            BaseClassName::_get_property_listv(p_list, p_reversed);                                                      \
        }                                                                                                              \
        p_list->push_back(PropertyInfo(                                                                                \
                VariantType::NIL, get_class_static_name(), PropertyHint::None, nullptr, PROPERTY_USAGE_CATEGORY));     \
        if (!_is_gpl_reversed()) ClassDB::get_property_list(#m_class, p_list, true, this);                             \
        if (m_class::_get_get_property_list() != BaseClassName::_get_get_property_list()) {                              \
            _get_property_list(p_list);                                                                                \
        }                                                                                                              \
        if (_is_gpl_reversed()) ClassDB::get_property_list(#m_class, p_list, true, this);                              \
        if (p_reversed) {                                                                                              \
            BaseClassName::_get_property_listv(p_list, p_reversed);                                                      \
        }                                                                                                              \
    }

#define OBJ_CATEGORY(m_category)                                                                                       \
protected:                                                                                                             \
    static constexpr const char * _get_category() { return m_category; }                                               \
                                                                                                                       \
private:


#define OBJ_SAVE_TYPE(m_class)                                                                                         \
public:                                                                                                                \
    const char *get_save_class() const override { return #m_class; }                                                   \
                                                                                                                       \
private:

#define INVOCABLE
#define HAS_BINDS static void _bind_methods();


class ScriptInstance;
class ObjectRC;

class GODOT_EXPORT Object {
    //Q_GADGET
    // Non-copyable

    //Q_DISABLE_COPY(Object)
    //Q_PROPERTY(RefPtr script READ get_script WRITE set_script)
    static constexpr TypeInfo typeInfoStatic = TypeInfo( "Object", nullptr);
public:

    struct Connection {

        Vector<Variant> binds;
        StringName signal;
        StringName method;
        Object *source = nullptr;
        Object *target = nullptr;
        uint32_t flags = 0;

        bool operator<(const Connection &p_conn) const noexcept;

        operator Variant() const;
        Connection() = default;
        Connection(const Variant &p_variant);
    };

private:
    enum {
        MAX_SCRIPT_INSTANCE_BINDINGS = 8
    };

#ifdef DEBUG_ENABLED
    friend struct _ObjectDebugLock;
#endif
    friend void Object_change_notify(Object *self,StringName p_property);
    friend bool GODOT_EXPORT predelete_handler(Object *);
    friend void GODOT_EXPORT postinitialize_handler(Object *);

    struct Signal;
    struct ObjectPrivate;
    Dictionary metadata;
    ObjectPrivate *private_data;
    void *_script_instance_bindings[MAX_SCRIPT_INSTANCE_BINDINGS];
    ScriptInstance *script_instance;
    RefPtr script;
    ObjectID _instance_id;
    mutable StringName _class_name;
    mutable const StringName *_class_ptr;

    uint32_t instance_binding_count;
    int _predelete_ok;
    bool _block_signals;
    bool _can_translate;
    bool _emitting;
    bool _is_queued_for_deletion; // set to true by SceneTree::queue_delete()


#ifdef DEBUG_ENABLED
	std::atomic<ObjectRC *> _rc;
#endif
    bool _predelete();
    void _postinitialize();
public:
    void _add_user_signal(const StringName &p_name, const Array &p_args = Array());
    bool _has_user_signal(const StringName &p_name) const;
    Variant _emit_signal(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Array _get_signal_list() const;
    Array _get_signal_connection_list(StringName p_signal) const;
    Array _get_incoming_connections() const;
    void _set_bind(const StringName &p_set, const Variant &p_value);
    Variant _get_bind(const StringName &p_name) const;
    void _set_indexed_bind(const NodePath &p_name, const Variant &p_value);
    Variant _get_indexed_bind(const NodePath &p_name) const;
    void property_list_changed_notify();
private:
    friend class RefCounted;
protected:
    virtual bool _initialize_classv() { return initialize_class(); }
    virtual bool _setv(const StringName & /*p_name*/, const Variant & /*p_property*/) { return false; }
    virtual bool _getv(const StringName & /*p_name*/, Variant & /*r_property*/) const { return false; }
    virtual void _get_property_listv(Vector<PropertyInfo> *, bool /*p_reversed*/) const {}
    virtual void _notificationv(int /*p_notification*/, bool /*p_reversed*/){}

    static const char *_get_category() { return ""; }
    static void _bind_methods();
    bool _set(const StringName & /*p_name*/, const Variant & /*p_property*/) { return false; }
    bool _get(const StringName & /*p_name*/, Variant & /*r_property*/) const { return false; }
    void _get_property_list(Vector<PropertyInfo> * /*p_list*/) const {}
    void _notification(int /*p_notification*/){}
    bool wrap_is_class(StringView p_class) const;

    static void (*_get_bind_methods())() {
        return &Object::_bind_methods;
    }
    static constexpr bool (Object::*_get_get() )(const StringName &p_name, Variant &r_ret) const {
        return &Object::_get;
    }
    bool (Object::*_get_set() const)(const StringName &p_name, const Variant &p_property) {
        return &Object::_set;
    }
    void (Object::*_get_get_property_list() const)(Vector<PropertyInfo> *p_list) const {
        return &Object::_get_property_list;
    }
    void (Object::*_get_notification() const)(int) {
        return &Object::_notification;
    }

public:
    void cancel_delete();

    virtual void _changed_callback(Object *p_changed, StringName p_prop);
    Variant _call_bind(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Variant _call_deferred_bind(const Variant **p_args, int p_argcount, Callable::CallError &r_error);

    virtual const StringName *_get_class_namev() const {
        if (!_class_name)
            _class_name = get_class_static_name();
        return &_class_name;
    }
public:
    PoolStringArray _get_meta_list_bind() const;
    Array _get_property_list_bind() const;
    Array _get_method_list_bind() const;

    void _clear_internal_resource_paths(const Variant &p_var);

    friend class ClassDB;
    virtual void _validate_property(PropertyInfo &property) const;

    void _disconnect(const StringName &p_signal, Object *p_to_object, const StringName &p_to_method, bool p_force = false);

public: //should be protected, but bug in clang++
    static bool initialize_class();
    static void register_custom_data_to_otdb() {}

public:
    static const void *get_class_ptr_static() {
        return get_type_info_static();
    }
#ifdef DEBUG_ENABLED
	ObjectRC *_use_rc();
#endif
    bool _is_gpl_reversed() const { return false; }

    ObjectID get_instance_id() const { return _instance_id; }

    // this is used for editors

    enum {

        NOTIFICATION_POSTINITIALIZE = 0,
        NOTIFICATION_PREDELETE = 1
    };

    /* TYPE API */
    static constexpr const TypeInfo *get_type_info_static() { return &typeInfoStatic; }
    static constexpr const char * get_class_static() { return "Object"; }
    static StringName get_class_static_name() { return StringName("Object"); }
    static const char * get_parent_class_static() { return nullptr; }

    virtual const TypeInfo *get_type_info() const { return get_type_info_static(); }
    virtual const char *get_class() const { return get_type_info()->getTypeName(); }
    String wrap_get_class() const;

    virtual const char *get_save_class() const { return get_class(); } //class stored when saving

    virtual bool is_class(StringView p_class) const { return p_class == "Object"; }
    virtual bool is_class_ptr(void *p_ptr) const { return get_type_info_static() == p_ptr; }

    _FORCE_INLINE_ const StringName &get_class_name() const {
        if (!_class_ptr) {
            return *_get_class_namev();
        }
        return *_class_ptr;
    }

    /* IAPI */
    //void set(const String& p_name, const Variant& p_value);
    //Variant get(const String& p_name) const;

    void set(const StringName &p_name, const Variant &p_value, bool *r_valid = nullptr);
    Variant get(const StringName &p_name, bool *r_valid = nullptr) const;
    template<class TGT_TYPE>
    TGT_TYPE getT(const StringName &p_name, bool *r_valid = nullptr) const {
        return (TGT_TYPE)get(p_name, r_valid);
    }
    void set_indexed(const Vector<StringName> &p_names, const Variant &p_value, bool *r_valid = nullptr);
    Variant get_indexed(const Vector<StringName> &p_names, bool *r_valid = nullptr) const;

    void get_property_list(Vector<PropertyInfo> *p_list, bool p_reversed = false) const;

    bool has_method(const StringName &p_method) const;
    void get_method_list(Vector<MethodInfo> *p_list) const;
    Variant callv(const StringName &p_method, const Array &p_args);
    virtual Variant call(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    virtual void call_multilevel(const StringName &p_method, const Variant **p_args, int p_argcount);
    virtual void call_multilevel_reversed(const StringName &p_method, const Variant **p_args, int p_argcount);

    Variant call_va(const StringName &p_name, VARIANT_ARG_LIST); // C++ helper
    void call_multilevel(const StringName &p_name, VARIANT_ARG_LIST); // C++ helper

    void notification(int p_notification, bool p_reversed = false);
    String to_string();

    //used mainly by script, get and set all INCLUDING string
    virtual Variant getvar(const Variant &p_key, bool *r_valid = nullptr) const;
    virtual void setvar(const Variant &p_key, const Variant &p_value, bool *r_valid = nullptr);

    /* SCRIPT */

    void set_script(const RefPtr &p_script);
    RefPtr get_script() const;

    /* SCRIPT */

    bool has_meta(StringView p_name) const;
    void set_meta(StringView p_name, const Variant &p_value);
    void remove_meta(StringView p_name);
    Variant get_meta(StringView p_name) const;
    void get_meta_list(List<String> *p_list) const;

    IObjectTooling *get_tooling_interface() const;

    void set_script_instance(ScriptInstance *p_instance);
    ScriptInstance *get_script_instance() const { return script_instance; }

    void set_script_and_instance(const RefPtr &p_script, ScriptInstance *p_instance);

    void add_user_signal(MethodInfo &&p_signal);
    Error emit_signal(const StringName &p_name, VARIANT_ARG_LIST);
    Error emit_signal(const StringName &p_name, const Variant **p_args, int p_argcount);
    bool has_signal(const StringName &p_name) const;
    void get_signal_list(Vector<MethodInfo> *p_signals) const;
    void get_signal_connection_list(const StringName &p_signal, List<Connection> *p_connections) const;
    void get_all_signal_connections(List<Connection> *p_connections) const;
    int get_persistent_signal_connection_count() const;
    void get_signals_connected_to_this(List<Connection> *p_connections) const;

    Error connect(const StringName &p_signal, Object *p_to_object, const StringName &p_to_method, const Vector<Variant> &p_binds =
        null_variant_pvec, uint32_t p_flags = 0);
    Error connectF(const StringName& p_signal, eastl::function<void(void)> p_to_object, uint32_t p_flags = 0);
    void disconnect(const StringName &p_signal, Object *p_to_object, const StringName &p_to_method);
    bool is_connected(const StringName &p_signal, Object *p_to_object, const StringName &p_to_method) const;

    void call_deferred(const StringName &p_method, VARIANT_ARG_LIST);
    void call_deferred(eastl::function<void()> func);

    void set_deferred(const StringName &p_property, const Variant &p_value);

    void set_block_signals(bool p_block);
    bool is_blocking_signals() const;

    VariantType get_static_property_type(const StringName &p_property, bool *r_valid = nullptr) const;
    VariantType get_static_property_type_indexed(const Vector<StringName> &p_path, bool *r_valid = nullptr) const;

    virtual void get_translatable_strings(List<StringName> *p_strings) const;
    virtual void get_argument_options(const StringName &p_function, int p_idx, List<String> *r_options) const;
#ifdef DEBUG_ENABLED
    /// Used in gObjectDB().cleanup() warning print
    virtual const char *get_dbg_name() const { return nullptr; }
#endif

    StringName tr(const StringName &p_message) const; // translate message (internationalization)

    bool is_queued_for_deletion() const;
    void deleteLater() { _is_queued_for_deletion = true; }

    void set_message_translation(bool p_enable) { _can_translate = p_enable; }
    bool can_translate_messages() const { return _can_translate; }


    //used by script languages to store binding data
    void *get_script_instance_binding(int p_script_language_index);
    bool has_script_instance_binding(int p_script_language_index);
    void set_script_instance_binding(int p_script_language_index, void *p_data);

    void clear_internal_resource_paths();

    Object();
    virtual ~Object();

    Object(Object &&) noexcept = default;
};

template <class T>
T *object_cast(Object *p_object) {
#ifdef RTTI_ENABLED
    return dynamic_cast<T *>(p_object);
#else
    if (!p_object)
        return nullptr;
    if (p_object->is_class_ptr(T::get_class_ptr_static()))
        return static_cast<T *>(p_object);
    else
        return nullptr;
#endif
}

template <class T>
const T *object_cast(const Object *p_object) {
#ifdef RTTI_ENABLED
    return dynamic_cast<const T *>(p_object);
#else
    if (!p_object)
        return nullptr;
    if (p_object->is_class_ptr(T::get_class_ptr_static()))
        return static_cast<const T *>(p_object);
    else
        return nullptr;
#endif
}
namespace ObjectNS
{
    //Q_NAMESPACE
    enum ConnectFlags : uint8_t {

        CONNECT_QUEUED = 1,
        CONNECT_PERSIST = 2, // hint for scene to save this connection
        CONNECT_ONESHOT = 4,
        CONNECT_REFERENCE_COUNTED = 8,
    };
    //Q_ENUM_NS(ConnectFlags)
    template<class T>
    T* cast_to(::Object *f) {
        return object_cast<T>(f);
    }
} // end of ObjectNS namespace

bool GODOT_EXPORT predelete_handler(Object *p_object);
void GODOT_EXPORT postinitialize_handler(Object *p_object);
