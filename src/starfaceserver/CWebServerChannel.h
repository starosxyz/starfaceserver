/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     CWebServerChannel.h
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
#ifndef WEBSERVERCHANNEL_
#define WEBSERVERCHANNEL_
#include "IAWHttpManager.h"

class CStarfaceServer;
class CStarfaceWebClient;
class CSession;

class CWebServerChannel : public IAWTransportSink, public CAWReferenceControlSingleThread
{
public:
    CWebServerChannel(uint32_t id,IAWChannelHttpServer *channel,CStarfaceServer *pserver);
    virtual ~CWebServerChannel();

    void HandleMainResource(const std::string &url,CSession *psession);
    //IAWTransportSink
    void OnReceive(CAWMessageBlock &data, IAWTransport *trptId, CAWTransportParameter *para = NULL);
    void OnSend(IAWTransport *trptId, CAWTransportParameter *para = NULL);
    void OnDisconnect(CAWResult aReason, IAWTransport *aTrptId);
    void NotFound(const std::string &nofoundurl);
    std::string GetLocation();
    void ResponseOK(const std::string &strurl);
    void Redirect(const std::string &locationur);
    CAWResult RestfullAuthRequest(const CAWString &strjson, 
                                                    const CAWString &strpath, 
                                                    const CAWString &strmethod);
    CAWResult RestfullRequest(const CAWString &strjson, 
                                                const CAWString &strpath,
                                                const CAWString &strmethod,
                                                CSession *psession);
    void OnRestfulResponse(const CAWString &strjson,const std::string &controllerip,const std::string &controllerport, const std::string &controllerusername, const std::string &controllerpwd);
    bool IsAPI(const CAWString &strpath);
    uint32_t GetChannelId();
    CAWString GetClientAddr();
public:
        //CAWReferenceControlSingleThread
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();
private:
    uint32_t m_id;
    CAWAutoPtr<IAWChannelHttpServer> m_pChannel;
    CStarfaceServer *m_pserver;
    CStarfaceWebClient *m_pwebclient;
    std::string m_username;
    std::string m_htmlpath;
    bool m_bisauth;
    CAWInetAddr m_clientaddr;
};
#endif  //WEBSERVERCHANNEL_