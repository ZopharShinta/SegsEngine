/*************************************************************************/
/*  power_x11.cpp                                                        */
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

/*
Adapted from corresponding SDL 2.0 code.
*/

/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "power_x11.h"

#include <cstdio>
#include <unistd.h>

#include "core/error_macros.h"
#include "core/string_utils.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace eastl;
// CODE CHUNK IMPORTED FROM SDL 2.0

static const char *proc_apm_path = "/proc/apm";
static const char *proc_acpi_battery_path = "/proc/acpi/battery";
static const char *proc_acpi_ac_adapter_path = "/proc/acpi/ac_adapter";
static const char *sys_class_power_supply_path = "/sys/class/power_supply";

FileAccessRef PowerX11::open_power_file(const char *base, const char *node, const char *key) {
    se_string path = se_string(base) + "/" + se_string(node) + "/" + se_string(key);
    FileAccessRef f = FileAccess::open(path, FileAccess::READ);
    return f;
}

bool PowerX11::read_power_file(const char *base, const char *node, const char *key, char *buf, size_t buflen) {
    ssize_t br = 0;
    FileAccessRef fd = open_power_file(base, node, key);
    if (!fd) {
        return false;
    }
    br = fd->get_buffer(reinterpret_cast<uint8_t *>(buf), buflen - 1);
    fd->close();
    if (br < 0) {
        return false;
    }
    buf[br] = '\0'; // null-terminate the string
    return true;
}

bool PowerX11::make_proc_acpi_key_val(char **_ptr, char **_key, char **_val) {
    char *ptr = *_ptr;

    while (*ptr == ' ') {
        ptr++; /* skip whitespace. */
    }

    if (*ptr == '\0') {
        return false; /* EOF. */
    }

    *_key = ptr;

    while ((*ptr != ':') && (*ptr != '\0')) {
        ptr++;
    }

    if (*ptr == '\0') {
        return false; /* (unexpected) EOF. */
    }

    *(ptr++) = '\0'; /* terminate the key. */

    while (*ptr == ' ') {
        ptr++; /* skip whitespace. */
    }

    if (*ptr == '\0') {
        return false; /* (unexpected) EOF. */
    }

    *_val = ptr;

    while ((*ptr != '\n') && (*ptr != '\0')) {
        ptr++;
    }

    if (*ptr != '\0') {
        *(ptr++) = '\0'; /* terminate the value. */
    }

    *_ptr = ptr; /* store for next time. */
    return true;
}

void PowerX11::check_proc_acpi_battery(const char *node, bool *have_battery, bool *charging) {
    const char *base = proc_acpi_battery_path;
    char info[1024];
    char state[1024];
    char *ptr = nullptr;
    char *key = nullptr;
    char *val = nullptr;
    bool charge = false;
    bool choose = false;
    int maximum = -1;
    int remaining = -1;
    int secs = -1;
    int pct = -1;

    if (!read_power_file(base, node, "state", state, sizeof(state))) {
        return;
    } else {
        if (!read_power_file(base, node, "info", info, sizeof(info)))
            return;
    }

    ptr = &state[0];
    while (make_proc_acpi_key_val(&ptr, &key, &val)) {
        se_string_view key_sv(key);
        se_string_view val_sv(val);
        if (key_sv == "present"_sv) {
            if (val_sv == "yes"_sv) {
                *have_battery = true;
            }
        } else if (key_sv == "charging state"_sv) {
            /* !!! FIXME: what exactly _does_ charging/discharging mean? */
            if (val_sv == "charging/discharging"_sv) {
                charge = true;
            } else if (val_sv == "charging"_sv) {
                charge = true;
            }
        } else if (key_sv == "remaining capacity"_sv) {
            const int cvt = StringUtils::to_int(val_sv);
            remaining = cvt;
        }
    }

    ptr = &info[0];
    while (make_proc_acpi_key_val(&ptr, &key, &val)) {
        se_string_view key_sv(key);
        if (key_sv == "design capacity"_sv) {
            se_string_view sval(val);
            const int cvt = StringUtils::to_int(sval);
            maximum = cvt;
        }
    }

    if ((maximum >= 0) && (remaining >= 0)) {
        pct = (int)((((float)remaining) / ((float)maximum)) * 100.0f);
        if (pct < 0) {
            pct = 0;
        } else if (pct > 100) {
            pct = 100;
        }
    }

    /* !!! FIXME: calculate (secs). */

    /*
     * We pick the battery that claims to have the most minutes left.
     *  (failing a report of minutes, we'll take the highest percent.)
     */
    // -- GODOT start --
    //if ((secs < 0) && (this->nsecs_left < 0)) {
    if (this->nsecs_left < 0) {
        // -- GODOT end --
        if ((pct < 0) && (this->percent_left < 0)) {
            choose = true; /* at least we know there's a battery. */
        }
        if (pct > this->percent_left) {
            choose = true;
        }
    } else if (secs > this->nsecs_left) {
        choose = true;
    }

    if (choose) {
        this->nsecs_left = secs;
        this->percent_left = pct;
        *charging = charge;
    }
}

