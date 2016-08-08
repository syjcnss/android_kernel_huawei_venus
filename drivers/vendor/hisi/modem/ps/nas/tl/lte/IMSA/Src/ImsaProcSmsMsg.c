/******************************************************************************

   Copyright(C)2013,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : ImsaProcSmsMsg.c
  Description     : ��C�ļ�ʵ��Sms��Ϣ������Sms��Ϣ����
  History           :
     1.sunbing 49683      2013-06-19  Draft Enact

******************************************************************************/

/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include "PsTypeDef.h"
#include "ImsaProcSmsMsg.h"
#include "ImsaPublic.h"
#include "ImsaImsInterface.h"
#include "ImsaProcSmsMsg.h"
#include "ImsaImsAdaption.h"
#include "MnMsgApi.h"
/*lint -esym(960,*)*/
/*lint -e767*/
#define    THIS_FILE_ID          PS_FILE_ID_IMSAPROCSMSMSG_C
#define    THIS_NAS_FILE_ID      NAS_FILE_ID_IMSAPROCSMSMSG_C
/*lint +e767*/
/*lint +esym(960,*)*/


/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == FEATURE_IMS)
/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/

VOS_UINT8   gucImsaSmrMR = 0;

extern VOS_VOID IMSA_SMS_SndMtErrInd(VOS_UINT32 ulErrCode);
extern VOS_VOID IMSA_SMS_GetSmscFromRpData(const VOS_UINT8* pucRpData, VOS_CHAR* pcSmsc);
extern VOS_VOID IMSA_ProcSmsMsgRpDataReq(const MSG_IMSA_RP_DATA_REQ_STRU *pstRpDataReq);
extern VOS_VOID IMSA_ProcSmsMsgReportReq(const MSG_IMSA_REPORT_REQ_STRU *pstReportReq);
extern VOS_VOID IMSA_ProcSmsMsgSmmaReq(const MSG_IMSA_SMMA_REQ_STRU *pstSmmaReq);
extern VOS_VOID IMSA_SMS_SndCancelImsMoSms(VOS_VOID);
/*****************************************************************************
  3 Function
*****************************************************************************/
/*lint -esym(960,*)*/

/*lint -e960*/
/*lint -e961*/
/*****************************************************************************
 Function Name  : IMSA_ProcSmsMsgRpDataReq()
 Description    : MO SMS��Ϣ��������
 Input          : MSG_IMSA_RP_DATA_REQ_STRU *pstRpDataReq
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-08  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_ProcSmsMsgRpDataReq(const MSG_IMSA_RP_DATA_REQ_STRU *pstRpDataReq)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[255];
    VOS_CHAR                             acSmsc[IMSA_IMS_ALPHA_STRING_SZ+1];

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    if(IMSA_SMS_SMR_STATE_IDLE == pstSmrMoEntity->enState)
    {
        if((pstRpDataReq->ulDataLen == 0)||(pstRpDataReq->ulDataLen >= 255))
        {
            IMSA_WARN_LOG("IMSA_ProcSmsMsgRpDataReq:Data Lenth is ERR! ");
            TLPS_PRINT2LAYER_WARNING(IMSA_ProcSmsMsgRpDataReq_ENUM, LNAS_PARAM_INVALID);
            return;
        }

        switch (IMSA_DsdsGetResourceStatus(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS))
        {
            case IMSA_RESOURCE_STATUS_ACTIVE:

                /*�����������ݣ�MSG������aucData����һ���ֽ��Ƕ��ŷ�����IMS���ԣ�
                  ���ݳ����ǲ�����������������ݵ�*/

        IMSA_MEM_SET_S( aucTmpData, sizeof(aucTmpData), 0, 255);
        IMSA_MEM_CPY_S( aucTmpData,
                        sizeof(aucTmpData),
                        &pstRpDataReq->aucData[1],
                        pstRpDataReq->ulDataLen);

                pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/
                aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                IMSA_INFO_LOG1("IMSA_ProcSmsMsgRpDataReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgRpDataReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

                /*��ȡ�������ĵ�ַ*/
                IMSA_MEM_SET_S(acSmsc, sizeof(acSmsc), 0, IMSA_IMS_ALPHA_STRING_SZ+1);

                IMSA_SMS_GetSmscFromRpData(aucTmpData, acSmsc);

                IMSA_SMS_SndImsSmsMsg(aucTmpData,pstRpDataReq->ulDataLen,acSmsc);
                break;
            #if (FEATURE_ON == FEATURE_DSDS)
            case IMSA_RESOURCE_STATUS_APPLY_NOT_NEED_WAIT_CNF:

                IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                              IMSA_RF_RES_APPLY_TRIGER_SRC_MO_SMS,
                                              VOS_FALSE,
                                              VOS_FALSE);

                /*�����������ݣ�MSG������aucData����һ���ֽ��Ƕ��ŷ�����IMS���ԣ�
                  ���ݳ����ǲ�����������������ݵ�*/

                IMSA_MEM_SET_S( aucTmpData, sizeof(aucTmpData), 0, 255);
                IMSA_MEM_CPY_S( aucTmpData,
                                sizeof(aucTmpData),
                                &pstRpDataReq->aucData[1],
                                pstRpDataReq->ulDataLen);

                pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/
                aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                IMSA_INFO_LOG1("IMSA_ProcSmsMsgRpDataReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgRpDataReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

                /*��ȡ�������ĵ�ַ*/
                IMSA_MEM_SET_S(acSmsc, IMSA_IMS_ALPHA_STRING_SZ+1, 0, IMSA_IMS_ALPHA_STRING_SZ+1);

                IMSA_SMS_GetSmscFromRpData(aucTmpData, acSmsc);

                IMSA_SMS_SndImsSmsMsg(aucTmpData,pstRpDataReq->ulDataLen,acSmsc);
                break;

            case  IMSA_RESOURCE_STATUS_NULL:

                IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                              IMSA_RF_RES_APPLY_TRIGER_SRC_MO_SMS,
                                              VOS_FALSE,
                                              VOS_TRUE);

                IMSA_DsdsSaveMoSmsMsg(pstRpDataReq);
                break;
            #endif
            default:
                break;

        }
    }
    else
    {
        /*lint -e56*/
        IMSA_SMS_SndMsgReportInd((SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_SMS_OPERATING), VOS_NULL_PTR, 0);
        /*lint +e56*/

        /*״̬�����ϱ�����*/
        IMSA_WARN_LOG("IMSA_ProcSmsMsgRpDataReq: Smr Mo State is IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK!");
        TLPS_PRINT2LAYER_WARNING(IMSA_ProcSmsMsgRpDataReq_ENUM, IMSA_STATE_ERROR);
    }
}


