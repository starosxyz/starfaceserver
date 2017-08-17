/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     COpenflowServer.cpp
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

#include "COpenflowServer.h"
#include "CStarOpenflow.h"
#include "COpenflowTransport.h"

const char * CONTROLDBNAME="ControllerDB";
const char * CONTROLKEYNAME="ControllerKey";

struct strcontrollerkey
{
    char key[32];
};


COpenflowServer::COpenflowServer (CStarOpenflow *process)
    :m_process(process)
{
    CAW_INFO_TRACE("COpenflowServer::COpenflowServersssss");
    m_ids.Init(1,9000);
    std::string strjson;
    ReadDb(strjson);
    Json::Reader reader;
    Json::Value root;    
    if (!reader.parse(strjson, root)) 
        return;    
    Json::Value add_value = root["controlmgs"];
    m_jsonsize = add_value.size();
    
    for (int i = 0; i < add_value.size(); ++i)
    {
        Json::Value onejson = add_value[i];
        std::string controllerip = onejson["controllerip"].asString();
        std::string controllerport = onejson["controllerport"].asString();
        m_controlleraddr.push_back(controllerip);
        m_controlleraddr.push_back(controllerport);
    }
}
COpenflowServer::~COpenflowServer () 
{
    CAW_INFO_TRACE("COpenflowServer::~COpenflowServer");
    if (m_process)
    {
        m_process=NULL;
    }
    if (!m_transportlist.empty())
        RemoveTransport(m_id);
}

void COpenflowServer::OnConnectIndication(
        CAWResult aReason,
        IAWTransport *aTrpt,
        IAWAcceptorConnectorId *aRequestId) 
{
    CAW_INFO_TRACE("COpenflowServer::OnConnectIndicationssss");
    CAWAutoPtr<COpenflowTransport> ptransport(new COpenflowTransport(this, aTrpt, NULL));
    m_id = ptransport->GetId(); 
    //1,get controller list   
    if (GetControllerList(m_controlleraddr) != CAW_OK)
    {
        CAW_INFO_TRACE("controller list read error");
        return;
    }
            
    for (auto it = m_controlleraddr.begin(); it != m_controlleraddr.end(); ++it)
        std::cout << *it << std::endl;
    
    std::string controllerip = m_controlleraddr.front();
    m_controlleraddr.pop_front();
    std::string controllerport = m_controlleraddr.front();
    m_controlleraddr.pop_front();
    m_controlleraddr.push_back(controllerip);
    m_controlleraddr.push_back(controllerport);
    
    //int controllerport_temp = atoi(controllerport.c_str()); 
    CAWInetAddr addr (controllerip.c_str(), 6633);
    //2. connect congtroller
    ptransport->ConnectController(addr);
    m_transportlist[m_id]=ptransport;
}

void COpenflowServer::OnSSLConnected(IAWSSLTransport *aServer) 
{
    CAW_INFO_TRACE("COpenflowServer::OnSSLConnected");
    if (aServer == NULL)
    {
        CAW_ERROR_TRACE("COpenflowServer::OnSSLConnected aServer is NULL");
        return;
    }

    //1,get controller list   
    if (GetControllerList(m_controlleraddr) != CAW_OK)
    {
        CAW_INFO_TRACE("controller list read error");        
        return;
    }
            
    for (auto it = m_controlleraddr.begin(); it != m_controlleraddr.end(); ++it)
        std::cout << *it << std::endl;
    
    std::string controllerip = m_controlleraddr.front();
    m_controlleraddr.pop_front();
    std::string controllerport = m_controlleraddr.front();
    m_controlleraddr.pop_front();
    m_controlleraddr.push_back(controllerip);
    m_controlleraddr.push_back(controllerport);
    
    //int controllerport_temp = atoi(controllerport.c_str()); 
    CAWInetAddr addr (controllerip.c_str(), 6633);
    
    //2. connect congtroller
    CAWAutoPtr<COpenflowTransport> pssltransport(new COpenflowTransport(this, NULL, aServer));
    m_id = pssltransport->GetId();   
    pssltransport->ConnectController(addr);
    m_transportlist[m_id]=pssltransport;
    
}
void COpenflowServer::RemoveTransport(uint32_t id)
{
    std::unordered_map<uint32_t, CAWAutoPtr<COpenflowTransport>>::iterator it = m_transportlist.find(id);
    if (it != m_transportlist.end())
    {
        m_transportlist.erase(it);
    } 
}

