#include <iostream>
#include <string>

#include "xexec.h"
#include "user_input.h"

using namespace std;

void TestXExec(const char* cmd)
{
    XExec exec;
    exec.Start(cmd);

    string result;
    cout << "while" << endl;
    while (true)
    {
        std::string line = exec.GetOutput();  // 阻塞获取
            std::cout << line << std::flush;
        
        if (!exec.Running())
            break;
    }
}

void TestUserInput()
{
    UserInput input;

    input
        // 注册参数处理函数
        .Register("-s", [](const string& s)
            {
                cout << "-s" << s << endl;
            }
        )
        .Register("-d", [](const string& s)
            {
                cout << "-d" << s << endl;
            }
        )
        .Register("-p", [](const string& s)
            {
                cout << "-p password" << s << endl;
            }
        )
        // 注册任务处理函数
        .RegisterTask("cv", []{ cout << "cv function" << endl; })
    ;

    input.Start();
}

int main()
{
    TestXExec("ffplay run.mp4");
}