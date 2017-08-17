/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description: 
 * Others:
 * Version:          V1.0
 * Author:           hu peng
 * Date:             2017-04-04
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/

#include "CControllerManager.h"
#include "CStarfaceServer.h"


const char * CONTROLDBNAME="ControllerDB";
const char * CONTROLKEYNAME="ControllerKey";

struct strcontrollerkey
{
    char key[32];
};

struct strcontrollervalue
{
    uint32_t length;
    char strjson[0];
};

CControllerManager::CControllerManager(CStarfaceServer *pserver)
   :m_pserver(pserver)
   ,m_databaseid(0)
{
    CAW_INFO_TRACE("CControllermanager::CControllerManager");
}

CControllerManager::~CControllerManager()
{
    CAW_INFO_TRACE("CControllermanager::~CControllerManager");
}

void CControllerManager::Init()
{
    CAW_INFO_TRACE("CControllermanager::Init");
    CAWString category("WEB服务器配置");
    CAWString id("controllermgr");
    CAWString label("控制器管理");
    CAWString js("/opt/staros.xyz/starfaceserver/resources/app/view/controllermgr/controllermgr.js");
    CAWString css("/opt/staros.xyz/starfaceserver/resources/app/view/controllermgr/controllermgr.css");
    CAWString html("/opt/staros.xyz/starfaceserver/resources/app/view/controllermgr/controllermgr.html");

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

    m_uiext->AddRequestHandler(CAWString("ControllerDataRequest"), CAWString("ControllerDataResponse"), this);
}

void CControllerManager::process(long sid, Json::Value &payload, IUiExtension *pUi)
{
    CAW_INFO_TRACE("CControllermanager::process");
    CAWString responseEventType("ControllerDataResponse");
    static int reqid=0;
    Json::Value arrayjson;
    reqid++;
    Json::Value requestype = payload["requestype"];
    std::string strrequestype=requestype.asString();
    CAW_INFO_TRACE("CControllerManager::process strrequestype="<<strrequestype);
    if(strrequestype=="addInfo")
    {
       Json::Value controllerip = payload["controllerip"];
       Json::Value controllerport = payload["controllerport"];
       Json::Value controllerusername = payload["controllerusername"];
       Json::Value controllerpassword = payload["controllerpassword"];

       std::string strcontrollerip=controllerip.asString();
       std::string strcontrollerport=controllerport.asString();
       std::string strcontrollerusername=controllerusername.asString();
       std::string strcontrollerpassword=controllerpassword.asString();   

       
       CAW_INFO_TRACE("CControllerManager::process, controllerip="<<strcontrollerip
                  <<",controllerport="<<strcontrollerport
                  <<",controllerusername="<<strcontrollerusername
                  <<",controllerpassword="<<strcontrollerpassword); 

       Json::Value json_object_writer;
       json_object_writer["controllerid"]=to_string(m_databaseid++);
       json_object_writer["controllerip"]=strcontrollerip;
       json_object_writer["controllerport"]= strcontrollerport;
       json_object_writer["controllerusername"]=strcontrollerusername;
       json_object_writer["controllerpassword"]=strcontrollerpassword;
       CAW_INFO_TRACE("CControllerManager::process addInfo WriteDb");
       int writeresult=WriteDb(json_object_writer);
       if(writeresult==CAW_OK)
       {
            Json::Value json_object_reader;
            int readresult=ReadDb(json_object_reader);
            if(readresult==CAW_OK)
            {
                CAW_INFO_TRACE("CControllerManager::process getInfo SendMessage");
                pUi->SendMessage(responseEventType,reqid, json_object_reader);
            }
       }
    }
    if(strrequestype=="getInfo")
    {
        Json::Value json_object_reader;
        int readresult=ReadDb(json_object_reader);
        if(readresult==CAW_OK)
        {
            CAW_INFO_TRACE("CControllerManager::process getInfo SendMessage");
            pUi->SendMessage(responseEventType,reqid, json_object_reader);
        }
    }
    if(strrequestype=="deleteInfo")
    {
        Json::Value controllerid = payload["controllerid"];
        std::string strcontrollerid=controllerid.asString();
        CAW_INFO_TRACE("CControllerManager::process, deleteInfo controllerid="<<strcontrollerid);
        int deleteresult=DeleteDbByID(strcontrollerid);
        if(deleteresult==CAW_OK)
        {
            Json::Value json_object_reader;
            int readresult=ReadDb(json_object_reader);
            if(readresult==CAW_OK)
            {
                CAW_INFO_TRACE("CControllerManager::process getInfo SendMessage");
                pUi->SendMessage(responseEventType,reqid, json_object_reader);
            }
        }
    }
}

