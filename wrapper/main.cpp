#include <iostream>
#include <wrapper.hpp>

void get_process_list()
{
	auto process_ids = winapi::process::enum_processes();
	for (auto process_id : process_ids)
	{
		auto process = winapi::process::open_process(
			winapi::process::query_limited_information,
			false,
			process_id);
		if (process)
		{
			auto process_name = winapi::process::get_process_image_filename(process);
			winapi::handle::close_handle(process);
			std::wcout << "[" << process_id << "] " << process_name << std::endl;
		}
	}
}

void get_process_list_snapshot()
{
	/*broken*/
	auto snapshot = winapi::process::create_toolhelp32_snapshot(
		winapi::process::snap_process,
		0);
	winapi::process_entry32 entry;
	entry.size = sizeof(winapi::process_entry32);
	winapi::process::process32_first(snapshot, &entry);
	std::wcout << entry.file << std::endl;
	while (winapi::process::process32_next(snapshot, &entry))
	{
		std::wcout << entry.file << std::endl;
	}
}

void get_process_list_nt()
{
	auto proc_info = nt::system::query_system_information<
		nt::system_process_information>();
	for (auto proc : proc_info)
	{
		if (proc->image_name.length > 0)
		{
			auto process_id = reinterpret_cast<std::uint32_t>(
				proc->process_id);
			auto process_name = proc->image_name.buffer;
			std::wcout 
				<< "[" << process_id << "] " 
				<< process_name << std::endl;
		}
	}
}

void random_stuff()
{
	const std::wstring global = L"\\Global??";

	auto directory = nt::string::init_unicode_string(global);
	auto attributes = nt::object::initialize_attributes(
		&directory,
		nt::object::case_insensitive,
		nullptr);
	auto handle = nt::directory::open(
		nt::directory::dir_query,
		&attributes);

	std::cout << "\\Global?? = " << handle << std::endl;

	auto objects = nt::directory::query(handle);
	for (auto& [index, object] : objects)
	{
		std::wcout << "[" << index << "] " << object->name() << std::endl;
	}
}

int main()
{
	get_process_list();
	//get_process_list_snapshot();
	//get_process_list_nt();
	random_stuff();
	return 0;
}
