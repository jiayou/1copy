
#include <iostream>
#include <filesystem>
#include <ctime>
#include <locale.h>

#include "pch.h"
#include "onecopy.hpp"
#include "OptionParser.h"
#include "spdlog/spdlog.h"

using namespace std;
namespace fs = std::experimental::filesystem;

OneCopyConfig config;

void help(string errMsg)
{
	cout << (" Usage:                                                            \n");
	cout << (" 1copy [SOURCE-DIR] [--target=TARGET-DIR] [--undo] [-n|--dry-run]  \n");
	cout << ("                                                                   \n");
	cout << (" Examples:                                                         \n");
	cout << ("                                                                   \n");
	cout << (" 1copy --target=c:\\user\\photo                                    \n");
	cout << (" 1copy --t c:\\user\\photo E:\\DCIM                                \n");
	cout << (" 1copy c:\\user\\photo                                             \n");
}

void print_dup_json(vector<Duplication> dup)
{
	cout << "dup: [" << endl;
	for (auto &d : dup)
	{
		cout << "{" << endl;

		cout << "\t"
			 << "filename: \"" << ws2s(d.filename) << "\"," << endl;
		cout << "\t"
			 << "filesize: " << std::to_string(d.filesize) << "," << endl;
		cout << "\t"
			 << "paths: [" << endl;
		for (auto &p : d.paths)
		{
			string abfs = fs::system_complete(p).string();
			cout << "\t"
				 << "\t"
				 << "\"" << abfs << "\"," << endl;
		}
		cout << "\t"
			 << "]" << endl;
		cout << "}," << endl;
	}
	cout << "]" << endl;
}

void remove_trailing_separator(fs::path &p)
{
	string ps = p.string();
	if (!p.empty() && ps.back() == fs::path::preferred_separator){
		ps.pop_back();
		p = fs::path(ps);
	}
}

void make_1copy_backup(fs::path oldp, fs::path src)
{
	remove_trailing_separator(src);
	string ps = oldp.string();
	ps.insert(src.string().size(), "\\.1copy");
	fs::path newp = fs::path(ps);
	
	spdlog::debug("1COPY: {}", newp.string().c_str());
	if (config.dryrun) return;
	try
	{
		fs::create_directories( newp.parent_path() );
		fs::rename(oldp, newp);
	}
	catch (const std::exception& e) 
	{
		spdlog::error("1COPY: {}", newp.string().c_str());
		spdlog::error(" FAIL: {}", e.what());
	}
}

void moveDuplicateFiles(vector<Duplication> dup, fs::path src)
{
    for (auto &f : dup){
		make_1copy_backup(f.path, src);
    }
}

void undo_1copy_backup(fs::path oldp, fs::path bak)
{
}

void undoMoveDuplicateFiles(fs::path src, fs::path bak)
{
	if (!fs::exists(bak)){
		spdlog::warn("Nothing to undo.");
		return;
	}

	vector<MetaInfo> bak_file_list, left_file_list;
	fs::path oldp, newp;

	GetDirectoryContents(bak.c_str(), bak_file_list);
	spdlog::info("bak: found {} files", bak_file_list.size());

	for(auto& f:bak_file_list){
		oldp = f.path;
		wstring ps = oldp.wstring();
		size_t st = bak.wstring().size();
		size_t ed = ps.size();
		newp = src;
		newp /= ps.substr(st, ed);

		spdlog::debug("RESTORE: {}", ws2s(newp).c_str());
		if (fs::exists(newp)){
			spdlog::warn("  EXIST: {}", ws2s(newp).c_str());
		}

		if (config.dryrun){
			continue;
		}

		try{
			fs::rename(oldp, newp);
		}
		catch (const std::exception& e) 
		{
			spdlog::error("  ERROR: {}", e.what()) ;
		}
	}	

	GetDirectoryContents(bak.c_str(), left_file_list);
	if (left_file_list.size() > 0){
		spdlog::info("bak: left {} files", left_file_list.size());
		return;
	}

	spdlog::debug("CLEAN: {}", ws2s(bak).c_str());
	try{
		std::uintmax_t n = fs::remove_all(bak);
	}
	catch (const std::exception& e)
	{
		spdlog::error("  ERROR: {}", e.what()) ;
	}

}


