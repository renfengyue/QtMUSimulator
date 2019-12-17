/*
***********************************************************************************
**
** File         : eth.c
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
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include "all.h"
#include "util.h"

/*------------------------------------------------------------------------------
  PREPROCESSOR CONSTANTS
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PREPROCESSOR MACROS
  ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  PUBLIC STRUCTURE DEFINITIONS
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PUBLIC VARIABLES
  ----------------------------------------------------------------------------*/
pcap_t *EthInterface;
ETH_INTERFACE_LIST EthInterfaces;

/*------------------------------------------------------------------------------
  PRIVATE VARIABLES
  ----------------------------------------------------------------------------*/
static int8 ErrBuffer[PCAP_ERRBUF_SIZE+1];

/*------------------------------------------------------------------------------
  EXTERNAL REFERENCES
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PRIVATE FUNCTION PROTOTYPES
  ----------------------------------------------------------------------------*/
void decode_frame(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);


/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: PrintMACaddress
*/
#if defined WIN32
void print_mac_address (BYTE *addr)
{
    for (int i=0; i<6; i++)
    {
        if (i<5)
            MUSIM_STD_MSG1 ("%02X-", *addr++);
        else
            MUSIM_STD_MSG1 ("%02X", *addr++);
    }
    MUSIM_STD_MSG0 ("\n");
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: get_local_mac
*/
void get_local_mac()
{
    IP_ADAPTER_INFO AdapterInfo[16];
    char localMac[8];
    DWORD dwBufLen = sizeof (AdapterInfo);

    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
    {
        MUSIM_ERR_MSG1("%s, line %d, GetAdaptersInfo failed. err=%d\n", GetLastError ());
        return;
    }

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    do
    {
        PrintMACaddress(pAdapterInfo->Address);
        strcpy(localMac, (const char *)pAdapterInfo->Address);
        pAdapterInfo = pAdapterInfo->Next;
    } while (pAdapterInfo);
}
#endif

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: get_local_mac
*/
#if defined _LINUX
void get_local_mac(void)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        printf("socket error\n");
        return;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
    {
        printf("ioctl error\n");
        return;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    char szMac[64];
    int count = 0;

    for (; it!=end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
                {
                    unsigned char *ptr;
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
                    snprintf(szMac,64,"%02X:%02X:%02X:%02X:%02X:%02X",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
                    MUSIM_STD_MSG3("DeviceNo.:%d Name:%s, MACAddr:%s\n", count, ifr.ifr_name, szMac);
                    count ++;
                }
            }
        }
        else
        {
            MUSIM_ERR_MSG0("%s, line %d, get mac info error\n");
            return;
        }
    }

    close(sock);
}
#endif

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: detect_eth_interface
*/
void detect_eth_interface(ETH_INTERFACE_LIST *pEthInterfaceDetected)
{
    int i;
    pcap_if_t *list_interfaces;
    unsigned int interface_number = 0;

    memset(pEthInterfaceDetected, 0, sizeof(ETH_INTERFACE_LIST));

    if (pcap_findalldevs(&list_interfaces, ErrBuffer) == -1)
    {
        pEthInterfaceDetected->number = 0;
    }
    else
    {
        for (i=0; i<MAX_ETH_NUM; i++)
        {
            if (list_interfaces)
            {
                strcpy(pEthInterfaceDetected->name[interface_number], list_interfaces->name);
                MUSIM_STD_MSG2("Device number:%d Name:%s\n", interface_number, list_interfaces->name);
                interface_number++;
                list_interfaces = list_interfaces->next;
            }
            else
            {
                break;
            }
        }

        pEthInterfaceDetected->number = interface_number;

        pcap_freealldevs(list_interfaces);
    }
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: open_port_pcap
*/
int8 open_port_pcap(uint8 PortNum)
{
    if((EthInterface = pcap_open_live(EthInterfaces.name[PortNum], 8192, 0, 0, ErrBuffer)) == NULL)
    {
        MUSIM_ERR_MSG1("%s, line %d, couldn't open device : %s\n", ErrBuffer);
        return -1;
    }
    else
        return 0;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: send_frame_pcap
*/
void send_frame_pcap(MU_CONF *p_mu_conf)
{
    int i = 0;

    for(i=0; i<p_mu_conf->mu_no; i++)
    {
        if (LNData[i].state.enable)
        {
            if(pcap_sendpacket(EthInterface, FrameData[i].data, FrameData[i].length) != 0)
            {
                MUSIM_ERR_MSG0("%s, line %d, Pcap failed to publish SV.\n");
            }
        }
    }
}

void send_frame_pcap_with_lost_frame(MU_CONF *p_mu_conf, uint8 bFrameIsLost)
{
    int i = 0;

    for(i = 0; i < p_mu_conf->mu_no; i++)
    {
        if((bFrameIsLost == 1)&&(p_mu_conf->enableMULostFrame[i] == 1))
        {
            continue;
        }
        if (LNData[i].state.enable)
        {
            if(pcap_sendpacket(EthInterface, FrameData[i].data, FrameData[i].length) != 0)
            {
                MUSIM_ERR_MSG0("%s, line %d, Pcap failed to publish SV.\n");
            }
        }
    }

}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: close_port_pcap
*/
void close_port_pcap(void)
{
    pcap_close(EthInterface);
    MUSIM_STD_MSG0("pcap_close_port_OK.\n");
}

/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
