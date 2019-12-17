/*
***********************************************************************************
**
** File         : frame.c
** Description  : This file is used to make package and publish SV via link layer.
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
#include <time.h>

#include "all.h"
#include "eth.h"
#include "util.h"
#include "frame.h"

#if defined _LINUX
#undef _WIN32
#endif

/*------------------------------------------------------------------------------
  PREPROCESSOR CONSTANTS
  ----------------------------------------------------------------------------*/
/* The number of nano seconds per sec. */
#define NSEC_PER_SEC                    (1000000000)

/*
 * we use 99 as the PRREMPT_RT use 50 as the priority of kernel tasklets
 * and interrupt handler by default.
 */
#define MUSIM_PRIORITY                  (99)

#define NB_FRAMES_PER_CYCLE_61850_9_2   (80)
#define NB_FRAMES_PER_CYCLE_61869_50HZ  (96)
#define NB_FRAMES_PER_CYCLE_61869_60HZ  (80)

#define NB_NS_PER_CIRCLE_61850_9_2      (NSEC_PER_SEC / NB_FRAMES_PER_CYCLE_61850_9_2)
#define NB_NS_PER_CIRCLE_61869_50HZ     (NSEC_PER_SEC / NB_FRAMES_PER_CYCLE_61869_50HZ)
#define NB_NS_PER_CIRCLE_61869_60HZ     (NSEC_PER_SEC / NB_FRAMES_PER_CYCLE_61869_60HZ)

#define SR_OF_2                         (1.41421356237)

#define TWO_PI                          (2.0 * PI)
#define ANG_2_RAD                       (PI / 180.0)

/*------------------------------------------------------------------------------
  PREPROCESSOR MACROS
  ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  PUBLIC STRUCTURE DEFINITIONS
  ----------------------------------------------------------------------------*/
struct TRANSDATASET
{
    uint16 SmpCnt;
    struct CHANNEL DataI[9];
    struct CHANNEL DataU[9];
} LNDataset, LNDatasetTmp;

/*------------------------------------------------------------------------------
  PUBLIC VARIABLES
  ----------------------------------------------------------------------------*/
int32      InterfaceNo;
uint8      FrameSendEnabled;
uint16     FramesInPeriod;
FRAMES_SV  FramesSV[MAX_NB_LN];
FRAMEDATA  FrameData[MAX_NB_LN];
LNDATA     LNData[MAX_NB_LN];

pthread_t  ThreadSend, ThreadRec;

/*------------------------------------------------------------------------------
  PRIVATE VARIABLES
  ----------------------------------------------------------------------------*/
static int32 ComtradeTime;

static int32 Samples[MAX_NB_LN][NB_CHNNL_PER_LN][NB_FRAMES_PER_CYCLE_61869_50HZ];

static uint16 FrameSmpCnt;

static struct sched_param ParamSent;

static const uint8 MulticastMAC[6] = {0x01, 0x0c, 0xcd, 0x04, 0x00, 0x00};
static const uint8 SourceMAC[6]    = {0x00, 0x02, 0x84, 0x90, 0x69, 0x01};

/*------------------------------------------------------------------------------
  EXTERNAL REFERENCES
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PRIVATE FUNCTION PROTOTYPES
  ----------------------------------------------------------------------------*/
uint16 check_length(uint32 length);
uint8 check_copylength(unsigned char* length, unsigned char* addr);
uint16 check_length_special(uint32 length);
uint8 check_copylength_special(unsigned char* length, unsigned char* addr);

#if defined _WIN32
HIGH_ACC_TMR highAccTmr;

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: delay_us
*/
int8 delay_us(const int duration)
{
    int delayTime = duration;

    memset(&highAccTmr, 0, sizeof(highAccTmr));

    if (!QueryPerformanceFrequency(&highAccTmr.liPerfFreq))
        return -1;

    QueryPerformanceCounter(&highAccTmr.liPerfStart);

    for(;;)
    {
        QueryPerformanceCounter(&highAccTmr.liPerfNow);
        double time = (((highAccTmr.liPerfNow.QuadPart - highAccTmr.liPerfStart.QuadPart) * 1000000) /
                       (double) highAccTmr.liPerfFreq.QuadPart);

        if (time >= delayTime)
            break;
    }

    return 0;
}


