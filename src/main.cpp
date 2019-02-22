
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

int main(int argc, const char **argv)
{
	setlocale(LC_CTYPE, "");

	using optparse::OptionParser;

	OptionParser parser = OptionParser().description("just an example");

	parser.add_option("-t", "--target")
		.action("store")
		.type("string")
		.dest("target")
		.help("destination FOLDER");
	parser.add_option("-z", "--undo")
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
		.set_default(true)
		.help("display, do NOT change anything");
	parser.add_option("-x", "--exec")
		.action("store_false")
		.dest("dryrun")
		.set_default(false)
		.help("execute");

	optparse::Values options = parser.parse_args(argc, argv);
	vector<string> args = parser.args();
	wstring src;
	wstring dst;

	spdlog::set_level(spdlog::level::warn);
	int verbose = (int)(options.get("verbose"));
	if (verbose-- > 0)
		spdlog::set_level(spdlog::level::info);
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
		src = s2ws(args[0]);
	}

	if (!fs::exists(src))
	{
		spdlog::error("SRC-DIR does not exist: {}", ws2s(src).c_str());
		return 1;
	}
	else
	{
		spdlog::info("SRC = {}", ws2s(src).c_str());
	}

	if ((bool)(options.get("undo")))
	{
		spdlog::info("Undo...");
		undoMoveDuplicateFiles(src);
		spdlog::info("done!");
		return 0;
	}

	string target = (const char *)(options.get("target"));
	spdlog::info("DST = {}", target.c_str());

	if (target.size() > 0)
	{
		dst = s2ws(target);
		if (!fs::exists(dst))
		{
			spdlog::error("DST-DIR does not exist: {}", ws2s(dst).c_str());
			return 1;
		}
		vector<MetaInfo> src_file_list, dst_file_list;

		GetDirectoryContents(src.c_str(), src_file_list);
		spdlog::info("src: found {} files", src_file_list.size());

		GetDirectoryContents(dst.c_str(), dst_file_list);
		spdlog::info("dst: found {} files", dst_file_list.size());

		vector<Duplication> dup = findExistingFile(src_file_list, dst_file_list);
		spdlog::info("found {} duplications", dup.size());

		print_dup_json(dup);
	}
	else // dst missing, find duplicate file
	{
		spdlog::debug("dst missing, find duplicate file");
		vector<MetaInfo> src_file_list;
		GetDirectoryContents(src.c_str(), src_file_list);

		spdlog::info("found {} files", src_file_list.size());

		vector<Duplication> dup = findDuplicateFile(src_file_list);
		spdlog::info("found {} duplications", dup.size());

		print_dup_json(dup);
	}

	return 0;
}

void time_it()
{
	clock_t begin = clock();

	string x = getHomePath();
	cout << x << endl;

	// do something

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "elaspsed time:" << elapsed_secs << endl;
}
