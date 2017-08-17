/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     CControllerTransport.cpp
 * Others:
 * Version:          V1.0
 * Author:           Yang XiangRui
 * Date:             2017-06-07
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/

#include "CControllerTransport.h"
#include "COpenflowTransport.h"
#include "json_value.h"
#include "json_reader.h"

CControllerTransport::CControllerTransport (IAWTransport *ptrans,COpenflowTransport *pserver )
    :m_ptransport(pserver)
    ,m_pMbSendBuf(NULL)
{
    CAW_INFO_TRACE("CControllerTransport::CControllerTransport");
    CAWAutoPtr<IAWTransport> temp(ptrans);
    m_transport=temp;
    m_transport->OpenWithSink(this);
    
}
CControllerTransport::~CControllerTransport ()
{
    CAW_INFO_TRACE("CControllerTransport::~CControllerTransport");
    if (m_transport.Get())
    {
        m_transport->Disconnect(CAW_OK);
        m_transport=NULL;
    }
    if (m_pMbSendBuf)
    {
        m_pMbSendBuf->DestroyChained();
        m_pMbSendBuf=NULL;
    }
}
void CControllerTransport::OnReceive(
            CAWMessageBlock &aData,
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara = NULL) 
{
    CAW_INFO_TRACE("CControllerTransport::OnReceive");
    if (m_ptransport)
    {
        m_ptransport->SendData(aData);
    }
}

void CControllerTransport::OnSend(
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara = NULL)
{
    CAW_INFO_TRACE("CControllerTransport::OnSend");
    CAW_ASSERTE(m_ptransport == aTrptId);

    if (m_pMbSendBuf) {
        CAWResult rv = SendData(*m_pMbSendBuf);
        if (CAW_FAILED(rv))
        {
            CAW_INFO_TRACE_THIS("CControllerTransport::OnSend, failure, rv="<<rv);
            return;
        }
    }

}
    
void CControllerTransport::OnDisconnect(
        CAWResult aReason,
        IAWTransport *aTrptId)
{
    CAW_INFO_TRACE("CControllerTransport::OnDisconnect");
    if (m_ptransport)
    {
        m_ptransport->Disconnect();
    }
}

       
void CControllerTransport::Disconnect()
{
    CAW_INFO_TRACE("CControllerTransport::Disconnect");
    if (m_transport)
    {
        m_transport->Disconnect(CAW_OK);
        m_transport=NULL;
    }
}

CAWResult CControllerTransport::SendData(CAWMessageBlock &aData)
{
    CAWResult rv;
    CAW_ASSERTE_RETURN(m_transport, CAW_ERROR_NOT_INITIALIZED);
    if (m_transport==NULL)
    {
        return CAW_ERROR_FAILURE;
    }
    if (!m_pMbSendBuf) {
        rv = m_transport->SendData(aData);
        if (CAW_FAILED(rv)) {
            m_pMbSendBuf = aData.DuplicateChained();
        }
    }
    else {
        if (m_pMbSendBuf != &aData) {
            
            m_pMbSendBuf->Append(aData.DuplicateChained());
        }
        rv = m_transport->SendData(*m_pMbSendBuf);
        m_pMbSendBuf = m_pMbSendBuf->ReclaimGarbage();
    }
    return rv;
}


