#include "world_save_preview.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
struct Cursor {
    const unsigned char* data;
    size_t size;
    size_t offset;
};

bool take(Cursor& cursor, size_t count, const unsigned char*& result) {
    if (cursor.offset > cursor.size || count > cursor.size - cursor.offset) return false;
    result = cursor.data + cursor.offset;
    cursor.offset += count;
    return true;
}

bool read_u8(Cursor& cursor, uint8_t& value) {
    const unsigned char* bytes = nullptr;
    if (!take(cursor, 1, bytes)) return false;
    value = bytes[0];
    return true;
}

bool read_u16(Cursor& cursor, uint16_t& value) {
    const unsigned char* bytes = nullptr;
    if (!take(cursor, 2, bytes)) return false;
    value = static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) | bytes[1]);
    return true;
}

bool read_i32(Cursor& cursor, int32_t& value) {
    const unsigned char* bytes = nullptr;
    if (!take(cursor, 4, bytes)) return false;
    const uint32_t raw = (static_cast<uint32_t>(bytes[0]) << 24) |
                         (static_cast<uint32_t>(bytes[1]) << 16) |
                         (static_cast<uint32_t>(bytes[2]) << 8) |
                         static_cast<uint32_t>(bytes[3]);
    value = static_cast<int32_t>(raw);
    return true;
}

bool read_i64(Cursor& cursor, int64_t& value) {
    const unsigned char* bytes = nullptr;
    if (!take(cursor, 8, bytes)) return false;
    uint64_t raw = 0;
    for (int i = 0; i < 8; ++i) raw = (raw << 8) | bytes[i];
    value = static_cast<int64_t>(raw);
    return true;
}

bool read_string(Cursor& cursor, std::string& value) {
    uint16_t length = 0;
    const unsigned char* bytes = nullptr;
    if (!read_u16(cursor, length) || !take(cursor, length, bytes)) return false;
    value.assign(reinterpret_cast<const char*>(bytes), length);
    return true;
}

bool skip_payload(Cursor& cursor, uint8_t type, unsigned depth) {
    if (depth > 64) return false;

    int32_t length = 0;
    const unsigned char* ignored = nullptr;

    switch (type) {
        case 1: return take(cursor, 1, ignored);
        case 2: return take(cursor, 2, ignored);
        case 3: return take(cursor, 4, ignored);
        case 4: return take(cursor, 8, ignored);
        case 5: return take(cursor, 4, ignored);
        case 6: return take(cursor, 8, ignored);
        case 7:
            if (!read_i32(cursor, length) || length < 0 || length > 16 * 1024 * 1024) return false;
            return take(cursor, static_cast<size_t>(length), ignored);
        case 8: {
            std::string ignored_string;
            return read_string(cursor, ignored_string);
        }
        case 9: {
            uint8_t child_type = 0;
            if (!read_u8(cursor, child_type) || !read_i32(cursor, length) ||
                length < 0 || length > 1000000) return false;
            if (child_type == 0) return length == 0;
            for (int32_t i = 0; i < length; ++i) {
                if (!skip_payload(cursor, child_type, depth + 1)) return false;
            }
            return true;
        }
        case 10:
            for (;;) {
                uint8_t child_type = 0;
                if (!read_u8(cursor, child_type)) return false;
                if (child_type == 0) return true;
                std::string child_name;
                if (!read_string(cursor, child_name) ||
                    !skip_payload(cursor, child_type, depth + 1)) return false;
            }
        case 11:
        case 12: {
            if (!read_i32(cursor, length) || length < 0 || length > 4000000) return false;
            const size_t element_size = type == 11 ? 4u : 8u;
            return take(cursor, static_cast<size_t>(length) * element_size, ignored);
        }
        default:
            return false;
    }
}

