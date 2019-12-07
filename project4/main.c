#include "filesys.c"




int main() {


    char buff[20];
    printf("Enter disk file name: \n");
    fgets(buff, 20, stdin);





    make_fs(buff);

    shell();




    return 0;
}