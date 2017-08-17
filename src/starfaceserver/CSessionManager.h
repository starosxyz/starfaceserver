/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description:     SessionManager.h
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
#ifndef _CSESSIONMANAGER_HPP_
#define _CSESSIONMANAGER_HPP_
#include <string>
#include "CAWACEWrapper.h"
#include "IStarGui.h"

class CSession;
class CStarfaceServer;

class CSessionManager : public IRequestHandler
{
public:
    CSessionManager(CStarfaceServer &server);
    virtual ~CSessionManager();
    
    void Init();
    void RemoveSession(const std::string &id);
    CSession *FindSession(const std::string &id);
    CAWResult AddSession(const std::string &id, CAWAutoPtr<CSession> &psession);
    std::string GenSessionId();
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);
private:
    CAWMutexThread m_mutex;
    CStarfaceServer &m_server;
    std::unordered_map<std::string ,CAWAutoPtr<CSession>> m_sessions;
    uint32_t m_sessionidbase;
    CAWAutoPtr<IUiExtension> m_uiext;

};
#endif  //_CSESSIONMANAGER_HPP_
