/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:      CWebServerChannelManager
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

#include "CWebServerChannelManager.h"
#include "CStarfaceServer.h"
#include "CWebServerChannel.h"
CWebServerChannelManager::CWebServerChannelManager(CStarfaceServer &server)
    :m_server(server)
{
    CAW_INFO_TRACE("CWebServerChannelManager::CWebServerChannelManager");
}
CWebServerChannelManager::~CWebServerChannelManager()
{
    CAW_INFO_TRACE("CWebServerChannelManager::~CWebServerChannelManager");
}

void CWebServerChannelManager::Init()
{
    CAW_INFO_TRACE("CWebServerChannelManager::Init");

    CAWString category("WEB服务器配置");

    CAWString id("starfaceserverchannel");
    CAWString label("客户端管理");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/channelmgr/channelmgr.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/channelmgr/channelmgr.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/channelmgr/channelmgr.html");


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

    m_uiext->AddRequestHandler(CAWString("StarfaceChannelDataRequest"), CAWString("StarfaceChannelDataResponse"), this);

}

CAWResult CWebServerChannelManager::AddChannel(uint32_t id, CAWAutoPtr<CWebServerChannel> &channel)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);
    std::unordered_map<uint32_t ,CAWAutoPtr<CWebServerChannel>>::iterator it = m_channels.find(id);
    if (it != m_channels.end())
    {
        return CAW_ERROR_FAILURE;
    }
    m_channels[id] = channel;
    return CAW_OK;
}
void CWebServerChannelManager::RemoveChannel(uint32_t id)
{   
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);
    m_channels.erase(id);
}
CWebServerChannel* CWebServerChannelManager::FindChannel(uint32_t id)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);
    std::unordered_map<uint32_t ,CAWAutoPtr<CWebServerChannel>>::iterator it = m_channels.find(id);
    if (it == m_channels.end())
    {
        return NULL;
    }
    CAWAutoPtr<CWebServerChannel> &channel = it->second;
    return channel.Get();
}

void CWebServerChannelManager::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    CAW_INFO_TRACE("CWebServerChannelManager::process");
    CAWString responseEventType("StarfaceChannelDataResponse");
    static int reqid=0;
    Json::Value response;
    Json::Value arrayObj;
    reqid++;
    
    std::unordered_map<uint32_t ,CAWAutoPtr<CWebServerChannel>>::iterator it = m_channels.begin();
    while(it != m_channels.end())
    {
        CAWAutoPtr<CWebServerChannel> &pchannel = it->second;
        if (pchannel.Get())
        {
            Json::Value onenode;
            onenode["channelid"]= Json::Value(pchannel->GetChannelId());
            onenode["clientaddr"]= Json::Value(pchannel->GetChannelId());
            arrayObj.append(onenode);
        }
        it++;
    } 
    
    response["channels"]= arrayObj;
    pUi->SendMessage(responseEventType,reqid, response);

}


