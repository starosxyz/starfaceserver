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
#ifndef CCONTROLLEROPENFLOWTRANSPORT_H
#define CCONTROLLEROPENFLOWTRANSPORT_H

#include "CAWACEWrapper.h"
#include "IAWSSLUtils.h"

class COpenflowTransport;

class CControllerTransport : public IAWTransportSink
    , public CAWReferenceControlSingleThread
{
public:
    CControllerTransport (IAWTransport *ptrans, COpenflowTransport *pserver);
    virtual ~CControllerTransport ();
    
    virtual void OnReceive(
            CAWMessageBlock &aData,
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara);
    
    virtual void OnSend(
            IAWTransport *aTrptId,
            CAWTransportParameter *aPara);
    
    virtual void OnDisconnect(
        CAWResult aReason,
        IAWTransport *aTrptId);
    
    void Disconnect();
    CAWResult SendData(CAWMessageBlock& aData);
    

private:
    CAWAutoPtr<IAWTransport> m_transport;
    COpenflowTransport *m_ptransport; 
    CAWMessageBlock *m_pMbSendBuf;
};
#endif //COpenflowTransport

