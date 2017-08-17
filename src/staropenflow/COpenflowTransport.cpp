/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     COpenflowTransport.cpp
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

#include "COpenflowTransport.h"
#include "COpenflowServer.h"
#include "CControllerTransport.h"



COpenflowTransport::COpenflowTransport (COpenflowServer *pserver, IAWTransport *ptrans, IAWSSLTransport *pssltrans)
    :m_pserver(pserver)
{
    CAW_INFO_TRACE("COpenflowTransport::COpenflowTransport");
    m_paTrptId=ptrans;
    if (ptrans != NULL)
    {
        CAWAutoPtr<IAWTransport> temp(ptrans);
        m_transport=temp;
        m_transport->OpenWithSink(this);
        m_transport->GetOption(CAW_OPT_TRANSPORT_PEER_ADDR, &m_addrPeer);
    }
    else
    {
        m_transport=NULL;
    }
    
    if (pssltrans != NULL)
    {
        CAWAutoPtr<IAWSSLTransport> ssltemp(pssltrans);
        m_ssltransport=ssltemp;
        CAW_INFO_TRACE("AAAAAAAAAAAAAAAAAA");
        m_ssltransport->OpenWithSink(this);
        CAW_INFO_TRACE("DDDDDDDDDDDDDDDD");
        m_ssltransport->GetOption(CAW_OPT_TRANSPORT_PEER_ADDR, &m_addrPeer);
    }
    else
    {
        m_ssltransport=NULL;
    }
    m_id=pserver->GetIds().AllocID();

}
COpenflowTransport::~COpenflowTransport ()
{
    CAW_INFO_TRACE("COpenflowTransport::~COpenflowTransport");
    if (m_transport.Get())
    {
        m_transport->Disconnect(CAW_OK);
        m_transport = NULL;
    }
    if (m_ssltransport.Get())
    {
        m_ssltransport->Disconnect(CAW_OK);
        m_ssltransport = NULL;
    }
    if (m_pbuffer)
    {
        m_pbuffer = NULL;
    }
    if (m_controllertransport.Get())
    {
        m_controllertransport->Disconnect();
        m_controllertransport=NULL;
    }
    if (m_id)
    {
        m_pserver->GetIds().FreeID(m_id);
    }
}

 uint32_t COpenflowTransport::GetId()
{
     return m_id;
}
void COpenflowTransport::OnReceive(
            CAWMessageBlock &aData,
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara = NULL) 
{ 
    CAW_INFO_TRACE("COpenflowTransport::OnReceive");
    std::cout << "OnReceive" << std::endl;
    if (m_pbuffer == NULL)
    {
        m_pbuffer = aData.DuplicateChained();
    }
    else
    {
        CAWMessageBlock *pmsgblock = aData.DuplicateChained();
        m_pbuffer->Append(pmsgblock);
    }
    
    if (m_pbuffer->GetChainedLength() > 1024*1024)
    {
        m_transport->Disconnect(CAW_OK);
    }
    
    if (m_controllertransport.Get())
    {
        m_controllertransport->SendData(*m_pbuffer);
        m_pbuffer->DestroyChained();
        m_pbuffer=NULL;
    }
}
    
void COpenflowTransport::OnSend(
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara = NULL)
{
    CAW_INFO_TRACE("COpenflowTransport::OnSend");
    if (m_pMbSendBuf)
        SendData(*m_pMbSendBuf);
}
    
void COpenflowTransport::OnDisconnect(
        CAWResult aReason,
        IAWTransport *aTrptId)
{
    CAW_INFO_TRACE("COpenflowTransport::OnDisconnect");
    if (m_controllertransport)
    {
       m_controllertransport->OnDisconnect(aReason, m_paTrptId); 
    }

    m_pserver->RemoveTransport(m_id);
}

void COpenflowTransport::OnSSLDataReceive(CAWMessageBlock &aData, IAWSSLTransport *ptrans)
{
    CAW_INFO_TRACE("COpenflowTransport::OnSSLReceive");
    std::cout << "OnSSLReceive" << std::endl;
    if (m_pbuffer == NULL)
    {
        m_pbuffer = aData.DuplicateChained();
    }
    else
    {
        CAWMessageBlock *pmsgblock = aData.DuplicateChained();
        m_pbuffer->Append(pmsgblock);
    }
    
    if (m_pbuffer->GetChainedLength() > 1024*1024)
    {
        m_transport->Disconnect(CAW_OK);
    }
    
    if (m_controllertransport.Get())
    {
        m_controllertransport->SendData(*m_pbuffer);
        m_pbuffer->DestroyChained();
        m_pbuffer=NULL;
    }
}
void COpenflowTransport::OnSSLDisconnect(CAWResult aReason, IAWSSLTransport *ptrans)
{
    CAW_INFO_TRACE("COpenflowTransport::OnSSLDisconnect");
    if (m_controllertransport)
    {
       m_controllertransport->OnDisconnect(aReason, m_paTrptId); 
    }

    m_pserver->RemoveTransport(m_id);
}

