/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description:      CStarfaceProcess.cpp
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
#include "CStarfaceProcess.h"
#include "CAWByteStream.h"
#include "json_features.h"
#include "json_value.h"
#include "json_reader.h"
#include "CController.h"
#include "CBackupTransportSink.h"




const char * DBNAME="StarfaceDB";
const char * KEYNAME="StarfaceKey";

struct DBKey
{
    char key[32];
};

struct DBValue
{
    char webip[32];
    char webhttpport[32];
    char webhttpsport[32];
    char controllerip[32];
};

CStarfaceProcess::CStarfaceProcess()
    :m_dipcProcess(NULL) 
    ,m_strwebip("0.0.0.0")
    ,m_strwebhttpport("8484")
    ,m_strwebhttpsport("8485")
    ,m_strcontrolerip("auto")
    ,m_strcontrolerport("0")
{
    CAW_INFO_TRACE("CStarfaceProcess::CStarfaceProcess");
    ISYSDBManager::Instance()->Open("/opt/staros.xyz/starfaceserver/system");
    if (ReadDb() != CAW_OK)
    {
        CAW_INFO_TRACE("CStarfaceProcess::WriteDb");
        WriteDb();
    }
}

CStarfaceProcess::~CStarfaceProcess()
{
    CAW_INFO_TRACE("CStarfaceProcess::~CStarfaceProcess");
    ISYSDBManager::Instance()->Close();
}

void CStarfaceProcess::OnBootOK()
{
    CAW_INFO_TRACE("CStarfaceProcess::OnBootOK");
}

void CStarfaceProcess::OnProcessRun(int argc, char** argv, IDIPCProcess *dipcProcess)
{
    CAW_INFO_TRACE("CStarfaceProcess::OnProcessRun"); 
    m_dipcProcess=dipcProcess;
    CAWString category("WEB服务器配置");
    CAWString id("starfaceserver");
    CAWString label("全局配置");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/starfaceserver/starfaceserver.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/starfaceserver/starfaceserver.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/starfaceserver/starfaceserver.html");

    CAWInetAddr listenHttpAddr(m_strwebip.c_str(), atoi(m_strwebhttpport.c_str()));
    CAWInetAddr listenHttpsAddr(m_strwebip.c_str(), atoi(m_strwebhttpsport.c_str()));

    m_server.Start(listenHttpAddr,listenHttpsAddr);
    m_server.InitUI();
    
    getPortFromList(m_strcontrolerport);
    m_server.SetControllerInfo(m_strcontrolerip,m_strcontrolerport,m_controllerusername,m_controllerpassword);


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
    m_uiext->AddRequestHandler(CAWString("WebConfigDataRequest"), CAWString("WebConfigDataResponse"), this);
    m_dipcProcess->ProcessRunFinishNotify();
}

void CStarfaceProcess::OnHAProcessConnected(uint32_t clusterid,
                                        uint32_t datacenterid,
                                        CAWAutoPtr<IDIPCTransport> &transport)
{
    CAW_INFO_TRACE("CStarfaceProcess::OnHAProcessConnected");
    CBackupTransportSink* psink = new CBackupTransportSink(transport,this);
    transport->OpenWithSink(psink);
    Json::Value response;
    CStarfaceProcess::ResponseData(response);
    std::string text=response.toStyledString();
    CAW_INFO_TRACE("CStarfaceProcess::OnHAProcessConnected text="<<text); 
    CAWMessageBlock MsgBlk( 
            text.size(), 
            text.c_str(), 
            CAWMessageBlock::DONT_DELETE,
            text.size() ); 
    transport->SendData(MsgBlk,NULL);
}

void CStarfaceProcess::writefile(){
#if 0
    fstream cerfile;
    fstream keyfile;
    if(!cerfile){
        ofstream cerfile;
        cerfile.open("/opt/staros.xyz/starfaceserver/system/servercert.pem",ios::trunc|ios::out);
        cerfile<<m_cerfile;
        cerfile.close();
    }else{
        unlink("/opt/staros.xyz/starfaceserver/system/servercert.pem");
        ofstream cerfile;
        cerfile.open("/opt/staros.xyz/starfaceserver/system/servercert.pem");
        cerfile<<m_cerfile;
        cerfile.close();
    }
    if(!keyfile){
        ofstream keyfile;
        keyfile.open("/opt/staros.xyz/starfaceserver/system/serverkey.pem");
        keyfile<<m_keyfile;
        keyfile.close();
    }else{
        unlink("/opt/staros.xyz/starfaceserver/system/serverkey.pem");
        ofstream keyfile;
        keyfile.open("/opt/staros.xyz/starfaceserver/system/serverkey.pem");
        keyfile<<m_keyfile;
        keyfile.close();
    }
#endif
    CFileObject cerfile;
    CAWMessageBlock mbn(m_cerfile.size(),m_cerfile.c_str(),0,m_cerfile.size());
    cerfile.WriteFile("/opt/staros.xyz/starfaceserver/system/servercert.pem",mbn);

    CFileObject keyfile;
    CAWMessageBlock mbn2(m_keyfile.size(),m_keyfile.c_str(),0,m_keyfile.size());
    keyfile.WriteFile("/opt/staros.xyz/starfaceserver/system/serverkey.pem",mbn2);


    
}

