/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     CWebServerChannel.cpp
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
#include "CWebServerChannel.h"
#include "CStarfaceServer.h"
#include "json_features.h"
#include "json_value.h"
#include "json_reader.h"
#include "CStarfaceWebClient.h"
#include "CStarfaceServer.h"
#include "CSession.h"
#include "WFFile.h"
#include "CAWUtils.h"

std::string urlprex("/starface/ui");
std::string notfoundreject("<!-- {INJECTED-USER-NOT-FOUND-URL} -->");
std::string jstype("text/javascript");
std::string htmltype("text/html");
std::string csstype("text/css");
std::string pngtype("image/png");
const unsigned long MAX_HTTP_SEND_PACKAGE_SIZE = 1024 * 16;
std::string userreject("<!-- {INJECTED_USER_NAME} -->");

std::string notfindhtml = "<html><head>"
"<title>Error 404 Not Found</title>"
"</head>"
"<body>"
"<h2>HTTP ERROR: 404</h2>"
"<p>Problem accessing  ";

std::string endnofindhtml=" Reason:"
"</p><pre>    Not Found</pre><p></p>"
"<hr><i><small>Powered by StarOS://</small></i></hr></body></html>";

static std::string SCRIPT_START = "\n<script>\n";
static std::string SCRIPT_END = "</script>\n\n";

void myReplace(std::string& str,
               const std::string& oldStr,
               const std::string& newStr)
{
  std::string::size_type pos = 0u;
  while((pos = str.find(oldStr, pos)) != std::string::npos){
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

std::string& trimstr(std::string &s)   
{  
    if (s.empty())   
    {  
        return s;  
    }  
    s.erase(0,s.find_first_not_of(" "));  
    s.erase(s.find_last_not_of(" ") + 1);  
    return s;  
} 


CWebServerChannel::CWebServerChannel(uint32_t id,IAWChannelHttpServer *channel,CStarfaceServer *pserver)
    :m_id(id)
    ,m_pChannel(channel)
    ,m_pserver(pserver)
    ,m_bisauth(false)
{
    CAW_INFO_TRACE("CWebServerChannel::CWebServerChannel m_id="<<id);
#if 0
    CAWString projectspath = GetBinHomePath();
    if (projectspath == CAWString(""))
    {
        CAW_ERROR_TRACE(" no home path\n");
    }

    m_htmlpath = projectspath.c_str();
    if(m_htmlpath.c_str()[m_htmlpath.length()-1] != '/')
    m_htmlpath=m_htmlpath+"/";
#endif
    m_htmlpath="/opt/staros.xyz/starface";
    IAWTransport *pTransport = m_pChannel->GetTcpTransport();
    if (pTransport)
        pTransport->GetOption(CAW_OPT_TRANSPORT_PEER_ADDR, &m_clientaddr);
}

CWebServerChannel::~CWebServerChannel()
{
    CAW_INFO_TRACE("CWebServerChannel::~CWebServerChannel "<<m_id);
    if (m_pChannel) 
    {
        //m_pChannel->OpenWithSink(NULL);
        m_pChannel->Disconnect(CAW_OK);
        m_pChannel = NULL;
    }
}

void CWebServerChannel::OnReceive(CAWMessageBlock &data, IAWTransport *trptId, CAWTransportParameter *para)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::OnReceive  m_pChannel==NULL");
        return;
    }

    CAWResult rv;
    
    CAW_INFO_TRACE("CWebServerChannel::OnReceive,"
        " len = " << data.GetChainedLength() <<
        " TrptId=" << trptId);

    CAWString strcontent = data.FlattenChained();

    CAW_ASSERTE(trptId == m_pChannel.Get());
    CAWString method;
    rv = m_pChannel->GetRequestMethod(method);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
    CAWString strPath;
    rv = m_pChannel->GetRequestPath(strPath);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
    
    CAW_INFO_TRACE("CWebServerChannel::OnReceive,"
        " method=" << method << 
        " strPath=" << strPath<<
        " strcontent="<<strcontent);

    //NotFound(strPath.c_str());

    CSession *psession=NULL;

    CAWString aValue;
    rv = m_pChannel->GetRequestHeader("Cookie",aValue);
    std::string strcookies(aValue.c_str()); 
    char *p;  
    p = strtok((char *)strcookies.c_str(), ";");
    while(p)  
    {   
        std::string presession(p);
        presession = trimstr(presession);
        if (presession.size()>11)
        {
            if (memcmp("jsessionid=", presession.c_str(), 11)==0)
            {
                psession = m_pserver->GetSessionMgr().FindSession(presession);
                if (psession)
                {
                    psession->ResetTime();
                }
            }
        }
        p = strtok(NULL, ";");     
    }
    if (IsAPI(strPath))
    {
        if (strPath == "/starface/ui/sdnmgr/api/authrequest")
        {
            if(RestfullAuthRequest(strcontent,strPath, method)==CAW_OK)
            {
                return;
            }
        }
        else 
        {
            if (psession == NULL)
            {
                Redirect("/starface/ui/login.html");
            }
            else
            {
                if(RestfullRequest(strcontent,strPath, method,psession)==CAW_OK)
                {
                    return;
                }
            } 
        }
    }   
    else if ((strPath == "/starface/ui/admin.html") || (strPath == "/starface/ui/user.html"))
    {
        if (psession == NULL)
        {
            Redirect("/starface/ui/login.html");
        }
        else
        {
            HandleMainResource(strPath.c_str(), psession);
        }
    }
    else if (strPath == "/starface/ui/logout")
    {
        if (psession)
        {
            m_pserver->GetSessionMgr().RemoveSession(psession->GetId());
        }
        Redirect("/starface/ui/login.html");
    }
    else if (strPath == "/starface/ui/login.html")
    {
        ResponseOK(strPath.c_str());
    }
    else if (strPath == "/starface/ui/error.html")
    {
        ResponseOK(strPath.c_str());
    }
    else
    {
        ResponseOK(strPath.c_str());
    }

    m_pserver->ServerDestroy(m_id);
}

