#ifndef TEST_H_
#define TEST_H_

#include "boot.h"


void TestFunction(void) {
    ComInit();

    // Jump to app
    JumpToApp(0x8006000);

    while (1)
        BootStateMachine();
}


#endif /* TEST_H_ */