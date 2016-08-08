

#ifndef __TAF_X_CALL_PROC_MMA_H__
#define __TAF_X_CALL_PROC_MMA_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

#include "vos.h"
#include "TafMmaInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  1 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID TAF_XCALL_RcvMmaPowerOffInd(VOS_VOID *pMsg);

VOS_VOID TAF_XCALL_RcvMmaServiceStatusInd(VOS_VOID *pMsg);
VOS_VOID TAF_XCALL_RcvMmaSysCfgInfoInd(VOS_VOID *pMsg);
#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TafXCallProcMma.h */

