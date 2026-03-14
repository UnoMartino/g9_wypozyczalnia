#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "build"
#define OBJ_DIR "build/obj"

#ifdef _WIN32
#define EXEC_PATH BUILD_DIR "/wypozyczalnia.exe"
#else
#define EXEC_PATH BUILD_DIR "/wypozyczalnia"
#endif

// --- Konfiguracja flag ---
const char *debug_flags[] = {
    "-ggdb", "-O0", "-I", "src", "-I", "include",
    "-Wall", "-Wextra", "-std=c++20"
};

const char *release_flags[] = {
    "-O3", "-I", "src", "-I", "include",
    "-Wall", "-Wextra", "-std=c++20"
};

Nob_File_Paths sources = {0};

bool collect_sources(Nob_Walk_Entry entry) {
    // klonujemy strukture folderow
    if (entry.type == NOB_FILE_DIRECTORY) {

        const char *mirrored_dir = nob_temp_sprintf("%s/%s", OBJ_DIR, entry.path);

        if (!nob_mkdir_if_not_exists(mirrored_dir)) {
            return false;
        }
        return true;
    }

    if (entry.type != NOB_FILE_REGULAR) return true;

    Nob_String_View path_sv = nob_sv_from_cstr(entry.path);
    if (nob_sv_end_with(path_sv, ".cpp")) {
        nob_da_append(&sources, nob_temp_strdup(entry.path));
    }

    return true;
}

const char *get_obj_path(const char *src_path) {
    return nob_temp_sprintf("%s/%s.o", OBJ_DIR, src_path);
}

bool ensure_build_dirs(void) {
    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return false;
    if (!nob_mkdir_if_not_exists(OBJ_DIR)) return false;
    return true;
}

// auto detekcja kompilatora clang++/g++
const char* detect_compiler() {
    // sprawdz zmienna srodowiskowa
    const char *cxx = getenv("CXX");
    if (cxx != NULL) return cxx;

    // test kompilatorow
    Nob_Cmd cmd = {0};
    Nob_Cmd_Opt opt = {0};

    // ustawiamy scieki na bledy, tak aby nie zasmicaly konsoli
    #ifdef _WIN32
    opt.stdout_path = "NUL"; opt.stderr_path = "NUL";
    #else
    opt.stdout_path = "/dev/null"; opt.stderr_path = "/dev/null";
    #endif

    // sprawdz clang++
    nob_cmd_append(&cmd, "clang++", "--version");
    if (nob_cmd_run_opt(&cmd, opt)) {
        nob_cmd_free(cmd);
        return "clang++";
    }

    // fallback: sprawdz czy dziala g++
    cmd.count = 0; // reset komenty
    nob_cmd_append(&cmd, "g++", "--version");
    if (nob_cmd_run_opt(&cmd, opt)) {
        nob_cmd_free(cmd);
        return "g++";
    }

    nob_cmd_free(cmd);
    nob_log(NOB_ERROR, "clang++ or g++ not found, cannot build");
    exit(1);
}

bool compile_source(const char *compiler, const char *src_path, const char **flags, size_t flags_count) {
    const char *obj_path = get_obj_path(src_path);

    if (!nob_needs_rebuild1(obj_path, src_path)) return true;

    nob_log(NOB_INFO, "Kompilowanie %s -> %s", src_path, obj_path);
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, compiler);

    for (size_t i = 0; i < flags_count; ++i) {
        nob_cmd_append(&cmd, flags[i]);
    }

    nob_cmd_append(&cmd, "-c", src_path, "-o", obj_path);

    bool success = nob_cmd_run(&cmd);
    nob_cmd_free(cmd);
    return success;
}

bool link_objects(const char *compiler) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, compiler, "-o", EXEC_PATH);

    bool needs_linking = false;
    for (size_t i = 0; i < sources.count; ++i) {
        const char *obj_path = get_obj_path(sources.items[i]);
        nob_cmd_append(&cmd, obj_path);

        if (nob_needs_rebuild1(EXEC_PATH, obj_path)) needs_linking = true;
    }

    if (!needs_linking) {
        nob_log(NOB_INFO, "executable is up-to-date");
        nob_cmd_free(cmd);
        return true;
    }

    nob_log(NOB_INFO, "linking...");
    bool success = nob_cmd_run(&cmd);
    nob_cmd_free(cmd);
    return success;
}

bool build_project(const char *compiler, const char **flags, size_t flags_count) {
    if (!ensure_build_dirs()) return false;
    if (!nob_walk_dir("src", collect_sources)) return false;

    if (sources.count == 0) {
        nob_log(NOB_WARNING, "no '.cpp' files found in 'src/'");
        return true;
    }

    for (size_t i = 0; i < sources.count; ++i) {
        if (!compile_source(compiler, sources.items[i], flags, flags_count)) return false;
    }

    return link_objects(compiler);
}

void clean_build(void) {
    nob_log(NOB_INFO, "cleaning....");
    if (!nob_walk_dir("src", collect_sources)) return;

    for (size_t i = 0; i < sources.count; ++i) {
        nob_delete_file(get_obj_path(sources.items[i]));
    }
    nob_delete_file(EXEC_PATH);
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    // usuwamy ./nob z argumentów
    nob_shift_args(&argc, &argv);

    // defaultowa akcja to build
    const char *action = argc > 0 ? nob_shift_args(&argc, &argv) : "build";

    if (strcmp(action, "clean") == 0) {
        clean_build();
        return 0;
    }

    // default to debug mode
    const char *mode = argc > 0 ? nob_shift_args(&argc, &argv) : "debug";

    const char **flags = debug_flags;
    size_t flags_count = NOB_ARRAY_LEN(debug_flags);

    if (strcmp(mode, "release") == 0) {
        flags = release_flags;
        flags_count = NOB_ARRAY_LEN(release_flags);
    } else if (strcmp(mode, "debug") != 0) {
        nob_log(NOB_ERROR, "you can only use 'debug' or 'relase' mode, not '%s'", mode);
        return 1;
    }

    const char *compiler = detect_compiler();
    nob_log(NOB_INFO, "compiler: %s | mode: %s", compiler, mode);

    if (!build_project(compiler, flags, flags_count)) return 1;

    if (strcmp(action, "run") == 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, EXEC_PATH);

        while (argc > 0) {
            nob_cmd_append(&cmd, nob_shift_args(&argc, &argv));
        }

        bool success = nob_cmd_run(&cmd);
        nob_cmd_free(cmd);
        return success ? 0 : 1;

    } else if (strcmp(action, "build") != 0) {
        nob_log(NOB_ERROR, "unknown '%s' action", action);
        return 1;
    }

    return 0;
}
