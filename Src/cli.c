/*********  cli.h  ***********/
#ifndef __CLI_H_
#define __CLI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


struct _cli_func_ {
    char *func_name;
    char *func_info;
    void (*func)(uint8_t len , void *param[] );
    struct _cli_func_ *sub_cli ;
};

void _cli_dump_(uint8_t len , void *param[] );
void _cli_reboot_(uint8_t len , void *param[] );


void _cli_rtt_clear_(uint8_t len , void *param[] );
void _cli_rtt_color_(uint8_t len , void *param[] );

extern struct _cli_func_ rtt_func[];

#define __CLI_TABLE__       { "?"       , "dump all func"  , _cli_dump_     , NULL     },\
                            { "reboot"  , "reboot"         , _cli_reboot_   , NULL     },\
                            { "rtt"     , "RTT config"     , NULL           , rtt_func },\
                            { NULL      , NULL             , NULL           , NULL     },\


#define __RTT_TABLE__       { "clear"   , "clear console"  , _cli_rtt_clear_ , NULL     },\
                            { "color"   , "set color"      , _cli_rtt_color_ , NULL     },\
                            { NULL      , NULL             , NULL            , NULL     },\
                            


struct _cli_func_ cli_func[] = {
    __CLI_TABLE__
};


struct _cli_func_ rtt_func[] = {
    __RTT_TABLE__
};



#endif

/*********  cli.c  ***********/

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"


void _cli_dump_(uint8_t len , void *param[] )
{
    uint8_t i = 0 ;
    
    for ( i = 0 ; i < sizeof(cli_func)/sizeof(struct _cli_func_)-1 ; i++ )
    {
        SEGGER_RTT_printf(0, "%s", cli_func[i].func_name);
        SEGGER_RTT_printf(0, "\t - \t");
        SEGGER_RTT_printf(0, "%s\n", cli_func[i].func_info);
    }
}

void _cli_reboot_(uint8_t len , void *param[] )
{
    SEGGER_RTT_printf(0, "Reboot! Bye~Bye!\n\n\n");
    vTaskDelay(500);
    
    NVIC_SystemReset();
}

void _cli_rtt_clear_(uint8_t len , void *param[] )
{
    SEGGER_RTT_printf(0, "%s\n$\n",RTT_CTRL_CLEAR);
}

void _cli_rtt_color_(uint8_t len , void *param[] )
{
    if (len<1)
    {
        SEGGER_RTT_printf(0, "%sBLACK ",RTT_CTRL_BG_BLACK);
        SEGGER_RTT_printf(0, "%sRED ",RTT_CTRL_BG_RED);
        SEGGER_RTT_printf(0, "%sGREEN ",RTT_CTRL_BG_GREEN);
        SEGGER_RTT_printf(0, "%sYELLOW ",RTT_CTRL_BG_YELLOW);
        SEGGER_RTT_printf(0, "%sBLUE ",RTT_CTRL_BG_BLUE);
        SEGGER_RTT_printf(0, "%sWHITE ",RTT_CTRL_BG_WHITE);
        SEGGER_RTT_printf(0, "%sRESET \n",RTT_CTRL_RESET);
        return ;
    }

    if ( !strcasecmp(param[0], "BLACK") )    SEGGER_RTT_printf(0, "%sBLACK\n", RTT_CTRL_BG_BLACK);
    if ( !strcasecmp(param[0], "RED") )      SEGGER_RTT_printf(0, "%sRED\n", RTT_CTRL_BG_RED);
    if ( !strcasecmp(param[0], "GREEN") )    SEGGER_RTT_printf(0, "%sGREEN\n", RTT_CTRL_BG_GREEN);
    if ( !strcasecmp(param[0], "YELLOW") )   SEGGER_RTT_printf(0, "%sYELLOW\n", RTT_CTRL_BG_YELLOW);
    if ( !strcasecmp(param[0], "BLUE") )     SEGGER_RTT_printf(0, "%sBLUE\n", RTT_CTRL_BG_BLUE);
    if ( !strcasecmp(param[0], "WHITE") )    SEGGER_RTT_printf(0, "%sWHITE\n", RTT_CTRL_BG_WHITE);
    if ( !strcasecmp(param[0], "RESET") )    SEGGER_RTT_printf(0, "%sRESET\n", RTT_CTRL_RESET);

}


static int8_t parse_cli( uint8_t len , char *param[] , struct _cli_func_ commands[] )
{    
    uint8_t cli_len = 0;

    // find cli len
    for (cli_len = 0; ; cli_len++)
    {
        if( commands[cli_len].func_name == NULL)
            break;
    }

    // dump cli info
    if (len == 0)
    {
        for (uint8_t i = 0 ; i < cli_len ; i++)
        {
            SEGGER_RTT_printf(0, "%s", commands[i].func_name);
            SEGGER_RTT_printf(0, "\t - \t");
            SEGGER_RTT_printf(0, "%s\n", commands[i].func_info);
        }
        return 0;
    }

    // parse cli
    for ( uint8_t i = 0 ; i < cli_len ; i++ )
    {
        if ( !strcmp( commands[i].func_name, param[0]) )
        {
            if ( commands[i].sub_cli )
            {
                parse_cli( len - 1 , (void *)(param + 1) , commands[i].sub_cli );
            }
            else
            {
                commands[i].func( len - 1, (void *)(param + 1) );
            }
            return 0;
        }
    }

    SEGGER_RTT_printf(0, "%sUnknow command!!!%s\n", RTT_CTRL_TEXT_RED, RTT_CTRL_RESET);
    return 1;
}


void cli_task(void *pvParameters){
    char str[256] = {0};
    uint16_t len = 0 ;
    char *param[128] = {NULL};
    
    SEGGER_RTT_printf(0, "%s start\n",__FUNCTION__);
        
    do {
        if( SEGGER_RTT_HasKey() )
        {
            str[len] = SEGGER_RTT_WaitKey();
            len++;
        }
        
        if( len>0 && str[len-1] == '\n' )
        {
            uint8_t i = 0;
            
            SEGGER_RTT_printf(0, "$ %s",str);
            str[len-1] = '\0';
            if( (len-2>0) && str[len-2] == '\r' ) str[len-2] = '\0';

            memset(param, NULL, sizeof(param));
            
            // parse cli
            param[i] = strtok(str, " ");
            while(param[i] != NULL)
            {
                i++;
                param[i] = strtok(NULL, " ");
            }
            len=0;
                        
            // search match cli
            parse_cli( i , param , cli_func );
        }
        
        vTaskDelay( 100 );
    } while (1);
    
    vTaskDelete(NULL);
}

