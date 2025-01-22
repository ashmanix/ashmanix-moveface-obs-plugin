// #ifdef TEST_BUILD
// We rely on extern "C" for C linkage so the unmangled symbol names match
extern "C" {

#include <stdlib.h>  // for free
#include <stdarg.h>  // for va_list, va_start, etc.
#include <stdio.h>   // for fputs, vfprintf

// Provide stubs (dummy impl) for each missing OBS symbol

// 1) bfree
void bfree(void *ptr)
{
    // Just free
    free(ptr);
}

// 2) obs_current_module
// Sometimes it's a pointer or extern variable. We'll define it as null
void* obs_current_module = nullptr;

// 3) obs_data_array_release
// If your code references obs_data_array_release
// you can do nothing or free a dummy object
void obs_data_array_release(void *arr)
{
    // do nothing
}

// 4) obs_data_get_array
// We'll just return nullptr
void* obs_data_get_array(void *data, const char *name)
{
    return nullptr;
}

// 5) obs_data_set_array
void obs_data_set_array(void *data, const char *name, void *arr)
{
    // do nothing
}

// 6) obs_find_module_file
const char *obs_find_module_file(const char *file)
{
    // Return dummy or null
    return nullptr;
}

// 7) obs_hotkey_load
bool obs_hotkey_load(void *data, unsigned long id, void *hotkey)
{
    // do nothing, return true
    return true;
}

// 8) obs_hotkey_register_frontend
bool obs_hotkey_register_frontend(const char *id, const char *desc, void *proc, void *data)
{
    return true;
}

// 9) obs_hotkey_save
bool obs_hotkey_save(void *data, unsigned long id, void *hotkey)
{
    return true;
}

// 10) obs_log
void obs_log(int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[TEST OBS LOG %d] ", level);
    vfprintf(stdout, format, args);
    va_end(args);
    fputc('\n', stdout);
}

} // extern "C"
// #endif // TEST_BUILD