/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: clock_gettime
*/
int clock_gettime(struct timespec *spec)
{
    __int64 wintime;

    GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime      -= 116444736000000000;
    spec->tv_sec  = wintime / 10000000;
    spec->tv_nsec = wintime % 10000000 *100;

    return 0;
}
#endif /* #if defined _WIN32 */

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: update_sync_state
*/
void update_sync_state(MU_CONF *p_mu_conf, int8 *sync)
{
    uint32 i, k;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        LNData[i].state.syn = sync[i] - 0x30;
        if (LNData[i].state.syn > 2)
        {
            LNData[i].state.syn = 1;
        }
        for (k=0; k<p_mu_conf->asdu_no; k++)
        {
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpSynch.value = LNData[i].state.syn;
        }
    }
    strcpy(p_mu_conf->sync, sync);
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: update_current_phsA
*/
void update_current_phsA(MU_CONF *p_mu_conf, float32 val)
{
    uint32 i;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        LNData[i].Imag[0][0] = val;
        p_mu_conf->ia[i] = val;
    }

    init_table_samples(p_mu_conf);
}


/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: update_voltage_phsA
*/
void update_voltage_phsA(MU_CONF *p_mu_conf, float32 val)
{
    uint32 i;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        LNData[i].Umag[0][0] = val;
        p_mu_conf->ua[i] = val;
    }

    init_table_samples(p_mu_conf);
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: init_frames_92
*/
void init_frames_92(MU_CONF *p_mu_conf)
{
    uint32 i, j, k;
    uint16 *p_data;
    uint32 length;

    for(i=0; i<MAX_NB_LN; i++)
    {
        memcpy(FramesSV[i].HeaderMAC.dst, MulticastMAC, 6);
        FramesSV[i].HeaderMAC.dst[5] += (i+1);
        memcpy(FramesSV[i].HeaderMAC.src, SourceMAC, 6);
        FramesSV[i].HeaderMAC.src[5] += (i+1);

        p_data = (uint16*) FramesSV[i].HeaderMAC.ethertype;
        *p_data = htons(0x8100);

        FramesSV[i].PrioTAG.priotag = 0x80;
        FramesSV[i].PrioTAG.VID = 0x01;
        *(uint16*) FramesSV[i].PrioTAG.ethertype = htons(0x88BA);

        *(uint16*) FramesSV[i].PDU.appid = htons(0x4001+i);
        p_data = (uint16*) FramesSV[i].PDU.reserved1;
        if (p_mu_conf->simul & (0x01 << i))
            *p_data |= 0x80;
        else
            *p_data &= 0x7F;
        *(uint16*) FramesSV[i].PDU.reserved2 = 0;

        FramesSV[i].PDU.apdu.savPDU.tag = 0x60;

        FramesSV[i].PDU.apdu.savPDU.noASDU.tag = 0x80;
        FramesSV[i].PDU.apdu.savPDU.noASDU.length = 0x01;
        FramesSV[i].PDU.apdu.savPDU.noASDU.value = p_mu_conf->asdu_no;

        FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.tag = 0xa2;

        for (k=0; k<p_mu_conf->asdu_no; k++)
        {
            length = 0;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].tag = 0x80;

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].tag = 0x30;

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.svID.tag = 0x80;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.svID.length = LNData[i].PIDLength;
            strcpy((char*)FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.svID.svID,LNData[i].PID);

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpCnt.tag = 0x82;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpCnt.length = 0x02;
            *(uint16*) FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpCnt.value = htons(0);

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.confRev.tag = 0x83;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.confRev.length = 0x04;
            *(uint32*) FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.confRev.value = htonl(1);

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpSynch.tag = 0x85;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpSynch.length = 0x01;
            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpSynch.value = LNData[i].state.syn;

            FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.tag = 0x87;

            length = 0x08 * p_mu_conf->channel_no;	//0x87 seqData length
            *(uint16*)FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.length = htons(length);

            length += check_length(length);
            length += 16 + LNData[i].PIDLength; //0x30 SeqOfASDU length
            *(uint16*)FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].length = htons(length);

            length += check_length(length);
            length += 1;	// 0xa2 SeqOfASDU length
        }

        length *= p_mu_conf->asdu_no;
        *(uint16*)FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.length = htons(length);

        if (p_mu_conf->asdu_no == 2)
            length += check_length_special(length);
        else
            length += check_length(length);
        length += 4;	// 0x60 savPDU length
        *(uint16*)FramesSV[i].PDU.apdu.savPDU.length = htons(length);

        length += check_length(length);
        length += 9;	// PDU.length
        *(uint16*) FramesSV[i].PDU.length = htons(length);

        if(LNData[i].state.test)
        {
            for(j=0; j<p_mu_conf->channel_no; j++)
            {
                for(k=0; k<p_mu_conf->asdu_no; k++)
                {
                    FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet.data[j].quality[2] = 0x08;
                }
            }
        }
        else
        {
            for(j=0; j<p_mu_conf->channel_no; j++)
            {
                for(k=0; k<p_mu_conf->asdu_no; k++)
                {
                    FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet.data[j].quality[2] = 0x00;
                }
            }
        }

        if(LNData[i].state.validity)
        {
            for(j=0; j<p_mu_conf->channel_no; j++)
            {
                for(k=0; k<p_mu_conf->asdu_no; k++)
                {
                    FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet.data[j].quality[3] = LNData[i].state.validity;
                }
            }
        }
        else
        {
            for(j=0; j<p_mu_conf->channel_no; j++)
            {
                for(k=0; k<p_mu_conf->asdu_no; k++)
                {
                    FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet.data[j].quality[3] = 0x00;
                }
            }
        }
    }
}

