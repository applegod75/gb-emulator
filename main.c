#include <string.h>
#include <stdio.h>
#include "argparse.h"
#include "gb_emulator.h"

int main(int argc, char** argv){
    Args* a = parseArgs(argc, argv);
    if (a == NULL) return 1;
    State* state = initGameboy(a);
}
