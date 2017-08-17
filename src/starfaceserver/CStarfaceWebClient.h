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
#ifndef CSTARTFACEWEBCLIENT
#define CSTARTFACEWEBCLIENT
#include "CAWACEWrapper.h"
#include "IAWHttpManager.h"
#include "CAWTimerWrapperID.h"


class CStarfaceServer;

class CStarfaceWebClient : public IAWChannelSink
    , public CAWReferenceControlSingleThread
    , public CAWTimerWrapperIDSink
{
public:
    CStarfaceWebClient(CStarfaceServer *pserver,
            const std::string &ip, 
            const std::string &port,
            uint32_t channelid);
    virtual ~CStarfaceWebClient();

    CAWResult RestfullRequest(const CAWString &strurl, const CAWString &strmethod, const CAWString &strjson);

    //IAWChannelSink
    virtual void OnConnect(CAWResult aReason, IAWChannel *aChannelId);
    CAWResult SendData(const CAWString &strjson);
    void SetBasicAuthInfo(const CAWString &username, const CAWString &password);
    //CAWReferenceControlSingleThread
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();
    virtual void OnTimer(CAWTimerWrapperID* aId);
public:
    virtual void OnReceive(
    CAWMessageBlock &Data /* {out} */,
    IAWTransport*   pTransport,
    CAWTransportParameter*  aPara = NULL );

    virtual void OnSend( 
    IAWTransport*       pTransport,
    CAWTransportParameter*  aPara = NULL );

    virtual void OnDisconnect(
    CAWResult        aReason,
    IAWTransport*   pTransport );

    std::string GetIP();
    std::string GetPort();
    std::string GetUserName();
    std::string GetPassword();
    std::string GetRestfulJson();
    std::string GetChannelId();
    std::string GetMethod();
private:
    CStarfaceServer *m_pserver;
    CAWAutoPtr<IAWChannelHttpClient> m_httpclient;
    IAWChannel *m_ChannelId;
    std::string m_ip;
    std::string m_port;
    CAWString m_password;
    CAWString m_username;
    CAWString m_strjson;
    uint32_t m_nchannelid;
    CAWString m_strmethod;
    CAWTimerWrapperID m_timer;
};


#endif//CSTARTFACEWEBCLIENT
