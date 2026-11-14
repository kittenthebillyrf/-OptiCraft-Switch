#include "opticraft_jar_classloader.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <zlib.h>

static void le16(std::vector<uint8_t> &v, uint16_t x) {
    v.push_back(static_cast<uint8_t>(x));
    v.push_back(static_cast<uint8_t>(x >> 8));
}
static void le32(std::vector<uint8_t> &v, uint32_t x) {
    le16(v, static_cast<uint16_t>(x));
    le16(v, static_cast<uint16_t>(x >> 16));
}
static void append(std::vector<uint8_t> &v, const uint8_t *p, size_t n) {
    v.insert(v.end(), p, p + n);
}
int main() {
    const uint8_t cls[] = {
        0xca,0xfe,0xba,0xbe,0,0,0,49,0,8,
        1,0,1,'T',7,0,1,1,0,16,'j','a','v','a','/','l','a','n','g','/','O','b','j','e','c','t',7,0,3,
        1,0,4,'m','a','i','n',1,0,22,'(','[','L','j','a','v','a','/','l','a','n','g','/','S','t','r','i','n','g',';',')','V',
        1,0,4,'C','o','d','e',
        0,0x21,0,2,0,4,0,0,0,0,0,1,0,9,0,5,0,6,0,1,0,7,0,0,0,13,
  0,1,0,1,0,0,0,1,0,0,0,0,0,0,0
    };
    const std::string name = "T.class";
    const uint32_t crc = static_cast<uint32_t>(crc32(0L, cls, sizeof(cls)));
    std::vector<uint8_t> jar;

    le32(jar, 0x04034b50); le16(jar, 20); le16(jar, 0); le16(jar, 0);
    le16(jar, 0); le16(jar, 0); le32(jar, crc);
    le32(jar, sizeof(cls)); le32(jar, sizeof(cls));
    le16(jar, static_cast<uint16_t>(name.size())); le16(jar, 0);
    append(jar, reinterpret_cast<const uint8_t *>(name.data()), name.size());
    append(jar, cls, sizeof(cls));

    const uint32_t directory_offset = static_cast<uint32_t>(jar.size());
    le32(jar, 0x02014b50); le16(jar, 20); le16(jar, 20);
    le16(jar, 0); le16(jar, 0); le16(jar, 0); le16(jar, 0);
    le32(jar, crc); le32(jar, sizeof(cls)); le32(jar, sizeof(cls));
    le16(jar, static_cast<uint16_t>(name.size())); le16(jar, 0); le16(jar, 0);
    le16(jar, 0); le16(jar, 0); le32(jar, 0); le32(jar, 0);
    append(jar, reinterpret_cast<const uint8_t *>(name.data()), name.size());
    const uint32_t directory_size = static_cast<uint32_t>(jar.size()) - directory_offset;
    le32(jar, 0x06054b50); le16(jar, 0); le16(jar, 0);
    le16(jar, 1); le16(jar, 1); le32(jar, directory_size);
    le32(jar, directory_offset); le16(jar, 0);

    const std::string path = "/tmp/opticraft-loader-smoke-" +
                             std::to_string(static_cast<long long>(getpid())) + ".jar";
    {
        std::ofstream file(path, std::ios::binary);
        assert(file.good());
        file.write(reinterpret_cast<const char *>(jar.data()),
                   static_cast<std::streamsize>(jar.size()));
        assert(file.good());
    }

    OptiJar *archive = nullptr;
    assert(opticraft_jar_open_file(path.c_str(), 1024 * 1024, &archive));
    OptiLoadedClass *loaded = nullptr;
    assert(opticraft_jar_load_class(archive, "T", &loaded));
    const char *class_name = nullptr;
    uint16_t class_name_length = 0;
    assert(opticraft_loaded_class_name(loaded, 0, &class_name, &class_name_length));
    assert(std::string(class_name, class_name_length) == "T");
    const char *super_name = nullptr;
    uint16_t super_name_length = 0;
    assert(opticraft_loaded_class_name(loaded, 1, &super_name, &super_name_length));
    assert(std::string(super_name, super_name_length) == "java/lang/Object");
    uint32_t code_length = 0;
    assert(opticraft_loaded_class_find_method(
        loaded, "main", "([Ljava/lang/String;)V", nullptr, &code_length));
    assert(code_length == 1);

    opticraft_loaded_class_destroy(loaded);
    opticraft_jar_close(archive);
    std::remove(path.c_str());
    return 0;
}