/*****************************************************************************
 Function Name  : IMSA_ProcSmsMsgReportReq()
 Description    : SMS Report��Ϣ��������
 Input          : MSG_IMSA_REPORT_REQ_STRU *pstReportReq
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-08  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_ProcSmsMsgReportReq(const MSG_IMSA_REPORT_REQ_STRU *pstReportReq)
{
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;
    VOS_UINT8                            aucTmpData[255];
    VOS_UINT32                           ulLength;

    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    if(IMSA_SMS_SMR_STATE_WAIT_TO_SND_RP_ACK == pstSmrMtEntity->enState)
    {
        /*״̬ת��Ϊ����̬*/
        pstSmrMtEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

        /*ֹͣTI_IMSA_SMS_TR2M��ʱ��*/
        IMSA_StopTimer(&pstSmrMtEntity->stTimerInfo);

        /* û��User Data �� rp_ack */
        if(pstReportReq->ulDataLen <= 2)
        {
            /* ��Ϣ����                                 */
            ulLength = 2;

            /* ��Ϣ����                                 */
            aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_ACK;

            /* MR                                       */
            aucTmpData[1] = pstSmrMtEntity->ucMessageReference;
        }
        else
        {
            /*������������*/
            IMSA_MEM_SET_S(aucTmpData, sizeof(aucTmpData), 0, 255);

            ulLength = pstReportReq->ulDataLen;

            /*MSG������aucData��report���ݲ�������Ϣ*/
            IMSA_MEM_CPY_S( aucTmpData,
                            sizeof(aucTmpData),
                            &pstReportReq->aucData[0],
                            pstReportReq->ulDataLen);

            /* MR                                       */
            aucTmpData[1] = pstSmrMtEntity->ucMessageReference;
        }

        IMSA_INFO_LOG1("IMSA_ProcSmsMsgReportReq: Snd SMR MR = ", pstSmrMtEntity->ucMessageReference);
        TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgReportReq_ENUM, 1, pstSmrMtEntity->ucMessageReference);

        /*��Ϣ����IMSЭ��ջ*/
        IMSA_SMS_SndImsSmsMsg(aucTmpData, ulLength,VOS_NULL_PTR);

        #if (FEATURE_ON == FEATURE_DSDS)
        if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
            (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS)))
        {
            /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
            IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS);

           IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS);
        }
        #endif
        IMSA_CommonDeregProc();
    }
    else
    {
        /*״̬�����ϱ�����*/
        IMSA_WARN_LOG("IMSA_ProcSmsMsgReportReq: Smr Mt State is IMSA_SMS_SMR_STATE_WAIT_TO_SND_RP_ACK!");
        TLPS_PRINT2LAYER_WARNING(IMSA_ProcSmsMsgReportReq_ENUM, IMSA_STATE_ERROR);
    }

}