/*return the bytes occupied by length
 * Length <  0x80 1 byte ; b_2 not used ; b_1 not used 		  ; b_0 = real length
 * Length >= 0x80 2 bytes; b_2 not used ; b_1 = 0x81   		  ; b_0 = real length
 * Length >  0xff 3 bytes; b_2 = 0x82   ; b_1= real length MSB; b_0 = real length LSB
 *
 * */
uint16 check_length(uint32 length)
{
    if (length < 0x80)
        return 1;
    else if (length > 0xff)
        return 3;
    else
        return 2;
}

/*return the bytes occupied by length
 * Length <  0x80 1 byte ; b_2 not used ; b_1 not used 		  ; b_0 = real length
 * Length >= 0x80 2 bytes; b_2 not used ; b_1 = 0x81   		  ; b_0 = real length
 * Length >  0xff 3 bytes; b_2 = 0x82   ; b_1= real length MSB; b_0 = real length LSB
 *
 * */
uint8 check_copylength(unsigned char* hlen,unsigned char* addr)
{
    unsigned char l_1 = 0x81;
    unsigned char l_2 = 0x82;
    uint16 length = ntohs(*(uint16*)hlen);

    if (length < 0x80)
    {
        memcpy(addr, hlen+1, 1);
        return 1;
    }
    else if (length > 0xff)
    {
        memcpy(addr, &l_2, 1);
        memcpy(addr+1, hlen, 2);
        return 3;
    }
    else
    {
        memcpy(addr, &l_1, 1);
        memcpy(addr+1, hlen+1, 1);
        return 2;
    }
}

/*return the bytes occupied by length
 * Length <= 0xff 2 bytes; b_2 not used ; b_1 = 0x81   		  ; b_0 = real length
 * Length >  0xff 3 bytes; b_2 = 0x82   ; b_1= real length MSB; b_0 = real length LSB
 *
 * */
uint16 check_length_special(uint32 length)
{
    (void)length;
    return 3;
}

/*return the bytes occupied by length
 * Length <= 0xff 2 bytes; b_2 not used ; b_1 = 0x81   		  ; b_0 = real length
 * Length >  0xff 3 bytes; b_2 = 0x82   ; b_1= real length MSB; b_0 = real length LSB
 *
 * */
