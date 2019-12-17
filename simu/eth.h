/*
***********************************************************************************
**
** File         : all.h
** Description  : Header file ethernet interface routines
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

#ifndef __ETH_H__
#define __ETH_H__

#include "all.h"

/*------------------------------------------------------------------------------
  PUBLIC FUNCTION PROTOTYPES
  ----------------------------------------------------------------------------*/
void detect_eth_interface(ETH_INTERFACE_LIST *pEthInterfaceDetected);
void get_local_mac(void);
int8 open_port_pcap(uint8 PortNum);
void send_frame_pcap(MU_CONF *p_mu_conf);
void send_frame_pcap_with_lost_frame(MU_CONF *p_mu_conf, uint8 bFrameIsLost);
void close_port_pcap(void);
void send_GOOSE(void);
uint8 set_eth_filter(uint8 PortNum);
void decode_frame(u_char *args, const struct pcap_pkthdr *header,
                 const u_char *packet);
                 uint8 set_eth_filter(uint8 PortNum);

#endif /* __ETH_H__ */

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