/*****************************************************************************
 Function Name  : IMSA_ProcSmsMsgSmmaReg()
 Description    : SMS Smma��Ϣ��������
 Input          : MSG_IMSA_SMMA_REQ_STRU *pstSmmaReq
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-08  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_ProcSmsMsgSmmaReq(const MSG_IMSA_SMMA_REQ_STRU *pstSmmaReq)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[2];

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    switch(pstSmrMoEntity->enState)
    {

    case IMSA_SMS_SMR_STATE_IDLE:

        if (SMT_SMR_ABORT_FLG_NO_EXIST == pstSmmaReq->ucAbortFlg )
        {
            switch (IMSA_DsdsGetResourceStatus(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS))
            {
                case IMSA_RESOURCE_STATUS_ACTIVE:

                    pstSmrMoEntity->ucMemAvailFlg      = IMSA_SMS_TRUE;
                    pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PERMIT;
                    pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;

                    /* ��д��Ϣ���� */
                    aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
                    aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                    /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                    IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                    IMSA_INFO_LOG1("IMSA_ProcSmsMsgSmmaReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                    TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgSmmaReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

                    IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);
                    break;
                #if (FEATURE_ON == FEATURE_DSDS)
                case IMSA_RESOURCE_STATUS_APPLY_NOT_NEED_WAIT_CNF:

                    IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                                  IMSA_RF_RES_APPLY_TRIGER_SRC_MO_SMMA,
                                                  VOS_FALSE,
                                                  VOS_FALSE);

                    pstSmrMoEntity->ucMemAvailFlg      = IMSA_SMS_TRUE;
                    pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PERMIT;
                    pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;

                    /* ��д��Ϣ���� */
                    aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
                    aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                    /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                    IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                    IMSA_INFO_LOG1("IMSA_ProcSmsMsgSmmaReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                    TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgSmmaReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

                    IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);
                    break;

                case  IMSA_RESOURCE_STATUS_NULL:

                    IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                                  IMSA_RF_RES_APPLY_TRIGER_SRC_MO_SMMA,
                                                  VOS_FALSE,
                                                  VOS_TRUE);

                    IMSA_DsdsSaveMoSmmaMsg(pstSmmaReq);
                    break;
                #endif
                default:
                    break;

            }
        }
        break;
    case IMSA_SMS_SMR_STATE_WAIT_FOR_RETRANS_TIMER:

        if (SMT_SMR_ABORT_FLG_EXIST == pstSmmaReq->ucAbortFlg )
        {
            /*ת״̬��ֹͣTI_IMSA_SMS_TRAM������ʱ��*/
            pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;
            IMSA_StopTimer(&pstSmrMoEntity->stTRAMTimer);

            IMSA_SMS_SndMsgReportInd(SMR_SMT_ERROR_USER_ABORT, VOS_NULL_PTR, 0);

            pstSmrMoEntity->ucMemAvailFlg      = IMSA_SMS_FALSE;
            pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PERMIT;

            #if (FEATURE_ON == FEATURE_DSDS)
            if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
                (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS)))
            {
                /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
                IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);

                IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);
            }
            #endif
        }
        break;

    default:

        break;
    }

}

/*****************************************************************************
 Function Name  : IMSA_SMS_SndCancelImsMoSms()
 Description    : ����ȡ�����ŷ�����Ϣ
 Input          : VOS_VOID
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-09-06  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_SndCancelImsMoSms(VOS_VOID)
{
    IMSA_IMS_INPUT_EVENT_STRU                    *pstImsaImsInputEvent;

    /*�Ѷ��ŷ���IMSЭ��ջ*/
    pstImsaImsInputEvent = IMSA_MEM_ALLOC(sizeof(IMSA_IMS_INPUT_EVENT_STRU));

    if(VOS_NULL_PTR == pstImsaImsInputEvent)
    {
        IMSA_ERR_LOG("IMSA_SMS_SndCancelImsMoSms:Alloc mem Err! ");
        TLPS_PRINT2LAYER_ERROR(IMSA_SMS_SndCancelImsMoSms_ENUM, LNAS_NULL_PTR);
        return;
    }

    IMSA_MEM_SET_S( &pstImsaImsInputEvent->evt.stInputSmsEvent,
                    sizeof(IMSA_IMS_INPUT_SMS_EVENT_STRU),
                    0,
                    sizeof(IMSA_IMS_INPUT_SMS_EVENT_STRU));

    pstImsaImsInputEvent->enEventType = IMSA_IMS_EVENT_TYPE_SMS;
    pstImsaImsInputEvent->evt.stInputSmsEvent.enInputSmsReason = IMSA_IMS_INPUT_SMS_REASON_TR1M_EXP;
    pstImsaImsInputEvent->evt.stInputSmsEvent.ulOpId = IMSA_AllocImsOpId();

    IMSA_SndImsMsgSmsEvent(pstImsaImsInputEvent);

    IMSA_MEM_FREE(pstImsaImsInputEvent);

}


VOS_VOID IMSA_SMS_SndCancelImsMtSms(VOS_VOID)
{
    IMSA_IMS_INPUT_EVENT_STRU                    *pstImsaImsInputEvent;

    /*�Ѷ��ŷ���IMSЭ��ջ*/
    pstImsaImsInputEvent = IMSA_MEM_ALLOC(sizeof(IMSA_IMS_INPUT_EVENT_STRU));

    if(VOS_NULL_PTR == pstImsaImsInputEvent)
    {
        IMSA_ERR_LOG("IMSA_SMS_SndCancelImsMtSms:Alloc mem Err! ");
        TLPS_PRINT2LAYER_ERROR(IMSA_SMS_SndCancelImsMtSms_ENUM, LNAS_NULL_PTR);
        return;
    }

    IMSA_MEM_SET_S( &pstImsaImsInputEvent->evt.stInputSmsEvent,
                    sizeof(IMSA_IMS_INPUT_SMS_EVENT_STRU),
                    0,
                    sizeof(IMSA_IMS_INPUT_SMS_EVENT_STRU));

    pstImsaImsInputEvent->enEventType = IMSA_IMS_EVENT_TYPE_SMS;
    pstImsaImsInputEvent->evt.stInputSmsEvent.enInputSmsReason = IMSA_IMS_INPUT_SMS_REASON_TR2M_EXP;
    pstImsaImsInputEvent->evt.stInputSmsEvent.ulOpId = IMSA_AllocImsOpId();

    IMSA_SndImsMsgSmsEvent(pstImsaImsInputEvent);

    IMSA_MEM_FREE(pstImsaImsInputEvent);

}


