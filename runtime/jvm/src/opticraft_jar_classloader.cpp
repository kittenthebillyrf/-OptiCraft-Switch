#include "opticraft_jar_classloader.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <new>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
uint16_t le16(const uint8_t *p) {
    return static_cast<uint16_t>(p[0] | (uint16_t(p[1]) << 8));
}
uint32_t le32(const uint8_t *p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
bool range_ok(size_t at, size_t count, size_t total) {
    return at <= total && count <= total - at;
}
struct ZipEntry {
    std::string name;
    uint16_t flags = 0;
    uint16_t method = 0;
    uint32_t crc = 0;
    uint32_t packed_size = 0;
    uint32_t unpacked_size = 0;
    uint32_t local_offset = 0;
};

bool find_zip_entry(const std::vector<uint8_t> &zip, const std::string &wanted,
                    ZipEntry &result, std::string &error) {
    if (zip.size() < 22) {
        error = "JAR is too small";
        return false;
    }
    const size_t lower = zip.size() > 65557 ? zip.size() - 65557 : 0;
    size_t eocd = zip.size();
    for (size_t p = zip.size() - 22;; --p) {
        if (le32(&zip[p]) == 0x06054b50u &&
            range_ok(p, 22u + le16(&zip[p + 20]), zip.size()) &&
            p + 22u + le16(&zip[p + 20]) == zip.size()) {
            eocd = p;
            break;
        }
        if (p == lower) break;
    }
    if (eocd == zip.size()) {
        error = "ZIP end-of-central-directory not found";
        return false;
    }

    const uint16_t disk = le16(&zip[eocd + 4]);
    const uint16_t directory_disk = le16(&zip[eocd + 6]);
    const uint16_t disk_entries = le16(&zip[eocd + 8]);
    const uint16_t total_entries = le16(&zip[eocd + 10]);
    const uint32_t directory_size = le32(&zip[eocd + 12]);
    const uint32_t directory_offset = le32(&zip[eocd + 16]);
    if (disk != 0 || directory_disk != 0 || disk_entries != total_entries ||
        total_entries == 0xffff || directory_size == 0xffffffffu ||
        directory_offset == 0xffffffffu ||
        !range_ok(directory_offset, directory_size, zip.size()) ||
        size_t(directory_offset) + directory_size > eocd) {
        error = "multi-disk, ZIP64 or malformed archive is unsupported";
        return false;
    }

    size_t cursor = directory_offset;
    for (uint32_t i = 0; i < total_entries; ++i) {
        if (!range_ok(cursor, 46, zip.size()) ||
            le32(&zip[cursor]) != 0x02014b50u) {
            error = "invalid ZIP central directory entry";
            return false;
        }
        ZipEntry entry;
        entry.flags = le16(&zip[cursor + 8]);
        entry.method = le16(&zip[cursor + 10]);
        entry.crc = le32(&zip[cursor + 16]);
        entry.packed_size = le32(&zip[cursor + 20]);
        entry.unpacked_size = le32(&zip[cursor + 24]);
        entry.local_offset = le32(&zip[cursor + 42]);
        const uint16_t name_length = le16(&zip[cursor + 28]);
        const uint16_t extra_length = le16(&zip[cursor + 30]);
        const uint16_t comment_length = le16(&zip[cursor + 32]);
        const size_t record_length = 46u + name_length + extra_length + comment_length;
        if (!range_ok(cursor, record_length, zip.size())) {
            error = "truncated ZIP central directory entry";
            return false;
        }
        entry.name.assign(reinterpret_cast<const char *>(&zip[cursor + 46]),
                          name_length);
        if (entry.name == wanted) {
            result = entry;
            return true;
        }
        cursor += record_length;
    }
    error = "class entry not found in JAR";
    return false;
}

bool extract_entry(const std::vector<uint8_t> &zip, const ZipEntry &entry,
                   std::vector<uint8_t> &output, std::string &error) {
    if ((entry.flags & 1u) != 0) {
        error = "encrypted ZIP entries are unsupported";
        return false;
    }
    if (entry.method != 0 && entry.method != 8) {
        error = "unsupported ZIP compression method";
        return false;
    }
    if (entry.unpacked_size > 64u * 1024u * 1024u ||
        entry.packed_size > std::numeric_limits<uInt>::max() ||
        entry.unpacked_size > std::numeric_limits<uInt>::max()) {
        error = "class entry exceeds safe size limit";
        return false;
    }
    if (!range_ok(entry.local_offset, 30, zip.size()) ||
        le32(&zip[entry.local_offset]) != 0x04034b50u) {
        error = "invalid local ZIP header";
        return false;
    }
    const uint16_t local_name_length = le16(&zip[entry.local_offset + 26]);
    const uint16_t local_extra_length = le16(&zip[entry.local_offset + 28]);
    const size_t data_offset = size_t(entry.local_offset) + 30u +
                               local_name_length + local_extra_length;
    if (!range_ok(size_t(entry.local_offset) + 30u,
                  size_t(local_name_length) + local_extra_length, zip.size()) ||
        !range_ok(data_offset, entry.packed_size, zip.size())) {
        error = "class payload exceeds archive bounds";
        return false;
    }
    const std::string local_name(
        reinterpret_cast<const char *>(&zip[entry.local_offset + 30]),
        local_name_length);
    if (local_name != entry.name) {
        error = "local and central directory names differ";
        return false;
    }

    output.assign(entry.unpacked_size, 0);
    if (entry.method == 0) {
        if (entry.packed_size != entry.unpacked_size) {
            error = "stored entry size mismatch";
            return false;
        }
        if (!output.empty()) {
            std::copy(zip.begin() + data_offset,
                      zip.begin() + data_offset + entry.packed_size,
                      output.begin());
        }
    } else {
        z_stream stream{};
        stream.next_in = const_cast<Bytef *>(
            reinterpret_cast<const Bytef *>(&zip[data_offset]));
        stream.avail_in = static_cast<uInt>(entry.packed_size);
        uint8_t empty_output = 0;
        stream.next_out = output.empty()
            ? &empty_output : reinterpret_cast<Bytef *>(output.data());
        stream.avail_out = output.empty()
            ? 1u : static_cast<uInt>(output.size());
        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
            error = "raw DEFLATE initialization failed";
            return false;
        }
        const int status = inflate(&stream, Z_FINISH);
        const bool valid = status == Z_STREAM_END &&
                           stream.total_in == entry.packed_size &&
                           stream.total_out == entry.unpacked_size;
        inflateEnd(&stream);
        if (!valid) {
            error = "invalid or truncated DEFLATE stream";
            return false;
        }
    }

    const uLong crc = crc32(
        0L, output.empty() ? Z_NULL :
            reinterpret_cast<const Bytef *>(output.data()),
        static_cast<uInt>(output.size()));
    if (static_cast<uint32_t>(crc) != entry.crc) {
        error = "class entry CRC32 mismatch";
        return false;
    }
    return true;
}
} // namespace

