/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd  
 * Description:      CStarfaceServer
 * Others:
 * Version:          V1.0
 * Author:           Yang Yang
 * Date:             2017-03-24
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/

#ifndef CSTARFACESERVER_H
#define CSTARFACESERVER_H
#include "CAWACEWrapper.h"
#include "CWebServerChannelManager.h"
#include "CSessionManager.h"
#include "IAWHttpManager.h"
#include "CStarfaceWebClient.h"
#include "CAWIDAllocator.h"
#include "CStarfaceWebClientManager.h"
#include "CControllerManager.h"
class CStarfaceServer: public IAWChannelServerSink
{
public:
    CStarfaceServer();
    virtual ~CStarfaceServer();
    CAWResult InitUI();
    CAWResult Start(const CAWInetAddr& listenHttpAddr,const CAWInetAddr& listenHttpsAddr);
    CAWResult Start_i();
    void OnServerCreation(IAWChannelHttpServer* server);
    void SetControllerInfo(const std::string &ip, const std::string &port, const std::string &name, const std::string &pwd);
    CSessionManager &GetSessionMgr();
    CWebServerChannelManager &GetServerChannelMgr();
    void ServerDestroy(uint32_t id);
    CStarfaceWebClientManager &GetClientMgr();
    CControllerManager &GetControllerMgr();    
private:
    CWebServerChannelManager m_channelmgr;
    CSessionManager m_sessionmgr;
    CControllerManager m_controllermgr;
    CStarfaceWebClientManager m_clientmgr;
    CAWAutoPtr<IAWChannelServerAcceptor> m_pServerAcceptor;
    CAWAutoPtr<IAWChannelServerAcceptor> m_pServerHttpsAcceptor;

    CAWInetAddr m_listenHttpAddr;
    CAWInetAddr m_listenHttpsAddr;
    CAWIDAllocator m_idmsgr;
};
#endif//