/*****************************************************************************
 Function Name  : IMSA_SMS_SndImsSmsMsg()
 Description    : ���Ͷ�������
 Input          : pucData �������ݣ�
                  ulLength ���ų���
                  pcSmsc ��������
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-08  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_SndImsSmsMsg(const VOS_UINT8 *pucData,
                                            VOS_UINT32 ulLength,
                                            VOS_CHAR *pcSmsc)
{
    IMSA_IMS_INPUT_EVENT_STRU                    *pstImsaImsInputEvent;

    /*�Ѷ��ŷ���IMSЭ��ջ*/
    pstImsaImsInputEvent = IMSA_MEM_ALLOC(sizeof(IMSA_IMS_INPUT_EVENT_STRU));

    if(VOS_NULL_PTR == pstImsaImsInputEvent)
    {
        IMSA_ERR_LOG("IMSA_SMS_SndImsSmsMsg:Alloc mem Err! ");
        TLPS_PRINT2LAYER_ERROR(IMSA_SMS_SndImsSmsMsg_ENUM, LNAS_NULL_PTR);
        return;
    }

    IMSA_MEM_SET_S( pstImsaImsInputEvent,
                    sizeof(IMSA_IMS_INPUT_EVENT_STRU),
                    0,
                    sizeof(IMSA_IMS_INPUT_EVENT_STRU));

    pstImsaImsInputEvent->enEventType = IMSA_IMS_EVENT_TYPE_SMS;
    pstImsaImsInputEvent->evt.stInputSmsEvent.enInputSmsReason = IMSA_IMS_INPUT_SMS_REASON_SEND_MESSAGE;
    pstImsaImsInputEvent->evt.stInputSmsEvent.ulOpId = IMSA_AllocImsOpId();

    /*��������*/
    pstImsaImsInputEvent->evt.stInputSmsEvent.bitOpPdu = IMSA_OP_TRUE;
    pstImsaImsInputEvent->evt.stInputSmsEvent.ucPduLen = (VOS_UINT8)ulLength;

    IMSA_MEM_SET_S( pstImsaImsInputEvent->evt.stInputSmsEvent.acPdu,
                    IMSA_IMS_SMS_STRING_SZ + 1,
                    0,
                    ulLength+1);
    IMSA_MEM_CPY_S( pstImsaImsInputEvent->evt.stInputSmsEvent.acPdu,
                    IMSA_IMS_SMS_STRING_SZ,
                    pucData,
                    ulLength);

    /*�������ĵ�ַ��Ϊ�գ���ҪЯ����IMSЭ��ջ*/
    if(VOS_NULL_PTR != pcSmsc)
    {
        pstImsaImsInputEvent->evt.stInputSmsEvent.bitOpSmsc = IMSA_OP_TRUE;
        (VOS_VOID)VOS_StrNCpy(pstImsaImsInputEvent->evt.stInputSmsEvent.acSmsc, pcSmsc, IMSA_IMS_ALPHA_STRING_SZ);
    }
    else
    {
        pstImsaImsInputEvent->evt.stInputSmsEvent.bitOpSmsc = 0;
    }

    IMSA_SndImsMsgSmsEvent(pstImsaImsInputEvent);

    IMSA_MEM_FREE(pstImsaImsInputEvent);

}

/*****************************************************************************
 Function Name  : IMSA_SMS_GetSmscFromRpData()
 Description    : �Ӷ��������л�ȡ�������ĵ�ַ
 Input          :
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-09-12  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_GetSmscFromRpData(const VOS_UINT8* pucRpData, VOS_CHAR* pcSmsc)
{
    VOS_UINT8             ucSmscLength;
    VOS_UINT8             ucCallTon;

    if((VOS_NULL_PTR == pucRpData)||(VOS_NULL_PTR == pcSmsc))
        return;

    /*�������ĵĳ�����RP DATA�ĵ�4���ֽ�(���������͵ĳ��ȣ�һ���ֽ�)��
      ��6���ֽڿ�ʼ��BCD��ʽ�ĺ���*/
    ucSmscLength = pucRpData[3]-1;

    /*���ȳ����������Ĵ�ſռ䣬���˳����������*/
    if(ucSmscLength > IMSA_IMS_ALPHA_STRING_SZ/2)
    {
        IMSA_WARN_LOG("IMSA_SMS_GetSmscFromRpData:Data Lenth is ERR! ");
        TLPS_PRINT2LAYER_WARNING(IMSA_SMS_GetSmscFromRpData_ENUM, LNAS_PARAM_INVALID);
        return;
    }

    /*RP DATA��5���ֽ��Ǳ������ͣ������0x91��˵�������+�ţ������0x81�������Ӻ�*/
    /*24.008 10.5.4.9,Type of Number,5,6,7bit of the MN_CALL_NUM_TYPE_ENUM_U8*/
    ucCallTon = (pucRpData[4]>>4)&0x7;

    /*���ʺ��룬��������"+"*/
    if(ucCallTon == MN_CALL_TON_INTERNATIONAL)
    {
        /*��IMS�Ķ������ĺ��룬���ϡ�+��*/
        pcSmsc[0] = '+';
        (VOS_VOID)IMSA_UtilBcdNumberToAscii(&pucRpData[5], ucSmscLength, &pcSmsc[1]);
    }
    else
    {
        (VOS_VOID)IMSA_UtilBcdNumberToAscii(&pucRpData[5], ucSmscLength, pcSmsc);
    }
}


