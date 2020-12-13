#include "shell.h"

int main(int argc, char *argv[])
{
    Shell *test_shell= new Shell();
    test_shell->shellRunning(argc, argv);
    return 0;
}