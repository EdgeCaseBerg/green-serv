#ifndef __SHA_256_H__
    #define __SHA_256_H__
    #include <openssl/sha.h>
    #include <stdio.h>
    #include <string.h>

    void sha256(char *string, char outputBuffer[65]);
#endif