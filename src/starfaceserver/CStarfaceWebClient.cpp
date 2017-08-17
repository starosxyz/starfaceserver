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
#include "CStarfaceWebClient.h"
#include "IAWHttpManager.h"
#include "CAWConnectionInterface.h"
#include <string>
#include <list>
#include "CStarfaceServer.h"
#include "CStarfaceWebClientManager.h"
#include "CAWTimeValue.h"


CStarfaceWebClient::CStarfaceWebClient(CStarfaceServer *pserver,
            const std::string &ip, 
            const std::string &port,
            uint32_t channelid)
    :m_pserver(pserver)
    ,m_ChannelId(NULL)
    ,m_ip(ip)
    ,m_port(port)
    ,m_nchannelid(channelid)
{
    CAW_INFO_TRACE("CStarfaceWebClient::CStarfaceWebClient "<<m_nchannelid);
}
CStarfaceWebClient::~CStarfaceWebClient()
{
    if (m_httpclient.Get())
    {
        m_httpclient->Disconnect(CAW_OK);
        m_httpclient=NULL;
    }


    CAW_INFO_TRACE("CStarfaceWebClient::~CStarfaceWebClient "<<m_nchannelid);
    m_timer.Cancel();
}

void CStarfaceWebClient::OnTimer(CAWTimerWrapperID* aId)
{
    if (m_pserver)
    {
        m_pserver->ServerDestroy(m_nchannelid);
    }
}


CAWResult CStarfaceWebClient::RestfullRequest(const CAWString &strurl, const CAWString &strmethod, const CAWString &strjson)
{
    if (m_httpclient.Get())
    {
        m_httpclient->Disconnect(CAW_OK);
        m_httpclient=NULL;
    }

    m_strjson = strjson;
    m_strmethod=strmethod;
    CAWString url="http://"+CAWString(m_ip.c_str())+":"+CAWString(m_port.c_str())+strurl;

    CAW_INFO_TRACE(" CStarfaceWebClient::RequestURL, ip="<<m_ip
        <<",port="<<m_port
        <<",url="<<url);

    
    CAWResult rv = IAWHttpManager::Instance()->CreateChannelHttpClient(m_httpclient.ParaOut(),url);
    if (rv != CAW_OK)
    {
        CAW_ERROR_TRACE("create http client error");
        return CAW_ERROR_FAILURE;
    }

    CAWResult cmResult = m_httpclient->AsyncOpen( this );
    if (CAW_FAILED(cmResult) )
    {
        CAW_ERROR_TRACE( "CStarfaceWebClient::Connect(), AsyncOpen() Failed, " 
        << "cmResult = " << cmResult );
        return -1;
    }
	m_httpclient->BasicAuthenInfo(m_username, m_password);
    CAWTimeValue timevalue(5,0);
    m_timer.Schedule(this, timevalue, 0);


    return CAW_OK;   
}


void CStarfaceWebClient::OnConnect(CAWResult aReason, IAWChannel *aChannelId)
{
    CAW_INFO_TRACE("CStarfaceWebClient::OnConnect");

    if (m_httpclient.Get() == NULL)
    {
        CAW_ERROR_TRACE("CStarfaceWebClient::OnConnect");
        return;
    }
    
    CAWResult rv;

    m_timer.Cancel();
    
    if (CAW_FAILED(aReason) )
    {
        CAW_ERROR_TRACE( "CRestfulHttpConnector::OnConnect(), Failed to connect, "
        << "cmResult = " << aReason );
        m_pserver->ServerDestroy(m_nchannelid);
        return;
    }

    CAW_ASSERTE(aChannelId == m_httpclient.Get());

    CAW_INFO_TRACE( "CRestfulHttpConnector::OnConnect(), ChannelId = " << aChannelId );

    DWORD dwSendBuffer = 1024 * 1024;
    CAWResult cmResult = m_httpclient->SetOption(CS_OPT_MAX_SENDBUF_LEN, &dwSendBuffer );
    if ( CAW_FAILED(cmResult) )
    {
        CAW_ERROR_TRACE( "CRestfulHttpConnector::OnConnect(), SetOption() Failed, "
        << "cmResult = " << cmResult );

        m_httpclient->Disconnect(CAW_OK);
        m_httpclient = NULL;
        return;
    }
    rv = m_httpclient->SetRequestMethod(m_strmethod);
    if (CAW_FAILED(rv) )
    {
        CAW_ERROR_TRACE("CRestfulHttpConnector::OnConnect http method error");
        return;
    }
    if (m_strjson.size()>0)
    {
        m_httpclient->SetOrAddRequestHeader("Content-Type","application/json; charset=utf-8");
        rv=SendData(m_strjson);
    }
    else
    {
        CAWMessageBlock mbZero(0UL);
        rv=m_httpclient->SendData(mbZero);
    }
    if (CAW_FAILED(rv) )
    {
        CAW_ERROR_TRACE( "CRestfulHttpConnector::OnConnect() Failed, "
            << "cmResult = " << rv );
        return;
    }

}