void PowerX11::check_proc_acpi_ac_adapter(const char *node, bool *have_ac) {
    const char *base = proc_acpi_ac_adapter_path;
    char state[256];
    char *ptr = nullptr;
    char *key = nullptr;
    char *val = nullptr;

    if (!read_power_file(base, node, "state", state, sizeof(state))) {
        return;
    }

    ptr = &state[0];
    while (make_proc_acpi_key_val(&ptr, &key, &val)) {
        se_string_view skey(key);
        if (skey == "state"_sv) {
            se_string_view sval(val);
            if (sval == "on-line"_sv) {
                *have_ac = true;
            }
        }
    }
}

bool PowerX11::GetPowerInfo_Linux_proc_acpi() {
    se_string node;
    DirAccess *dirp = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
    bool have_battery = false;
    bool have_ac = false;
    bool charging = false;

    this->nsecs_left = -1;
    this->percent_left = -1;
    this->power_state = OS::POWERSTATE_UNKNOWN;

    dirp->change_dir_utf8(proc_acpi_battery_path);
    Error err = dirp->list_dir_begin();

    if (err != OK) {
        return false; /* can't use this interface. */
    } else {
        node = dirp->get_next();
        while (!node.empty()) {
            check_proc_acpi_battery(node.c_str(), &have_battery, &charging /*, seconds, percent*/);
            node = dirp->get_next();
        }
    }
    dirp->change_dir_utf8(proc_acpi_ac_adapter_path);
    err = dirp->list_dir_begin();
    if (err != OK) {
        return false; /* can't use this interface. */
    } else {
        node = dirp->get_next();
        while (!node.empty()) {
            check_proc_acpi_ac_adapter(node.c_str(), &have_ac);
            node = dirp->get_next();
        }
    }

    if (!have_battery) {
        this->power_state = OS::POWERSTATE_NO_BATTERY;
    } else if (charging) {
        this->power_state = OS::POWERSTATE_CHARGING;
    } else if (have_ac) {
        this->power_state = OS::POWERSTATE_CHARGED;
    } else {
        this->power_state = OS::POWERSTATE_ON_BATTERY;
    }

    memdelete(dirp);
    return true; /* definitive answer. */
}

bool PowerX11::next_string(char **_ptr, char **_str) {
    char *ptr = *_ptr;
    char *str = *_str;

    while (*ptr == ' ') { /* skip any spaces... */
        ptr++;
    }

    if (*ptr == '\0') {
        return false;
    }

    str = ptr;
    while ((*ptr != ' ') && (*ptr != '\n') && (*ptr != '\0'))
        ptr++;

    if (*ptr != '\0')
        *(ptr++) = '\0';

    *_str = str;
    *_ptr = ptr;
    return true;
}

bool PowerX11::int_string(char *str, int *val) {
    se_string_view sval(str);
    *val = StringUtils::to_int(sval);
    return (*str != '\0');
}