CAWResult CControllerManager::SaveDataToList()
{
    Json::Value json_object_reader;
    CAWResult readresult=ReadDb(json_object_reader);
    m_vec.clear();
    if(readresult==CAW_OK)
    {
        for(int i=0;i<json_object_reader["controlmgs"].size();i++)
        {
            Json::Value json_object=json_object_reader["controlmgs"][i];
            /*CController *pnew = new CController(
                json_object["controllerip"].asString()
                ,json_object["controllerport"].asString()
                ,json_object["controllerusername"].asString()
                ,json_object["controllerpassword"].asString()
                ,0);*/
            CController controller(json_object["controllerip"].asString()
                ,json_object["controllerport"].asString()
                ,json_object["controllerusername"].asString()
                ,json_object["controllerpassword"].asString());
            m_vec.push_back(controller);
        }
    }
    return readresult;
}

CAWResult CControllerManager::GetControllerList(std::vector<CController> & vec)
{
    CAWResult result=SaveDataToList();
    if(result==CAW_OK)
    {
        for(CController p:m_vec)
        {
            vec.push_back(p);
        }
    }
    return result;
}

CAWResult CControllerManager::ReadDb(Json::Value &json_object)
{
    CAW_INFO_TRACE("CControllerManager::ReadDb");

    struct strcontrollerkey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),CONTROLKEYNAME);
    
    char *pvalue = NULL;
    size_t valuesize= 0;

    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(CONTROLDBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    
    int result=pdb->DBLookup((char *)&Key, sizeof(Key), pvalue, valuesize);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb is not DBLookup"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    if (pvalue==NULL)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb can not find the db"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    if (valuesize == 0)
    {
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;   
    }

    CAW_INFO_TRACE("CControllerManager::ReadDb,valuesize="<<valuesize);
    std::string strjson(pvalue, valuesize);
    CAW_INFO_TRACE("CControllerManager::ReadDb strjson="<<strjson);

    Json::Reader reader; 
    if (!reader.parse(strjson, json_object))
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb reader parse is not corrected"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    return CAW_OK;
}

CAWResult CControllerManager::WriteDb(Json::Value json_object,bool isDeleteOther)
{
    CAW_INFO_TRACE("CControllerManager::WriteDb");

    struct strcontrollerkey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),CONTROLKEYNAME);
    
    Json::Value json_info;
    Json::Value json_db;
    int readResult=ReadDb(json_info);
    
    if(isDeleteOther)
    {
        if(json_object.isNull())
        {
            Json::Value arr_value(Json::arrayValue);
            json_db["controlmgs"]=arr_value;
        }
        else
        {
        json_db=json_object;
    }
    }
    else
    {
        if(readResult==CAW_ERROR_FAILURE||json_info.isNull())
        {
            json_db["controlmgs"].append(json_object);
        }
        else
        {
            json_info["controlmgs"].append(json_object);
            json_db=json_info;
        }
    }
    
    std::string str_db=json_db.toStyledString();

    int json_len=str_db.size();

    char *pcontrollervalue = new char[json_len];
    if(!pcontrollervalue)
    {
        CAW_ERROR_TRACE("CControllerManager::WriteDb pcontrollervalue is null");
        return CAW_ERROR_FAILURE;
    }
    ::memset(pcontrollervalue, 0, json_len);
    memcpy(pcontrollervalue, str_db.c_str(), str_db.size());

    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(CONTROLDBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CControllerManager::WriteDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    pdb->DBRemove((char *)&Key, sizeof(Key));
   
    int result=pdb->DBInsert((char *)&Key, sizeof(Key), pcontrollervalue, json_len);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CControllerManager::WriteDb is not DBInsert"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    delete[] pcontrollervalue;
    
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    return CAW_OK;
}

CAWResult CControllerManager::DeleteDbByID(std::string id)
{
    Json::Value json_object_reader;
    Json::Value json_object_write;
    int readresult=ReadDb(json_object_reader);
    if(readresult==CAW_ERROR_FAILURE)
    {
        CAW_ERROR_TRACE("CControllerManager::DeleteDbByID read is not sucsses"); 
        return CAW_ERROR_FAILURE;
    }
    for(int i=0; i<json_object_reader["controlmgs"].size(); i++)
    {
        std::string idstr=json_object_reader["controlmgs"][i]["controllerid"].asString();
        CAW_INFO_TRACE("CControllerManager::DeleteDbByID, controllerid="<<idstr
                  <<",id="<<id);
        if(idstr==id)
        {
            continue;
        }
        json_object_write["controlmgs"].append(json_object_reader["controlmgs"][i]);
    }
    CAW_ERROR_TRACE("CStarfaceProcess::DeleteDbByID read is sucsses");
    return WriteDb(json_object_write,true);
}

