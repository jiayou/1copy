
# 1copy

A backup tool that prevent duplicate files. (WIN32)

## Usage:
### find duplicate files
* 1copy C:\User\photo

[] list files in source folder
[] find duplication 
[] open result in web browser

### cross check duplicate files
* 1copy E:\DCIM --target=C:\User\photo

[] list files in source folder, target folder
[] find duplication 
[] move duplicate files to source/.1copy, keep folder structure
[] do NOT move files if .1copy already exists
[] open result in web browser

* 1copy --undo E:\DCIM

[] merge back duplicate files (undo move)
[] delete dup.1copy

# Dev Notes
* sqlite performance too bad

* ascii (174) (R) character in file name cause std::filesystem::path::tostring fail on win32
=> turn to WIN32API

* unicode, locale, etc:
https://www.cnblogs.com/dejavu/archive/2012/09/16/2687586.html

* string to wstring (WIN32API)
https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
Replacing every std::string with std::wstring,
Replacing every char with a wchar_t*
Replacing every literal string("") with L""
Making use of the TCHAR support in Windows etc. 


