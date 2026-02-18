#ifndef COMMON_H
#define COMMON_H

#define STATUS_ERROR   -1
#define STATUS_SUCCESS 0

typedef enum {
    PROTO_HELLO,
} proto_type_e;

typedef struct {
    proto_type_e type;
    unsigned int len;
} proto_hdr_t;

#endif
