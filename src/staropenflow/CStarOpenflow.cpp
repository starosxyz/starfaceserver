/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     CStarOpenflow.cpp
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

#include "CStarOpenflow.h"
#include "COpenflowServer.h"
#include "CAWByteStream.h"
#include "json_features.h"
#include "json_value.h"
#include "json_reader.h"
#include <fstream>

const char * DBNAME="OpenflowDB";
const char * KEYNAME="OpenflowKey";

struct DBKey
{
    char key[32];
};

struct DBValue
{
    char openflowip[32];
    char openflowport[32];
    char openflowtype[32];
};

CStarOpenflow::CStarOpenflow()
    :m_pdipcProcess(NULL)
    ,m_openflowserver(this)
    ,m_stropenflowip("0.0.0.0")
    ,m_stropenflowport("8899")
    ,m_stropenflowtype("TCP")
{
    CAW_INFO_TRACE("CStarOpenflow::CStarOpenflow");
    ISYSDBManager::Instance()->Open("/opt/staros.xyz/starfaceserver/system");
    if (ReadDb() != CAW_OK)
    {
        CAW_INFO_TRACE("CStarOpenflow::WriteDb");
        WriteDb();
    }
}

CStarOpenflow::~CStarOpenflow()
{
    CAW_INFO_TRACE("CStarOpenflow::~CStarOpenflow");
    ISYSDBManager::Instance()->Close();
}

void CStarOpenflow::OnBootOK()
{
    CAW_INFO_TRACE("CStarOpenflow::OnBootOK");
}
void CStarOpenflow::OnProcessRun(int argc, char** argv, IDIPCProcess *dipcProcess)
{
    CAW_INFO_TRACE("CStarOpenflow::OnProcessRun"); 
    CAWString category("OPENFLOW配置");
    CAWString id("openflowmgr");
    CAWString label("全局配置");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/openflowmgr/openflowmgr.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/openflowmgr/openflowmgr.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/openflowmgr/openflowmgr.html");

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
    m_uiext->AddRequestHandler(CAWString("OpenflowMgrDataRequest"), CAWString("OpenflowMgrDataResponse"), this);
    m_openflowserver.StarUI();
    
    m_pdipcProcess = dipcProcess;
    m_openflowserver.StarServer(m_stropenflowip.c_str(),atoi(m_stropenflowport.c_str()));
    m_pdipcProcess->ProcessRunFinishNotify();
}
void CStarOpenflow::OnHAProcessConnected(uint32_t clusterid,
                                        uint32_t datacenterid,
                                        CAWAutoPtr<IDIPCTransport> &transport)
{
    CAW_INFO_TRACE("CStarOpenflow::OnHAProcessConnected");
}

void CStarOpenflow::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAW_INFO_TRACE("CStarOpenflow::process");
    
    if (m_pdipcProcess == NULL)
    {
        return;
    }
    CAWString responseEventType("OpenflowMgrDataResponse");
    
    static int reqid=0;
    Json::Value response;
    reqid++;
    Json::Value requestype = payload["requestype"];
    std::string strrequestype=requestype.asString();
    CAW_INFO_TRACE("CStarOpenflow::process strrequestype="<<strrequestype);
    
    if(strrequestype=="getconfig")
    {
        ResponseData(response);
    }
    if(strrequestype=="setconfig")
    {
        Json::Value openflowip = payload["openflowip"];
        Json::Value openflowport = payload["openflowport"];
        Json::Value openflowtype = payload["openflowtype"];

        m_stropenflowip=openflowip.asString();
        m_stropenflowport=openflowport.asString();
        m_stropenflowtype=openflowtype.asString();
        
        if (m_stropenflowip.size()>0&&m_stropenflowport.size()>0&&m_stropenflowtype.size()>0)
        {
            WriteDb();
            response["result"]="set config success";
            m_openflowserver.StarServer(m_stropenflowip.c_str(),atoi(m_stropenflowport.c_str()));
        }else{
            response["result"]="set config failure";   
        }
        ResponseData(response);
    }
    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("CStarfaceProcess::process str_db="<<str_db); 
    pUi->SendMessage(responseEventType,reqid, response);
}

CAWResult CStarOpenflow::ResponseData(Json::Value &response)
{
    CAW_INFO_TRACE("CStarOpenflow::ResponseData"); 
    
    ReadDb();
    
    response["openflowip"]=m_stropenflowip;
    response["openflowport"]=m_stropenflowport;
    response["openflowtype"]=m_stropenflowtype;
    
    std::string str_db=response.toStyledString();
    CAW_INFO_TRACE("CStarOpenflow::ResponseData str_db="<<str_db); 
    return CAW_OK;
}

CAWResult CStarOpenflow::ReadDb()
{
    CAW_INFO_TRACE("CStarOpenflow::ReadDb");
    struct DBKey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),KEYNAME);
    
    char *pvalue = NULL;
    size_t valuesize= 0;
    
    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(DBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CStarOpenflow::ReadDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    int result=pdb->DBLookup((char *)&Key, sizeof(Key), pvalue, valuesize);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CStarOpenflow::ReadDb is not DBLookup"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    if (pvalue==NULL)
    {
        CAW_ERROR_TRACE("CStarOpenflow::ReadDb can not find the db"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    if (sizeof(DBValue) != valuesize)
    {
        CAW_ERROR_TRACE("CStarOpenflow::ReadDb valuesize is not corrected"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    DBValue *pdbvalue = (DBValue*)pvalue;
    m_stropenflowip=std::string(pdbvalue->openflowip, sizeof(pdbvalue->openflowip));
    m_stropenflowport=std::string(pdbvalue->openflowport, sizeof(pdbvalue->openflowport));
    m_stropenflowtype=std::string(pdbvalue->openflowtype, sizeof(pdbvalue->openflowtype));

    CAW_INFO_TRACE("CStarOpenflow::ReadDb, m_stropenflowip="<<m_stropenflowip
        <<",m_stropenflowport="<<m_stropenflowport
        <<",m_stropenflowtype="<<m_stropenflowtype); 

    ISYSDBManager::Instance()->DestroySYSDB(pdb);

    return CAW_OK;
}

CAWResult CStarOpenflow::WriteDb()
{
    CAW_INFO_TRACE("CStarOpenflow::WriteDb, m_stropenflowip="<<m_stropenflowip
        <<",m_stropenflowport="<<m_stropenflowport
        <<",m_stropenflowtype="<<m_stropenflowtype);
    
    struct DBKey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),KEYNAME);
    
    struct DBValue value;
    ::memset(&value, 0, sizeof(value));
    BufferStringCopy(value.openflowip, sizeof(value.openflowip), m_stropenflowip.c_str());
    BufferStringCopy(value.openflowport, sizeof(value.openflowport), m_stropenflowport.c_str());
    BufferStringCopy(value.openflowtype, sizeof(value.openflowtype), m_stropenflowtype.c_str());
   
    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(DBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CStarOpenflow::WriteDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    pdb->DBRemove((char *)&Key, sizeof(Key));
    int result=pdb->DBInsert((char *)&Key, sizeof(Key), (char *)&value, sizeof(value));
    if(result!=0)
    {
        CAW_ERROR_TRACE("CStarOpenflow::WriteDb is not DBInsert"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    
    return CAW_OK;
}