void CWebServerChannel::HandleMainResource(const std::string &url,CSession *psession)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::HandleMainResource  m_pChannel==NULL");
        return;
    }
    if(psession==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::HandleMainResource  psession==NULL");
        return;
    }
    CAWResult rv;
    std::string strauth = "var StarosAuth='" + psession->GetUserName() + "';\n";
    std::string strref= "var StarosPrefs={};\n";
    std::string strcode = "console.log('Starface Server Logging in as user >" + psession->GetUserName() + "<');\n";

    std::string struser = SCRIPT_START+strauth+strref+strcode+SCRIPT_END;

    string fullfile = m_htmlpath+"/html"+url;

    char *m_pcontent=NULL;
    size_t m_contentsize = 0;
    
    if (WFFileGetFileContent(fullfile.c_str(), &m_pcontent, m_contentsize) == -1)
    {
        //CAW_ERROR_TRACE("can not find the page,%s\n",fullfile.c_str());
        NotFound(url);
        return;
    }
    std::string strcontent(m_pcontent, m_contentsize);
   
    myReplace(strcontent, userreject, struser);
    m_pChannel->SetOrAddResponseHeader("Content-Type",CAWString(htmltype.c_str()));

    //printf("CWebServerSink::HandleMainResource, send size=%d\n", strcontent.size());
    m_pChannel->SetOrAddResponseHeader("Server","StarOS GUI");
    m_pChannel->SetOrAddResponseHeader("Connection","close");

    CAWMessageBlock mb(strcontent.size(), 
        strcontent.c_str(), 
        CAWMessageBlock::DONT_DELETE, 
        strcontent.size());
    CAWTransportParameter Param;
    Param.m_dwHaveSent=0;
    Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    if (m_pChannel->SendData(mb,&Param) != CAW_OK)
    {
        CAW_ERROR_TRACE("CWebServerSink::ResponseOK send data error\n");
    }
    delete[] m_pcontent;
    return;

}


void CWebServerChannel::OnSend(IAWTransport *trptId, CAWTransportParameter *para)
{
    CAW_INFO_TRACE("CWebServerChannel::OnSend,"
        " TrptId=" << trptId);
}

