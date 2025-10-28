#ifndef __LV_RESTORE_H__
#define __LV_RESTORE_H__
#include "common.h"
#include "lg_sys.h"
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

#define LVERR_NODEVICE    0x80000010l
#define LVERR_DEVICELOST  0x80000000l
#define LVERR_CANTRECOVER 0x80000001l
#define LVERR_DEVICERESET 0x80000002l


L_result LV_ValidateDevice(L3DGame* lpGame);

L_result LV_ValidateGraphics(L3DGame* lpGame);
L_result LV_InvalidateGraphics(L3DGame* lpGame);

L_result LV_Restart(L3DGame* lpGame);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* __LV_RESTORE_H__*/