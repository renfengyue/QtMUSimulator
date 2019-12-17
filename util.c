/*
***********************************************************************************
**
** File         : util.c
** Description  : This file is used to implement all common functions.
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
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include "all.h"
#include "util.h"
#include <time.h>
#include <string.h>

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

/*------------------------------------------------------------------------------
  EXTERNAL REFERENCES
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  PRIVATE FUNCTION PROTOTYPES
  ----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: delay_ms
*/
void delay_ms(unsigned long msec)
{
	clock_t now, start;

	start = clock();
	do
	{
		now = clock();
	} while(now - start < msec);
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: is_all_whitespace
*/
char is_all_whitespace (const char *astr)
{
    char bRet = 1;
    size_t   len;
    char *tmp;

    if (!astr)
        return (-1);
    if ( (len = strlen (astr)) == 0)
        return (1);

    for (tmp= (char *) astr; tmp<astr+len; ++tmp)
    {
        if (!isspace (*tmp))
        {
            bRet = 0;
            break;
        }
    }

    return (bRet);
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: ascii_to_ulong
*/
char ascii_to_ulong (const char *astr, unsigned long *u_long)
{
    char *retPtr;
    int errno;

    *u_long = 0;

    if (strchr (astr, '-'))
        return (-1);

    errno = 0;
    *u_long = strtoul (astr, &retPtr, 10);
    if (retPtr == astr || errno != 0)
        return (-1);

    if (!is_all_whitespace (retPtr))
        return (-1);

    return 0;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: ascii_to_uint
*/
char ascii_to_uint (const char *astr, unsigned int *u_int)
{
    char ret;
    unsigned long u_long = 0;

    *u_int = 0;

    ret = ascii_to_ulong (astr, &u_long);
    if (ret)
        return (ret);

#if (UINT_MAX != ULONG_MAX)
    if (u_long > UINT_MAX)
        return -1;
#endif

    *u_int = (unsigned int) u_long;
    return 0;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: ascii_to_float
*/
char ascii_to_float (const char *astr, float *float_num)
{
    char *retPtr;
    double doubleNum = 0.0;
    int errno;

    *float_num = 0.0;
    errno = 0;

    doubleNum = strtod (astr, &retPtr);
    if (retPtr == astr || errno != 0)
        return (-1);

    if (!is_all_whitespace (retPtr))
        return (-1);

    if (fabs (doubleNum) > FLT_MAX)
        return -1;

    *float_num = (float) doubleNum;
    return 0;
}


/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: bin_str_to_oct
*/

uint32 bin_str_to_oct(const int8 *p_str)
{
    int i, num = 0;

    assert(strlen(p_str) >= 1);

    for (i=0; i<strlen(p_str)-1; i++)
    {
        num = num*2 + ((p_str[i] - '0') > 0 ? 1 : 0);
    }

    return num;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: read_mu_config
*/
void read_mu_config(MU_CONF *p_mu_conf, const char* conf_file)
{
    FILE *fp;
    int16 item_index = 0, idx;
    char mu_conf_line[128];
    char *p_token, *p_token2;
    char str_path[MAX_PATH+1];
    //size_t read_num = 0;
    char *read_char;
    char str[128];
    char svid[64];
    char sync[MAX_NB_LN+1];

    memset(str_path, 0, sizeof(str_path));
    //memset(conf_file, 0, sizeof(conf_file));

//#ifdef _WIN32
//    GetModuleFileName (NULL, (LPSTR) str_path, MAX_PATH);
//    strncpy (conf_file, str_path, strrchr (str_path, '\\') - str_path + 1);
//    strcat (conf_file, "MUConfig.txt");
//#else
//    read_num = readlink("/proc/self/exe", str_path, MAX_PATH);
//    if (read_num > 0)
//        strncpy (conf_file, str_path, strrchr (str_path, '/') - str_path + 1);
//    else
//        return;
//    strcat (conf_file, "MUConfig");
//#endif


    if ((fp = fopen (conf_file, "r")) == NULL)
    {
        MUSIM_ERR_MSG0("%s, line %d, can not get configuration file.\n");
        return;
    }

    for (idx=0; idx<20; idx++)
        p_mu_conf->lost[idx] = -1;

    while (!feof (fp))
    {
        memset (mu_conf_line, 0, sizeof (mu_conf_line));
        read_char = fgets (mu_conf_line, 128, fp);
        if (read_char == NULL) return;
        p_token = strtok (mu_conf_line, "=");
        while (p_token != NULL)
        {
            p_token = strtok (NULL, "=");
            if (p_token == NULL)
                break;
            if (item_index == 0)
            {
                (void) ascii_to_uint (p_token, &p_mu_conf->mu_no);
                if ((p_mu_conf->mu_no > MAX_NB_LN) || (p_mu_conf->mu_no <= 0))
                    p_mu_conf->mu_no = 1;
                MUSIM_EXTSTD_MSG1("MUCount:\t\t%d\n", p_mu_conf->mu_no);
            }
            else if (item_index == 1)
            {
                (void) ascii_to_uint (p_token, &p_mu_conf->freq);
                (void) ascii_to_uint (p_token, &p_mu_conf->freq);
                if ((p_mu_conf->freq < 50) || (p_mu_conf->freq > 60))
                    p_mu_conf->freq = 50;
                MUSIM_EXTSTD_MSG1("Freq:\t\t%d\n", p_mu_conf->freq);
            }
            else if (item_index == 2)
            {
                (void) ascii_to_uint (p_token, &p_mu_conf->asdu_no);
                if (p_mu_conf->asdu_no > 2 || p_mu_conf->asdu_no <= 0)
                    p_mu_conf->asdu_no = 1;
                MUSIM_EXTSTD_MSG1("ASDU:\t\t%d\n", p_mu_conf->asdu_no);
            }
            else if (item_index == 3)
            {
                (void) ascii_to_uint (p_token, &p_mu_conf->channel_no);
                if (p_mu_conf->channel_no > 24 || p_mu_conf->channel_no <= 0)
                    p_mu_conf->channel_no = 8;
                MUSIM_EXTSTD_MSG1("ChnnlNo:\t%d\n", p_mu_conf->channel_no);
            }
            else if (item_index == 4)
            {
                p_mu_conf->test = bin_str_to_oct(p_token);
                if (p_mu_conf->test > 255)
                    p_mu_conf->test = 0;
                MUSIM_EXTSTD_MSG1("Test:\t\t%s", p_token);
            }
            else if (item_index == 5)
            {
                p_mu_conf->invalid = bin_str_to_oct(p_token);
                if (p_mu_conf->invalid > 255)
                    p_mu_conf->invalid = 0;
                MUSIM_EXTSTD_MSG1("Invalid:\t%s", p_token);
            }
            else if (item_index == 6)
            {
                p_mu_conf->quest = bin_str_to_oct(p_token);
                if (p_mu_conf->quest > 255)
                    p_mu_conf->quest = 0;
                MUSIM_EXTSTD_MSG1("Quest:\t\t%s", p_token);
            }
            else if (item_index == 7)
            {
                memset(p_mu_conf->sync, 0, sizeof(p_mu_conf->sync));
                memset(sync, 0, sizeof(sync));
                strncpy(sync, p_token, MAX_NB_LN);
                if ((sync[strlen(sync)-1] == '\n') || (sync[strlen(sync)-1] == '\r'))
                    sync[strlen(sync)-1] = '\0';
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if ((sync[idx] - 0x30) < 0)
                    {
                        sync[idx] = 0x30;
                    }
                    else if ((sync[idx] - 0x30) > 2)
                    {
                        sync[idx] = 0x30 + 1;
                    }
                }
                strncpy(p_mu_conf->sync, sync, MAX_NB_LN);
                MUSIM_EXTSTD_MSG1("Sync:\t\t%s", p_token);
            }
            else if (item_index == 8)
            {
                unsigned int sim_type = (unsigned int)(p_mu_conf->sim_type);
                (void) ascii_to_uint (p_token, &sim_type);
                if ((p_mu_conf->sim_type < SIM_TYPE_NORMAL) || (p_mu_conf->sim_type >= SIM_TYPE_MAX))
                    p_mu_conf->sim_type = SIM_TYPE_NORMAL;
                MUSIM_EXTSTD_MSG1("Type:\t\t%d\n", p_mu_conf->sim_type);
            }
            else if (item_index == 9)
            {
                (void) ascii_to_uint (p_token, &p_mu_conf->offset);
                if (p_mu_conf->offset > MAX_OFFSET)
                    p_mu_conf->offset = MAX_OFFSET;
                MUSIM_EXTSTD_MSG1("Offset:\t\t%d\n", p_mu_conf->offset);
            }
            else if (item_index == 10)
            {
                p_mu_conf->simul = bin_str_to_oct(p_token);
                if (p_mu_conf->simul > 255)
                    p_mu_conf->simul = 0;
                MUSIM_EXTSTD_MSG1("Simulation:\t%s", p_token);
            }
            else if (item_index == 11)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                p_mu_conf->lost_num = 0;
                while (p_token2 != NULL)
                {
                    p_token2 = strtok (NULL, ":");
                    p_mu_conf->lost_num++;
                }
                if (p_mu_conf->lost_num > 20)
                {
                    MUSIM_ERR_MSG1("%s, line %d, wrong lost samples detected, %s, exits", p_token);
                }
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<p_mu_conf->lost_num; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_uint (p_token2, (uint32 *)&p_mu_conf->lost[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Lost:\t\t%s", p_token);
            }
            else if (item_index == 12)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ia[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Ia:\t\t%s", p_token);
            }
            else if (item_index == 13)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ib[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Ib:\t\t%s", p_token);
            }
            else if (item_index == 14)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ic[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Ic:\t\t%s", p_token);
            }
            else if (item_index == 15)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ua[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Ua:\t\t%s", p_token);
            }
            else if (item_index == 16)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ub[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Ub:\t\t%s", p_token);
            }
            else if (item_index == 17)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->uc[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("Uc:\t\t%s", p_token);
            }
            else if (item_index == 18)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->phase_a[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("PhsA:\t\t%s", p_token);
            }
            else if (item_index == 19)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->phase_b[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("PhsB:\t\t%s", p_token);
            }
            else if (item_index == 20)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->phase_c[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("PhsC:\t\t%s", p_token);
            }
            else if (item_index == 21)
            {
                memset(p_mu_conf->sv_id, 0, sizeof(p_mu_conf->sv_id));
                memset(svid, 0, sizeof(svid));
                strncpy(svid, p_token, strlen(p_token));
                if ((svid[strlen(svid)-1] == '\n') || (svid[strlen(svid)-1] == '\r'))
                    svid[strlen(svid)-1] = '\0';
                strncpy(p_mu_conf->sv_id, svid, (strlen(svid) <= 34) ? strlen(svid) : 34);
                MUSIM_EXTSTD_MSG1("SvID:\t\t%s", p_token);
            }
            else if (item_index == 22)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->ctratio[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("CTRatio:\t%s", p_token);
            }
            else if (item_index == 23)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                        (void) ascii_to_float (p_token2, &p_mu_conf->vtratio[idx]);
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("VTRatio:\t%s", p_token);
            }
            else if (item_index == 24)
            {
                memset(str, 0, sizeof(str));
                strncpy(str, p_token, strlen(p_token)-1);
                p_token2 = strtok (str, ":");
                for (idx=0; idx<MAX_NB_LN; idx++)
                {
                    if (p_token2 != NULL)
                    {
                        unsigned int bEnable = (unsigned int)(p_mu_conf->enableMULostFrame[idx]);
                        (void) ascii_to_uint (p_token2, &bEnable);
                    }
                    p_token2 = strtok (NULL, ":");
                }
                MUSIM_EXTSTD_MSG1("EnableMULostFrame:\t%s", p_token);
            }
            else
            {
                MUSIM_ERR_MSG0("%s, line %d, unused parameter in MUConfig.\n");
            }
        }
        item_index++;
    }

    fclose (fp);

    return;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: print_hex_ascii_line
*/
void print_hex_ascii_line(const u_char *payload, int len, int offset)
{
    int i;
    int gap;
    const u_char *ch;

    /* offset */
    printf("%05d   ", offset);

    /* hex */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        printf("%02x ", *ch);
        ch++;
        /* print extra space after 8th byte for visual aid */
        if(i == 7)
            printf(" ");
    }

    /* print space to handle line less than 8 bytes */
    if(len < 8)
        printf(" ");

    /* fill hex gap with spaces if not full line */
    if(len < 16)
    {
        gap = 16 - len;
        for(i = 0; i < gap; i++)
            printf("   ");
    }

    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        if(isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");

    return;
}

/*------------------------------------------------------------------------------
** PUBLIC FUNCTION
**------------------------------------------------------------------------------
** Name: print_payload
*/
void print_payload(const uint8 *payload, int len)
{
    int len_rem = len;
    int line_width = 16; /* number of bytes per line */
    int line_len;
    int offset = 0; /* zero-based offset counter */
    const uint8 *ch = payload;

    if(len <= 0)
        return;

    /* data fits on one line */
    if(len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    /* data spans multiple lines */
    for(;;)
    {
        /* compute current line length */
        line_len = line_width % len_rem;
        /* print line */
        print_hex_ascii_line(ch, line_len, offset);
        /* compute total remaining */
        len_rem = len_rem - line_len;
        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;
        /* add offset */
        offset = offset + line_width;
        /* check if we have line width chars or less */
        if(len_rem <= line_width)
        {
            /* print last line and get out */
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }

    return;
}
/*
********************************************************************************
** END OF FILE
********************************************************************************
*/