bool parse_data_compound(Cursor& cursor, OpticraftWorldPreview& output) {
    for (;;) {
        uint8_t type = 0;
        if (!read_u8(cursor, type)) return false;
        if (type == 0) return true;

        std::string name;
        if (!read_string(cursor, name)) return false;

        if (name == "LevelName" && type == 8) {
            std::string value;
            if (!read_string(cursor, value)) return false;
            size_t count = value.size();
            if (count >= sizeof(output.level_name)) count = sizeof(output.level_name) - 1;
            std::memcpy(output.level_name, value.data(), count);
            output.level_name[count] = 0;
            output.has_level_name = 1;
        } else if (name == "Time" && type == 4) {
            if (!read_i64(cursor, output.time)) return false;
            output.has_time = 1;
        } else if ((name == "SpawnX" || name == "SpawnY" || name == "SpawnZ") && type == 3) {
            int32_t value = 0;
            if (!read_i32(cursor, value)) return false;
            if (name == "SpawnX") output.spawn_x = value;
            else if (name == "SpawnY") output.spawn_y = value;
            else output.spawn_z = value;
            output.has_spawn = 1;
        } else if (!skip_payload(cursor, type, 0)) {
            return false;
        }
    }
}

bool parse_nbt(const std::vector<unsigned char>& bytes, OpticraftWorldPreview& output) {
    Cursor cursor{bytes.data(), bytes.size(), 0};
    uint8_t root_type = 0;
    if (!read_u8(cursor, root_type) || root_type != 10) return false;

    std::string root_name;
    if (!read_string(cursor, root_name)) return false;

    for (;;) {
        uint8_t type = 0;
        if (!read_u8(cursor, type)) return false;
        if (type == 0) return output.has_level_name != 0;

        std::string name;
        if (!read_string(cursor, name)) return false;
        if (name == "Data" && type == 10) {
            if (!parse_data_compound(cursor, output)) return false;
        } else if (!skip_payload(cursor, type, 0)) {
            return false;
        }
    }
}

bool read_file(const char* path, std::vector<unsigned char>& bytes) {
    FILE* file = std::fopen(path, "rb");
    if (!file) return false;
    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return false;
    }
    const long length = std::ftell(file);
    if (length <= 0 || length > 2 * 1024 * 1024) {
        std::fclose(file);
        return false;
    }
    if (std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file);
        return false;
    }
    bytes.resize(static_cast<size_t>(length));
    const bool ok = std::fread(bytes.data(), 1, bytes.size(), file) == bytes.size();
    std::fclose(file);
    return ok;
}

bool decompress_gzip(const std::vector<unsigned char>& input,
                     std::vector<unsigned char>& output) {
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());
    if (inflateInit2(&stream, 15 + 32) != Z_OK) return false;

    unsigned char chunk[16384];
    int status = Z_OK;
    while (status == Z_OK) {
        stream.next_out = chunk;
        stream.avail_out = sizeof(chunk);
        status = inflate(&stream, Z_NO_FLUSH);
        const size_t produced = sizeof(chunk) - stream.avail_out;
        if (output.size() + produced > 16u * 1024u * 1024u) {
            inflateEnd(&stream);
            return false;
        }
        output.insert(output.end(), chunk, chunk + produced);
    }
    inflateEnd(&stream);
    return status == Z_STREAM_END;
}
} // namespace

extern "C" int opticraft_world_save_read_preview(const char* world_directory,
                                                   OpticraftWorldPreview* output) {
    if (!world_directory || !output) return -2;
    std::memset(output, 0, sizeof(*output));

    const char* filenames[] = {"level.dat", "level.dat_old"};
    for (const char* filename : filenames) {
        char path[512];
        const int length = std::snprintf(path, sizeof(path), "%s/%s", world_directory, filename);
        if (length < 0 || static_cast<size_t>(length) >= sizeof(path)) return -3;

        std::vector<unsigned char> compressed;
        std::vector<unsigned char> plain;
        if (!read_file(path, compressed)) continue;
        if (!decompress_gzip(compressed, plain)) continue;
        if (parse_nbt(plain, *output)) return 0;
        std::memset(output, 0, sizeof(*output));
    }
    return -1;
}
