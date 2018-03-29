#if !defined(__SCAN_FILE_H__)
#define __SCAN_FILE_H__

#include <string>
#include <vector>
#include <regex.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include "dstream.h"
#include "user_exception.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::vector;
using util::bad_file;
using util::bad_system;
    
struct scan_info
{
    string file_dir;
    string file_name;
    int create_time;
};

// An operator() which means lhs > rhs is needed if you wan't to get file in ascending order.
class scan_compare
{
public:
    // Sort by file creation time and file_name in descending order, so get file in back will be
    // in ascending order.
    bool operator()(const scan_info& lhs, const scan_info& rhs)
    {
        if (lhs.file_name > rhs.file_name)
            return true;
        //else if (lhs.create_time == rhs.create_time && lhs.file_name > rhs.file_name)
        //    return true;
        else
            return false;
    }
};

template <typename compare = scan_compare>
class scan_file
{
public:
    // Scan file in single-directory mode.
    scan_file(const string& file_dir, const string& pattern_, int file_count = 1024)
        : dir_vector(1, file_dir)
    {
        if (regcomp(&reg, pattern_.c_str(), REG_NOSUB | REG_EXTENDED))
            throw bad_system(__FILE__, __LINE__, 179, bad_system::bad_reg, pattern_);
        file_vector.reserve(file_count);
    }
    
    // Scan file in multi-directory mode.
    scan_file(const vector<string>& dir_vector_, const string& pattern_, int file_count = 1024)
        : dir_vector(dir_vector_)
    {
        if (regcomp(&reg, pattern_.c_str(), REG_NOSUB | REG_EXTENDED))
            throw bad_system(__FILE__, __LINE__, 179, bad_system::bad_reg, pattern_);
        file_vector.reserve(file_count);
    }

    // Scan file in dir/sub-dirs mode.
    scan_file(const string& dir, const vector<string>& sub_dirs, const string& pattern_, int file_count = 1024)
    {
        vector<string>::const_iterator iter;
        for (iter = sub_dirs.begin(); iter != sub_dirs.end(); ++iter)
            dir_vector.push_back(dir + '/' + *iter);
        if (regcomp(&reg, pattern_.c_str(), REG_NOSUB | REG_EXTENDED))
            throw bad_system(__FILE__, __LINE__, 179, bad_system::bad_reg, pattern_);
        file_vector.reserve(file_count);
    }

    virtual ~scan_file()
    {
        regfree(&reg);
    }
    
    // Get a file in given directories. Upon file found, return true, otherwise return false.
    // In single-directory mode, return file name, otherwise return full name.
    bool get_file(string& file_name)
    {
        DIR* dirp;
        dirent ent;
        dirent* result;
        struct stat stat_buf;
        string full_name;
        scan_info file_info;
        dout << "begin of get_file" << endl ;
        dout << "sizeof dir_vector " << dir_vector.size() << endl ;
        while (file_vector.size() > 0)
        {
            vector<scan_info>::iterator iter = file_vector.begin();
            if (access((iter->file_dir + '/' + iter->file_name).c_str(), F_OK) == -1)
            {
                std::pop_heap(file_vector.begin(), file_vector.end(), compare());
                file_vector.pop_back();
                continue;
            }
            if (dir_vector.size() == 1)
                file_name = iter->file_name;
            else
                file_name = iter->file_dir + '/' + iter->file_name;
            std::pop_heap(file_vector.begin(), file_vector.end(), compare());
            file_vector.pop_back();
            return true;
        }

        vector<string>::const_iterator dir_iter;
        for (dir_iter = dir_vector.begin(); dir_iter != dir_vector.end(); ++dir_iter)
        {
            dirp = opendir(dir_iter->c_str());
            if(!dirp)
                throw bad_file(__FILE__, __LINE__, 180, bad_file::bad_opendir, *dir_iter);

            while (readdir_r(dirp, &ent, &result) == 0 && result != 0)
            {
                if (strcmp(ent.d_name, ".") == 0
                    || strcmp(ent.d_name, "..") == 0)
                    continue;
                if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) != 0)
                    continue;
                full_name = *dir_iter + '/' + ent.d_name;
                if (::lstat(full_name.c_str(), &stat_buf) < 0){

                    char tmpbuf[1024] ;
                    sprintf(tmpbuf, "%s : error is [%d]%s", full_name.c_str(), errno, strerror(errno)) ;
                    full_name = tmpbuf ;

                    throw bad_file(__FILE__, __LINE__, 38, bad_file::bad_lstat, full_name);
                }
                    
                if (S_ISDIR(stat_buf.st_mode) == 0)
                {
                    file_info.file_dir = *dir_iter;
                    file_info.file_name = ent.d_name;
                    file_info.create_time = stat_buf.st_mtime;
                    file_vector.push_back(file_info);
                }
            }
            closedir(dirp);
        }

        if (file_vector.size() > 0)
        {
            std::make_heap(file_vector.begin(), file_vector.end(), compare());
            while (file_vector.size() > 0)
            {
                vector<scan_info>::iterator iter = file_vector.begin();
                if (access((iter->file_dir + '/' + iter->file_name).c_str(), F_OK) == -1)
                {
                    std::pop_heap(file_vector.begin(), file_vector.end(), compare());
                    file_vector.pop_back();
                    continue;
                }
                if (dir_vector.size() == 1)
                    file_name = iter->file_name;
                else
                    file_name = iter->file_dir + '/' + iter->file_name;
                std::pop_heap(file_vector.begin(), file_vector.end(), compare());
                file_vector.pop_back();
                return true;
            }
            return false;
        }
        else
        {
            return false;
        }
    }

    // Get all files in given directories.
    // In single-directory mode, return file name, otherwise return full name.
    void get_files(vector<string>& files)
    {
        DIR* dirp;
        dirent ent;
        dirent* result;
        struct stat stat_buf;
        string full_name;

        files.resize(0);
        vector<string>::const_iterator dir_iter;
        for (dir_iter = dir_vector.begin(); dir_iter != dir_vector.end(); ++dir_iter)
        {
            dirp = opendir(dir_iter->c_str());
            if(!dirp)
                throw bad_file(__FILE__, __LINE__, 180, bad_file::bad_opendir, *dir_iter);

            while (readdir_r(dirp, &ent, &result) == 0 && result != 0)
            {
                if (strcmp(ent.d_name, ".") == 0
                	|| strcmp(ent.d_name, "..") == 0)
                	continue;
                full_name = *dir_iter + '/' + ent.d_name;
                if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) != 0)
                    continue;
                if (::lstat(full_name.c_str(), &stat_buf) < 0)
                    throw bad_file(__FILE__, __LINE__, 38, bad_file::bad_lstat, full_name);
                if (S_ISDIR(stat_buf.st_mode) == 0)
                {
                    if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) == 0)
                        files.push_back(ent.d_name);
                }
            }
            closedir(dirp);
        }
    }
    
private:
    vector<string> dir_vector;
    regex_t reg;
    vector<scan_info> file_vector;
};

}

#endif
