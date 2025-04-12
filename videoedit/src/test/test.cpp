#include "xvideo_input.h"
#include "xtask_factory.h"

#include "log_factory.h"

#include <iostream>

using namespace std;

int main()
{
    XLOGINIT();
    LOG_DEBUG("log debug");
    // XDir dir;
    // auto files = dir.GetFiles(".");
    // for (auto& f : files)
    //     cout << f.name << endl;
    XVideoInput input;
    input.Start(XTaskFactory::Create());
}