VOS_VOID IMSA_SMS_MemNotifyRetrans( VOS_UINT32 ulErrCode )
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;

    IMSA_INFO_LOG("IMSA_SMS_MemNotifyRetrans: enter.");
    TLPS_PRINT2LAYER_INFO(IMSA_SMS_MemNotifyRetrans_ENUM, LNAS_ENTRY);

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    if ( IMSA_SMR_RETRANS_PERMIT != pstSmrMoEntity->ucRetransFlg )
    {                                                                           /* RETRANS FLAG ��λ                        */
        IMSA_SMS_SndMsgReportInd(ulErrCode, VOS_NULL_PTR, 0);                   /* ����SMRL_REPORT_IND                      */
        pstSmrMoEntity->ucRetransFlg  = IMSA_SMR_RETRANS_PERMIT;                /* ��RETRANS FLAG��0���                    */
        pstSmrMoEntity->ucMemAvailFlg = IMSA_SMS_FALSE;                         /* �����־λ                               */
        pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;                      /* ״̬Ǩ�Ƶ�����״̬                       */
        #if (FEATURE_ON == FEATURE_DSDS)
        if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
            (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS)))
        {
            /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
            IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);

            IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);
        }
        #endif
        IMSA_CommonDeregProc();
    }
    else
    {
        /*���ǰ�����Ϣ*/
        pstSmrMoEntity->ucRetransFlg = IMSA_SMR_RETRANS_NO_PERMIT;              /* ��RETRANS FLAG��0���                    */

        pstSmrMoEntity->enState      = IMSA_SMS_SMR_STATE_WAIT_FOR_RETRANS_TIMER;/* ״̬Ǩ��                                 */
        IMSA_StartTimer(&pstSmrMoEntity->stTRAMTimer);

        #if (FEATURE_ON == FEATURE_DSDS)
        if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
            (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS)))
        {
            /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
            IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);

            IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);
        }
        #endif
    }
}

/*****************************************************************************
 Function Name  : IMSA_SMS_ProcTimerMsgTr1m()
 Description    : Tr2m��ʱ����ʱ��������
 Input          : VOS_VOID *pTimerMsg
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-09  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_ProcTimerMsgTr1m(const VOS_VOID *pTimerMsg)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;

    (VOS_VOID)pTimerMsg;

    IMSA_INFO_LOG("IMSA_SMS_ProcTimerMsgTr1m: Tr1M expired.");
    TLPS_PRINT2LAYER_INFO(IMSA_SMS_ProcTimerMsgTr1m_ENUM, LNAS_TIMER_EXPIRED);

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    if(IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK == pstSmrMoEntity->enState)
    {
        if (IMSA_SMS_TRUE == pstSmrMoEntity->ucMemAvailFlg )
        {
            if ( IMSA_SMR_RETRANS_PERMIT != pstSmrMoEntity->ucRetransFlg )
            {
                /*TR1M��ʱ��֪ͨIMSЭ��ջȡ������*/
                IMSA_SMS_SndCancelImsMoSms();
            }
            IMSA_SMS_MemNotifyRetrans(SMR_SMT_ERROR_TR1M_TIMEOUT);         /* �����ط����̵Ĵ���                       */
        }
        else
        {
            /*TR1M��ʱ��֪ͨIMSЭ��ջȡ������*/
            IMSA_SMS_SndCancelImsMoSms();

            /*���ϲ�MSG�ϱ�����ָʾ*/
            IMSA_SMS_SndMsgReportInd(SMR_SMT_ERROR_TR1M_TIMEOUT, VOS_NULL_PTR, 0);

            /*MOʵ��������̬*/
            pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

            #if (FEATURE_ON == FEATURE_DSDS)
            if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
                (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS)))
            {
                /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
                IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);

                IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS);
            }
            #endif

            IMSA_CommonDeregProc();
        }
    }
}

/*****************************************************************************
 Function Name  : IMSA_SMS_ProcTimerMsgTr2m()
 Description    : Tr2m��ʱ����ʱ��������
 Input          : VOS_VOID *pTimerMsg
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-09  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_ProcTimerMsgTr2m(const VOS_VOID *pTimerMsg)
{
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;

    (VOS_VOID)pTimerMsg;

    IMSA_INFO_LOG("IMSA_SMS_ProcTimerMsgTr2m: Tr2M expired.");
    TLPS_PRINT2LAYER_INFO(IMSA_SMS_ProcTimerMsgTr2m_ENUM, LNAS_TIMER_EXPIRED);

    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    if(IMSA_SMS_SMR_STATE_WAIT_TO_SND_RP_ACK == pstSmrMtEntity->enState)
    {
        /*TR2M��ʱ�������¼�֪ͨ��IMS*/
        IMSA_SMS_SndCancelImsMtSms();

        /*���ϲ�MSG�ϱ�����ָʾ*/
        IMSA_SMS_SndMtErrInd(SMR_SMT_ERROR_TR2M_TIMEOUT);

        /*MOʵ��������̬*/
        pstSmrMtEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

        #if (FEATURE_ON == FEATURE_DSDS)
        if ((PS_RRM_RF_SHARED == IMSA_Dsds_RRM_RfResourceIsShared()) &&
            (IMSA_TASKTYPE_STATUS_NULL != IMSA_DsdsGetTaskStatusByTaskType(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS)))
        {
            /* ����ӳٻظ���ռ��ʱ����ʱ��Ӧ��ֱ�ӻظ�CNF */
            IMSA_DsdsProcResourceOccupyCnf(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS);

            IMSA_DsdsProcResourceRelease(RRM_PS_TASK_TYPE_LTE_IMS_MT_SMS);
        }
        #endif

        IMSA_CommonDeregProc();

    }
}