int main(int argc, const char **argv)
{
	setlocale(LC_CTYPE, "");
	spdlog::set_level(spdlog::level::trace);

	using optparse::OptionParser;
	OptionParser parser = OptionParser().description("just an example");

	parser.add_option("-t", "--target")
		.action("store")
		.type("string")
		.dest("target")
		.help("destination FOLDER");
	parser.add_option("-Z", "--undo")
		.action("store_true")
		.dest("undo")
		.set_default(false)
		.help("cancel removing duplications");
	parser.add_option("-v", "--verbose")
		.action("count")
		.dest("verbose")
		.help("display debug outputs");
	parser.add_option("-n", "--dry-run")
		.action("store_true")
		.dest("dryrun")
		.set_default(false)
		.help("display, do NOT change anything");
	parser.add_option("-x", "--exec")
		.action("store_false")
		.dest("dryrun")
		.set_default(true)
		.help("execute");

	optparse::Values options = parser.parse_args(argc, argv);
	vector<string> args = parser.args();
	fs::path src, dst, bak;

	spdlog::set_level(spdlog::level::info);
	int verbose = (int)(options.get("verbose"));
	if (verbose-- > 0)
		spdlog::set_level(spdlog::level::debug);
	if (verbose-- > 0)
		spdlog::set_level(spdlog::level::trace);

	spdlog::debug("Start 1copy ...");
	spdlog::debug("TAEGET: {}", (const char *)(options.get("target")));
	spdlog::debug("VERBOSE: {}", (int)(options.get("verbose")));
	spdlog::debug("DRY RUN: {}", (bool)(options.get("dryrun")));
	spdlog::debug("UNDO: {}", (bool)(options.get("undo")));

	config.dryrun = (bool)(options.get("dryrun"));

	std::string name1 = std::tmpnam(nullptr);
	spdlog::debug("temporary file name: {}", name1);

	if (args.size() == 0)
	{
		// src is empty
		src = fs::current_path();
	}
	else if (args.size() > 1)
	{
		help("1copy takes one argument.");
		return 1;
	}
	else
	{
		src = fs::path(args[0]);
	}

	if (!fs::exists(src))
	{
		spdlog::error("SRC-DIR does not exist: {}", ws2s(src).c_str());
		return 1;
	}
	remove_trailing_separator(src);
	spdlog::debug("SRC = {}", ws2s(src).c_str());
	bak = src;
	bak /= ".1copy";
	spdlog::debug("BAK = {}", ws2s(bak).c_str());

	if ((bool)(options.get("undo")))
	{
		spdlog::debug("UNDO: {}", ws2s(bak).c_str());
		undoMoveDuplicateFiles(src, bak);
		spdlog::debug("DONE.");
		return 0;
	}

	if (fs::exists(bak) && !config.dryrun)
	{
		spdlog::error("STOP! {} already exists.", ws2s(bak).c_str());
		return 1;
	}

	string target = (const char *)(options.get("target"));
	spdlog::debug("DST = {}", target.c_str());

	if (target.size() > 0)
	{
		dst = fs::path(target);
		remove_trailing_separator(dst);

		if (!fs::exists(dst))
		{
			spdlog::error("DST-DIR does not exist: {}", ws2s(dst).c_str());
			return 1;
		}
		vector<MetaInfo> src_file_list, dst_file_list;

		GetDirectoryContents(src.c_str(), src_file_list);
		spdlog::info("src: found {} files in {}", src_file_list.size(), ws2s(src).c_str());

		GetDirectoryContents(dst.c_str(), dst_file_list);
		spdlog::info("dst: found {} files in {}", dst_file_list.size(), ws2s(dst).c_str());

		vector<Duplication> dup = findExistingFile(src_file_list, dst_file_list);
		spdlog::info("found {} duplications", dup.size());

		// print_dup_json(dup);
		moveDuplicateFiles(dup, src);
	}
	else // dst missing, find duplicate file
	{
		spdlog::debug("dst missing, find duplicate file");
		vector<MetaInfo> src_file_list;
		GetDirectoryContents(src.c_str(), src_file_list);

		spdlog::info("found {} files in {}", src_file_list.size(), ws2s(src).c_str());

		vector<Duplication> dup = findDuplicateFile(src_file_list);
		spdlog::info("found {} duplications", dup.size());

		print_dup_json(dup);
	}

	return 0;
}

void time_it()
{
	clock_t begin = clock();

	// do something

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "elaspsed time:" << elapsed_secs << endl;
}

void fs_test()
{
	fs::path oldp, newp, root1, root2;

	root1 = "C:\\test\\a";
	root2 = "C:\\test\\a\\";
	remove_trailing_separator(root1);
	remove_trailing_separator(root2);

	wcout << root1 << endl;
	wcout << root2 << endl;

	oldp = "C:\\test\\a\\b\\c.txt";
	newp = "C:\\test\\.1copy\\a\\b\\c.txt";
	wcout << newp.parent_path() << endl;
	try{
		fs::create_directories( newp.parent_path() );
		fs::rename(oldp, newp); 
	}catch (const std::exception& e) 
	{
		 std::cout << e.what();
	}

}