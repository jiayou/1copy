
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <windows.h>
#include "spdlog/spdlog.h"

#include "onecopy.hpp"

using namespace std;
namespace fs = std::experimental::filesystem;

std::wstring s2ws(const std::string &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string ws2s(const std::wstring &s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    char *buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;
}

bool GetDirectoryContents(const wchar_t *sDir, vector<MetaInfo> &M)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    MetaInfo f;

    wchar_t sPath[2048];

    //Specify a file mask. *.* = We want everything!
    wsprintf(sPath, L"%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Path not found: [%s]\n", sDir);
        return false;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // wprintf(L"Directory: %s\n", sPath);
                GetDirectoryContents(sPath, M); //Recursion, I love it!
            }
            else
            {
                // wprintf(L"File: %s\n", sPath);
                f.path = sPath;
                f.filename = fdFile.cFileName;
                f.filesize = fdFile.nFileSizeLow;
                f.checked = false;
                M.push_back(f);
            }
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

bool ListDirectoryContents(const wchar_t *sDir)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    wchar_t sPath[2048];

    //Specify a file mask. *.* = We want everything!
    wsprintf(sPath, L"%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Path not found: [%s]\n", sDir);
        return false;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                wprintf(L"Directory: %s\n", sPath);
                ListDirectoryContents(sPath); //Recursion, I love it!
            }
            else
            {
                wprintf(L"File: %s\n", sPath);
            }
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

vector<wstring> get_all_files_names_within_folder(string folder)
{
    vector<wstring> names;
    string search_path = folder + "/*.*";
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(s2ws(search_path).c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                names.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return names;
}

void createArray(string dst)
{
    for (fs::recursive_directory_iterator i(dst), end; i != end; ++i)
    {
        if (!fs::is_directory(i->path()))
        {
            cout << i->path() << endl;
        }
    }
}

string getHomePath()
{
    string drive = std::getenv("HOMEDRIVE");
    string path = std::getenv("HOMEPATH");

    return drive + path;
}

string getOneCopyPath()
{
    return getHomePath() + "\\.1copy";
}

string getDateString()
{
    std::time_t t = std::time(nullptr);
    char mbstr[1000];
    cout << "43" << endl;
    if (std::strftime(mbstr, sizeof(mbstr), "%Y%m%dT%H%M%S", std::localtime(&t)))
    {
        return string(mbstr);
    }
    return string("TIME");
}

string generateDatabaseName(string dst)
{
    return "1copy_" + getDateString() + string(".db");
}

vector<Duplication> findDuplicateFile(vector<MetaInfo> src_file_list)
{
    vector<Duplication> dup;
    Duplication d;

    while (!src_file_list.empty())
    {
        auto &f = src_file_list.back();
        if (f.checked)
        {
            src_file_list.pop_back();
            continue;
        }

        d.filename = f.filename;
        d.filesize = f.filesize;
        d.path = f.path;
        d.paths.clear();
        d.paths.push_back(f.path);

        spdlog::debug("compare {}", ws2s(d.path).c_str());
        for (auto &v : src_file_list)
        {
            spdlog::trace("  with {}", ws2s(v.filename).c_str());
            if (0 == d.filename.compare(v.filename) &&
                d.filesize == v.filesize &&
                0 != d.path.compare(v.path) &&
                !v.checked)
            {
                spdlog::debug("  identical: {} ", ws2s(v.path).c_str());
                d.paths.push_back(v.path);
                v.checked = true;
            }
        }
        src_file_list.pop_back();

        if (d.paths.size() > 1)
        {
            dup.push_back(d);
        }
    }
    return dup;
}

vector<Duplication> findExistingFile(vector<MetaInfo> src_file_list, vector<MetaInfo> dst_file_list)
{
    vector<Duplication> dup;
    Duplication d;

    for (auto &f : src_file_list)
    {
        d.filename = f.filename;
        d.filesize = f.filesize;
        d.path = f.path;
        d.paths.clear();

        for (auto &v : dst_file_list)
        {
            if (0 == f.filename.compare(v.filename) && f.filesize == v.filesize)
            {
                d.paths.push_back(v.path);
            }
        }

        if (!d.paths.empty())
        {
            dup.push_back(d);
        }
    }
    return dup;
}

extern OneCopyConfig config;

void moveDuplicateFiles(vector<Duplication> dup, wstring root)
{
    wstring new_root = root + L"\\.1copy\\";
    for (auto &f : dup)
    {
        wstring src = f.path;
        wstring dst = src; //.replace(root, new_root);

        if (config.dryrun)
        {
            fs::create_directories(fs::path(dst).parent_path());
            fs::rename(src, dst);
        }
        else
        {
            wprintf(L"Move %s\n", src.c_str());
        }
    }
}

void undoMoveDuplicateFiles(wstring root)
{
}

void Serialization()
{
    /* 
        http://uscilab.github.io/cereal/quickstart.html
    */
}
