/*************************************************************************/
/*  editor_help.h                                                        */
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

#ifndef EDITOR_HELP_H
#define EDITOR_HELP_H

#include "editor/code_editor.h"
#include "editor/doc/doc_data.h"
#include "editor/editor_plugin.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/panel_container.h"

#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"

#include "scene/main/timer.h"

class RichTextLabel;

class FindBar : public HBoxContainer {
    GDCLASS(FindBar,HBoxContainer)

    LineEdit *search_text;
    ToolButton *find_prev;
    ToolButton *find_next;
    Label *matches_label;
    TextureButton *hide_button;
    String prev_search;

    RichTextLabel *rich_text_label;

    int results_count;

    void _show_search();
    void _hide_bar();

    void _search_text_changed(se_string_view p_text);
    void _search_text_entered(se_string_view p_text);

    void _update_results_count();
    void _update_matches_label();

    void _update_size();

protected:
    void _notification(int p_what);
    void _unhandled_input(const Ref<InputEvent> &p_event);

    bool _search(bool p_search_previous = false);

    static void _bind_methods();

public:
    void set_rich_text_label(RichTextLabel *p_rich_text_label);

    void popup_search();

    bool search_prev();
    bool search_next();

    FindBar();
};

class EditorHelp : public VBoxContainer {

    GDCLASS(EditorHelp,VBoxContainer)

    enum Page {

        PAGE_CLASS_LIST,
        PAGE_CLASS_DESC,
        PAGE_CLASS_PREV,
        PAGE_CLASS_NEXT,
        PAGE_SEARCH,
        CLASS_SEARCH,

    };

    bool select_locked;

    se_string prev_search;

    StringName edited_class;

    Vector<Pair<se_string, int> > section_line;
    Map<se_string, int> method_line;
    Map<se_string, int> signal_line;
    Map<se_string, int> property_line;
    Map<se_string, int> theme_property_line;
    Map<se_string, int> constant_line;
    Map<se_string, int> enum_line;
    Map<se_string, Map<se_string, int> > enum_values_line;
    int description_line;

    RichTextLabel *class_desc;
    HSplitContainer *h_split;
    static DocData *doc;

    ConfirmationDialog *search_dialog;
    LineEdit *search;
    FindBar *find_bar;

    se_string base_path;

    Color title_color;
    Color text_color;
    Color headline_color;
    Color base_type_color;
    Color type_color;
    Color comment_color;
    Color symbol_color;
    Color value_color;
    Color qualifier_color;

    void _init_colors();
    void _help_callback(se_string_view p_topic);

    void _add_text(se_string_view p_bbcode);
    bool scroll_locked;

    //void _button_pressed(int p_idx);
    void _add_type(se_string_view p_type, se_string_view p_enum = {});
    void _add_method(const DocData::MethodDoc &p_method, bool p_overview = true);

    void _class_list_select(se_string_view p_select);
    void _class_desc_select(se_string_view p_select);
    void _class_desc_input(const Ref<InputEvent> &p_input);
    void _class_desc_resized();

    Error _goto_desc(se_string_view p_class, int p_vscr = -1);
    //void _update_history_buttons();
    void _update_doc();

    void _request_help(se_string_view p_string);
    void _search(bool p_search_previous=false);

    void _unhandled_key_input(const Ref<InputEvent> &p_ev);

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    static void generate_doc();
    static DocData *get_doc_data() { return doc; }

    void go_to_help(se_string_view p_help);
    void go_to_class(se_string_view p_class, int p_scroll = 0);

    Vector<Pair<se_string, int> > get_sections();
    void scroll_to_section(int p_section_index);

    void popup_search();
    void search_again(bool p_search_previous=false);

    const char * get_class() const override;

    void set_focused();

    int get_scroll() const;
    void set_scroll(int p_scroll);

    EditorHelp();
    ~EditorHelp() override;
};

class EditorHelpBit : public PanelContainer {

    GDCLASS(EditorHelpBit,PanelContainer)

    RichTextLabel *rich_text;
    void _go_to_help(const StringName &p_what);
    void _meta_clicked(se_string_view p_select);

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    RichTextLabel *get_rich_text() { return rich_text; }
    void set_text(se_string_view p_text);
    EditorHelpBit();
};

#endif // EDITOR_HELP_H
