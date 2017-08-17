/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description: 
 * Others:
 * Version:          V1.0
 * Author:           Yi Jian <yijian@stros.xyz>
 * Date:             2017-02-02
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/


#ifndef CSTARFACEWEBCLIENTMANAGER_H
#define CSTARFACEWEBCLIENTMANAGER_H
#include "CAWACEWrapper.h"
#include "IStarGui.h"

class CStarfaceWebClient;
class CStarfaceServer;
class CController;
class CSession;
class CStarfaceWebClientManager  : public IRequestHandler
{
public:
    CStarfaceWebClientManager(CStarfaceServer *pserver);
    virtual ~CStarfaceWebClientManager();
    void Init();
    void OnRestfulResponse(uint32_t channelid,const CAWString &strjson);
    CAWResult RestfullRequest(uint32_t channelid,
                                const CAWString &strurl, 
                            const CAWString &strmethod, 
                            const CAWString &strjson,
                            CSession* psession = NULL);
    void RemoveClient(uint32_t channelid);
    void SetControllerInfo(const std::string &ip, const std::string &port, const std::string &name, const std::string &pwd);
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);

    CAWResult GetController(std::string &ip, std::string &port, std::string &name, std::string &pwd);
    
private:
    std::unordered_map<uint32_t , CAWAutoPtr<CStarfaceWebClient>> m_clients;
    std::string m_ip;
    std::string m_port;
    CStarfaceServer *m_pserver;
    CAWMutexThread m_mutex;
    CAWAutoPtr<IUiExtension> m_uiext;
    uint32_t m_nnumber;
    std::string m_selectip;
    std::string m_selectport;
    std::string m_controllerusername;
    std::string m_controllerpassword;
};


#endif//CSTARFACEWEBCLIENTMANAGER_H

