#include <string>
using namespace std;

class OneCopyConfig
{
  public:
    bool dryrun = true;
};

struct Duplication
{
    wstring filename;
    unsigned long long filesize;
    wstring path;
    list<wstring> paths;
};

struct MetaInfo
{
    wstring path;
    wstring filename;
    unsigned int filesize;
    bool checked;
};

std::wstring s2ws(const std::string &s);
std::string ws2s(const std::wstring &s);
std::string getHomePath();

bool GetDirectoryContents(const wchar_t *sDir, vector<MetaInfo> &M);
vector<Duplication> findExistingFile(vector<MetaInfo> src_file_list, vector<MetaInfo> dst_file_list);
vector<Duplication> findDuplicateFile(vector<MetaInfo> src_file_list);
