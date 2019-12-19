#pragma once

#include "core/property_info.h"
#include "core/variant.h"
#include "core/map.h"
#include "core/error_list.h"

template <class T>
class Vector;
class EditorServiceInterface;

class ResourceImporterInterface {
protected:
    EditorServiceInterface *m_editor_interface;

public:
    struct ImportOption {
        PropertyInfo option;
        Variant default_value;

        ImportOption(const PropertyInfo &p_info, const Variant &p_default) : option(p_info), default_value(p_default) {}
        ImportOption() = default;
    };
    void set_editor_interface(EditorServiceInterface *i) { m_editor_interface = i; }

    virtual StringName get_importer_name() const = 0;
    virtual StringName get_visible_name() const = 0;
    virtual void get_recognized_extensions(PODVector<se_string> &p_extensions) const = 0;
    virtual StringName get_save_extension() const = 0;
    virtual StringName get_resource_type() const = 0;
    virtual float get_priority() const = 0;
    virtual int get_import_order() const = 0;
    virtual int get_preset_count() const = 0;
    virtual StringName get_preset_name(int /*p_idx*/) const = 0;
    virtual void get_import_options(ListPOD<ImportOption> *r_options, int p_preset = 0) const = 0;
    virtual bool get_option_visibility(const StringName &p_option, const Map<StringName, Variant> &p_options) const = 0;
    virtual StringName get_option_group_file() const = 0;
    virtual Error import(se_string_view p_source_file, se_string_view p_save_path,
            const Map<StringName, Variant> &p_options, DefList<se_string> *r_platform_variants,
            DefList<se_string> *r_gen_files = nullptr, Variant *r_metadata = nullptr) = 0;
    virtual Error import_group_file(se_string_view p_group_file,
            const Map<se_string, Map<StringName, Variant>> &p_source_file_options,
            const Map<se_string, se_string> &p_base_paths) = 0;
    virtual bool are_import_settings_valid(se_string_view p_path) const = 0;
    virtual se_string get_import_settings_string() const = 0;
    // Currently only implemented by ResourceImporterTexture
    /**
     * @brief build_reconfigured_list will use the resource's configuration and current state of the object as set by user
     * and build a list of resources that need to be reimported.
     * @param tgt will contain a vector of all resource names that need to be reimported
     * @note this method should not be called until editor ends it's current scan/import proces
     */
    virtual void build_reconfigured_list(Vector<se_string> & /*tgt*/) {}
    virtual ~ResourceImporterInterface() = default;
};