/* http://lxr.linux.no/linux+v2.6.29/drivers/char/apm-emulation.c */
bool PowerX11::GetPowerInfo_Linux_proc_apm() {
    using namespace eastl; // for _sv suffix
    bool need_details = false;
    int ac_status = 0;
    int battery_status = 0;
    int battery_flag = 0;
    int battery_percent = 0;
    int battery_time = 0;
    FileAccessRef fd = FileAccess::open(se_string_view(proc_apm_path), FileAccess::READ);
    char buf[128];
    char *ptr = &buf[0];
    char *str = nullptr;
    ssize_t br;

    if (!fd) {
        return false; /* can't use this interface. */
    }

    br = fd->get_buffer(reinterpret_cast<uint8_t *>(buf), sizeof(buf) - 1);
    fd->close();

    if (br < 0) {
        return false;
    }

    buf[br] = '\0'; /* null-terminate the string. */
    if (!next_string(&ptr, &str)) { /* driver version */
        return false;
    }
    if (!next_string(&ptr, &str)) { /* BIOS version */
        return false;
    }
    if (!next_string(&ptr, &str)) { /* APM flags */
        return false;
    }

    if (!next_string(&ptr, &str)) { /* AC line status */
        return false;
    } else if (!int_string(str, &ac_status)) {
        return false;
    }

    if (!next_string(&ptr, &str)) { /* battery status */
        return false;
    } else if (!int_string(str, &battery_status)) {
        return false;
    }
    if (!next_string(&ptr, &str)) { /* battery flag */
        return false;
    } else if (!int_string(str, &battery_flag)) {
        return false;
    }
    if (!next_string(&ptr, &str)) { /* remaining battery life percent */
        return false;
    }
//    String sstr = str;
//    if (sstr[sstr.length() - 1] == '%') {
//        sstr[sstr.length() - 1] = '\0';
//    }
    if (!int_string(str, &battery_percent)) {
        return false;
    }

    if (!next_string(&ptr, &str)) { /* remaining battery life time */
        return false;
    } else if (!int_string(str, &battery_time)) {
        return false;
    }

    if (!next_string(&ptr, &str)) { /* remaining battery life time units */
        return false;
    } else if (se_string_view(str) == "min"_sv) {
        battery_time *= 60;
    }

    if (battery_flag == 0xFF) { /* unknown state */
        this->power_state = OS::POWERSTATE_UNKNOWN;
    } else if (battery_flag & (1 << 7)) { /* no battery */
        this->power_state = OS::POWERSTATE_NO_BATTERY;
    } else if (battery_flag & (1 << 3)) { /* charging */
        this->power_state = OS::POWERSTATE_CHARGING;
        need_details = true;
    } else if (ac_status == 1) {
        this->power_state = OS::POWERSTATE_CHARGED; /* on AC, not charging. */
        need_details = true;
    } else {
        this->power_state = OS::POWERSTATE_ON_BATTERY;
        need_details = true;
    }

    this->percent_left = -1;
    this->nsecs_left = -1;
    if (need_details) {
        const int pct = battery_percent;
        const int secs = battery_time;

        if (pct >= 0) { /* -1 == unknown */
            this->percent_left = (pct > 100) ? 100 : pct; /* clamp between 0%, 100% */
        }
        if (secs >= 0) { /* -1 == unknown */
            this->nsecs_left = secs;
        }
    }

    return true;
}

/* !!! FIXME: implement d-bus queries to org.freedesktop.UPower. */

