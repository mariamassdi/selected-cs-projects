#include <iostream>
#include "signal.h"
#include "signals.h"
#include "Commands.h"
#include <unistd.h>
using namespace std;

void ctrlCHandler(int sig_num) {

  
    std::cout << "smash: got ctrl-C"<<endl;
    SmallShell &smash = SmallShell::getInstance();
    if (smash.getPID() != -1)
    {
        if (kill(smash.getPID(), SIGKILL) == -1)
        {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.getPID() << " was killed"<<endl;
        smash.setPID(-1);
    }
    }