bool CStarfaceProcess::IsKeyValite(const std::string& m_keyfile)
{
    
    std::string strheader="-----BEGIN RSA PRIVATE KEY-----";
    if (m_keyfile.size()<=strheader.size())
    {
        return false;
    }

    if (memcmp(strheader.c_str(), m_keyfile.c_str(), strlen(strheader.c_str()))==0)
    {
        return true;
    }

    return true;
}
bool CStarfaceProcess::IsCertValite(const std::string& m_cerfile)
{

    std::string strheader2="-----BEGIN CERTIFICATE-----";

    if (m_cerfile.size()<=strheader2.size())
    {
        return false;
    }

    if (memcmp(strheader2.c_str(), m_cerfile.c_str(), strlen(strheader2.c_str()))==0)
    {
        return true;
}

    return true;
}





void CStarfaceProcess::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAW_INFO_TRACE("CStarfaceProcess::process");
    if (m_dipcProcess == NULL)
    {
        return;
    }
    CAWString responseEventType("WebConfigDataResponse");
    static int reqid=0;
    Json::Value response;
    reqid++;
    Json::Value requestype = payload["requestype"];
    std::string strrequestype=requestype.asString();
    CAW_INFO_TRACE("CStarfaceProcess::process strrequestype="<<strrequestype);
    if(strrequestype=="getconfig")
    {
        ResponseData(response);
    }
    if(strrequestype=="setconfig")
    {
        Json::Value webip = payload["webip"];
        Json::Value webhttpport = payload["webhttpport"];
        Json::Value webhttpsport = payload["webhttpsport"];
        Json::Value controlerip = payload["controlerip"];
        Json::Value cerfile = payload["cerfile"];
        Json::Value keyfile = payload["keyfile"];


        m_strwebip=webip.asString();
        m_strwebhttpport=webhttpport.asString();
        m_strwebhttpsport=webhttpsport.asString();
        m_strcontrolerip=controlerip.asString();
        //CAW_INFO_TRACE("CStarfaceProcess::process!!!!! m_strcontrolerip="<<m_strcontrolerip);
        m_cerfile=cerfile.asString();
        CAW_INFO_TRACE("CStarfaceProcess::process m_cerfile="<<m_cerfile);
        m_keyfile=keyfile.asString();
        CAW_INFO_TRACE("CStarfaceProcess::process m_keyfile="<<m_keyfile);
        if ((m_cerfile.size()!=0) && (m_keyfile.size()!=0))
        {
            if (IsCertValite(m_cerfile) == false)
            {
                response["result"]="set cerfile Access Validation Error";
            }
            else 
            {
                if (IsKeyValite(m_keyfile) == false)
                {
                    response["result"]="set keyfile Access Validation Error";
                }
                else
                {
                    writefile();
               }
           }
            
        }

        if (m_strwebip.size()>0&&m_strwebhttpport.size()>0&&m_strwebhttpsport.size()>0&&m_strcontrolerip.size()>0)
        {
            if(memcmp(m_strwebhttpport.c_str(), m_strwebhttpsport.c_str(), strlen(m_strwebhttpport.c_str()))==0){

                 response["result"]="set config failure"; 
            }else{
          
            WriteDb();
            response["result"]="set config success";

        CAWInetAddr listenHttpAddr(m_strwebip.c_str(), atoi(m_strwebhttpport.c_str()));
        CAWInetAddr listenHttpsAddr(m_strwebip.c_str(), atoi(m_strwebhttpsport.c_str()));
        m_server.Start(listenHttpAddr,listenHttpsAddr);

        getPortFromList(m_strcontrolerport);
        m_server.SetControllerInfo(m_strcontrolerip,m_strcontrolerport,m_controllerusername,m_controllerpassword);

            //m_dipcProcess->RestartProcess("STARFACESERVER");
            }
        }else{
            response["result"]="set config failure";   
        }

        ResponseData(response);

        
        CAW_INFO_TRACE("CStarfaceProcess::process, m_strwebip="<<m_strwebip
            <<",m_strwebhttpport="<<m_strwebhttpport
            <<",m_strwebhttpsport="<<m_strwebhttpsport
            <<",m_strcontrolerip="<<m_strcontrolerip); 
        
    }
    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("CStarfaceProcess::process str_db="<<str_db); 
    pUi->SendMessage(responseEventType,reqid, response);
    
}

