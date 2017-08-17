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
#ifndef CWEBSERVERCHANNELMANAGER_H
#define CWEBSERVERCHANNELMANAGER_H
#include "CAWACEWrapper.h"
#include "IStarGui.h"
class CWebServerChannel;
class CStarfaceServer;
class CWebServerChannelManager : public IRequestHandler
{
public:
    CWebServerChannelManager(CStarfaceServer &server);
    virtual ~CWebServerChannelManager();
    void Init();
    CAWResult AddChannel(uint32_t id, CAWAutoPtr<CWebServerChannel> &channel);
    void RemoveChannel(uint32_t id);
    CWebServerChannel* FindChannel(uint32_t id);
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);
private:
    CAWMutexThread m_mutex;
    CStarfaceServer &m_server;
    std::unordered_map<uint32_t ,CAWAutoPtr<CWebServerChannel>> m_channels;
    CAWAutoPtr<IUiExtension> m_uiext;
};
#endif//CWEBSERVERCHANNELMANAGER_H
