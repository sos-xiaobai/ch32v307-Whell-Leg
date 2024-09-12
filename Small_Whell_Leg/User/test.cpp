#include "test.h"


void Class_Test::Cnt_Pluss()
{
    cnt++;
    if(cnt>1000){
        cnt=0;
        full_flag = true;
    }
}


Class_Test test_1;
