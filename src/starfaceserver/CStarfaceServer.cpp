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

#include "CStarfaceServer.h"
#include "json.h"
#include "CWebServerChannel.h"
#include "CController.h"
class CEventStarWebServer : public IAWEvent
{
public:
    CEventStarWebServer(CStarfaceServer *pServer):m_pServer(pServer)
    {
    }
    virtual ~CEventStarWebServer()
    {
        
    }
    virtual CAWResult OnEventFire()
    {
        m_pServer->Start_i();
        return CAW_OK;
    }
private:
    CStarfaceServer *m_pServer;
};

CStarfaceServer::CStarfaceServer()
    :m_channelmgr(*this)
    ,m_sessionmgr(*this)
    ,m_clientmgr(this)
    ,m_controllermgr(this)
{
    m_idmsgr.Init(1, 8000);
    CAW_INFO_TRACE("CStarfaceServer::CStarfaceServer");
}
CStarfaceServer::~CStarfaceServer()
{
    CAW_INFO_TRACE("CStarfaceServer::~CStarfaceServer");
    if (m_pServerAcceptor) 
    {
        m_pServerAcceptor->StopListen(CAW_OK);
        m_pServerAcceptor = NULL;
    }
    if (m_pServerHttpsAcceptor)
    {
        m_pServerHttpsAcceptor->StopListen(CAW_OK);
        m_pServerHttpsAcceptor=NULL;
    }
}

CAWResult CStarfaceServer::InitUI()
{
    m_channelmgr.Init();
    m_sessionmgr.Init();
    m_clientmgr.Init();
    m_controllermgr.Init();
    return CAW_OK;
}

CAWResult CStarfaceServer::Start(const CAWInetAddr& listenHttpAddr,const CAWInetAddr& listenHttpsAddr)
{
    m_listenHttpAddr = listenHttpAddr;
    m_listenHttpsAddr = listenHttpsAddr;

    CAW_INFO_TRACE("CStarfaceServer::Start m_listenHttpAddr="<<m_listenHttpAddr.GetIpDisplayName());
    CAW_INFO_TRACE("CStarfaceServer::Start m_listenHttpsAddr="<<m_listenHttpsAddr.GetIpDisplayName());
    
    CEventStarWebServer *pnew = new CEventStarWebServer(this);
    CAWThread *pThread=CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_NETWORK);
    if (pThread)
    {
        pThread->GetEventQueue()->PostEvent(pnew);
    }
    
    //CAW_INFO_TRACE(listenAddr);
    
    return CAW_OK;
}

CAWResult CStarfaceServer::Start_i()
{
    if (m_pServerAcceptor) 
    {
        m_pServerAcceptor->StopListen(CAW_OK);
        m_pServerAcceptor = NULL;
    }
    if (m_pServerHttpsAcceptor)
    {
        m_pServerHttpsAcceptor->StopListen(CAW_OK);
        m_pServerHttpsAcceptor=NULL;
    }

    CAW_INFO_TRACE("CStarfaceServer::Start_i listenaddr="<<m_listenHttpAddr.GetIpDisplayName());
    CAW_INFO_TRACE("CStarfaceServer::Start_i listenaddr="<<m_listenHttpsAddr.GetIpDisplayName());

    CAWResult rvhttp = IAWHttpManager::Instance()->CreateHttpAcceptor(
                  m_pServerAcceptor.ParaOut());

    rvhttp = m_pServerAcceptor->StartListen(this, m_listenHttpAddr);

    if (rvhttp==CAW_OK){
        CAW_INFO_TRACE("rvhttp=ok!!");
    }else{
        CAW_INFO_TRACE("rvhttp=fales!!");

    }

    CAWResult rvhttps = IAWHttpManager::Instance()->CreateHttpsAcceptor(
                  m_pServerHttpsAcceptor.ParaOut(),
                  "/opt/staros.xyz/starfaceserver/system/servercert.pem",
                  "/opt/staros.xyz/starfaceserver/system/serverkey.pem");

    rvhttps = m_pServerHttpsAcceptor->StartListen(this, m_listenHttpsAddr);
    if (rvhttps==CAW_OK){
        CAW_INFO_TRACE("rvhttps=ok!!");
    }else{
        CAW_INFO_TRACE("rvhttps=fales!!");

    }
    
    //CAW_INFO_TRACE(m_listenHttpAddr);
    
    return CAW_OK;
}


void CStarfaceServer::OnServerCreation(IAWChannelHttpServer *server)
{
    CAW_INFO_TRACE("CStarfaceServer::OnServerCreation. serverName =" << server);
    if(server == NULL)
    {
        CAW_INFO_TRACE("CStarfaceServer::OnServerCreation server is null");
        return;
    }
    CAW_ASSERTE(server);
    uint32_t id = m_idmsgr.AllocID();
    if(id == 0)
    {
        CAW_INFO_TRACE("CStarfaceServer::OnServerCreation m_idmsgr.AllocID=0");
        return;
    }
    CAWAutoPtr<CWebServerChannel> channel(new CWebServerChannel(id, server, this));
    if(channel.Get() == NULL)
    {
        CAW_INFO_TRACE("CStarfaceServer::OnServerCreation new out of memory");
        m_idmsgr.FreeID(id);
        return;
    }    
    server->OpenWithSink(channel.Get());
    CAWResult rv = m_channelmgr.AddChannel(id,channel);
    if(rv != CAW_OK)
    {
        CAW_INFO_TRACE("CStarfaceServer::OnServerCreation AddChannel error");
        m_idmsgr.FreeID(id);
        server->Disconnect(CAW_OK);
        return;
    }
}

void CStarfaceServer::ServerDestroy(uint32_t id)
{
    CAW_INFO_TRACE("CStarfaceServer::ServerDestroy. id =" << id);
  
    CWebServerChannel *pchannel= m_channelmgr.FindChannel(id);
    if(pchannel != NULL)
    {
        m_idmsgr.FreeID(id);
        m_channelmgr.RemoveChannel(id);
        m_clientmgr.RemoveClient(id);
    }
}

void CStarfaceServer::SetControllerInfo(const std::string &ip, const std::string &port, const std::string &name, const std::string &pwd)
{
    m_clientmgr.SetControllerInfo(ip,port,name,pwd);  
}


CSessionManager &CStarfaceServer::GetSessionMgr()
{
    return m_sessionmgr;
}

CWebServerChannelManager &CStarfaceServer::GetServerChannelMgr()
{
    return m_channelmgr;
}


CStarfaceWebClientManager &CStarfaceServer::GetClientMgr()
{
    return m_clientmgr;
}
CControllerManager &CStarfaceServer::GetControllerMgr(){
    return m_controllermgr;
}