void CWebServerChannel::OnDisconnect(CAWResult aReason, IAWTransport *aTrptId)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::OnDisconnect  m_pChannel==NULL");
        return;
    }

    CAW_INFO_TRACE("CWebServerChannel::OnDisconnect,"
        " Reason = " << aReason <<
        " TrptId = " << aTrptId<<",id="<<m_id);
    CAW_ASSERTE(aTrptId == m_pChannel.Get());
    if (m_pChannel) 
    {
        //m_pChannel->OpenWithSink(NULL);
        m_pChannel->Disconnect(CAW_OK);
        m_pChannel = NULL;
    }
    if (m_pserver)
    {
        m_pserver->ServerDestroy(m_id); 
    }
}

void CWebServerChannel::NotFound(const std::string &nofoundurl)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::NotFound  m_pChannel==NULL");
        return;
    }

    std::string formattedStr=notfindhtml+nofoundurl+endnofindhtml;

    //formattedStr = formattedStr.replace(formattedStr.find(notfoundreject), notfoundreject.size(), nofoundurl);
    m_pChannel->SetResponseStatus(404, "Error.");
    m_pChannel->SetOrAddResponseHeader("Content-Type","text/html");

    CAWMessageBlock mb(formattedStr.length(), 
        formattedStr.c_str(), 
        CAWMessageBlock::DONT_DELETE, 
        formattedStr.length());
    CAWTransportParameter Param;
    Param.m_dwHaveSent=0;
    Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    if (m_pChannel->SendData(mb,&Param) != CAW_OK)
    {
        CAW_ERROR_TRACE("CWebServerSink::NotFound send data error\n");
    }

    return;
}

std::string CWebServerChannel::GetLocation()
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::GetLocation  m_pChannel==NULL");
        return "";
    }

    CAWInetAddr                 addrLocal;

    IAWTransport *pTransport = m_pChannel->GetTcpTransport();
    pTransport->GetOption(CAW_OPT_TRANSPORT_LOCAL_ADDR, &addrLocal);

    WORD localport = addrLocal.GetPort();

    char buffer[1024]={0};
    CAWString localip = addrLocal.GetIpDisplayName();
    snprintf(buffer, sizeof(buffer), "http://%s:%d",localip.c_str(), localport);
    return std::string(buffer);
}

void CWebServerChannel::Redirect(const std::string &locationur)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::Redirect  m_pChannel==NULL");
        return;
    }

    CAWResult rv;
    //std::string location=GetLocation()+locationur;
    CAW_INFO_TRACE("Redirect, locationurl="<<locationur);
    m_pChannel->SetOrAddResponseHeader("Location",locationur.c_str());
    m_pChannel->SetOrAddResponseHeader("Server","StarOS GUI");
    m_pChannel->SetOrAddResponseHeader("Connection","close");
    rv = m_pChannel->SetResponseStatus(302, "Found");
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
    CAWMessageBlock mbZero(0UL);
    rv = m_pChannel->SendData(mbZero);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
}