void COpenflowServer::StarUI(){
    CAW_INFO_TRACE("COpenflowServer::StarUI");
    CAWString category("OPENFLOW配置");
    CAWString id("openflow");
    CAWString label("连接管理");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/openflow/openflow.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/openflow/openflow.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/openflow/openflow.html");

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
    m_uiext->AddRequestHandler(CAWString("OpenflowDataRequest"), CAWString("OpenflowDataResponse"), this);
}

CAWResult COpenflowServer::StarServer(const CAWString &ip, uint16_t port)
{
    StartTCPServer(ip, port);
}

CAWResult COpenflowServer::StartTCPServer(const CAWString &ip, uint16_t port)
{
    CAW_INFO_TRACE("COpenflowServer::StartTCPServer");
    CAWInetAddr addr(ip.c_str(),port);

    if (m_acceptor.Get())
    {
        m_acceptor->StopListen(CAW_OK);
        m_acceptor=NULL;
    }

    std::unordered_map<uint32_t, CAWAutoPtr<COpenflowTransport>>::iterator it = m_transportlist.begin();
    while(it != m_transportlist.end())
    {
        CAWAutoPtr<COpenflowTransport> &transport = it->second;
        transport->Disconnect();
        it++;         
    }
    m_transportlist.clear();
    
    CAWConnectionManager::Instance()->CreateConnectionServer(CAWConnectionManager::CTYPE_TCP, m_acceptor.ParaOut());
    if(m_acceptor.Get()==NULL)
    {
        return CAW_ERROR_FAILURE;
    }
    m_acceptor->StartListen(this, addr);
}

CAWResult COpenflowServer::StartSSLServer(const CAWString &ip, uint16_t port)
{
    CAW_INFO_TRACE("COpenflowServer::StartSSLServer");
    CAWInetAddr addr(ip.c_str(),port);

    if (m_sslacceptor.Get())
    {
        m_sslacceptor->StopListen(CAW_OK);
        m_sslacceptor=NULL;
    }

    std::unordered_map<uint32_t, CAWAutoPtr<COpenflowTransport>>::iterator it = m_transportlist.begin();
    while(it != m_transportlist.end())
    {
        CAWAutoPtr<COpenflowTransport> &transport = it->second;
        transport->Disconnect();
        it++;         
    }
    m_transportlist.clear();
    IAWSSLManager::Instance().CreateSSLAcceptor(m_sslacceptor);
    if(m_sslacceptor.Get()==NULL)
    {
        return CAW_ERROR_FAILURE;
    }
        m_sslacceptor->PrivateKey("/opt/staros.xyz/starcore/system/serverkey.pem");
    m_sslacceptor->Certificate("/opt/staros.xyz/starcore/system/servercert.pem");
    m_sslacceptor->StartListen(this, addr, CAWConnectionManager::CTYPE_TCP);
    return CAW_OK;

}

void COpenflowServer::process(long sid, Json::Value &payload, IUiExtension *pUi){
    CAW_INFO_TRACE("COpenflowServer::process");
    
    CAWString responseEventType("OpenflowDataResponse");
    
    static int reqid=0;
    Json::Value response;
    reqid++;
    Json::Value requestype = payload["requestype"];
    std::string strrequestype=requestype.asString();
    CAW_INFO_TRACE("CStarOpenflow::process strrequestype="<<strrequestype);
    
    if(strrequestype=="getInfo")
    {
        ResponseData(response);
    }
   
    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("COpenflowServer::process str_db="<<str_db); 
    pUi->SendMessage(responseEventType,reqid, response);
}

