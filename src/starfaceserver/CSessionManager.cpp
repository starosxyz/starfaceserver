/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description:     SessionManager.cpp
 * Others:
 * Version:          V1.0
 * Author:           Zhang XinXiang
 * Date:             2017-03-24
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/
#include "CSessionManager.h"
#include "CSession.h"
#include "CStarfaceServer.h"

CSessionManager::CSessionManager(CStarfaceServer &server)
    :m_server(server)
    ,m_sessionidbase(0)
{
    CAW_INFO_TRACE("CSessionManager::CSessionManager");
}

CSessionManager::~CSessionManager()
{
    CAW_INFO_TRACE("CSessionManager::~CSessionManager");
}

void CSessionManager::Init()
{
    CAW_INFO_TRACE("CSessionManager::Init");

    CAWString category("WEB服务器配置");
    CAWString id("starfaceserversession");
    CAWString label("session管理");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/sessionmgr/sessionmgr.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/sessionmgr/sessionmgr.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/sessionmgr/sessionmgr.html");


    CAWResult rv = IStarGui::Instance().Register(category,
                                id,
                                label,
                                js,
                                css,
                                html,
                                m_uiext.ParaOut());

    if (rv == CAW_OK)
    {
        CAW_INFO_TRACE("Register ui ok\n");
    }
    else
    {
        CAW_ERROR_TRACE("register ui failure\n");
    }

    m_uiext->AddRequestHandler(CAWString("StarfaceSessionDataRequest"), CAWString("StarfaceSessionDataResponse"), this);

}

void CSessionManager::RemoveSession(const std::string &id)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    CAW_INFO_TRACE("CSessionManager::RemoveSession id =" << id);
    m_sessions.erase(id);
}
CSession *CSessionManager::FindSession(const std::string &id)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    CAW_INFO_TRACE("CSessionManager::FindSession id =" << id);
    std::unordered_map<std::string ,CAWAutoPtr<CSession>>::iterator it = m_sessions.find(id);
    if (it == m_sessions.end())
    {
        return NULL;
    }
    CAWAutoPtr<CSession> &psession = it->second;
    return psession.Get();
}

CAWResult CSessionManager::AddSession(const std::string &id, CAWAutoPtr<CSession> &psession)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    CAW_INFO_TRACE("CSessionManager::AddSession id =" << id);
    std::unordered_map<std::string ,CAWAutoPtr<CSession>>::iterator it = m_sessions.find(id);
    if (it != m_sessions.end())
    {
        return CAW_ERROR_FAILURE;
    }
    m_sessions[id]=psession;
    return CAW_OK;
}

std::string CSessionManager::GenSessionId()
{
    uint32_t id = m_sessionidbase++;
    char buffer[1024]={0};
    snprintf(buffer,sizeof(buffer),"%d", id);
    return std::string("jsessionid=")+std::string(buffer);
}

void CSessionManager::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    CAW_INFO_TRACE("CSessionManager::process");
    CAWString responseEventType("StarfaceSessionDataResponse");
    static int reqid=0;
    Json::Value response;
    Json::Value arrayObj;
    reqid++;
    
    std::unordered_map<std::string ,CAWAutoPtr<CSession>>::iterator it = m_sessions.begin();
    while(it != m_sessions.end())
    {
        CAWAutoPtr<CSession> &psession=it->second;
        if (psession.Get())
        {
            Json::Value onenode;
            onenode["sessionid"]= Json::Value(psession->GetId());
            onenode["clientip"]= Json::Value(psession->GetId());
            onenode["expire"]= Json::Value(psession->GetExpire());
            onenode["username"]= Json::Value(psession->GetUserName());
        
            arrayObj.append(onenode);
        }
        it++;
    } 
    
    response["sessions"]= arrayObj;
    pUi->SendMessage(responseEventType,reqid, response);

}


