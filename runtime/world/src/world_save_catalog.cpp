#include "world_save_catalog.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

namespace {

bool is_directory(const char* path) {
    struct stat info {};
    return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

bool file_exists(const char* path) {
    struct stat info {};
    return stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

bool append_path(char* output, size_t capacity, const char* root, const char* leaf) {
    const int written = snprintf(output, capacity, "%s/%s", root, leaf);
    return written >= 0 && static_cast<size_t>(written) < capacity;
}

} // namespace

extern "C" int opticraft_world_catalog_scan(
    const char* saves_root,
    OpticraftWorldCatalogEntry* entries,
    size_t capacity) {
    if (!saves_root || (!entries && capacity != 0)) return -2;

    DIR* directory = opendir(saves_root);
    if (!directory) return -1;

    size_t count = 0;
    struct dirent* item = nullptr;
    while ((item = readdir(directory)) != nullptr) {
        if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) continue;

        char world_path[512];
        if (!append_path(world_path, sizeof(world_path), saves_root, item->d_name)) continue;
        if (!is_directory(world_path)) continue;

        char level_path[560];
        char old_level_path[560];
        if (!append_path(level_path, sizeof(level_path), world_path, "level.dat") ||
            !append_path(old_level_path, sizeof(old_level_path), world_path, "level.dat_old")) {
            continue;
        }
        if (!file_exists(level_path) && !file_exists(old_level_path)) continue;

        if (count < capacity) {
            OpticraftWorldCatalogEntry& entry = entries[count];
            const size_t length = strlen(item->d_name);
            const size_t copy_length = length < sizeof(entry.folder) - 1
                ? length : sizeof(entry.folder) - 1;
            memcpy(entry.folder, item->d_name, copy_length);
            entry.folder[copy_length] = '\0';
            entry.has_level_dat = file_exists(level_path) ? 1u : 0u;
        }
        ++count;
    }

    closedir(directory);
    if (count > capacity) return static_cast<int>(capacity);
    return static_cast<int>(count);
}