VOS_VOID IMSA_SMS_ProcTimerMsgTram(const VOS_VOID *pTimerMsg)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[255];

    (VOS_VOID)pTimerMsg;

    IMSA_INFO_LOG("IMSA_SMS_ProcTimerMsgTram: TrAM expired.");
    TLPS_PRINT2LAYER_INFO(IMSA_SMS_ProcTimerMsgTram_ENUM, LNAS_TIMER_EXPIRED);

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    if(IMSA_SMS_SMR_STATE_WAIT_FOR_RETRANS_TIMER == pstSmrMoEntity->enState)
    {
        switch (IMSA_DsdsGetResourceStatus(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS))
        {
            case IMSA_RESOURCE_STATUS_ACTIVE:

                pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PROCESS;

                pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/

                /* ��д��Ϣ���� */
                aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
                aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                IMSA_INFO_LOG1("IMSA_SMS_ProcTimerMsgTram: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                TLPS_PRINT2LAYER_INFO1(IMSA_SMS_ProcTimerMsgTram_ENUM, 2, pstSmrMoEntity->ucMessageReference);

                IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);
                break;
            #if (FEATURE_ON == FEATURE_DSDS)
            case IMSA_RESOURCE_STATUS_APPLY_NOT_NEED_WAIT_CNF:

                IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                              IMSA_RF_RES_APPLY_TRIGER_SRC_SMMA_RETRANS,
                                              VOS_FALSE,
                                              VOS_FALSE);

                pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PROCESS;

                pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/

                /* ��д��Ϣ���� */
                aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
                aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

                /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
                pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
                IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

                IMSA_INFO_LOG1("IMSA_SMS_ProcTimerMsgTram: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
                TLPS_PRINT2LAYER_INFO1(IMSA_SMS_ProcTimerMsgTram_ENUM, 2, pstSmrMoEntity->ucMessageReference);

                IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);
                break;

            case  IMSA_RESOURCE_STATUS_NULL:

                IMSA_DsdsProcResourceApplyReq(RRM_PS_TASK_TYPE_LTE_IMS_MO_SMS,
                                              IMSA_RF_RES_APPLY_TRIGER_SRC_SMMA_RETRANS,
                                              VOS_FALSE,
                                              VOS_FALSE);

                //IMSA_DsdsSaveMoSmsMsg(pstRpDataReq);
                break;
            #endif
            default:
                break;

        }

    }
}


/*****************************************************************************
 Function Name  : IMSA_ProcSmsMsg()
 Description    : MSG��Ϣ��������
 Input          : VOS_VOID *pRcvMsg
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-08  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_ProcSmsMsg(const VOS_VOID *pRcvMsg)
{
    /* ������Ϣͷָ��*/
    PS_MSG_HEADER_STRU          *pHeader = VOS_NULL_PTR;

    /* ��ȡ��Ϣͷָ��*/
    pHeader = (PS_MSG_HEADER_STRU *) pRcvMsg;

    switch(pHeader->ulMsgName)
    {
        case ID_MSG_IMSA_RP_DATA_REQ:
            IMSA_ProcSmsMsgRpDataReq((MSG_IMSA_RP_DATA_REQ_STRU*)pRcvMsg);
            break;

        case ID_MSG_IMSA_REPORT_REQ:
            IMSA_ProcSmsMsgReportReq((MSG_IMSA_REPORT_REQ_STRU*)pRcvMsg);
            break;

        case ID_MSG_IMSA_SMMA_REQ:
            IMSA_ProcSmsMsgSmmaReq((MSG_IMSA_SMMA_REQ_STRU*)pRcvMsg);
            break;

        default:
            break;
    }

}

/*****************************************************************************
 Function Name  : IMSA_SMS_SndMtErrInd()
 Description    : ����MT err ind��Ϣ
 Input          : ucRpStatus��ucRpCause
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-09  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_SndMtErrInd(VOS_UINT32 ulErrCode)
{
    IMSA_MSG_MT_ERR_IND_STRU    *pstMtErrInd;

    pstMtErrInd = (VOS_VOID*)IMSA_ALLOC_MSG(sizeof(IMSA_MSG_MT_ERR_IND_STRU));

    /*����Ƿ����ɹ�*/
    if (VOS_NULL_PTR == pstMtErrInd)
    {
        /*��ӡ�쳣��Ϣ*/
        IMSA_ERR_LOG("IMSA_SMS_SndMtErrInd:ERROR:Alloc Msg fail!");
        TLPS_PRINT2LAYER_ERROR(IMSA_SMS_SndMtErrInd_ENUM, LNAS_NULL_PTR);
        return ;
    }

    IMSA_WRITE_MSG_MSG_HEAD(pstMtErrInd,ID_IMSA_MSG_MT_ERR_IND);

    pstMtErrInd->enErrorCode = ulErrCode;

    /*������Ϣ���ͺ��� */
    IMSA_SND_MSG(pstMtErrInd);


}


