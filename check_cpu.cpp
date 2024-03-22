
#include "check_cpu.h"


std::istream& operator>>(std::istream &in, std::vector<uint32_t> &time) 
{
    std::string name_proc;
    in >> name_proc \
        >> time[0] \
        >> time[1] \
        >> time[2] \
        >> time[3] \
        >> time[4] \
        >> time[5] \
        >> time[6] \
        >> time[7] \
        >> time[8] \
        >> time[9];
    // std::cout << "Proc name " << name_proc << std::endl;    
    return in;
}