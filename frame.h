/*
***********************************************************************************
**
** File         : frame.h
** Description  : Header file which is used to define public structures
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
#ifndef __FRAME_H__
#define __FRAME_H__

#include "all.h"

/*------------------------------------------------------------------------------
  PUBLIC STRUCTURE DEFINITIONS
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PUBLIC FUNCTION PROTOTYPES
  ----------------------------------------------------------------------------*/
void *pub_sv_thread(void *fparam);
void *transmit_thread(void *fparam);
void init_table_samples(MU_CONF *p_mu_conf);
void init_annalog(MU_CONF *p_mu_conf);
void init_frames_92(MU_CONF *p_mu_conf);
void read_comtrade(uint16 index);
void get_config(void);
void update_sync_state(MU_CONF *p_mu_conf, int8 *sync);
void update_current_phsA(MU_CONF *p_mu_conf, const float32 val);
void update_voltage_phsA(MU_CONF *p_mu_conf, const float32 val);

#endif /* __FRAME_H__ */

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/

