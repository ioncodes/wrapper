﻿#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <winternl.h>

#include <vector>
#include <string>
#include <unordered_map>

namespace winapi
{
#pragma region constants
    constexpr std::uint32_t max_path = MAX_PATH;
#pragma endregion constants

    struct module_info
    {
        void* base_of_dll;
        std::uint32_t size_of_image;
        void* entrypoint;
    };

    struct process_entry32
    {
        std::uint32_t size;
        std::uint32_t usage_count;
        std::uint32_t process_id;
        std::uint64_t* default_heap_id;
        std::uint32_t module_info;
        std::uint32_t thread_count;
        std::uint32_t parent_process_id;
        long class_base;
        std::uint32_t flags;
        wchar_t file[winapi::max_path];
    };

    struct security_attributes
    {
        std::uint32_t length;
        void* security_descriptor;
        bool inherit_handle;
    };

    class process
    {
    public:
#pragma region constants
        static constexpr std::uint32_t terminate = PROCESS_TERMINATE;
        static constexpr std::uint32_t create_thread = PROCESS_CREATE_THREAD;
        static constexpr std::uint32_t set_sessionid = PROCESS_SET_SESSIONID;
        static constexpr std::uint32_t vm_operation = PROCESS_VM_OPERATION;
        static constexpr std::uint32_t vm_read = PROCESS_VM_READ;
        static constexpr std::uint32_t vm_write = PROCESS_VM_WRITE;
        static constexpr std::uint32_t dup_handle = PROCESS_DUP_HANDLE;
        static constexpr std::uint32_t create_process = PROCESS_CREATE_PROCESS;
        static constexpr std::uint32_t set_quota = PROCESS_SET_QUOTA;
        static constexpr std::uint32_t set_information = PROCESS_SET_INFORMATION;
        static constexpr std::uint32_t query_information = PROCESS_QUERY_INFORMATION;
        static constexpr std::uint32_t suspend_resume = PROCESS_SUSPEND_RESUME;
        static constexpr std::uint32_t query_limited_information = PROCESS_QUERY_LIMITED_INFORMATION;
        static constexpr std::uint32_t set_limited_information = PROCESS_SET_LIMITED_INFORMATION;
        static constexpr std::uint32_t all_access = PROCESS_ALL_ACCESS;

        static constexpr uint32_t snap_heap_list = TH32CS_SNAPHEAPLIST;
        static constexpr uint32_t snap_process = TH32CS_SNAPPROCESS;
        static constexpr uint32_t snap_thread = TH32CS_SNAPTHREAD;
        static constexpr uint32_t snap_module = TH32CS_SNAPMODULE;
        static constexpr uint32_t snap_module32 = TH32CS_SNAPMODULE32;
        static constexpr uint32_t snap_all = TH32CS_SNAPALL;
        static constexpr uint32_t inherit = TH32CS_INHERIT;
#pragma endregion constants

        static inline void* open_process(
            std::uint32_t access,
            bool inherit_handle,
            std::uint32_t process_id)
        {
            return OpenProcess(
                access,
                inherit_handle,
                process_id);
        }
        static inline std::vector<std::uint32_t> enum_processes()
        {
            DWORD processes[1024];
            DWORD needed;
            if (EnumProcesses(
                processes,
                sizeof(processes),
                &needed))
            {
                int size = sizeof(processes) / sizeof(processes[0]);
                return std::vector<uint32_t>(
                    processes,
                    processes + size);
            }

            return std::vector<uint32_t>();
        }
        static inline std::wstring get_process_image_filename(
            void* process)
        {
            wchar_t file[MAX_PATH];
            GetProcessImageFileNameW(
                process,
                file,
                sizeof(file) / sizeof(file[0]));
            return std::wstring(file);
        }
        static inline void* create_toolhelp32_snapshot(
            std::uint32_t flags,
            std::uint32_t process_id)
        {
            return CreateToolhelp32Snapshot(flags, process_id);
        }
        static inline bool process32_first(
            void* snapshot,
            winapi::process_entry32* entry)
        {
            return Process32FirstW(
                snapshot,
                reinterpret_cast<PROCESSENTRY32W*>(entry));
        }
        static inline bool process32_next(
            void* snapshot,
            winapi::process_entry32* entry)
        {
            return Process32NextW(
                snapshot,
                reinterpret_cast<PROCESSENTRY32W*>(entry));
        }
        static inline void sleep(
            std::uint32_t milliseconds)
        {
            Sleep(milliseconds);
        }
    };

