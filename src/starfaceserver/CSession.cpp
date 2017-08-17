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

#include "CSession.h"
#include "CSessionManager.h"
#include "CAWTimeValue.h"

CSession::CSession(const std::string &sessionid, CSessionManager &server)
    :m_id(sessionid)
    ,m_server(server)
    ,m_username("")
{
    CAWTimeValue timevalue(60*5,0);
    m_timer.Schedule(this, timevalue, 0);
    /* Get expire time */
    GetExpireTime(m_exprieTime, timevalue);
    CAW_INFO_TRACE("CSession::CSession");
}
CSession::~CSession()
{
    CAW_INFO_TRACE("CSession::~CSession");
}

std::string CSession::GetId()
{
    return m_id;
}

std::string CSession::GetExpire()
{
    return m_exprieTime.c_str();
}

std::string CSession::GetUserName()
{
    return m_username;
}

void CSession::SetUserName(std::string username)
{
    m_username=username;
}

void CSession::ResetTime()
{
    m_timer.Cancel();
    CAWTimeValue timevalue(60*5,0);
    m_timer.Schedule(this, timevalue, 0);
    GetExpireTime(m_exprieTime, timevalue);
}
void CSession::OnTimer(CAWTimerWrapperID* aId)
{
    m_server.RemoveSession(m_id);
}

uint32_t CSession::Random(uint32_t start,uint32_t end)
{
    srand((unsigned)time(NULL));
    uint32_t num=end-start;
     if(num<=0)
    {
       return 0;
    }
    return rand()%num+start;   
}

std::string CSession::GetClientIP()
{
    return "0.0.0.0";
}

void CSession::GetExpireTime(CAWString &exprieTime,  CAWTimeValue &timevalue)
{
    CAWTimeValue	tv = CAWTimeValue::GetTimeOfDay()+timevalue;
    time_t  lTime = tv.GetSec();
    tm	*tmNow = localtime(&lTime);

    char    strTime[20];
    strftime(strTime, 20, "%Y%m%d-%H%M%S", tmNow);
    exprieTime = strTime;
}

DWORD CSession::AddReference()
{
    return CAWReferenceControlSingleThread::AddReference();
}

DWORD CSession::ReleaseReference()
{
    return CAWReferenceControlSingleThread::ReleaseReference();
}
void CSession::SetIP(std::string ip)
{
    this->m_ip = ip;
}
void CSession::SetPORT(std::string port)
{
    this->m_port = port;
}
void CSession::SetControllerusername(std::string controllerusername)
{
    this->m_controllerusername = controllerusername;
}
void CSession::SetControllerpassword(std::string password)
{
    this->m_controllerpassword = password;
}

