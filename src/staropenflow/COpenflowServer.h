/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     COpenflowServer.h
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

#ifndef COPENFLOWSERVER_H
#define COPENFLOWSERVER_H

#include "IStarGui.h"
#include "CAWACEWrapper.h"
#include "CAWIDAllocator.h"
#include "COpenflowTransport.h"
#include "IAWSSLUtils.h"

class CStarOpenflow;
class CAWIDAllocator;

class COpenflowServer : public IAWAcceptorConnectorSink 
    , public IRequestHandler
    , public IAWSSLConnectionSink
{
public:
    COpenflowServer (CStarOpenflow *process);
    virtual ~COpenflowServer ();
    virtual void OnConnectIndication(
        CAWResult aReason,
        IAWTransport *aTrpt,
        IAWAcceptorConnectorId *aRequestId);
    virtual void OnSSLConnected(IAWSSLTransport *aServer);
    void StarUI();
    CAWResult StarServer(const CAWString &ip, uint16_t port);
    CAWResult StartTCPServer(const CAWString &ip, uint16_t port);
    CAWResult StartSSLServer(const CAWString &ip, uint16_t port);
    void RemoveTransport (uint32_t id);
    CAWResult ReadDb(std::string &strjson);  

    //IRequestHandler
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);

    CAWResult ResponseData(Json::Value &response);
    CAWResult GetControllerList(std::list<string>& tmp);
    CAWIDAllocator& GetIds();
    
private:
    CAWAutoPtr<IUiExtension> m_uiext;
    CStarOpenflow *m_process;
    CAWAutoPtr<IAWAcceptor> m_acceptor;
    CAWAutoPtr<IAWSSLAcceptor> m_sslacceptor;
    std::unordered_map<uint32_t, CAWAutoPtr<COpenflowTransport>> m_transportlist;
    list <std::string> m_controlleraddr;
    int m_jsonsize;
    uint32_t m_id;
    CAWIDAllocator m_ids;
};
#endif //COpenflowServer
