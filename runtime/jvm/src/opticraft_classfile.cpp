#include "opticraft_classfile.h"
#include <new>
#include <string>
#include <vector>

namespace {
bool range_ok(size_t at, size_t n, size_t size) {
    return at <= size && n <= size - at;
}

struct Reader {
    const uint8_t *bytes;
    size_t size;
    size_t pos = 0;

    bool take(size_t count) {
        if (!range_ok(pos, count, size)) return false;
        pos += count;
        return true;
    }
    bool u1(uint8_t &value) {
        if (!take(1)) return false;
        value = bytes[pos - 1];
        return true;
    }
    bool u2(uint16_t &value) {
        if (!take(2)) return false;
        value = static_cast<uint16_t>(
            (static_cast<uint16_t>(bytes[pos - 2]) << 8) | bytes[pos - 1]);
        return true;
    }
    bool u4(uint32_t &value) {
        if (!take(4)) return false;
        value = (static_cast<uint32_t>(bytes[pos - 4]) << 24) |
                (static_cast<uint32_t>(bytes[pos - 3]) << 16) |
                (static_cast<uint32_t>(bytes[pos - 2]) << 8) |
                static_cast<uint32_t>(bytes[pos - 1]);
        return true;
    }
};

struct CpEntry {
    uint8_t tag = 0;
    uint16_t a = 0;
    uint16_t b = 0;
    std::string text;
};

struct MethodEntry {
    uint16_t access = 0;
    uint16_t name = 0;
    uint16_t descriptor = 0;
    uint32_t code_length = 0;
};

bool skip_attributes(Reader &reader, uint16_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        uint16_t name_index;
        uint32_t length;
        if (!reader.u2(name_index) || !reader.u4(length) ||
            !reader.take(static_cast<size_t>(length))) {
            return false;
        }
    }
    return true;
}

bool parse_code_attribute(Reader &reader, uint32_t attribute_length,
                          uint32_t &code_length) {
    const size_t start = reader.pos;
    uint16_t max_stack;
    uint16_t max_locals;
    uint32_t bytecode_length;
    if (!reader.u2(max_stack) || !reader.u2(max_locals) ||
        !reader.u4(bytecode_length) ||
        !reader.take(static_cast<size_t>(bytecode_length))) {
        return false;
    }

    uint16_t exception_count;
    if (!reader.u2(exception_count) ||
        !reader.take(static_cast<size_t>(exception_count) * 8)) {
        return false;
    }
    uint16_t nested_attributes;
    if (!reader.u2(nested_attributes) ||
        !skip_attributes(reader, nested_attributes)) {
        return false;
    }

    if (reader.pos - start != attribute_length) return false;
    code_length = bytecode_length;
    (void)max_stack;
    (void)max_locals;
    return true;
}
} // namespace

struct OptiClassFile {
    OptiClassSummary summary{};
    std::vector<CpEntry> constant_pool;
    std::vector<MethodEntry> methods;
};

extern "C" int opticraft_classfile_parse(const uint8_t *bytes, size_t length,
                                           OptiClassFile **out_file) {
    if (out_file == nullptr) return 0;
    *out_file = nullptr;
    if (bytes == nullptr) return 0;

    Reader reader{bytes, length};
    uint32_t magic;
    uint16_t minor;
    uint16_t major;
    uint16_t cp_count;
    if (!reader.u4(magic) || magic != 0xcafebabeu ||
        !reader.u2(minor) || !reader.u2(major) ||
        !reader.u2(cp_count) || cp_count == 0) {
        return 0;
    }

    OptiClassFile *file = new (std::nothrow) OptiClassFile();
    if (file == nullptr) return 0;

    try {
        file->constant_pool.resize(cp_count);
        for (uint32_t i = 1; i < cp_count; ++i) {
            uint8_t tag;
            if (!reader.u1(tag)) {
                delete file;
                return 0;
            }
            CpEntry &entry = file->constant_pool[i];
            entry.tag = tag;
            uint32_t value;
            switch (tag) {
            case 1: {
                uint16_t text_length;
                if (!reader.u2(text_length) ||
                    !range_ok(reader.pos, text_length, reader.size)) {
                    delete file;
                    return 0;
                }
                entry.text.assign(
                    reinterpret_cast<const char *>(reader.bytes + reader.pos),
                    text_length);
                if (!reader.take(text_length)) {
                    delete file;
                    return 0;
                }
                break;
            }
            case 3:
            case 4:
                if (!reader.u4(value)) {
                    delete file;
                    return 0;
                }
                entry.a = static_cast<uint16_t>(value >> 16);
                entry.b = static_cast<uint16_t>(value);
                break;
            case 5:
            case 6:
                if (!reader.take(8) || ++i >= cp_count) {
                    delete file;
                    return 0;
                }
                break;
            case 7:
            case 8:
            case 16:
            case 19:
            case 20:
                if (!reader.u2(entry.a)) {
                    delete file;
                    return 0;
                }
                break;
            case 9:
            case 10:
            case 11:
            case 12:
            case 17:
            case 18:
                if (!reader.u2(entry.a) || !reader.u2(entry.b)) {
                    delete file;
                    return 0;
                }
                break;
            case 15: {
                uint8_t reference_kind;
                if (!reader.u1(reference_kind) || !reader.u2(entry.a)) {
                    delete file;
                    return 0;
                }
                entry.b = reference_kind;
                break;
            }
            default:
                delete file;
                return 0;
            }
        }

        uint16_t access_flags;
        uint16_t this_class;
        uint16_t super_class;
        uint16_t interfaces_count;
        uint16_t fields_count;
        if (!reader.u2(access_flags) || !reader.u2(this_class) ||
            !reader.u2(super_class) || !reader.u2(interfaces_count) ||
            !reader.take(static_cast<size_t>(interfaces_count) * 2) ||
            !reader.u2(fields_count)) {
            delete file;
            return 0;
        }

        for (uint32_t i = 0; i < fields_count; ++i) {
            uint16_t field_access;
            uint16_t name_index;
            uint16_t descriptor_index;
            uint16_t attributes_count;
            if (!reader.u2(field_access) || !reader.u2(name_index) ||
                !reader.u2(descriptor_index) ||
                !reader.u2(attributes_count) ||
                !skip_attributes(reader, attributes_count)) {
                delete file;
                return 0;
            }
            (void)field_access;
            (void)name_index;
            (void)descriptor_index;
        }

        uint16_t methods_count;
        if (!reader.u2(methods_count)) {
            delete file;
            return 0;
        }
        file->methods.reserve(methods_count);

        for (uint32_t i = 0; i < methods_count; ++i) {
            MethodEntry method{};
            uint16_t attributes_count;
            if (!reader.u2(method.access) || !reader.u2(method.name) ||
                !reader.u2(method.descriptor) ||
                !reader.u2(attributes_count)) {
                delete file;
                return 0;
            }

            for (uint32_t j = 0; j < attributes_count; ++j) {
                uint16_t attribute_name_index;
                uint32_t attribute_length;
                if (!reader.u2(attribute_name_index) ||
                    !reader.u4(attribute_length) ||
                    !range_ok(reader.pos, attribute_length, reader.size)) {
                    delete file;
                    return 0;
                }

                const std::string attribute_name =
                    (attribute_name_index < cp_count &&
                     file->constant_pool[attribute_name_index].tag == 1)
                    ? file->constant_pool[attribute_name_index].text
                    : std::string();

                if (attribute_name == "Code") {
                    if (!parse_code_attribute(reader, attribute_length,
                                              method.code_length)) {
                        delete file;
                        return 0;
                    }
                } else if (!reader.take(attribute_length)) {
                    delete file;
                    return 0;
                }
            }
            file->methods.push_back(method);
        }

        uint16_t class_attributes_count;
        if (!reader.u2(class_attributes_count) ||
            !skip_attributes(reader, class_attributes_count) ||
            reader.pos != reader.size) {
            delete file;
            return 0;
        }

        file->summary = {minor, major, access_flags, cp_count,
                         fields_count, methods_count, this_class, super_class};
        *out_file = file;
        return 1;
    } catch (...) {
        delete file;
        return 0;
    }
}

