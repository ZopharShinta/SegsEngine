/*************************************************************************/
/*  file_access_compressed.h                                             */
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

#pragma once

#include "core/io/compression.h"
#include "core/os/file_access.h"
#include "core/se_string.h"
#include "core/vector.h"

class FileAccessCompressed : public FileAccess {

    Compression::Mode cmode=Compression::MODE_ZSTD;
    bool writing=false;
    uint32_t write_pos;
    uint8_t *write_ptr=nullptr;
    uint32_t write_buffer_size=0;
    uint32_t write_max=0;
    uint32_t block_size=0;
    mutable bool read_eof=false;
    mutable bool at_end=false;

    struct ReadBlock {
        int csize;
        int offset;
    };

    mutable Vector<uint8_t> comp_buffer;
    uint8_t *read_ptr=nullptr;
    mutable int read_block=0;
    int read_block_count=0;
    mutable int read_block_size=0;
    mutable int read_pos=0;
    Vector<ReadBlock> read_blocks;
    uint32_t read_total=0;

    se_string magic;
    mutable Vector<uint8_t> buffer;
    FileAccess *f = nullptr;

public:
    void configure(se_string_view p_magic, Compression::Mode p_mode = Compression::MODE_ZSTD, int p_block_size = 4096);

    Error open_after_magic(FileAccess *p_base);

    Error _open(se_string_view p_path, int p_mode_flags) override; ///< open a file
    void close() override; ///< close a file
    bool is_open() const override; ///< true when file is open

    void seek(size_t p_position) override; ///< seek to a given position
    void seek_end(int64_t p_position = 0) override; ///< seek from the end of file
    size_t get_position() const override; ///< get position in the file
    size_t get_len() const override; ///< get size of the file

    bool eof_reached() const override; ///< reading passed EOF

    uint8_t get_8() const override; ///< get a byte
    int get_buffer(uint8_t *p_dst, int p_length) const override;

    Error get_error() const override; ///< get last error

    void flush() override;
    void store_8(uint8_t p_dest) override; ///< store a byte

    bool file_exists(se_string_view p_name) override; ///< return true if a file exists

    uint64_t _get_modified_time(se_string_view p_file) override;
    uint32_t _get_unix_permissions(se_string_view p_file) override;
    Error _set_unix_permissions(se_string_view p_file, uint32_t p_permissions) override;

    FileAccessCompressed();
    ~FileAccessCompressed() override;
};
