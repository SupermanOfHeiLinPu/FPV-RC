#ifndef __CONFIG_PROTOCOL_H__
#define __CONFIG_PROTOCOL_H__

#include <stdint.h>

#pragma pack(1)
typedef struct msg_head_t {
    uint16_t msg_len;
    uint8_t type;
} MsgHead;

typedef struct dcp_broad_msg_t {
    MsgHead head;
    uint8_t channel;
}DcpBroadMsg;
#pragma pack()


#endif