extern "C" void opticraft_classfile_destroy(OptiClassFile *file) {
    delete file;
}

extern "C" int opticraft_classfile_summary(const OptiClassFile *file,
                                              OptiClassSummary *out_summary) {
    if (file == nullptr || out_summary == nullptr) return 0;
    *out_summary = file->summary;
    return 1;
}

extern "C" int opticraft_classfile_utf8(const OptiClassFile *file,
                                          uint16_t index,
                                          const char **out_text,
                                          uint16_t *out_length) {
    if (file == nullptr || out_text == nullptr || out_length == nullptr ||
        index == 0 || index >= file->constant_pool.size() ||
        file->constant_pool[index].tag != 1 ||
        file->constant_pool[index].text.size() > 65535) {
        return 0;
    }
    *out_text = file->constant_pool[index].text.c_str();
    *out_length = static_cast<uint16_t>(
        file->constant_pool[index].text.size());
    return 1;
}

extern "C" int opticraft_classfile_find_method(
    const OptiClassFile *file, const char *name, const char *descriptor,
    uint16_t *out_access_flags, uint32_t *out_code_length) {
    if (file == nullptr || name == nullptr || descriptor == nullptr) return 0;

    for (const MethodEntry &method : file->methods) {
        if (method.name >= file->constant_pool.size() ||
            method.descriptor >= file->constant_pool.size()) {
            continue;
        }
        const CpEntry &name_entry = file->constant_pool[method.name];
        const CpEntry &descriptor_entry =
            file->constant_pool[method.descriptor];
        if (name_entry.tag == 1 && descriptor_entry.tag == 1 &&
            name_entry.text == name &&
            descriptor_entry.text == descriptor) {
            if (out_access_flags != nullptr) {
                *out_access_flags = method.access;
            }
            if (out_code_length != nullptr) {
                *out_code_length = method.code_length;
            }
            return 1;
        }
    }
    return 0;
}

extern "C" int opticraft_classfile_class_name(const OptiClassFile *file,
                                                int which,
                                                const char **out_text,
                                                uint16_t *out_length) {
    if (file == nullptr || out_text == nullptr || out_length == nullptr ||
        (which != 0 && which != 1)) {
        return 0;
    }
    const uint16_t class_index = which == 0
        ? file->summary.this_class : file->summary.super_class;
    if (class_index == 0 && which == 1) {
        *out_text = "";
        *out_length = 0;
        return 1;
    }
    if (class_index == 0 || class_index >= file->constant_pool.size()) return 0;
    const CpEntry &class_entry = file->constant_pool[class_index];
    if (class_entry.tag != 7 || class_entry.a == 0 ||
        class_entry.a >= file->constant_pool.size()) {
        return 0;
    }
    const CpEntry &name_entry = file->constant_pool[class_entry.a];
    if (name_entry.tag != 1 || name_entry.text.size() > 65535) return 0;
    *out_text = name_entry.text.c_str();
    *out_length = static_cast<uint16_t>(name_entry.text.size());
    return 1;
}
