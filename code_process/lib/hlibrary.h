#ifndef HLIBRARY_H
#define HLIBRARY_H

#include <list>

#define DEAD -1
#define READY 0
#define RUNNING 1
#define BLOCK 2
#define INIT 0
#define USER 1
#define SYSTEM 2
#define FREE 0
#define ALLOCATED 1
#define WAIT_NONE -1
#define CR_LTH 3
#define DE_LTH 2
#define REQ_LTH 3
#define REL_LTH 3
#define TO_LTH 1
#define LP_LTH 1
#define LR_LTH 1
#define ALLOC_ERROR -1
#define REL_ERROR -2
#define WAKE_NONE -1
#define WAKEN 1

class PCB;
#endif