bool PowerX11::GetPowerInfo_Linux_sys_class_power_supply(/*PowerState *state, int *seconds, int *percent*/) {
    using namespace eastl; // for _sv suffix

    const char *base = sys_class_power_supply_path;
    se_string name;

    DirAccess *dirp = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
    dirp->change_dir_utf8(base);
    Error err = dirp->list_dir_begin();

    if (err != OK) {
        return false;
    }

    this->power_state = OS::POWERSTATE_NO_BATTERY; /* assume we're just plugged in. */
    this->nsecs_left = -1;
    this->percent_left = -1;

    name = dirp->get_next();

    while (!name.empty()) {
        bool choose = false;
        char str[64];
        OS::PowerState st;
        int secs;
        int pct;

        if ((name == ".") || (name == "..")) {
            name = dirp->get_next();
            continue; //skip these, of course.
        } else {
            if (!read_power_file(base, name.c_str(), "type", str, sizeof(str))) {
                name = dirp->get_next();
                continue; // Don't know _what_ we're looking at. Give up on it.
            } else {
                if (se_string_view(str) != "Battery\n"_sv) {
                    name = dirp->get_next();
                    continue; // we don't care about UPS and such.
                }
            }
        }
        /* some drivers don't offer this, so if it's not explicitly reported assume it's present. */
        if (read_power_file(base, name.c_str(), "present", str, sizeof(str)) && (se_string_view(str) == "0\n"_sv)) {
            st = OS::POWERSTATE_NO_BATTERY;
        } else if (!read_power_file(base, name.c_str(), "status", str, sizeof(str))) {
            st = OS::POWERSTATE_UNKNOWN; /* uh oh */
        } else if (se_string_view(str) == "Charging\n"_sv) {
            st = OS::POWERSTATE_CHARGING;
        } else if (se_string_view(str) == "Discharging\n"_sv) {
            st = OS::POWERSTATE_ON_BATTERY;
        } else if ((se_string_view(str) == "Full\n"_sv) || (se_string_view(str) == "Not charging\n"_sv)) {
            st = OS::POWERSTATE_CHARGED;
        } else {
            st = OS::POWERSTATE_UNKNOWN; /* uh oh */
        }

        if (!read_power_file(base, name.c_str(), "capacity", str, sizeof(str))) {
            pct = -1;
        } else {
            pct = StringUtils::to_int(str);
            pct = (pct > 100) ? 100 : pct; /* clamp between 0%, 100% */
        }

        if (!read_power_file(base, name.c_str(), "time_to_empty_now", str, sizeof(str))) {
            secs = -1;
        } else {
            secs = StringUtils::to_int(str);
            secs = (secs <= 0) ? -1 : secs; /* 0 == unknown */
        }

        /*
         * We pick the battery that claims to have the most minutes left.
         *  (failing a report of minutes, we'll take the highest percent.)
         */
        if ((secs < 0) && (this->nsecs_left < 0)) {
            if ((pct < 0) && (this->percent_left < 0)) {
                choose = true; /* at least we know there's a battery. */
            } else if (pct > this->percent_left) {
                choose = true;
            }
        } else if (secs > this->nsecs_left) {
            choose = true;
        }

        if (choose) {
            this->nsecs_left = secs;
            this->percent_left = pct;
            this->power_state = st;
        }

        name = dirp->get_next();
    }

    memdelete(dirp);
    return true; /* don't look any further*/
}

bool PowerX11::UpdatePowerInfo() {
    if (GetPowerInfo_Linux_sys_class_power_supply()) { // try method 1
        return true;
    }
    if (GetPowerInfo_Linux_proc_acpi()) { // try further
        return true;
    }
    if (GetPowerInfo_Linux_proc_apm()) { // try even further
        return true;
    }
    return false;
}

PowerX11::PowerX11() :
        nsecs_left(-1),
        percent_left(-1),
        power_state(OS::POWERSTATE_UNKNOWN) {
}

PowerX11::~PowerX11() {
}

OS::PowerState PowerX11::get_power_state() {
    if (UpdatePowerInfo()) {
        return power_state;
    } else {
        return OS::POWERSTATE_UNKNOWN;
    }
}

int PowerX11::get_power_seconds_left() {
    if (UpdatePowerInfo()) {
        return nsecs_left;
    } else {
        return -1;
    }
}

int PowerX11::get_power_percent_left() {
    if (UpdatePowerInfo()) {
        return percent_left;
    } else {
        return -1;
    }
}
