#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]){
    //parse argments
    for(int i = 0; argv[i]; i++){
        printf("%s\n", argv[i]);
    }
}