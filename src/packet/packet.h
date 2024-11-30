//
// Created by Maxence on 29/11/2024.
//
#ifndef PACKET
#define TYPE_INT32 0
#define TYPE_INT64 1
#define TYPE_FLOAT 2
#define TYPE_CHAR 3
#define OTHERS 4

#include <stdint.h>

typedef struct packet{
    int32_t type;
    char message[32];
} PACKET;

#endif //PACKET