CAWResult CStarfaceProcess::ResponseData(Json::Value &response)
{
    CAW_INFO_TRACE("CStarfaceProcess::ResponseData"); 
    
    ReadDb();
    
    response["webip"]=m_strwebip;
    response["webhttpport"]=m_strwebhttpport;
    response["webhttpsport"]=m_strwebhttpsport;

    /*getPortFromList(m_strcontrolerport);
    response["controlerport"]=m_strcontrolerport;*/

    Json::Value controlerips;
    controlerips.append(m_strcontrolerip);

    std :: vector < CController > controllerlist;
    m_server.GetControllerMgr().GetControllerList(controllerlist);
    for(CController pcontroller : controllerlist)
    {

        /*CAW_INFO_TRACE("CStarfaceProcess::ResponseData, pcontroller->GetControllerip()="<<pcontroller->GetControllerip()
            <<",m_strcontrolerip="<<m_strcontrolerip); 
        int Controllerip_len=pcontroller->GetControllerip().size();
        char * p_Controllerip=new char[Controllerip_len];
        strcmp(p_Controllerip,pcontroller->GetControllerip().c_str());

        int mControllerip_len=m_strcontrolerip.size();
        char * p_mControllerip=new char[mControllerip_len];
        strcmp(p_mControllerip,m_strcontrolerip.c_str());
        if (!(memcmp(p_Controllerip,p_mControllerip,strlen(p_mControllerip))==0))
        {
            CAW_INFO_TRACE("CStarfaceProcess::ResponseData ---------------------------------"); 
            controlerips.append(pcontroller->GetControllerip());
        }   
        
        delete[] p_Controllerip;
        delete[] p_mControllerip;*/
        controlerips.append(pcontroller.GetControllerip());
    }
    response["controlerips"]=controlerips;
    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("CStarfaceProcess::ResponseData str_db="<<str_db); 
    return CAW_OK;
}

void CStarfaceProcess::getPortFromList(std::string      & strcontrolerport)
{
    CAW_INFO_TRACE("CStarfaceProcess::getPortFromList"); 
    
    std :: vector < CController> controllerlist;
    m_server.GetControllerMgr().GetControllerList(controllerlist);
    for(CController pcontroller : controllerlist)
    {
        if (pcontroller.GetControllerip() == m_strcontrolerip)
        {
            strcontrolerport=pcontroller.GetControllerport();
        }
}
}


CAWResult CStarfaceProcess::ReadDb()
{
    CAW_INFO_TRACE("CStarfaceProcess::ReadDb");
    struct DBKey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),KEYNAME);
    
    char *pvalue = NULL;
    size_t valuesize= 0;
    
    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(DBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::ReadDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    int result=pdb->DBLookup((char *)&Key, sizeof(Key), pvalue, valuesize);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::ReadDb is not DBLookup"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    if (pvalue==NULL)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::ReadDb can not find the db"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    if (sizeof(DBValue) != valuesize)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::ReadDb valuesize is not corrected"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    DBValue *pdbvalue = (DBValue*)pvalue;
    m_strwebip=std::string(pdbvalue->webip, sizeof(pdbvalue->webip));
    m_strwebhttpport=std::string(pdbvalue->webhttpport, sizeof(pdbvalue->webhttpport));
    m_strwebhttpsport=std::string(pdbvalue->webhttpsport, sizeof(pdbvalue->webhttpsport));
    m_strcontrolerip=std::string(pdbvalue->controllerip, sizeof(pdbvalue->controllerip));

    CAW_INFO_TRACE("CStarfaceProcess::ReadDb, m_strwebip="<<m_strwebip
        <<",m_strwebhttpport="<<m_strwebhttpport
        <<",m_strwebhttpsport="<<m_strwebhttpsport
        <<",m_strcontrolerip="<<m_strcontrolerip); 

    
    ISYSDBManager::Instance()->DestroySYSDB(pdb);

    return CAW_OK;
}

CAWResult CStarfaceProcess::WriteDb()
{
    CAW_INFO_TRACE("CStarfaceProcess::WriteDb, m_strwebip="<<m_strwebip
        <<",m_strwebhttpport="<<m_strwebhttpport
        <<",m_strwebhttpsport="<<m_strwebhttpsport
        <<",m_strcontrolerip="<<m_strcontrolerip); 
    
    struct DBKey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),KEYNAME);
    
    struct DBValue value;
    ::memset(&value, 0, sizeof(value));
    BufferStringCopy(value.webip, sizeof(value.webip), m_strwebip.c_str());
    BufferStringCopy(value.webhttpport, sizeof(value.webhttpport), m_strwebhttpport.c_str());
    BufferStringCopy(value.webhttpsport, sizeof(value.webhttpsport), m_strwebhttpsport.c_str());
    BufferStringCopy(value.controllerip, sizeof(value.controllerip), m_strcontrolerip.c_str());
    
    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(DBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::WriteDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    pdb->DBRemove((char *)&Key, sizeof(Key));
    int result=pdb->DBInsert((char *)&Key, sizeof(Key), (char *)&value, sizeof(value));
    if(result!=0)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::WriteDb is not DBInsert"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    
    return CAW_OK;
}
CStarfaceServer& CStarfaceProcess::GetStarfaceServer()
{
    return m_server;
}