    class module
    {
    public:
        static inline std::wstring get_module_file_name_ex(
            void* process,
            void* module)
        {
            wchar_t name[MAX_PATH];
            if (GetModuleFileNameExW(
                process,
                (HMODULE)module,
                name,
                sizeof(name) / sizeof(wchar_t)))
            {
                return std::wstring(name);
            }

            throw std::exception("GetModuleFileNameEx failed");
        }
        static inline winapi::module_info get_module_information(
            void* process,
            void* module)
        {
            MODULEINFO info = {};
            if (GetModuleInformation(
                process,
                (HMODULE)module,
                &info,
                sizeof(MODULEINFO)))
            {
                return module_info
                {
                    info.lpBaseOfDll,
                    info.SizeOfImage,
                    info.EntryPoint
                };
            }

            return module_info
            {
                nullptr,
                0,
                nullptr
            };
        }
        static inline std::wstring get_module_base_name(
            void* process,
            void* module)
        {
            wchar_t name[MAX_PATH] = L"";
            if (GetModuleBaseNameW(
                process,
                (HMODULE)module,
                name,
                sizeof(name) / sizeof(wchar_t)))
            {
                return std::wstring(name);
            }

            throw std::exception("GetModuleBaseName failed");
        }
        static inline std::vector<void*> enum_process_modules(
            void* process)
        {
            HMODULE modules[1024];
            DWORD needed;
            if (EnumProcessModules(
                process,
                modules,
                sizeof(modules),
                &needed))
            {
                int size = sizeof(modules) / sizeof(modules[0]);
                return std::vector<void*>(
                    modules,
                    modules + size);
            }

            return std::vector<void*>();
        }
        static inline void* get_proc_address(
            void* module,
            std::string name)
        {
            return GetProcAddress(
                (HMODULE)module,
                name.data());
        }
        static inline void* get_module_handle(
            std::wstring module)
        {
            return GetModuleHandleW(module.data());
        }
        static inline void* load_library(
            std::wstring module,
            std::uint32_t flags)
        {
            return LoadLibraryExW(
                module.data(),
                nullptr,
                flags);
        }
    };

    class memory
    {
    public:
#pragma region constants
        static constexpr std::uint32_t commit = MEM_COMMIT;
        static constexpr std::uint32_t reserve = MEM_RESERVE;
        static constexpr std::uint32_t reset = MEM_RESET;
        static constexpr std::uint32_t reset_undo = MEM_RESET_UNDO;
        static constexpr std::uint32_t large_pages = MEM_LARGE_PAGES;
        static constexpr std::uint32_t physical = MEM_PHYSICAL;
        static constexpr std::uint32_t top_down = MEM_TOP_DOWN;
        static constexpr std::uint32_t write_watch = MEM_WRITE_WATCH;
        static constexpr std::uint32_t release = MEM_RELEASE;

        static constexpr std::uint32_t execute = PAGE_EXECUTE;
        static constexpr std::uint32_t execute_read = PAGE_EXECUTE_READ;
        static constexpr std::uint32_t execute_read_write = PAGE_EXECUTE_READWRITE;
        static constexpr std::uint32_t execute_write_copy = PAGE_EXECUTE_WRITECOPY;
        static constexpr std::uint32_t read_write = PAGE_READWRITE;
        static constexpr std::uint32_t targets_invalid = PAGE_TARGETS_INVALID;
        static constexpr std::uint32_t targets_no_update = PAGE_TARGETS_NO_UPDATE;
        static constexpr std::uint32_t guard = PAGE_GUARD;
        static constexpr std::uint32_t no_cache = PAGE_NOCACHE;
        static constexpr std::uint32_t write_combine = PAGE_WRITECOMBINE;
#pragma endregion constants

        static inline std::vector<uint8_t> read_process_memory(
            void* process,
            void* address,
            std::size_t size)
        {
            uint8_t* buffer = (uint8_t*)malloc(size);
            SIZE_T read;
            if (ReadProcessMemory(
                process,
                address,
                buffer,
                size,
                &read))
            {
                std::vector<uint8_t> memory(buffer, buffer + read);
                free(buffer);
                return memory;
            }

            return std::vector<uint8_t>();
        }
        static inline bool write_process_memory(
            void* process,
            int address,
            std::vector<uint8_t> data)
        {
            SIZE_T written;
            return WriteProcessMemory(
                process,
                (void*)address,
                std::data(data),
                data.size(),
                &written);
        }
        static inline void* virtual_alloc(
            void* address,
            std::size_t size,
            std::uint32_t allocation_type,
            std::uint32_t protection
        )
        {
            return VirtualAlloc(
                address,
                size,
                allocation_type,
                protection);
        }
        static inline bool virtual_free(
            void* address,
            std::size_t size,
            std::uint32_t free_type)
        {
            return VirtualFree(
                address,
                size,
                free_type);
        }
    };

