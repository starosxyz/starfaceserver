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

#include "CStarfaceWebClientManager.h"
#include "CStarfaceWebClient.h"
#include "CStarfaceServer.h"
#include "CWebServerChannel.h"
#include "CController.h"
#include "CSession.h"
#define MAX_INT    (((unsigned int)(-1))>>1)

CStarfaceWebClientManager::CStarfaceWebClientManager(CStarfaceServer *pserver)
    :m_pserver(pserver)
    ,m_nnumber(1)
{
	m_ip="auto";
    m_port="0";
    m_selectip="auto";
    m_selectport="0";
    m_controllerusername="0";
    m_controllerpassword="0";
}

CStarfaceWebClientManager::~CStarfaceWebClientManager()
{
}

void CStarfaceWebClientManager::Init()
{
    CAW_INFO_TRACE("CStarfaceWebClientManager::Init");

    CAWString category("WEB服务器配置");

    CAWString id("starfaceserverclient");
    CAWString label("控制器会话管理");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/clientmgr/clientmgr.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/clientmgr/clientmgr.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/clientmgr/clientmgr.html");


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

    m_uiext->AddRequestHandler(CAWString("StarfaceClientDataRequest"), CAWString("StarfaceClientDataResponse"), this);

}

void CStarfaceWebClientManager::SetControllerInfo(const std::string &ip, const std::string &port, const std::string &name, const std::string &pwd)
{
	CAW_INFO_TRACE("CStarfaceWebClientManager::SetControllerInfo, ip="<<ip);
	CAW_INFO_TRACE("CStarfaceWebClientManager::SetControllerInfo, port="<<port);   
	m_ip = ip;
	m_port=port;
    m_controllerusername = name;
    m_controllerpassword = pwd;
}

CAWResult CStarfaceWebClientManager::GetController(std::string &ip, std::string &port, std::string &name, std::string &pwd)
{       
    std::vector<CController> controllerlist;
    CAWResult result=m_pserver->GetControllerMgr().GetControllerList(controllerlist);
    if (controllerlist.size() == 0)
    {
        return CAW_ERROR_FAILURE;
    }

    int index = m_nnumber % controllerlist.size() ;

    CAW_INFO_TRACE("CStarfaceWebClientManager::SetControllerInfo, number="<<m_nnumber<<",index="<<index);
    
    CController &controller = controllerlist[index];

    ip = controller.GetControllerip();
    port = controller.GetControllerport();
    name = controller.GetControllerusername();
    pwd = controller.GetControllerpassword();
    m_nnumber++;

    CAW_INFO_TRACE("CStarfaceWebClientManager::SetControllerInfo, number="<<m_nnumber<<",index="<<index
            <<",ip="<<ip<<",port="<<port);
    

    
    return result;
}


CAWResult CStarfaceWebClientManager::RestfullRequest(uint32_t channelid, const CAWString &strurl, 
                                                        const CAWString &strmethod, 
                                                        const CAWString &strjson,
                                                        CSession* psession)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    //m_ip = GetController()->GetControllerip();
    //m_port = GetController()->GetControllerport();
    CAWResult result = CAW_ERROR_FAILURE;
    if (m_ip.size()<strlen("auto"))
    {
        return result;
    }
	if (memcmp((char *)m_ip.c_str(), "auto", strlen("auto")) == 0)
    //if (m_ip == "auto")
    {
        CAW_INFO_TRACE("CStarfaceWebClientManager::RestfullRequest auto");
        if (psession == NULL)
        {
            CAW_INFO_TRACE("CStarfaceWebClientManager::RestfullRequest auto session=null");
            result=GetController(m_selectip,m_selectport,m_controllerusername,m_controllerpassword);
            if (result != CAW_OK)
            {
                return result;
            }
        }
        else
        {
            m_selectip = psession->GetIP();
            m_selectport = psession->GetPORT();
            m_controllerusername = psession->GetControllerusername();
            m_controllerpassword = psession->GetControllerpassword();
            //CAW_INFO_TRACE("CStarfaceWebClientManager::RestfullRequest YYYYYYYYYY, username="<<m_username<<",password="<<m_password);
        }
    }
    else
    {           
        m_selectip = m_ip;
        m_selectport = m_port;
        m_controllerusername = m_controllerusername;
        m_controllerpassword = m_controllerpassword;
    }

    if (m_selectip.size()==0)
    {
        CAW_ERROR_TRACE("CStarfaceWebClientManager::RestfullRequest controller ip 0");
        return CAW_ERROR_FAILURE;
    }

    if (m_selectport.size()==0)
    {
        CAW_ERROR_TRACE("CStarfaceWebClientManager::RestfullRequest controller port 0");
        return CAW_ERROR_FAILURE;
    }

    CAW_INFO_TRACE("CStarfaceWebClientManager::RestfullRequest ip="<<m_selectip<<",port="<<m_selectport);

    if (m_selectip.size() != 0 && m_selectport.size() != 0)
    {
        CAWAutoPtr<CStarfaceWebClient> pclient(new CStarfaceWebClient(m_pserver, m_selectip, m_selectport, channelid));
        if (pclient.Get() == NULL)
        {
            return CAW_ERROR_FAILURE;
        }
        m_clients[channelid]=pclient;
        CAWString str_username = m_controllerusername.c_str();
        CAWString str_password = m_controllerpassword.c_str();
        pclient->SetBasicAuthInfo(str_username, str_password);
        pclient->RestfullRequest(strurl,strmethod, strjson);
        CAW_INFO_TRACE("CStarfaceWebClientManager::RestfullRequest connect client");
        return CAW_OK;
    }
    return result;
}


void CStarfaceWebClientManager::OnRestfulResponse(uint32_t channelid,const CAWString &strjson)
{
    CWebServerChannel *pchannel = m_pserver->GetServerChannelMgr().FindChannel(channelid);
    if (pchannel)
    {
        pchannel->OnRestfulResponse(strjson, m_selectip, m_selectport, m_controllerusername, m_controllerpassword);
    }
}


void CStarfaceWebClientManager::RemoveClient(uint32_t channelid)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    m_clients.erase(channelid);
}

void CStarfaceWebClientManager::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAWMutexGuardT<CAWMutexThread> cs(m_mutex);

    if (pUi == NULL)
    {
        return;
    }

    CAW_INFO_TRACE("CStarfaceWebClientManager::process");
    CAWString responseEventType("StarfaceClientDataResponse");
    static int reqid=0;
    Json::Value response;
    Json::Value arrayObj;
    reqid++;
    
    std::unordered_map<uint32_t , CAWAutoPtr<CStarfaceWebClient>>::iterator it = m_clients.begin();
    while(it != m_clients.end())
    {
        CAWAutoPtr<CStarfaceWebClient> &pchannel=it->second;
        if (pchannel.Get())
        {
            Json::Value onenode;
            onenode["controllerip"]= Json::Value(pchannel->GetIP());
            onenode["controllerport"]= Json::Value(pchannel->GetPort());
            onenode["username"]= Json::Value(pchannel->GetUserName());
            onenode["password"]= Json::Value(pchannel->GetPassword());
            onenode["restfuljson"]= Json::Value(pchannel->GetRestfulJson());
            onenode["method"]= Json::Value(pchannel->GetMethod());

            arrayObj.append(onenode);
        }
        it++;
    } 
    
    response["clients"]= arrayObj;
    pUi->SendMessage(responseEventType,reqid, response);
}

