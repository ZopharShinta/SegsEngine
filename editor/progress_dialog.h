/*************************************************************************/
/*  progress_dialog.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/popup.h"
#include "scene/gui/progress_bar.h"
#include "core/os/thread_safe.h"

class BackgroundProgress : public HBoxContainer {

    GDCLASS(BackgroundProgress,HBoxContainer)

    _THREAD_SAFE_CLASS_

    struct Task {

        HBoxContainer *hb;
        ProgressBar *progress;
    };

    Map<StringName, Task> tasks;
    Map<StringName, int> updates;
    void _update();

protected:
    void _add_task(const StringName &p_task, const StringName &p_label, int p_steps);
    void _task_step(const StringName &p_task, int p_step = -1);
    void _end_task(const StringName &p_task);

    static void _bind_methods();

public:
    void add_task(const StringName &p_task, const StringName &p_label, int p_steps);
    void task_step(const StringName &p_task, int p_step = -1);
    void end_task(const StringName &p_task);

    BackgroundProgress();
    ~BackgroundProgress() override;
};

class ProgressDialog : public Popup {

    GDCLASS(ProgressDialog,Popup)

    struct Task {

        se_string task;
        VBoxContainer *vb;
        ProgressBar *progress;
        Label *state;
    };
    HBoxContainer *cancel_hb;
    Button *cancel;

    Map<StringName, Task> tasks;
    VBoxContainer *main;
    uint64_t last_progress_tick;

    static ProgressDialog *singleton;
    void _popup();

    void _cancel_pressed();
    bool cancelled;

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    static ProgressDialog *get_singleton() { return singleton; }
    void add_task(const StringName &p_task, const StringName &p_label, int p_steps, bool p_can_cancel = false);
    bool task_step(const StringName &p_task, const StringName &p_state, int p_step = -1, bool p_force_redraw = true);
    bool task_step(const StringName &p_task, se_string_view p_state, int p_step = -1, bool p_force_redraw = true);
    void end_task(const StringName &p_task);

    ProgressDialog();
};

#endif // PROGRESS_DIALOG_H
