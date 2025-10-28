//lg_cmd.h - This is where all console commands
//should be registered, they allso need to be
//registered in the CLGame::LGC_RegisterCommands function.
#ifndef __LG_CMD_H__
#define __LG_CMD_H__


#define CONF_SET          0x00000001
#define CONF_GET          0x00000002
#define CONF_CLEAR        0x00000003
#define CONF_ECHO         0x00000004
#define CONF_CVARLIST     0x00000005
#define CONF_CMDLIST      0x00000006
#define CONF_CVARUPDATE   0x00000007
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
#define CONF_CONDUMP      0x000000B0
#define CONF_LOADCFG      0x000000C0
#define CONF_SAVECFG      0x000000D0
#define CONF_LOADMAP      0x000000E0
#define CONF_SETMESSAGE   0x000000F0
#define CONF_MUSIC_START  0x00000100
#define CONF_MUSIC_PAUSE  0x00000200
#define CONF_MUSIC_STOP   0x00000300
#define CONF_MUSIC_RESUME 0x00000400

#define CONF_SHOW_INFO    0x00000500

#define CONF_INIT_SRV     0x00000600
#define CONF_SHUTDOWN_SRV 0x00000700
#define CONF_CONNECT      0x00000800
#define CONF_DISCONNECT   0x00000900
#define CONF_PAUSE        0x00000A00

#define CONF_TEST         0x01000000

#endif /* __LG_CMD_H__ */