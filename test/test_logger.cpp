#include "logging.h"
#include <string>
#include <unistd.h>

using namespace easynet;

int main()
{
    char buff[250];
    getcwd(buff, 250);
    std::string filename = buff;
    filename += "/../build/mylog.txt";

    setlogfile(filename);
    info("abcdefghijklmn");
    return 0;
}