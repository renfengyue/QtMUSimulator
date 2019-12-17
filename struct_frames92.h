#if !defined __STRUCT_FRAMES_SV_H__
#define __STRUCT_FRAMES_SV_H__

//#include "all.h"

#define K_MAX_ASDU    8

struct CHANNEL
{
    unsigned char sample[4];
    unsigned char quality[4];
};

struct DATASET
{
    struct CHANNEL data[24];
};

struct SEQ_DATA
{
    unsigned char tag;
    unsigned char length[2];
    struct DATASET DataSet;
};

struct SMP_SYNCH
{
    unsigned char tag;
    unsigned char length;
    unsigned char value;
};

struct CONF_REV
{
    unsigned char tag;
    unsigned char length;
    unsigned char value[4];
};

struct SMP_CNT
{
    unsigned char tag;
    unsigned char length;
    unsigned char value[2];
};

struct SV_ID
{
    unsigned char tag;
    unsigned char length;
    unsigned char svID[256];
};

struct ASDU
{
    struct SV_ID svID;
    struct SMP_CNT smpCnt;
    struct CONF_REV confRev;
    struct SMP_SYNCH smpSynch;
    struct SEQ_DATA SeqOfData;
};

struct SEQ_ASDU1
{
    unsigned char tag;
    unsigned char length[2];
    struct ASDU ASDU;
};

struct SEQ_ASDU
{
    unsigned char tag;
    unsigned char length[3];
    struct SEQ_ASDU1 SeqOfASDU[K_MAX_ASDU];
};

struct NO_ASDU
{
    unsigned char tag;
    unsigned char length;
    unsigned char value;
};

struct SAV_PDU
{
    unsigned char tag;
    unsigned char length[3];
    struct NO_ASDU noASDU;
    struct SEQ_ASDU SeqOfASDU;
};


struct APDU
{
    struct SAV_PDU savPDU;
};

struct PDU_TYPE
{
    unsigned char appid[2];
    unsigned char length[2];
    unsigned char reserved1[2];
    unsigned char reserved2[2];
    struct APDU apdu;
};

struct PRIO_TAG
{
    unsigned char priotag;
    unsigned char VID;
    unsigned char ethertype[2]; // 88BA
};

struct HEADER_MAC
{
    unsigned char dst[6];
    unsigned char src[6];
    unsigned char ethertype[2]; // 8100
};

typedef struct tagFRAMES_SV
{
    struct HEADER_MAC HeaderMAC;
    struct PRIO_TAG PrioTAG;
    struct PDU_TYPE PDU;
} FRAMES_SV;

#endif

