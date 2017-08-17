/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     COpenflowTransport.h
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

#ifndef COPENFLOWTRANSPORT_H
#define COPENFLOWTRANSPORT_H

#include "CAWACEWrapper.h"
#include "CControllerTransport.h"
#include "CAWIDAllocator.h"
#include "IAWSSLUtils.h"


class COpenflowServer;
class COpenflowTransport : public IAWTransportSink
    , public CAWReferenceControlSingleThread
    , public IAWSSLTransportSink
    , public IAWAcceptorConnectorSink
{
public:
    COpenflowTransport (COpenflowServer *pserver,       IAWTransport *ptrans, IAWSSLTransport *pssltrans);
    virtual ~COpenflowTransport ();
    
    virtual void OnReceive(CAWMessageBlock &aData, IAWTransport *aTrptId, CAWTransportParameter *aPara);
    
    virtual void OnSend(IAWTransport *aTrptId, CAWTransportParameter *aPara);
    
    virtual void OnDisconnect(CAWResult aReason,IAWTransport *aTrptId);

    virtual void OnSSLDataReceive(CAWMessageBlock &aData, IAWSSLTransport *ptrans);
    virtual void OnSSLDisconnect(CAWResult aReason, IAWSSLTransport *ptrans);
    virtual void OnSSLDataSend(IAWSSLTransport *ptrans);

    
    CAWResult ConnectController(const CAWInetAddr &controlleraddr);
    virtual void OnConnectIndication(CAWResult aReason,IAWTransport *aTrpt,IAWAcceptorConnectorId *aRequestId);

    //IAWAcceptorConnectorSink
    CAWResult SendData(CAWMessageBlock & msg);
    void Disconnect();
    
    //IAWSSLConnectionSink
    CAWResult SendSSLData(CAWMessageBlock &aData);
    void SSLDisconnect();


    void GetClientAndControlArr(CAWInetAddr &controllerAddr,CAWInetAddr &clientAddr);
    uint32_t GetId();

private:
    uint32_t m_id;
    CAWAutoPtr<IAWTransport> m_transport;
    CAWAutoPtr<IAWSSLTransport> m_ssltransport;
    CAWAutoPtr<IAWConnector> m_connector;
    CAWAutoPtr<CControllerTransport> m_controllertransport;
    COpenflowServer *m_pserver;
    IAWTransport *m_paTrptId;
    CAWMessageBlock *m_pbuffer;
    CAWInetAddr m_addrPeer;
    CAWInetAddr m_controllerAddr;   
    CAWMessageBlock *m_pMbSendBuf;
};
#endif //COpenflowTransport

