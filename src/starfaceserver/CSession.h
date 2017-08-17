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
#ifndef CSESSION_H
#define CSESSION_H
#include "CAWACEWrapper.h"
#include "CAWTimerWrapperID.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


class CSessionManager;
class CSession : public CAWTimerWrapperIDSink, public CAWReferenceControlSingleThread
{
public:
    CSession(const std::string &sessionid, CSessionManager &server);
    virtual ~CSession();
    std::string GetId();
    virtual void OnTimer(CAWTimerWrapperID* aId);
    void ResetTime();

    uint32_t Random(uint32_t start,uint32_t end);
    std::string GetClientIP();
    std::string GetExpire();
    std::string GetUserName();
    void SetUserName(std::string username);
    void GetExpireTime(CAWString &exprieTime,  CAWTimeValue &timevalue);
    //CAWReferenceControlSingleThread
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();

    void SetIP(std::string);
    void SetPORT(std::string);
    void SetControllerusername(std::string);
    void SetControllerpassword(std::string);
    std::string GetIP(){return m_ip;}
    std::string GetPORT(){return m_port;}
    std::string GetControllerusername(){return m_controllerusername;}
    std::string GetControllerpassword(){return m_controllerpassword;}
private:
    std::string m_id;
    CAWTimerWrapperID m_timer;
    CSessionManager &m_server;
    std::string m_clientip;
    CAWString m_exprieTime;
    std::string m_username;
    std::string m_ip;
    std::string m_port;
    std::string m_controllerusername;
    std::string m_controllerpassword;
};




#endif//