struct OptiJar {
    std::vector<uint8_t> bytes;
    std::string error;
};

struct OptiLoadedClass {
    OptiClassFile *parsed = nullptr;
};

extern "C" int opticraft_jar_open_file(const char *path,
                                         size_t max_archive_bytes,
                                         OptiJar **out_jar) {
    if (out_jar == nullptr) return 0;
    *out_jar = nullptr;
    if (path == nullptr || path[0] == '\0') return 0;
    if (max_archive_bytes == 0) max_archive_bytes = 128u * 1024u * 1024u;

    OptiJar *jar = new (std::nothrow) OptiJar();
    if (jar == nullptr) return 0;
    try {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) {
            jar->error = "cannot open JAR file";
            delete jar;
            return 0;
        }
        const std::streamoff end = input.tellg();
        if (end < 22 || static_cast<uint64_t>(end) > max_archive_bytes) {
            jar->error = "JAR size is outside configured limits";
            delete jar;
            return 0;
        }
        jar->bytes.resize(static_cast<size_t>(end));
        input.seekg(0, std::ios::beg);
        if (!input.read(reinterpret_cast<char *>(jar->bytes.data()), end)) {
            jar->error = "failed to read complete JAR file";
            delete jar;
            return 0;
        }
    } catch (...) {
        delete jar;
        return 0;
    }
    *out_jar = jar;
    return 1;
}

extern "C" void opticraft_jar_close(OptiJar *jar) {
    delete jar;
}

extern "C" int opticraft_jar_load_class(OptiJar *jar,
                                          const char *internal_name,
                                          OptiLoadedClass **out_class) {
    if (out_class == nullptr) return 0;
    *out_class = nullptr;
    if (jar == nullptr || internal_name == nullptr ||
        internal_name[0] == '\0' || internal_name[0] == '/' ||
        std::string(internal_name).find("..") != std::string::npos) {
        return 0;
    }
    jar->error.clear();
    try {
        ZipEntry entry;
        const std::string path = std::string(internal_name) + ".class";
        if (!find_zip_entry(jar->bytes, path, entry, jar->error)) return 0;

        std::vector<uint8_t> class_bytes;
        if (!extract_entry(jar->bytes, entry, class_bytes, jar->error)) return 0;

        OptiLoadedClass *loaded = new (std::nothrow) OptiLoadedClass();
        if (loaded == nullptr) {
            jar->error = "out of memory allocating class object";
            return 0;
        }
        if (!opticraft_classfile_parse(class_bytes.data(), class_bytes.size(),
                                       &loaded->parsed)) {
            jar->error = "classfile parser rejected class entry";
            delete loaded;
            return 0;
        }

        const char *actual_name = nullptr;
        uint16_t actual_length = 0;
        if (!opticraft_classfile_class_name(loaded->parsed, 0,
                                            &actual_name, &actual_length) ||
            std::string(actual_name, actual_length) != internal_name) {
            jar->error = "requested class name does not match classfile this_class";
            opticraft_classfile_destroy(loaded->parsed);
            delete loaded;
            return 0;
        }
        *out_class = loaded;
        return 1;
    } catch (...) {
        jar->error = "exception while loading class entry";
        return 0;
    }
}

extern "C" void opticraft_loaded_class_destroy(OptiLoadedClass *loaded) {
    if (loaded == nullptr) return;
    opticraft_classfile_destroy(loaded->parsed);
    delete loaded;
}

extern "C" int opticraft_loaded_class_summary(const OptiLoadedClass *loaded,
                                                OptiClassSummary *summary) {
    return loaded != nullptr &&
           opticraft_classfile_summary(loaded->parsed, summary);
}

extern "C" int opticraft_loaded_class_name(const OptiLoadedClass *loaded,
                                             int which, const char **text,
                                             uint16_t *length) {
    return loaded != nullptr &&
           opticraft_classfile_class_name(loaded->parsed, which, text, length);
}

extern "C" int opticraft_loaded_class_find_method(
    const OptiLoadedClass *loaded, const char *name, const char *descriptor,
    uint16_t *access_flags, uint32_t *code_length) {
    return loaded != nullptr &&
           opticraft_classfile_find_method(loaded->parsed, name, descriptor,
                                           access_flags, code_length);
}

extern "C" const char *opticraft_jar_last_error(const OptiJar *jar) {
    return jar == nullptr ? "invalid JAR handle" : jar->error.c_str();
}
