/*************************************************************************/
/*  resource_importer_csv_translation.cpp                                */
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

#include "resource_importer_csv_translation.h"

#include "core/compressed_translation.h"
#include "core/io/resource_saver.h"
#include "core/list.h"
#include "core/os/file_access.h"
#include "core/string_utils.h"
#include "core/translation.h"

StringName ResourceImporterCSVTranslation::get_importer_name() const {

    return "csv_translation";
}

StringName ResourceImporterCSVTranslation::get_visible_name() const {

    return "CSV Translation";
}
void ResourceImporterCSVTranslation::get_recognized_extensions(PODVector<se_string> &p_extensions) const {

    p_extensions.push_back("csv");
}

StringName ResourceImporterCSVTranslation::get_save_extension() const {
    return ""; //does not save a single resource
}

StringName ResourceImporterCSVTranslation::get_resource_type() const {

    return "Translation";
}

bool ResourceImporterCSVTranslation::get_option_visibility(const StringName &p_option, const Map<StringName, Variant> &p_options) const {

    return true;
}

int ResourceImporterCSVTranslation::get_preset_count() const {
    return 0;
}
StringName ResourceImporterCSVTranslation::get_preset_name(int p_idx) const {

    return "";
}

void ResourceImporterCSVTranslation::get_import_options(ListPOD<ImportOption> *r_options, int p_preset) const {

    r_options->push_back(ImportOption(PropertyInfo(VariantType::BOOL, "compress"), true));
    r_options->push_back(ImportOption(PropertyInfo(VariantType::INT, "delimiter", PROPERTY_HINT_ENUM, "Comma,Semicolon,Tab"), 0));
}

Error ResourceImporterCSVTranslation::import(se_string_view p_source_file, se_string_view p_save_path, const Map<StringName, Variant> &p_options, List<se_string> *r_platform_variants, List<se_string> *r_gen_files, Variant *r_metadata) {

    bool compress = p_options.at("compress").as<bool>();

    char delimiter;
    switch ((int)p_options.at("delimiter")) {
        case 0: delimiter = ','; break;
        case 1: delimiter = ';'; break;
        case 2: delimiter = '\t'; break;
    }

    FileAccessRef f = FileAccess::open(p_source_file, FileAccess::READ);

    ERR_FAIL_COND_V_MSG(!f, ERR_INVALID_PARAMETER, "Cannot open file from path '" + p_source_file + "'.")

    Vector<se_string> line = f->get_csv_line(delimiter);
    ERR_FAIL_COND_V(line.size() <= 1, ERR_PARSE_ERROR)

    Vector<se_string> locales;
    Vector<Ref<Translation> > translations;

    for (int i = 1; i < line.size(); i++) {

        se_string_view  locale = line[i];
        ERR_FAIL_COND_V_MSG(!TranslationServer::is_locale_valid(locale), ERR_PARSE_ERROR, "Error importing CSV translation: '" + locale + "' is not a valid locale.")

        locales.push_back(se_string(locale));
        Ref<Translation> translation=make_ref_counted<Translation>();
        translation->set_locale(locale);
        translations.push_back(translation);
    }

    line = f->get_csv_line(delimiter);

    while (line.size() == locales.size() + 1) {

        se_string_view  key = line[0];
        if (!key.empty()) {

            for (int i = 1; i < line.size(); i++) {
                translations.write[i - 1]->add_message(StringName(key), StringName(StringUtils::c_unescape(line[i])));
            }
        }

        line = f->get_csv_line(delimiter);
    }

    for (int i = 0; i < translations.size(); i++) {
        Ref<Translation> xlt = translations[i];

        if (compress) {
            Ref<PHashTranslation> cxl(make_ref_counted<PHashTranslation>());
            cxl->generate(xlt);
            xlt = cxl;
        }

        se_string save_path = se_string(PathUtils::get_basename(p_source_file)) + "." + translations[i]->get_locale() + ".translation";

        ResourceSaver::save(save_path, xlt);
        if (r_gen_files) {
            r_gen_files->push_back(save_path);
        }
    }

    return OK;
}

ResourceImporterCSVTranslation::ResourceImporterCSVTranslation() {
}