CAWResult CStarfaceWebClient::SendData(const CAWString &strjson)
{
    CAW_INFO_TRACE( "CStarfaceWebClient::SendData()" );

    CAWResult cmResult = CAW_OK;
    if (m_httpclient.Get() == NULL)
    {
        return CAW_ERROR_FAILURE;
    }
    CAWMessageBlock MsgBlk( 
            strjson.size(), 
            strjson.c_str(), 
            CAWMessageBlock::DONT_DELETE,
            strjson.size() ); 
    CAWTransportParameter Param;
    Param.m_dwHaveSent=0;
    Param.m_Priority=CAWConnectionManager::CPRIORITY_HIGH;
    cmResult = m_httpclient->SendData( MsgBlk ,&Param);
    if (CAW_FAILED(cmResult) )
    {
        CAW_ERROR_TRACE( "CRestfulHttpConnector::SendData() Failed, "
            << "cmResult = " << cmResult );
        return -1;
    }

    return CAW_OK;
}


void CStarfaceWebClient::OnDisconnect(CAWResult aReason, IAWTransport* pTransport )
{
    CAW_INFO_TRACE( "CStarfaceWebClient::OnDisconnect() "<<m_nchannelid);

    CAW_ASSERTE( pTransport == m_httpclient.Get() );

    if (m_httpclient.Get())
    {
        m_httpclient->Disconnect(CAW_OK);
        m_httpclient=NULL;
    }
    if (m_pserver)
    {
        m_pserver->ServerDestroy(m_nchannelid);
    }
}

void CStarfaceWebClient::OnSend(IAWTransport* pTransport, CAWTransportParameter* aPara /* = NULL  */ )
{
    CAW_INFO_TRACE( "CStarfaceWebClient::OnSend()" );

    CAW_ASSERTE( pTransport == m_httpclient.Get() );
}

void CStarfaceWebClient::OnReceive(CAWMessageBlock& Data , 
                                IAWTransport* pTransport, 
                                CAWTransportParameter* aPara)
{
    CAW_INFO_TRACE( "CStarfaceWebClient::OnReceive()" );

    if (m_httpclient.Get()==NULL)
    {
        CAW_ERROR_TRACE( "CStarfaceWebClient::OnReceive httpclient param is NULL");
        return;
    }

    CAW_ASSERTE( pTransport == m_httpclient.Get() );

    CAW_INFO_TRACE( "CStarfaceWebClient::OnReceive(), Data length = " << Data.GetChainedLength() );
    CAW_INFO_TRACE("CStarfaceWebClient::OnReceive,"
        " len = " << Data.GetChainedLength() <<
        " TrptId=" << pTransport);

    CAWString strcontent = Data.FlattenChained();

    CAWResult rv;
    LONG Status;
    rv = m_httpclient->GetResponseStatus(Status);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
    
    CAW_INFO_TRACE("CStarfaceWebClient::OnReceive,"<<Status);
    if (m_pserver)
    {
        if (Status == 200)
        {
            m_pserver->GetClientMgr().OnRestfulResponse(m_nchannelid, strcontent);
        }
        m_pserver->ServerDestroy(m_nchannelid);
    }
}

void CStarfaceWebClient::SetBasicAuthInfo(const CAWString &username, const CAWString &password)
{
    m_username=username;
    m_password=password;

}

DWORD CStarfaceWebClient::AddReference()
{
    return CAWReferenceControlSingleThread::AddReference();
}

DWORD CStarfaceWebClient::ReleaseReference()
{
    return CAWReferenceControlSingleThread::ReleaseReference();
}

std::string CStarfaceWebClient::GetIP()
{
    return m_ip;
}
std::string CStarfaceWebClient::GetPort()
{
    return m_port;
}
std::string CStarfaceWebClient::GetUserName()
{
    return m_username.c_str();
}
std::string CStarfaceWebClient::GetPassword()
{
    return m_password.c_str();
}
std::string CStarfaceWebClient::GetRestfulJson()
{
    return "";
}
std::string CStarfaceWebClient::GetChannelId()
{
    char buffer[64]={0};
    sprintf(buffer,"%d", m_nchannelid);
    return std::string(buffer);
}

std::string CStarfaceWebClient::GetMethod()
{
    return m_strmethod.c_str();
}



