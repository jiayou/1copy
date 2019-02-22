#include <string>

using namespace std;

int createDatabase(string dst);
void createArray(string dst);
vector<wstring> get_all_files_names_within_folder(string folder);

std::string getHomePath();
bool ListDirectoryContents(const wchar_t *sDir);

class OneCopyConfig
{
	public:
	    bool dryrun=true;
    
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
bool GetDirectoryContents(const wchar_t *sDir, vector<MetaInfo>& M);

// vector<MetaInfo> GetDirectoryContents(const wchar_t *sDir);

void moveDuplicateFiles(vector<Duplication> dup, wstring root);
vector<Duplication> findExistingFile(vector<MetaInfo> src_file_list, vector<MetaInfo> dst_file_list);
vector<Duplication> findDuplicateFile(vector<MetaInfo> src_file_list);
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& s);

void undoMoveDuplicateFiles(wstring root);