#include "xtask_factory.h"

#include <iostream>

using namespace std;

int main()
{
    auto task = XTaskFactory::Create();
    XTask::Data data;
    data.src = "test.mp4";
    data.des = "output.mp4";
    data.task_type = "cv";
    task->Start(data);
    cout << task->Progress() << endl;
}