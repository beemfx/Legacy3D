#ifndef __LG_CMD_H__
#define __LG_CMD_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <lc_sys.h>

#define CONF_QUIT         0x00000010
#define CONF_VRESTART     0x00000020
#define CONF_DIR          0x00000030
#define CONF_EXTRACT      0x00000040
#define CONF_D3DCAPS      0x00000050
#define CONF_VIDMEM       0x00000060
//#define CONF_HARDVRESTART 0x00000070
#define CONF_TEXFORMATS   0x00000080

#define CONF_LOADMODEL    0x00000090

#define CONF_VERSION      0x000000A0

int LGC_RegConCmds(HLCONSOLE hConsole);
int LGC_ConCommand(unsigned long nCommand, const char* szParams, HLCONSOLE hConsole, void* pExtra);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LG_CMD_H__ */