    class handle
    {
    public:
        static inline void close_handle(
            void* handle)
        {
            CloseHandle(handle);
        }
        static inline uint32_t wait_for_single_object(
            void* handle,
            uint32_t milliseconds)
        {
            return WaitForSingleObject(
                handle,
                milliseconds);
        }
    };

    class event
    {
    public:
        static inline void* open_event(
            uint32_t access,
            bool inherit_handle,
            std::wstring name)
        {
            return OpenEventW(
                access,
                inherit_handle,
                name.data());
        }
        static inline void* create_event(
            winapi::security_attributes* attributes,
            bool manual_reset,
            bool initial_state,
            std::wstring name)
        {
            return CreateEventW(
                reinterpret_cast<SECURITY_ATTRIBUTES*>(attributes),
                manual_reset,
                initial_state,
                name.data());
        }
        static inline bool set_event(
            void* event_handle)
        {
            return SetEvent(event_handle);
        }
    };

    class file
    {
    public:
        static constexpr std::uint32_t generic_read = GENERIC_READ;

        static inline void* open_file_mapping(
            std::uint32_t access,
            bool inherit_handle,
            std::wstring name)
        {
            return OpenFileMappingW(
                access,
                inherit_handle,
                name.data());
        }
        static inline void* create_file_mapping(
            void* file,
            winapi::security_attributes* attributes,
            std::uint32_t protect,
            std::uint32_t max_size_high,
            std::uint32_t max_size_low,
            std::wstring name)
        {
            return CreateFileMappingW(
                file,
                reinterpret_cast<SECURITY_ATTRIBUTES*>(attributes),
                protect,
                max_size_high,
                max_size_low,
                name.data());
        }
        static inline void* map_view_of_file(
            void* file,
            std::uint32_t access,
            std::uint32_t offset_high,
            std::uint32_t offset_low,
            std::size_t size)
        {
            return MapViewOfFile(
                file,
                access,
                offset_high,
                offset_low,
                size);
        }
        static inline bool unmap_view_of_file(
            void* file)
        {
            return UnmapViewOfFile(file);
        }
        static inline void* create_file(
            std::wstring file,
            std::uint32_t access,
            std::uint32_t shared,
            winapi::security_attributes* security_attributes,
            std::uint32_t creation_disposition,
            std::uint32_t attributes,
            void* template_file)
        {
            return CreateFileW(
                file.data(),
                access,
                shared,
                reinterpret_cast<SECURITY_ATTRIBUTES*>(security_attributes),
                creation_disposition,
                attributes,
                template_file
            );
        }
    };
}

namespace nt
{
    struct unicode_string
    {
        wchar_t* buffer;
        unsigned short length;
        unsigned short max_length;

        /*
        unicode_string() {}
        unicode_string(const std::wstring& string)
        {
            buffer = const_cast<wchar_t*>(string.data());
            max_length = string.length() * sizeof(wchar_t);
            length = max_length - sizeof(wchar_t);
        }
        */
    };

    struct object_directory_information
    {
        UNICODE_STRING object_name;
        UNICODE_STRING type_name;

        std::wstring name() { return std::wstring(object_name.Buffer); }
        std::wstring type() { return std::wstring(type_name.Buffer); }
    };

    struct system_process_information
    {
        unsigned long next_entry_offset;
        unsigned long number_of_threads;
        std::uint64_t reserved[3];
        std::uint64_t create_time;
        std::uint64_t user_time;
        std::uint64_t kernel_time;
        nt::unicode_string image_name;
        unsigned long base_priority;
        void* process_id;
        void* inherited_from_process_id;
    };

    enum system_information_class
    {
        base_information = 0,
        performance_information = 2,
        time_of_day_information = 3,
        process_information = 5,
        processor_performance_information = 8,
        interrupt_information = 23,
        exception_information = 33,
        registry_quota_information = 37,
        lookaside_information = 45
    };

    class detail
    {
        friend class system;
        friend class object;
        friend class string;
        friend class directory;
        friend class symlink;
        friend class file;

