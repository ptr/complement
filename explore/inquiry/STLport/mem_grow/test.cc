// -*- C++ -*- Time-stamp: <02/09/25 12:11:17 ptr>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>

#include <string>
#include <list>
#include <iostream>

typedef std::list<std::string> file_list_t;

using namespace std;

void FileEnum(const std::string &_mask, file_list_t &_file_list)
{
    std::string file_name;
    std::string mask_name("/usr/bin");

    struct dirent **namelist;
    int n = scandir(mask_name.c_str(), &namelist, NULL, NULL);
    if (n < 0)
        return;
    else
    {
        while(n--)
        {
            if (fnmatch(_mask.c_str(), namelist[n]->d_name, 0) == 0)
            {
                file_name = namelist[n]->d_name;
                _file_list.push_back(file_name);
            }
            free(namelist[n]);
        }
        free(namelist);
    }
}

int main( int argc, char * const *argv )
{
    file_list_t fl;
    for (unsigned int i = 0; i < 10000000; i++)
    {
        fl.clear();
        FileEnum("z*", fl);
        if ((i % 100000) == 0) 
            std::cout<<"Processed: "<<i<<" directories, result is "<<fl.size()<<"."<<std::endl;
    }

  return 0;
}