/*****************************************************************************
 Function Name  : IMSA_SMS_Init()
 Description    : ����ģ���ʼ������
 Input          : VOS_VOID *pRcvMsg
 Output         : VOS_VOID
 Return Value   : VOS_VOID

 History        :
      1.sunbing 49683      2013-08-09  Draft Enact
*****************************************************************************/
VOS_VOID IMSA_SMS_Init(VOS_VOID)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;

    /*��ʼ������*/
    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    IMSA_MEM_SET_S( pstSmrMoEntity,
                    sizeof(IMSA_SMS_SMR_MO_STRU),
                    0,
                    sizeof(IMSA_SMS_SMR_MO_STRU));

    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    IMSA_MEM_SET_S( pstSmrMtEntity,
                    sizeof(IMSA_SMS_SMR_MT_STRU),
                    0,
                    sizeof(IMSA_SMS_SMR_MT_STRU));

    pstSmrMtEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

    /*���ö�ʱ�����ȣ����Ƶ�*/
    pstSmrMoEntity->stTR1MTimer.usName = TI_IMSA_SMS_TR1M;
    pstSmrMoEntity->stTR1MTimer.ulTimerLen = IMSA_SMS_TIMER_TR1M_LENGTH;
    pstSmrMoEntity->stTR1MTimer.ucMode = 0;

    pstSmrMoEntity->stTRAMTimer.usName = TI_IMSA_SMS_TRAM;
    pstSmrMoEntity->stTRAMTimer.ulTimerLen = IMSA_SMS_TIMER_TRAM_LENGTH;
    pstSmrMoEntity->stTRAMTimer.ucMode = 0;

    pstSmrMtEntity->stTimerInfo.usName = TI_IMSA_SMS_TR2M;
    pstSmrMtEntity->stTimerInfo.ulTimerLen = IMSA_SMS_TIMER_TR2M_LENGTH;
    pstSmrMtEntity->stTimerInfo.ucMode = 0;

}



VOS_VOID IMSA_SMS_ClearResource(VOS_VOID)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();
    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    /*ֹͣ��ʱ��*/
    if (VOS_TRUE == IMSA_IsTimerRunning(&pstSmrMoEntity->stTR1MTimer))
    {
        IMSA_StopTimer(&pstSmrMoEntity->stTR1MTimer);

        /*lint -e56*/
        IMSA_SMS_SndMsgReportInd(SMR_SMT_ERROR_IMSA_ERROR_BEGIN+MN_MSG_IMSA_CAUSE_NO_SERVICE, VOS_NULL_PTR, 0);
        /*lint +e56*/
    }

    if (VOS_TRUE == IMSA_IsTimerRunning(&pstSmrMoEntity->stTRAMTimer))
    {
        IMSA_StopTimer(&pstSmrMoEntity->stTRAMTimer);

        /*lint -e56*/
        IMSA_SMS_SndMsgReportInd(SMR_SMT_ERROR_IMSA_ERROR_BEGIN+MN_MSG_IMSA_CAUSE_NO_SERVICE, VOS_NULL_PTR, 0);
        /*lint +e56*/
    }

    if (VOS_TRUE == IMSA_IsTimerRunning(&pstSmrMtEntity->stTimerInfo))
    {
        IMSA_StopTimer(&pstSmrMtEntity->stTimerInfo);

        /*���ϲ�MSG�ϱ�����ָʾ*/
        /*lint -e56*/
        IMSA_SMS_SndMtErrInd(SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_NO_SERVICE);
        /*lint +e56*/
    }

    /*��ʼ������*/
    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;
    pstSmrMoEntity->ucMessageReference = 0;

    pstSmrMoEntity->ucMemAvailFlg = IMSA_SMS_FALSE;                   /* ���mem avail��־                        */
    pstSmrMoEntity->ucRetransFlg  = IMSA_SMR_RETRANS_PERMIT;          /* ��λ�ط���־                             */

    pstSmrMtEntity->enState = IMSA_SMS_SMR_STATE_IDLE;
    pstSmrMtEntity->ucMessageReference= 0;
}


VOS_UINT32 IMSA_IsSmsConnExist(VOS_VOID)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();
    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    if ((IMSA_SMS_SMR_STATE_IDLE != pstSmrMoEntity->enState) &&
        (IMSA_SMS_SMR_STATE_BUTT != pstSmrMoEntity->enState))
    {
        return VOS_TRUE;
    }

    if ((IMSA_SMS_SMR_STATE_IDLE != pstSmrMtEntity->enState) &&
        (IMSA_SMS_SMR_STATE_BUTT != pstSmrMtEntity->enState))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

#if (FEATURE_ON == FEATURE_DSDS)

VOS_VOID IMSA_SMSProcMoResourceApplyCnf
(
    const MSG_IMSA_RP_DATA_REQ_STRU *pstRpDataReq
)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[255];
    VOS_CHAR                             acSmsc[IMSA_IMS_ALPHA_STRING_SZ+1];

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();


    /*�����������ݣ�MSG������aucData����һ���ֽ��Ƕ��ŷ�����IMS���ԣ�
      ���ݳ����ǲ�����������������ݵ�*/
    IMSA_MEM_SET_S( aucTmpData, sizeof(aucTmpData), 0, 255);
    IMSA_MEM_CPY_S( aucTmpData,
                    sizeof(aucTmpData),
                    &pstRpDataReq->aucData[1],
                    pstRpDataReq->ulDataLen);

    pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/
    aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

    /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
    IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

    IMSA_INFO_LOG1("IMSA_ProcSmsMsgRpDataReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
    TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgRpDataReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

    /*��ȡ�������ĵ�ַ*/
    IMSA_MEM_SET_S( acSmsc, IMSA_IMS_ALPHA_STRING_SZ+1, 0, IMSA_IMS_ALPHA_STRING_SZ+1);

    IMSA_SMS_GetSmscFromRpData(aucTmpData, acSmsc);

    IMSA_SMS_SndImsSmsMsg(aucTmpData,pstRpDataReq->ulDataLen,acSmsc);

    return;
}