    private:
        static inline std::unordered_map<std::string, void*> m_functions = {};
    protected:
        template<class T>
        static T resolve_function(std::string name, std::wstring module = L"ntdll")
        {
            if (m_functions.find(name) == m_functions.end())
            {
                auto handle = winapi::module::get_module_handle(module);
                if (!handle)
                {
                    handle = winapi::module::load_library(
                        module,
                        DONT_RESOLVE_DLL_REFERENCES);
                }
                auto proc = winapi::module::get_proc_address(handle, name);
                m_functions.emplace(name, proc);
            }
            return (T)m_functions.at(name);
        }
    };

    class system
    {
    public:
        template<class T>
        static inline std::list<T*> query_system_information()
        {
            auto NtQuerySystemInformation =
                nt::detail::resolve_function<nt_query_system_information_t>(
                    "NtQuerySystemInformation");

            if (std::is_same<
                T,
                nt::system_process_information>::value)
            {
                unsigned long length = 0;
                NtQuerySystemInformation(
                    nt::system_information_class::process_information,
                    nullptr,
                    length,
                    &length);

                auto buffer = reinterpret_cast<nt::system_process_information*>(
                    winapi::memory::virtual_alloc(
                        nullptr,
                        length,
                        winapi::memory::commit,
                        winapi::memory::read_write));

                NtQuerySystemInformation(
                    nt::system_information_class::process_information,
                    buffer,
                    length,
                    &length);

                auto proc_info = buffer;
                std::list<T*> entries;
                do
                {
                    entries.push_back(proc_info);
                    auto offset = proc_info->next_entry_offset;
                    auto next = reinterpret_cast<uintptr_t>(proc_info) + offset;
                    proc_info = reinterpret_cast<nt::system_process_information*>(
                        next);
                } while (proc_info->next_entry_offset != 0);

                winapi::memory::virtual_free(
                    buffer,
                    length,
                    winapi::memory::release);

                return entries;
            }

            return {};
        }
    private:
        using nt_query_system_information_t = NTSTATUS(NTAPI*)(
            nt::system_information_class,
            void*,
            unsigned long,
            unsigned long*);
    };

    class string
    {
    public:
        static inline nt::unicode_string init_unicode_string(const std::wstring& string)
        {
            auto RtlInitUnicodeString =
                nt::detail::resolve_function<nt_rtl_init_unicode_string>(
                    "RtlInitUnicodeString", L"ntoskrnl.exe");
            nt::unicode_string unicode_string;
            RtlInitUnicodeString(&unicode_string, (wchar_t*)string.data());
            return unicode_string;
        }
    private:
        using nt_rtl_init_unicode_string = NTSTATUS(NTAPI*)(
            nt::unicode_string*,
            wchar_t*);
    };

    class object
    {
    public:
        static constexpr std::uint32_t inherit = OBJ_INHERIT;
        static constexpr std::uint32_t permanent = OBJ_PERMANENT;
        static constexpr std::uint32_t exclusive = OBJ_EXCLUSIVE;
        static constexpr std::uint32_t case_insensitive = OBJ_CASE_INSENSITIVE;
        static constexpr std::uint32_t openif = OBJ_OPENIF;
        static constexpr std::uint32_t kernel_handle = OBJ_KERNEL_HANDLE;
        static constexpr std::uint32_t force_access_check = OBJ_FORCE_ACCESS_CHECK;

        static inline OBJECT_ATTRIBUTES initialize_attributes(
            nt::unicode_string* name,
            unsigned long flags,
            void* root_handle/*, security descriptor*/)
        {
            OBJECT_ATTRIBUTES attributes;
            InitializeObjectAttributes(
                &attributes,
                (UNICODE_STRING*)name,
                flags,
                root_handle,
                nullptr);
            return attributes;
        }
    };

    class symlink
    {
    public:
        static constexpr std::uint32_t link_query = 0x0001;

        // this can be easily templated for directory and symlink
        static inline void* open(
            std::uint32_t access,
            POBJECT_ATTRIBUTES attributes)
        {
            auto NtOpenSymbolicLinkObject =
                nt::detail::resolve_function<nt_open_symbolic_link_object>(
                    "NtOpenSymbolicLinkObject");
            void* handle;
            auto status = NtOpenSymbolicLinkObject(
                &handle,
                access,
                attributes);
            return handle;
        }
        static inline std::wstring query(
            void* link)
        {
            auto NtQuerySymbolicLinkObject =
                nt::detail::resolve_function<nt_query_symbolic_link_object>(
                    "NtQuerySymbolicLinkObject");
            UNICODE_STRING target;
            target.Length = 0;
            target.MaximumLength = winapi::max_path;
            target.Buffer = reinterpret_cast<wchar_t*>(malloc(winapi::max_path));
            auto status = NtQuerySymbolicLinkObject(
                link,
                &target,
                nullptr);
            target.Buffer[target.Length / 2] = 0;
            return std::wstring(target.Buffer);
        }

