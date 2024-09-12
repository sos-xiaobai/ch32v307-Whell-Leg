#ifndef __TEST__
#define __TEST__

#include "stdint.h"

class Class_Test {
public:

    char16_t a;
    int16_t cnt;

    void Cnt_Pluss();

private:
    bool full_flag;
};

extern Class_Test test_1;

#endif