/*
CAWResult CControllerManager::ReadDb(Json::Value &json_object)
{
    CAW_INFO_TRACE("CControllerManager::ReadDb");

    struct strcontrollerkey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),CONTROLKEYNAME);
    
    char *pvalue = NULL;
    size_t valuesize= 0;

    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(CONTROLDBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    
    int result=pdb->DBLookup((char *)&Key, sizeof(Key), pvalue, valuesize);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb is not DBLookup"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    if (pvalue==NULL)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb can not find the db"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    
    strcontrollervalue * pcontrollervalue = (strcontrollervalue *)pvalue;
    if (valuesize != pcontrollervalue->length)
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb valuesize is not corrected, valuesize="<<valuesize
            <<",pcontrollervalue->length="<<pcontrollervalue->length); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;   
    }

    CAW_INFO_TRACE("CControllerManager::ReadDb,pwvalue->length="<<pcontrollervalue->length);
    CAW_INFO_TRACE("CControllerManager::ReadDb,valuesize="<<valuesize);
    

    //std::string strjjson(pcontrollervalue->strjson, pcontrollervalue->length-sizeof(strcontrollervalue));
    //CAW_INFO_TRACE("CControllerManager::ReadDb,pcontrollervalue->strjson="<<strjjson);
    //std::string strjjson(pvalue, valuesize);
    //CAW_INFO_TRACE("CControllerManager::ReadDb,pcontrollervalue->strjson="<<strjjson);

    Json::Reader reader;
    if (!reader.parse(pcontrollervalue->strjson, json_object))
    {
        CAW_ERROR_TRACE("CControllerManager::ReadDb reader parse is not corrected"); 
        return CAW_ERROR_FAILURE;
    }
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    return CAW_OK;
}

CAWResult CControllerManager::WriteDb(Json::Value json_object)
{
    CAW_INFO_TRACE("CStarfaceProcess::WriteDb");

    struct strcontrollerkey Key;
    ::memset(&Key, 0, sizeof(Key));
    BufferStringCopy(Key.key,sizeof(Key),CONTROLKEYNAME);
    
    Json::Value json_info;
    Json::Value json_db;
    int readResult=ReadDb(json_info);
    if(readResult==CAW_ERROR_FAILURE||json_info.isNull())
    {
        json_db["controlmgs"].append(json_object);
    }
    else
    {
        json_info["controlmgs"].append(json_object);
        json_db=json_info;
    }
    std::string str_db=json_db.toStyledString();
    CAW_ERROR_TRACE("CStarfaceProcess::WriteDb json_db.toStyledString()="<<str_db);

    int json_len=sizeof(strcontrollervalue)+str_db.size();

    strcontrollervalue *pcontrollervalue= (strcontrollervalue *)malloc(json_len);
    if(!pcontrollervalue)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::WriteDb pcontrollervalue is null");
        return CAW_ERROR_FAILURE;
    }

    ::memset(pcontrollervalue, 0, json_len);
    memcpy(pcontrollervalue->strjson, str_db.c_str(), str_db.size());
    pcontrollervalue->length=json_len;

    std::string strjson((char *)pcontrollervalue, json_len);
    CAW_INFO_TRACE("CStarfaceProcess::WriteDb pcontrollervalue="<<strjson);

    ISYSDB *pdb = ISYSDBManager::Instance()->CreateSYSDB(CONTROLDBNAME);
    if(pdb==NULL)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::WriteDb pdb is nullptr");
        return CAW_ERROR_FAILURE;
    }
    pdb->DBRemove((char *)&Key, sizeof(Key));
   
    int result=pdb->DBInsert((char *)&Key, sizeof(Key), (char *)pcontrollervalue, json_len);
    if(result!=0)
    {
        CAW_ERROR_TRACE("CStarfaceProcess::WriteDb is not DBInsert"); 
        ISYSDBManager::Instance()->DestroySYSDB(pdb);
        return CAW_ERROR_FAILURE;
    }
    free(pcontrollervalue);
    
    ISYSDBManager::Instance()->DestroySYSDB(pdb);
    Json::Value json_ijson;
    ReadDb(json_ijson);
    return CAW_OK;
}
*/