    private:
        using nt_open_symbolic_link_object = NTSTATUS(NTAPI*)(
            void*,
            std::uint32_t,
            POBJECT_ATTRIBUTES);
        using nt_query_symbolic_link_object = NTSTATUS(NTAPI*)(
            void*,
            PUNICODE_STRING,
            std::uint32_t*);
    };

    class directory
    {
    public:
        static constexpr std::uint32_t dir_query = 0x1; // DIRECTORY_QUERY

        static inline void* open(
            std::uint32_t access,
            POBJECT_ATTRIBUTES attributes)
        {
            auto NtOpenDirectoryObject =
                nt::detail::resolve_function<nt_open_directory_object>(
                    "NtOpenDirectoryObject");
            void* handle;
            NtOpenDirectoryObject(
                &handle,
                access,
                attributes);
            return handle;
        }
        static inline std::unordered_map<std::uint64_t, nt::object_directory_information*> query(
            void* directory)
        {
            auto NtQueryDirectoryObject =
                nt::detail::resolve_function<nt_query_directory_object>(
                    "NtQueryDirectoryObject");

            std::unordered_map<
                std::uint64_t,
                nt::object_directory_information*
            > objects;

            std::uint32_t index = 0;
            std::uint32_t ret_size = 0;
            bool end = false;

            do
            {
                auto size = 1;
                auto buffer = reinterpret_cast<nt::object_directory_information*>(
                    winapi::memory::virtual_alloc(
                        nullptr,
                        size,
                        winapi::memory::commit,
                        winapi::memory::read_write));

                NtQueryDirectoryObject(
                    directory,
                    buffer,
                    size,
                    true,
                    false,
                    &index,
                    &ret_size);

                winapi::memory::virtual_free(buffer, size, winapi::memory::release);

                size = ret_size;

                buffer = reinterpret_cast<nt::object_directory_information*>(
                    winapi::memory::virtual_alloc(
                        nullptr,
                        size,
                        winapi::memory::commit,
                        winapi::memory::read_write));

                NtQueryDirectoryObject(
                    directory,
                    buffer,
                    size,
                    true,
                    false,
                    &index,
                    &ret_size);

                end = buffer->object_name.Buffer == nullptr;
                objects.emplace(index, buffer);

                winapi::memory::virtual_free(buffer, size, winapi::memory::release);
            } while (!end);

            return objects;
        }

    private:
        using nt_open_directory_object = NTSTATUS(NTAPI*)(
            void*,
            std::uint32_t,
            POBJECT_ATTRIBUTES);
        using nt_query_directory_object = NTSTATUS(NTAPI*)(
            void* directory,
            void* buffer,
            std::uint32_t length,
            bool return_single_entry,
            bool restart_scan,
            std::uint32_t* context,
            std::uint32_t* return_length);
    };

    class file
    {
    public:
        static constexpr std::uint32_t attribute_normal = FILE_ATTRIBUTE_NORMAL;

        static constexpr std::uint32_t file_open_if = FILE_OPEN_IF;
        static constexpr std::uint32_t file_open = FILE_OPEN;

        static inline void* open(
            const std::wstring& file,
            std::uint32_t access,
            POBJECT_ATTRIBUTES object_attributes,
            std::uint32_t file_attributes,
            std::uint32_t share_access,
            std::uint32_t disposition)
        {
            auto NtCreateFile =
                nt::detail::resolve_function<nt_create_file>(
                    "NtCreateFile");

            void* handle;
            IO_STATUS_BLOCK io_status;
            memset(&io_status, 0, sizeof(io_status));
            auto status = NtCreateFile(
                &handle,
                access,
                object_attributes,
                &io_status,
                nullptr,
                file_attributes,
                share_access,
                disposition,
                0,
                nullptr,
                0);
            if (status != 0)
            {
                handle = nullptr;
            }
            return handle;
        }

    private:
        using nt_create_file = NTSTATUS(NTAPI*)(
            void*,
            std::uint32_t,
            POBJECT_ATTRIBUTES,
            PIO_STATUS_BLOCK,
            PLARGE_INTEGER,
            std::uint32_t,
            std::uint32_t,
            std::uint32_t,
            std::uint32_t,
            void*,
            std::uint32_t);
    };
}