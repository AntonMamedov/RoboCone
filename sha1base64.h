#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/sendfile.h>

#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))

#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]

#else
#error "Endianness not defined!"
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

typedef struct
 {
   uint32_t state[5];
   uint32_t count[2];
   unsigned char buffer[64];
 } SHA1_CTX;

static char GUIDKey[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //36
static unsigned char charset[]={"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

void SHA1Transform( uint32_t state[5], const unsigned char buffer[64]);


void SHA1Init( SHA1_CTX * context);


void SHA1Update( SHA1_CTX * context, const unsigned char *data, uint32_t len);


void SHA1Final( unsigned char digest[20], SHA1_CTX * context);


void SHA1(unsigned char *hash_out, const char *str, unsigned int len);


int base64_encode(unsigned char sha_key_in[], unsigned char base64_key_out[], int len);