void CWebServerChannel::ResponseOK(const std::string &strurl)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::ResponseOK  m_pChannel==NULL");
        return;
    }

    char buff[1024*2]={0};

    CAWResult rv;
    std::string contenttype;
    if (strurl.find(".") ==std::string::npos)
    {
        NotFound(strurl);
        return;
    }

    if (strurl.find(".html") !=std::string::npos)
    {
        contenttype=htmltype;
    }
    else if (strurl.find(".png") !=std::string::npos)
    {
        contenttype=pngtype;
    }
    else if (strurl.find(".js") !=std::string::npos)
    {
        contenttype=jstype;
    }
    else if (strurl.find(".css") !=std::string::npos)
    {
        contenttype=csstype;
    }
    else if (strurl.find(".gif") !=std::string::npos)
    {
        contenttype="image/gif";
    }
    else if (strurl.find(".woff") !=std::string::npos)
    {
        contenttype="application/x-font-woff";
    }
    else if (strurl.find(".ttf") !=std::string::npos)
    {
        contenttype="application/x-font-woff";
    }
    
    string fullfile = m_htmlpath+"/html"+strurl;

    //printf("CWebServerSink::ResponseOK, fullfile=%s\n", fullfile.c_str());
    CAW_INFO_TRACE("CWebServerChannel::ResponseOK, fullfile="<<fullfile);

    char *m_pcontent=NULL;
    size_t m_contentsize = 0;
    if (WFFileGetFileContent(fullfile.c_str(), &m_pcontent, m_contentsize) == -1)
    {
        //CAW_ERROR_TRACE("can not find the page,%s\n",fullfile.c_str());
        NotFound(strurl);
        return;
    }
    if (contenttype.size()>0)
    {
        m_pChannel->SetOrAddResponseHeader("Content-Type",CAWString(contenttype.c_str()));
    }
    //printf("CWebServerSink::ResponseOK, send size=%d\n", m_contentsize);
    
    m_pChannel->SetOrAddResponseHeader("Server","StarOS GUI");
    m_pChannel->SetOrAddResponseHeader("Connection","close");

    CAWMessageBlock mb(m_contentsize, 
        m_pcontent, 
        CAWMessageBlock::DONT_DELETE, 
        m_contentsize);
    CAWTransportParameter Param;
    Param.m_dwHaveSent=0;
    Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    
    if (m_pChannel->SendData(mb, &Param) != CAW_OK)
    {
        CAW_ERROR_TRACE("CWebServerSink::ResponseOK send data error\n");
    }
    
    delete[] m_pcontent;
    
    return;

}

CAWResult CWebServerChannel::RestfullAuthRequest(const CAWString &strjson, const CAWString &strpath, const CAWString &strmethod)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::RestfullAuthRequest  m_pChannel==NULL");
        return CAW_ERROR_FAILURE;
    }

    Json::Reader reader;  
    Json::Value root;  
    std::string strusername("");

    if (reader.parse(strjson.c_str(), root))
    {  
        if ((!root["username"].isNull()) && (root["username"].isString()))
        {
            strusername = root["username"].asString();
        }
    }

    if (strusername=="")
    {
        return CAW_ERROR_FAILURE;
    }

    char *ppath = (char *)(strlen("/starface/ui")+strpath.c_str());
    CAWString newpath(ppath);

    CAW_INFO_TRACE("CWebServerChannel::RestfullAuthRequest, strjson="<<strjson
        <<",newpath="<<newpath
        <<",method="<<strmethod);

    m_username=strusername;
    return m_pserver->GetClientMgr().RestfullRequest(m_id, newpath, strmethod, strjson);
}

CAWResult CWebServerChannel::RestfullRequest(const CAWString &strjson, const CAWString &strpath, const CAWString &strmethod,CSession *psession)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::RestfullRequest  m_pChannel==NULL");
        return CAW_ERROR_FAILURE;
    }
    if(psession==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::RestfullRequest  psession==NULL");
        return CAW_ERROR_FAILURE;
    }

    char *ppath = (char *)(strlen("/starface/ui")+strpath.c_str());
    if(ppath==NULL)
    {
        return CAW_ERROR_FAILURE;
    }
    CAWString newpath(ppath);
    CAW_INFO_TRACE("CWebServerChannel::RestfullRequest, strjson="<<strjson
        <<",newpath="<<newpath
        <<",method="<<strmethod);

    return m_pserver->GetClientMgr().RestfullRequest(m_id, newpath, strmethod, strjson, psession);
}


