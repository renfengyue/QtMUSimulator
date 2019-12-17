/*
***********************************************************************************
**
** File         : all.h
** Description  : This file is used to implement all functions related to ethernet
**                interfaces.
**
** Modification
**      History : See ClearCase version history.
**
** All rights reserved (c) 2018
** Schneider Electric, Energy Business
**
** This computer program may not be used, copied, distributed, corrected, modified,
** translated, transmitted or assigned without =S='s prior written authorisation.
***********************************************************************************
*/

/*------------------------------------------------------------------------------
  INCLUDE FILES
  ----------------------------------------------------------------------------*/
#ifndef _ALL_H_
#define _ALL_H_

typedef unsigned char           uint8;
typedef char                    int8;
typedef unsigned short          uint16;
typedef short                   int16;
typedef unsigned int            uint32;
typedef int                     int32;
typedef float                   float32;
typedef double                  float64;

#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sched.h>
#include <time.h>
#include <getopt.h>
#if !defined WIN32
#include <inttypes.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include <unistd.h>
#include <pcap.h>
#include <pthread.h>
#include "globaltype.h"

#include "struct_frames92.h"

/*------------------------------------------------------------------------------
  PREPROCESSOR CONSTANTS
  ----------------------------------------------------------------------------*/
#define MAX_NB_LN               8
#define NB_CHNNL_PER_LN         24
#define PI                      3.1415926535897932384626
#define DSET_LEN                64
#define MAX_ETH_NUM             20
#define MAX_PATH                256
#define MAX_OFFSET              10
#define GOOSE_LENGTH            121
#define QUAL_INVALID            0x0001
#define QUAL_QUEST              0x0003
#define QUAL_TEST_MASK          0x0800
#define QUAL_MASK               0x0803

/*------------------------------------------------------------------------------
  PREPROCESSOR MACROS
  ----------------------------------------------------------------------------*/
#define VERSION                 "V0.6.1"
/*------------------------------------------------------------------------------
  PUBLIC STRUCTURE DEFINITIONS
  ----------------------------------------------------------------------------*/
#if defined _WIN32
typedef struct tagHighAccTmr
{
    LARGE_INTEGER liPerfFreq;
    LARGE_INTEGER liPerfStart;
    LARGE_INTEGER liPerfNow;
} HIGH_ACC_TMR;

LARGE_INTEGER li_perf_freq;
LARGE_INTEGER li_perf_start;
LARGE_INTEGER li_perf_now;
#endif

typedef struct tagETHInterfaceList
{
    unsigned int number;
    char description[MAX_ETH_NUM][200];
    char name[MAX_ETH_NUM][100];
} ETH_INTERFACE_LIST;

typedef struct tagFRAMEDATA
{
    uint16 length;
    uint8 data[512];
} FRAMEDATA;

typedef struct tagLNDATA
{
    uint8 PIDLength;
    int8 PID[256];
    struct
    {
        uint16 enable:1;
        uint16 zero:1;
        uint16 test:1;
        uint16 un:1;
        uint16 in:1;
        uint16 syn:2;
        uint16 CTF:1;
        uint16 validity:2;
    } state;
    float Umag[4][2];
    float Imag[4][2];
    uint8 TransUSpIn[NB_CHNNL_PER_LN];
    uint8 TransISpIn[NB_CHNNL_PER_LN];
} LNDATA;

typedef struct tagComtradeData
{
    uint32 dataindex;
    uint32 datasize;
    uint8  UaIndex;
    uint8  UbIndex;
    uint8  UcIndex;
    uint8  UnIndex;
    uint8  IaIndex;
    uint8  IbIndex;
    uint8  IcIndex;
    uint8  InIndex;
    struct
    {
        uint8 ferror:1;
        uint8 rerror:1;
    } state;
    int8* data;
} COMTRADE_DATA;

typedef enum
{
    SIM_TYPE_NORMAL         = 0,  /* Normal SV */
    SIM_TYPE_LOST_FRAMES_PER_1_CYCLE    = 1,    /* Lost configured sample in every cycle */
    SIM_TYPE_SMPCNT_MESS_UP = 2,  /* Sampled value with messed up smpCnt */
    SIM_TYPE_OFFSET         = 3,  /* There is an offset between the smpCnt of all MUs */
    SIM_TYPE_LOST_FRAMES_BETWEEN_1_CYCLE   = 4,        /* Lost configured sample in between 1 cycle */
    SIM_TYPE_LOST_FRAMES_BETWEEN_50_CYCLES    = 5,    /* Lost configured sample in between 50 cycle */
    SIM_TYPE_LOST_FRAMES_BETWEEN_60_CYCLES   = 6,    /* Lost configured sample in between 60 cycle */

    SIM_TYPE_MAX
} SIM_TYPE;

typedef struct tagMUConf
{
    uint32   mu_no;
    int8     sv_id[64];
    uint32   freq;
    uint32   asdu_no;
    uint32   channel_no;
    uint32   test;
    uint32   invalid;
    uint32   quest;
    int8     sync[8+1];
    SIM_TYPE sim_type;
    uint32   offset;
    uint32   simul;
    uint32   lost_num;
    int32    lost[20];
    int8     enableMULostFrame[8];
    float32  ia[8];
    float32  ib[8];
    float32  ic[8];
    float32  ua[8];
    float32  ub[8];
    float32  uc[8];
    float32  phase_a[8];
    float32  phase_b[8];
    float32  phase_c[8];
    float32  ctratio[8];
    float32  vtratio[8];
    uint32   int_no;
    uint32   samples_per_sec;
    uint32   samples_per_cycle;
} MU_CONF;

/*------------------------------------------------------------------------------
  EXTERN PUBLICH VARIABLES
  ----------------------------------------------------------------------------*/
//eth.c
extern pcap_t            *EthInterface;
extern ETH_INTERFACE_LIST EthInterfaces;

//frame.c
extern pthread_t  ThreadSend, ThreadRec;
extern uint16     FramesInPeriod;
extern uint8      FrameSendEnabled;
extern int32      InterfaceNo;
extern FRAMES_SV  FramesSV[MAX_NB_LN];
extern FRAMEDATA  FrameData[MAX_NB_LN];
extern LNDATA     LNData[MAX_NB_LN];

extern uint8      GOOSEBuffer[GOOSE_LENGTH];
extern uint16     GOOSESeq;
extern uint8      GOOSEState;

#endif /* _ALL_H_ */

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