VOS_VOID IMSA_SMSProcSmmaResourceApplyCnf
(
    const MSG_IMSA_SMMA_REQ_STRU *pstSmmaReq
)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[2];

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    pstSmrMoEntity->ucMemAvailFlg      = IMSA_SMS_TRUE;
    pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PERMIT;
    pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;

    /* ��д��Ϣ���� */
    aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
    aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

    /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
    IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

    IMSA_INFO_LOG1("IMSA_ProcSmsMsgSmmaReq: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
    TLPS_PRINT2LAYER_INFO1(IMSA_ProcSmsMsgSmmaReq_ENUM, 1, pstSmrMoEntity->ucMessageReference);

    IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);

    return;
}


VOS_VOID IMSA_SMSProcSmmaRetransResourceApplyCnf
(
    VOS_VOID
)
{
    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;
    VOS_UINT8                            aucTmpData[255];

    IMSA_INFO_LOG("IMSA_SMS_ProcTimerMsgTram: TrAM expired.");
    TLPS_PRINT2LAYER_INFO(IMSA_SMS_ProcTimerMsgTram_ENUM, LNAS_TIMER_EXPIRED);

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    pstSmrMoEntity->ucRetransFlg       = IMSA_SMR_RETRANS_PROCESS;

    pstSmrMoEntity->ucMessageReference = ++gucImsaSmrMR;    /*��¼RP-MR*/

    /* ��д��Ϣ���� */
    aucTmpData[0] = IMSA_SMS_MTN_DATA_TYPE_RP_SMMA;
    aucTmpData[1] = pstSmrMoEntity->ucMessageReference;

    /*ת״̬������TI_IMSA_SMS_TR1M������ʱ��*/
    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_WAIT_FOR_RP_ACK;
    IMSA_StartTimer(&pstSmrMoEntity->stTR1MTimer);

    IMSA_INFO_LOG1("IMSA_SMS_ProcTimerMsgTram: Snd SMR MR = ", pstSmrMoEntity->ucMessageReference);
    TLPS_PRINT2LAYER_INFO1(IMSA_SMS_ProcTimerMsgTram_ENUM, 2, pstSmrMoEntity->ucMessageReference);

    IMSA_SMS_SndImsSmsMsg(aucTmpData, 2,VOS_NULL_PTR);

    return;
}


VOS_VOID IMSA_SMSProcMoResourceApplyTimerExp
(
    VOS_VOID
)
{
    /*���ϲ�MSG�ϱ�����ָʾ*/
    IMSA_SMS_SndMsgReportInd((SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_NO_RF), VOS_NULL_PTR, 0);

    return;
}


VOS_VOID IMSA_SMSProcSmmaResourceApplyTimerExp
(
    VOS_VOID
)
{
    /*���ϲ�MSG�ϱ�����ָʾ*/
    IMSA_SMS_MemNotifyRetrans((SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_NO_RF));

    return;
}




VOS_VOID IMSA_SMSProcMtResourceApplyTimerExp
(
    VOS_VOID
)
{

    /* ��ȡ���棬�������Ϣ���� */
    /* todo: */


    /* �������*/
    /* todo */

    /* ֪ͨ�ͷ� */

    return;
}

VOS_VOID IMSA_SMSProcMoSmsRfOccupiedDelayTimerExp
(
    VOS_VOID
)
{

    IMSA_SMS_SMR_MO_STRU                *pstSmrMoEntity;

    pstSmrMoEntity = IMSA_SMS_GetSmrMoEntityAddress();

    /* ֹͣTR1M��ʱ��*/
    IMSA_StopTimer(&pstSmrMoEntity->stTR1MTimer);

    /* ֪ͨIMSȡ�����ŷ��� */
    IMSA_SMS_SndCancelImsMoSms();

    /*���ϲ�MSG�ϱ�����ָʾ*/
    IMSA_SMS_SndMsgReportInd((SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_NO_RF), VOS_NULL_PTR, 0);

    /*MOʵ��������̬*/
    pstSmrMoEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

    IMSA_CommonDeregProc();
    return;
}


VOS_VOID IMSA_SMSProcMtSmsRfOccupiedDelayTimerExp
(
    VOS_VOID
)
{
    IMSA_SMS_SMR_MT_STRU                *pstSmrMtEntity;


    pstSmrMtEntity = IMSA_SMS_GetSmrMtEntityAddress();

    /* ֹͣTR2M��ʱ��*/
    IMSA_StopTimer(&pstSmrMtEntity->stTimerInfo);

    /*���ϲ�MSG�ϱ�����ָʾ*/
    IMSA_SMS_SndMtErrInd((SMR_SMT_ERROR_IMSA_ERROR_BEGIN + MN_MSG_IMSA_CAUSE_NO_RF));

    /*MOʵ��������̬*/
    pstSmrMtEntity->enState = IMSA_SMS_SMR_STATE_IDLE;

    IMSA_CommonDeregProc();
    return;
}

#endif

/*lint +e961*/
/*lint +e960*/

/*lint +esym(960,*)*/

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
/* end of ImsaProcSmsMsg.c */