uint8 check_copylength_special(unsigned char* hlen,unsigned char* addr)
{
    unsigned char l_2 = 0x82;

    memcpy(addr, &l_2, 1);
    memcpy(addr+1, hlen, 2);

    return 3;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: copy_frames_sv
*/
void copy_frames_sv(MU_CONF *p_mu_conf)
{
    uint16 loc_index;
    uint8 i;
    uint32 k;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        loc_index = 0;

        memcpy(FrameData[i].data + loc_index, (uint8*) &FramesSV[i], 27);
        loc_index += 27;

        // 0x60 savPDU.length check
        if (p_mu_conf->asdu_no == 2)
        {
            loc_index += check_copylength_special(FramesSV[i].PDU.apdu.savPDU.length,FrameData[i].data + loc_index);
                    memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.noASDU.tag, 4);
            loc_index += 4;

            // 0xa2 SeqOfASDU.length check
            loc_index += check_copylength_special(FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.length,FrameData[i].data + loc_index);
        }
        else
        {
            loc_index += check_copylength(FramesSV[i].PDU.apdu.savPDU.length,FrameData[i].data + loc_index);
            memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.noASDU.tag, 4);
            loc_index += 4;

            // 0xa2 SeqOfASDU.length check
            loc_index += check_copylength(FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.length,FrameData[i].data + loc_index);
        }

        for(k=0; k<p_mu_conf->asdu_no; k++)
        {
            memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].tag, 1);
            loc_index += 1;

            // 0x30 SeqOfASDU.length check
            loc_index += check_copylength(FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].length,FrameData[i].data + loc_index);

            memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.svID.tag, (2 + LNData[i].PIDLength));
            loc_index += 2 + LNData[i].PIDLength;

            memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpCnt.tag, 14);
            loc_index += 14;

            // 0x87 SeqOfASDU.length check
            loc_index += check_copylength(FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.length,FrameData[i].data + loc_index);

            memcpy(FrameData[i].data + loc_index, &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet, 0x08 * p_mu_conf->channel_no);
            loc_index += 0x08 * p_mu_conf->channel_no;
        }

        FrameData[i].length = loc_index;
    }
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: init_table_samples
*/
void init_table_samples(MU_CONF *p_mu_conf)
{
    uint i, j, k;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        for(j=0; j<p_mu_conf->channel_no; j++)
        {
            for(k=0; k<p_mu_conf->samples_per_cycle; k++)
            {
                if (j<(p_mu_conf->channel_no >> 1))
                    Samples[i][j][k] = LNData[i].Imag[j][0] * SR_OF_2 * 1000 * p_mu_conf->ctratio[i] *
                                       sin(TWO_PI * (float32)k / (float32)p_mu_conf->samples_per_cycle
                                           + LNData[i].Imag[j][1] * ANG_2_RAD);
                else
                    Samples[i][j][k] = LNData[i].Umag[j-(p_mu_conf->channel_no >> 1)][0] * SR_OF_2 * 100 * p_mu_conf->vtratio[i] *
                                       sin(TWO_PI * (float32)k / (float32)p_mu_conf->samples_per_cycle
                                           + LNData[i].Umag[j-(p_mu_conf->channel_no >> 1)][1] * ANG_2_RAD);
            }
        }
    }
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: init_annalog
*/
void init_annalog(MU_CONF *p_mu_conf)
{
    uint32 i;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        strncpy(LNData[i].PID, p_mu_conf->sv_id, strlen(p_mu_conf->sv_id));
        LNData[i].PID[2] = (char)(0x30+i+1);
        LNData[i].PIDLength = strlen(LNData[i].PID);

        if ((p_mu_conf->test & (0x01 << i)) >> i)
        {
            LNData[i].state.test = 0x01;
        }
        else
        {
            LNData[i].state.test = 0x00;
        }

        if ((p_mu_conf->invalid & (0x01 << i)) >> i)
        {
            LNData[i].state.validity = QUAL_INVALID;
        }
        else if ((p_mu_conf->quest & (0x01 << i)) >> i)
        {
            LNData[i].state.validity = QUAL_QUEST;
        }
        else
        {
            LNData[i].state.validity = 0x00;
        }

        LNData[i].state.syn = p_mu_conf->sync[i] - 0x30;
        if (LNData[i].state.syn > 2)
        {
            LNData[i].state.syn = 1;
        }

        LNData[i].Imag[0][0] = p_mu_conf->ia[i];
        LNData[i].Imag[1][0] = p_mu_conf->ib[i];
        LNData[i].Imag[2][0] = p_mu_conf->ic[i];
        LNData[i].Umag[0][0] = p_mu_conf->ua[i];
        LNData[i].Umag[1][0] = p_mu_conf->ub[i];
        LNData[i].Umag[2][0] = p_mu_conf->uc[i];
        LNData[i].Imag[3][0] = 0;
        LNData[i].Umag[3][0] = 0;
        LNData[i].Imag[0][1] = p_mu_conf->phase_a[i];
        LNData[i].Imag[1][1] = p_mu_conf->phase_b[i];
        LNData[i].Imag[2][1] = p_mu_conf->phase_c[i];
        LNData[i].Umag[0][1] = p_mu_conf->phase_a[i];
        LNData[i].Umag[1][1] = p_mu_conf->phase_b[i];
        LNData[i].Umag[2][1] = p_mu_conf->phase_c[i];
        LNData[i].Imag[3][1] = 0;
        LNData[i].Umag[3][1] = 0;
    }
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: update_frames
*/
void update_frames(MU_CONF *p_mu_conf)
{
    uint32 i, j, k;
    uint16 *p_smp_cnt;
    struct DATASET *p_dataset;
    int32 *p_data;
    uint16 frame_in_cycle;
    uint16 smp_cnt;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        for (k=0; k<p_mu_conf->asdu_no; k++)
        {
            p_smp_cnt = (uint16*) &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.smpCnt.value;
            if(LNData[i].state.enable)
            {
                if (p_mu_conf->sim_type == SIM_TYPE_OFFSET)
                {
                    *p_smp_cnt = htons((FrameSmpCnt+i*p_mu_conf->offset+k) % p_mu_conf->samples_per_sec);
                }
                else if (p_mu_conf->sim_type == SIM_TYPE_SMPCNT_MESS_UP)
                {
                    smp_cnt = FrameSmpCnt+k;
                    if (smp_cnt % 20 == 3)
                    {
                        smp_cnt += 2;
                    }
                    else if (smp_cnt % 20 == 4)
                    {
                        smp_cnt -= 1;
                    }
                    else if (smp_cnt % 20 == 5)
                    {
                        smp_cnt -= 1;
                    }
                    else if (smp_cnt % 20 == 10)
                    {
                        smp_cnt += 2;
                    }
                    else if (smp_cnt % 20 == 11)
                    {
                        smp_cnt -= 1;
                    }
                    else if (smp_cnt % 20 == 12)
                    {
                        smp_cnt -= 1;
                    }
                    smp_cnt = (smp_cnt % p_mu_conf->samples_per_sec);
                    *p_smp_cnt = htons(smp_cnt);
                }
                else
                {
                    smp_cnt = ((FrameSmpCnt+k) % p_mu_conf->samples_per_sec);
                    *p_smp_cnt = htons(smp_cnt);
                }

                p_dataset = &FramesSV[i].PDU.apdu.savPDU.SeqOfASDU.SeqOfASDU[k].ASDU.SeqOfData.DataSet;

                if(LNData[i].state.zero)
                {
                    for(j=0; j<p_mu_conf->channel_no; j++)
                    {
                        p_data = (int32*) p_dataset->data[j].sample;
                        *p_data = htonl(0);
                    }
                }
                else
                {
                    if (p_mu_conf->sim_type == SIM_TYPE_OFFSET)
                    {
                        if ((FramesInPeriod+i*p_mu_conf->offset) >= p_mu_conf->samples_per_cycle)
                            frame_in_cycle = FramesInPeriod+i*p_mu_conf->offset+k-p_mu_conf->samples_per_cycle;
                        else
                            frame_in_cycle = FramesInPeriod+i*p_mu_conf->offset+k;
                    }
                    else
                    {
                        frame_in_cycle = FramesInPeriod+k;
                    }
                    for(j=0; j<p_mu_conf->channel_no; j++)
                    {
                        p_data = (int32*) p_dataset->data[j].sample;
                        *p_data = htonl(Samples[i][j][frame_in_cycle]);
                    }
                }
            }
        }
    }

    copy_frames_sv(p_mu_conf);
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: is_smpcnt_valid
*/
uint8 is_smpcnt_valid(uint16 smp_cnt, MU_CONF *p_mu_conf)
{
    uint8 idx, ret = 1;

    for (idx=0; idx<p_mu_conf->lost_num; idx++)
    {
        if (smp_cnt == p_mu_conf->lost[idx])
        {
            ret = 0;
            break;
        }
    }

    if((ret == 1)&&(p_mu_conf->asdu_no == 2))
    {
        for (idx=0; idx<p_mu_conf->lost_num; idx++)
        {
            if (smp_cnt + 1 == p_mu_conf->lost[idx])
            {
                ret = 0;
                break;
            }
        }
    }

    return ret;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: pub_sv_thread
*/
void *pub_sv_thread(void *fparam)
{
    struct timespec t;
    float64 interval;
    uint32 samples_per_sec;
    uint16 samples_per_cycle;
    MU_CONF *p_mu_conf = (MU_CONF *)fparam;
    uint32 nCycleCounter = 0;
    int8   bEnableLostFrame = 0;
#if defined CORRECT_ERROR
    struct timespec t_old;
    struct timespec t_new;
    uint32 ts_err;
    uint32 acc_tm = 0, acc_target = 0;
    uint8 first_time = 1;
#endif

    if (p_mu_conf->asdu_no == 1)
    {
        interval = NB_NS_PER_CIRCLE_61850_9_2 / p_mu_conf->freq;
        samples_per_sec = NB_FRAMES_PER_CYCLE_61850_9_2 * p_mu_conf->freq;
        samples_per_cycle = NB_FRAMES_PER_CYCLE_61850_9_2;
#if defined CORRECT_ERROR
        acc_target = interval * NB_FRAMES_PER_CYCLE_61850_9_2;
#endif
    }
    else
    {
        if (p_mu_conf->freq == 60)
        {
            interval = p_mu_conf->asdu_no*NB_NS_PER_CIRCLE_61869_60HZ / p_mu_conf->freq;
            samples_per_sec = NB_FRAMES_PER_CYCLE_61869_60HZ * p_mu_conf->freq;
            samples_per_cycle = NB_FRAMES_PER_CYCLE_61869_60HZ;
#if defined CORRECT_ERROR
            acc_target = interval * NB_FRAMES_PER_CYCLE_61869_60HZ;
#endif
        }
        else
        {
            interval = p_mu_conf->asdu_no*NB_NS_PER_CIRCLE_61869_50HZ / p_mu_conf->freq;
            samples_per_sec = NB_FRAMES_PER_CYCLE_61869_50HZ * p_mu_conf->freq;
            samples_per_cycle = NB_FRAMES_PER_CYCLE_61869_50HZ;
#if defined CORRECT_ERROR
            acc_target = interval * NB_FRAMES_PER_CYCLE_61869_50HZ;
#endif
        }
    }

    FrameSmpCnt = 0;
    FramesInPeriod = 0;

    p_mu_conf->samples_per_sec = samples_per_sec;
    p_mu_conf->samples_per_cycle = samples_per_cycle;

    init_table_samples(p_mu_conf);

    if (0 == open_port_pcap(p_mu_conf->int_no))
    {
#if defined _LINUX
        memset(&ParamSent, 0, sizeof(ParamSent));

        ParamSent.sched_priority = MUSIM_PRIORITY - 1;
        if (sched_setscheduler(0, SCHED_FIFO, &ParamSent) == -1)
        {
            MUSIM_ERR_MSG0("%s, line %d, sched_setscheduler failed.\n");
            exit(-1);
        }

        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            MUSIM_ERR_MSG0("%s, line %d, mlockall failed.\n");
            exit(-2);
        }
#endif

        /* if Ethnernet interface is not found, exit here. */
        while(EthInterface == NULL)
        {
            exit(-1);
        }

#if defined _LINUX
        clock_gettime(CLOCK_MONOTONIC, &t);
        t.tv_nsec += interval;
        if (t.tv_nsec >= NSEC_PER_SEC)
        {
            t.tv_nsec -= NSEC_PER_SEC;
            t.tv_sec++;
        }
#else
        clock_gettime(&t);
#endif

        while (FrameSendEnabled)
        {
            update_frames(p_mu_conf);

#if defined _LINUX
#if defined CORRECT_ERROR
            clock_gettime(CLOCK_MONOTONIC, &t_new);
            if (0 == first_time)
                acc_tm += ((t_new.tv_nsec > t_old.tv_nsec) ? \
                           (t_new.tv_nsec - t_old.tv_nsec) : \
                           (NSEC_PER_SEC + t_old.tv_nsec - t_new.tv_nsec));
            else
                first_time = 0;

            t_old = t_new;
#endif

            /* delay to time t */
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

            t.tv_nsec += interval;
            while (t.tv_nsec >= NSEC_PER_SEC)
            {
                t.tv_nsec -= NSEC_PER_SEC;
                t.tv_sec++;
            }
#if defined CORRECT_ERROR
            if (FramesInPeriod == (samples_per_cycle - 1))
            {
                ts_err = (acc_tm - acc_target);
                if (t.tv_nsec >= ts_err)
                    t.tv_nsec -= ts_err;
                else
                {
                    t.tv_nsec = (NSEC_PER_SEC + t.tv_nsec - ts_err);
                    t.tv_sec--;
                }
                acc_tm = 0;
            }
#endif
#else
            delay_us(interval);
#endif

            ComtradeTime += interval / 1000;

            switch (p_mu_conf->sim_type)
            {
            case SIM_TYPE_NORMAL:
            case SIM_TYPE_OFFSET:
                send_frame_pcap(p_mu_conf);
                break;

            case SIM_TYPE_LOST_FRAMES_PER_1_CYCLE:
                send_frame_pcap_with_lost_frame(p_mu_conf, !is_smpcnt_valid((FrameSmpCnt % samples_per_cycle), p_mu_conf));
                break;
            case SIM_TYPE_LOST_FRAMES_BETWEEN_1_CYCLE:
            case SIM_TYPE_LOST_FRAMES_BETWEEN_50_CYCLES:
            case SIM_TYPE_LOST_FRAMES_BETWEEN_60_CYCLES:
                if(bEnableLostFrame == 1)
                {
                    send_frame_pcap_with_lost_frame(p_mu_conf, !is_smpcnt_valid((FrameSmpCnt % samples_per_cycle), p_mu_conf));
                }
                else if(bEnableLostFrame == 0)
                {
                    send_frame_pcap(p_mu_conf);
                }
                break;
            case SIM_TYPE_SMPCNT_MESS_UP:
                send_frame_pcap(p_mu_conf);
                break;

            default:
                send_frame_pcap(p_mu_conf);
                break;
            }

            FrameSmpCnt = (FrameSmpCnt + p_mu_conf->asdu_no) % samples_per_sec;
            FramesInPeriod = (FramesInPeriod + p_mu_conf->asdu_no) % samples_per_cycle;
            if(FramesInPeriod == samples_per_cycle - p_mu_conf->asdu_no)
            {
                nCycleCounter = (nCycleCounter + 1) % 300;
                switch (p_mu_conf->sim_type)
                {
                case SIM_TYPE_LOST_FRAMES_BETWEEN_50_CYCLES:
                    if(nCycleCounter % 50 == 0) bEnableLostFrame = !bEnableLostFrame;
                    break;
                case SIM_TYPE_LOST_FRAMES_BETWEEN_60_CYCLES:
                    if(nCycleCounter % 60 == 0) bEnableLostFrame = !bEnableLostFrame;
                    break;
                case SIM_TYPE_LOST_FRAMES_BETWEEN_1_CYCLE:
                    bEnableLostFrame = !bEnableLostFrame;
                    break;
                default:
                    break;
                }
            }
        }

#if defined _LINUX
        ParamSent.sched_priority = 0;
        sched_setscheduler(0, SCHED_OTHER, &ParamSent);
#endif

        close_port_pcap();
    }

    return NULL;
}

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