CAWResult COpenflowServer::ResponseData(Json::Value &response)
{
    CAW_INFO_TRACE("COpenflowServer::ResponseData"); 
    
    std::unordered_map<uint32_t, CAWAutoPtr<COpenflowTransport>>::iterator it = m_transportlist.begin();
    while(it != m_transportlist.end())
    {
        Json::Value info;
        CAWInetAddr controllerAddr;
        CAWInetAddr clientAddr;
        CAWAutoPtr<COpenflowTransport> &transport = it->second;
        transport->GetClientAndControlArr(controllerAddr,clientAddr);
        info["clientip"]=clientAddr.GetIpDisplayName().c_str();
        info["clientport"]=clientAddr.GetPort();
        info["controllerip"]=controllerAddr.GetIpDisplayName().c_str();
        info["controllerport"]=controllerAddr.GetPort();
        CAW_INFO_TRACE("COpenflowServer::ResponseData clientAddr.GetIpDisplayName().c_str()="<<clientAddr.GetIpDisplayName().c_str());
        response["openflows"].append(info);
        it++;         
    }

    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("COpenflowServer::ResponseData str_db="<<str_db); 
    return CAW_OK;
}


CAWResult COpenflowServer::ReadDb(std::string &strjson)
{
    CAW_INFO_TRACE("COpenflowServer::ReadDb");

    struct strcontrollerkey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),CONTROLKEYNAME);
    
    char *pvalue = NULL;
    size_t valuesize= 0;

    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(CONTROLDBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("COpenflowServer::ReadDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    
    int result=pdb->DBLookup((char *)&Key, sizeof(Key), pvalue, valuesize);
    if(result!=0)
    {
        CAW_ERROR_TRACE("COpenflowServer::ReadDb is not DBLookup"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    if (pvalue==NULL)
    {
        CAW_ERROR_TRACE("COpenflowServer::ReadDb can not find the db"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    if (valuesize == 0)
    {
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;   
    }

    CAW_INFO_TRACE("COpenflowServer::ReadDb,valuesize="<<valuesize);
    std::string strjsontest(pvalue, valuesize);
    strjson = strjsontest;
    CAW_INFO_TRACE("COpenflowServer::ReadDb strjson="<<strjson);
    return CAW_OK;
}

CAWResult COpenflowServer::GetControllerList(std::list<string>& tmp) 
{
    std::string strjson;
    ReadDb(strjson);
    Json::Reader reader;
    Json::Value root;
    
    if (!reader.parse(strjson, root)) 
        return CAW_ERROR_FAILURE;
    
    Json::Value add_value = root["controlmgs"];
    
    if (add_value.size() == 0)
    {
        m_transportlist[m_id]=NULL;
        return CAW_ERROR_FAILURE;
    }
    if (add_value.size() == 1)
    {
        tmp.clear();
        std::string controllerip;
        std::string controllerport;
        Json::Value onejson;
        Json::Value add_value = root["controlmgs"];
        onejson = add_value[0u];
        controllerip = onejson["controllerip"].asString();
        controllerport = onejson["controllerport"].asString();
        tmp.push_back(controllerip);
        tmp.push_back(controllerport);
        m_jsonsize = add_value.size();
    }
    // add controller
    if (add_value.size() > m_jsonsize) 
    {
        Json::Value add_value = root["controlmgs"];
        for (int i = m_jsonsize; i < add_value.size(); ++i)
        {
            Json::Value onejson = add_value[i];
            std::string controllerip = onejson["controllerip"].asString();
            std::string controllerport = onejson["controllerport"].asString();
            tmp.push_back(controllerip);
            tmp.push_back(controllerport);
        }
        m_jsonsize = add_value.size();
    }
    // remove controller
    if (add_value.size() < m_jsonsize && add_value.size() != 1)
    {
        tmp.clear();
        std::string controllerip;
        std::string controllerport;
        Json::Value onejson;
        Json::Value add_value = root["controlmgs"];
        for (int i = 2; i < add_value.size(); ++i)
        {
            onejson = add_value[i];
            controllerip = onejson["controllerip"].asString();
            controllerport = onejson["controllerport"].asString();
            tmp.push_back(controllerip);
            tmp.push_back(controllerport);
        }
        for (int i = 0; i <= 1; ++i)
        {
            onejson = add_value[i];
            controllerip = onejson["controllerip"].asString();
            controllerport = onejson["controllerport"].asString();
            tmp.push_back(controllerip);
            tmp.push_back(controllerport);
        }
        m_jsonsize = add_value.size();
    }
    if (tmp.empty())
        return CAW_ERROR_FAILURE;
    else
        return CAW_OK;
}

CAWIDAllocator& COpenflowServer::GetIds()
{
    return m_ids;
}


