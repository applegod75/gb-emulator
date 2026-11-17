#include <string.h>
#include <stdio.h>

typedef struct {
    int skip_bootrom;
    int cgb_mode;
    void* rom_data;
} Args;

Args parseArgs(int argc, char** argv){
    Args rtn;
}

int main(int argc, char** argv){
    if (argc < 2) {
        printf("Usage: gb_emulator <rom>\n");
        return 1;
    }



}
