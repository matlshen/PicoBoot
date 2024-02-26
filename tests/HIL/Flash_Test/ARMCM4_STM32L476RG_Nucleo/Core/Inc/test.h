#ifndef TEST_H_
#define TEST_H_

#include "boot.h"


void TestFunction(void) {
    ComInit();

    while (1)
        BootStateMachine();
}


#endif /* TEST_H_ */