
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

string getHomePath()
{
    string drive = std::getenv("HOMEDRIVE");
    string path = std::getenv("HOMEPATH");

    return drive + path;
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
