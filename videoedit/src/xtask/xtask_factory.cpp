#include "xtask_factory.h"
#include "fftask.h"

#include <iostream>

using namespace std;

unique_ptr<XTask> XTaskFactory::Create(int type)
{
    switch (type)
    {
    case 0:
        return make_unique<FFTask>();
    default:
        cerr << "Task type " << type << " not support!" << endl;
        return make_unique<FFTask>();
    }
}