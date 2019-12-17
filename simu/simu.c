/*
***********************************************************************************
**
** File         : main.c
** Description  : Main entrance function of the program.
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
#include <stdio.h>

#include "all.h"
#include "eth.h"
#include "util.h"
#include "frame.h"

typedef enum
{
    TYPE_SYNCH   = 0,
    TYPE_CURRENT_A = 1,
    TYPE_VOLTAGE_A = 2,
    TYPE_MAX,

} PARAM_TYPE;

void StartSimu(MU_CONF* MUConf)
{
    uint32 i;
    //int8 sync[MAX_NB_LN+1];
    //int32 type;
    //float32 val;

    MUSIM_STD_MSG0("\nLocal MAC address:\n");
    get_local_mac();

    for (i=0; i<MUConf->mu_no; i++)
        LNData[i].state.enable = 1;

    init_annalog(MUConf);
    init_frames_92(MUConf);

    FrameSendEnabled = 1;
    pthread_create(&ThreadSend, NULL, pub_sv_thread, MUConf);

    MUSIM_STD_MSG0("\nSV publishing is ongoing...\n");

//    while (1)
//    {
//        usleep(100000);
//        MUSIM_STD_MSG0("To modify parameter, enter\n\t0 - synch\n\t1 - current\n\t2 - voltage\n\n");
//        type = 0;
//        scanf("%d", &type);
//        MUSIM_STD_MSG1("You have entered %d, please input the new value:\n", type);

//        switch (type)
//        {
//        case TYPE_SYNCH:
//            memset(sync, 0, sizeof(sync));
//            scanf("%s", sync);
//            update_sync_state(&MUConf, sync);
//            break;

//        case TYPE_CURRENT_A:
//            val = 0.0;
//            scanf("%f", &val);
//            update_current_phsA(&MUConf, (const float32)val);
//            break;

//        case TYPE_VOLTAGE_A:
//            val = 0.0;
//            scanf("%f", &val);
//            update_voltage_phsA(&MUConf, (const float32)val);
//            break;
//        default:
//            break;
//        }
//    }
}


void DetectEthIF(ETH_INTERFACE_LIST* list)
{
    MUSIM_STD_MSG0("Sampled Value Simulator\n");
    MUSIM_STD_MSG1("Version: %s\n", VERSION);

    MUSIM_STD_MSG0("\nDevice list:\n");
    detect_eth_interface(list);
}

void ChooseEth(MU_CONF* MUConf, int32 EthIndex)
{
    InterfaceNo = EthIndex;
//    MUSIM_STD_MSG0("\nPlease input Ethernet device number(starts from 0):");
//    (void)scanf("%d", &InterfaceNo);

    MUConf->int_no = EthIndex;
}

void StopSimu()
{
    FrameSendEnabled = 0;
    pthread_cancel(ThreadSend);
}

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