void CWebServerChannel::OnRestfulResponse(const CAWString &strjson,const std::string &controllerip,const std::string &controllerport, const std::string &controllerusername, const std::string &controllerpwd)
{
    if(m_pChannel==NULL)
    {
        CAW_ERROR_TRACE("CWebServerChannel::OnRestfulResponse  m_pChannel==NULL");
        return;
    }
    Json::Reader reader;  
    Json::Value root;  
    std::string strcode("unkown");
    std::string strtype("unkown");
    CAW_INFO_TRACE("CWebServerChannel::OnRestfulResponse, strjson="<<strjson);

    if (reader.parse(strjson.c_str(), root))
    {  
        if ((!root["msgtype"].isNull()) && (root["msgtype"].isString()))
        {
            strtype=root["msgtype"].asString();
        }
    }
    else
    {
        m_pserver->ServerDestroy(m_id);
        return;
    }
    if (strtype == "authresponse")
    {
        if ((!root["code"].isNull()) && (root["code"].isString()))
        {
            strcode = root["code"].asString();
        }
        CAW_INFO_TRACE("CWebServerChannel::OnRestfulResponse, authresponse");
        if (strcode == "0")
        {
            std::string id = m_pserver->GetSessionMgr().GenSessionId();
            CAWAutoPtr<CSession> psession(new CSession(id, m_pserver->GetSessionMgr()));
            if(psession.Get())
            {
                psession->SetIP(controllerip);
                psession->SetPORT(controllerport);
                psession->SetControllerusername(controllerusername);
                psession->SetControllerpassword(controllerpwd);
                psession->SetUserName(m_username);
                m_pserver->GetSessionMgr().AddSession(id,psession);
                std::string strcookie=id+";Path=/starface/ui";
                m_pChannel->SetOrAddResponseHeader("Set-Cookie",strcookie.c_str()); 
                m_pChannel->SetOrAddResponseHeader("Access-Control-Allow-Credentials","true"); 
                m_pChannel->SetOrAddResponseHeader("Access-Control-Allow-Origin","*");
            }
        }
        if (strjson.size()>0)
        {
            m_pChannel->SetOrAddResponseHeader("Content-Type","application/json; charset=utf-8");
        }
        //printf("CWebServerSink::ResponseOK, send size=%d\n", m_contentsize);
    
        m_pChannel->SetOrAddResponseHeader("Server","StarOS GUI");
    
        CAWMessageBlock mb(strlen(strjson.c_str()), 
            strjson.c_str(), 
            CAWMessageBlock::DONT_DELETE, 
            strlen(strjson.c_str()));
        CAWTransportParameter Param;
        Param.m_dwHaveSent=0;
        Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    
        if (m_pChannel->SendData(mb, &Param) != CAW_OK)
        {
            CAW_ERROR_TRACE("CWebServerSink::ResponseOK send data error\n");
        }

        m_pserver->ServerDestroy(m_id); 
    }
    else 
    {
        if (strjson.size()>0)
        {
            m_pChannel->SetOrAddResponseHeader("Content-Type","application/json; charset=utf-8");
        }
        //printf("CWebServerSink::ResponseOK, send size=%d\n", m_contentsize);
    
        m_pChannel->SetOrAddResponseHeader("Server","StarOS GUI");
    
        CAWMessageBlock mb(strlen(strjson.c_str()), 
            strjson.c_str(), 
            CAWMessageBlock::DONT_DELETE, 
            strlen(strjson.c_str()));
        CAWTransportParameter Param;
        Param.m_dwHaveSent=0;
        Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    
        if (m_pChannel->SendData(mb, &Param) != CAW_OK)
        {
            CAW_ERROR_TRACE("CWebServerSink::ResponseOK send data error\n");
        }
        
        m_pserver->ServerDestroy(m_id); 
    }
}


bool CWebServerChannel::IsAPI(const CAWString &strpath)
{
     if(strpath.size()<strlen("/starface/ui/sdnmgr/api"))
     {   
        return false;
     }
     if(strpath.size()<strlen("/starface/ui/onos/v1"))
     {   
        return false;
     }



    if (::memcmp(strpath.c_str(),"/starface/ui/sdnmgr/api",strlen("/starface/ui/sdnmgr/api"))==0)
    {
        return true;
    }
     else if(::memcmp(strpath.c_str(),"/starface/ui/onos/v1",strlen("/starface/ui/onos/v1"))==0)
     {
        return true;
     }
    else {
        return false;
    }
}

uint32_t CWebServerChannel::GetChannelId()
{
    return m_id;
}

CAWString CWebServerChannel::GetClientAddr()
{
    return m_clientaddr.GetIpDisplayName();
}

DWORD CWebServerChannel::AddReference()
{
    return CAWReferenceControlSingleThread::AddReference();
}

DWORD CWebServerChannel::ReleaseReference()
{
    return CAWReferenceControlSingleThread::ReleaseReference();
}