void COpenflowTransport::OnSSLDataSend(IAWSSLTransport *ptrans)
{
    CAW_INFO_TRACE("COpenflowTransport::OnSSLDataSend");
    if (m_pMbSendBuf)
        SendData(*m_pMbSendBuf);
}

CAWResult COpenflowTransport::ConnectController(const CAWInetAddr &controlleraddr)
{
    m_controllerAddr=controlleraddr;
    CAW_INFO_TRACE("COpenflowTransport::ConnectControllerssss");
    CAWConnectionManager::Instance()->CreateConnectionClient(CAWConnectionManager::CTYPE_TCP, m_connector.ParaOut());
    if (m_connector.Get())
    {    
        m_connector->AsycConnect(this, m_controllerAddr);
        return CAW_OK;
    }
}

void COpenflowTransport::OnConnectIndication(
    CAWResult aReason,
    IAWTransport *aTrpt,
    IAWAcceptorConnectorId *aRequestId)
{
    if(aReason == CAW_OK && aTrpt != NULL)
    {
        CAW_INFO_TRACE("COpenflowTransport::OnConnectIndicationSSSS");
        CAWAutoPtr<CControllerTransport> temp(new CControllerTransport(aTrpt, this));
        m_controllertransport=temp;
        if (m_pbuffer)
        {
            m_controllertransport->SendData(*m_pbuffer);
            m_pbuffer->DestroyChained();
            m_pbuffer=NULL;
        }
    }

}


void COpenflowTransport::Disconnect()
{
    CAW_INFO_TRACE("COpenflowTransport::Disconnect");
    if (m_transport)
    {
        m_transport->Disconnect(CAW_OK);
        m_transport=NULL;
    }
    if (m_controllertransport)
    {
       m_controllertransport=NULL; 
    }
    m_pserver->RemoveTransport(m_id);
}

void COpenflowTransport::SSLDisconnect()
{
    CAW_INFO_TRACE("COpenflowTransport::Disconnect");
    if (m_ssltransport)
    {
        m_ssltransport->Disconnect(CAW_OK);
        m_ssltransport=NULL;
    }
    if (m_controllertransport)
    {
       m_controllertransport=NULL; 
    }
    m_pserver->RemoveTransport(m_id);

}

CAWResult COpenflowTransport::SendData(CAWMessageBlock & msg)
{
    CAW_INFO_TRACE("COpenflowTransport::SendData");
    CAWResult rv;
    CAW_ASSERTE_RETURN(m_transport, CAW_ERROR_NOT_INITIALIZED);
    if (!m_pMbSendBuf) {
        rv = m_transport->SendData(msg);
        if (CAW_FAILED(rv)) {
            m_pMbSendBuf = msg.DuplicateChained();
        }
    }
    else {
        if (m_pMbSendBuf != &msg) {
            
            m_pMbSendBuf->Append(msg.DuplicateChained());
        }
        rv = m_transport->SendData(*m_pMbSendBuf);
        m_pMbSendBuf = m_pMbSendBuf->ReclaimGarbage();
    }
    return rv;
}

CAWResult COpenflowTransport::SendSSLData(CAWMessageBlock &aData)
{
    CAW_INFO_TRACE("COpenflowTransport::SendData");
    CAWResult rv;
    CAW_ASSERTE_RETURN(m_transport, CAW_ERROR_NOT_INITIALIZED);
    if (!m_pMbSendBuf) {
        rv = m_ssltransport->SendSSLData(aData);
        if (CAW_FAILED(rv)) {
            m_pMbSendBuf = aData.DuplicateChained();
        }
    }
    else {
        if (m_pMbSendBuf != &aData) {
            
            m_pMbSendBuf->Append(aData.DuplicateChained());
        }
        rv = m_ssltransport->SendSSLData(aData);
        m_pMbSendBuf = m_pMbSendBuf->ReclaimGarbage();
    }
    return rv;
}

void COpenflowTransport::GetClientAndControlArr(CAWInetAddr &controllerAddr,CAWInetAddr &clientAddr)
{
    controllerAddr=m_controllerAddr;
    clientAddr=m_addrPeer;
}
