#include "world_save_catalog.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

static void make_file(const char* path) {
    FILE* file = std::fopen(path, "wb");
    assert(file != nullptr);
    std::fputs("fixture", file);
    std::fclose(file);
}

int main() {
    char root[] = "/tmp/opticraft-world-catalog-XXXXXX";
    assert(mkdtemp(root) != nullptr);

    char alpha[512], beta[512], ignored[512], path[560];
    std::snprintf(alpha, sizeof(alpha), "%s/Alpha World", root);
    std::snprintf(beta, sizeof(beta), "%s/Backup Only", root);
    std::snprintf(ignored, sizeof(ignored), "%s/Not A Save", root);
    assert(mkdir(alpha, 0700) == 0);
    assert(mkdir(beta, 0700) == 0);
    assert(mkdir(ignored, 0700) == 0);

    std::snprintf(path, sizeof(path), "%s/level.dat", alpha);
    make_file(path);
    std::snprintf(path, sizeof(path), "%s/level.dat_old", beta);
    make_file(path);
    std::snprintf(path, sizeof(path), "%s/readme.txt", ignored);
    make_file(path);

    OpticraftWorldCatalogEntry entries[8]{};
    const int count = opticraft_world_catalog_scan(root, entries, 8);
    assert(count == 2);

    bool found_alpha = false, found_beta = false;
    for (int i = 0; i < count; ++i) {
        if (std::strcmp(entries[i].folder, "Alpha World") == 0) {
            found_alpha = true;
            assert(entries[i].has_level_dat == 1);
        }
        if (std::strcmp(entries[i].folder, "Backup Only") == 0) {
            found_beta = true;
            assert(entries[i].has_level_dat == 0);
        }
    }
    assert(found_alpha && found_beta);
    assert(opticraft_world_catalog_scan(root, nullptr, 1) == -2);
    assert(opticraft_world_catalog_scan("/definitely/not/a/save/path", entries, 8) == -1);

    std::puts("PASS Java save folder discovery (level.dat / level.dat_old)");
    return 0;
}
