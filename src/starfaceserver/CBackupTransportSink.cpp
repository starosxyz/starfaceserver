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
#include "CBackupTransportSink.h"
#include "CControllerManager.h"
#include <vector>
#include "json_features.h"
#include "json_value.h"
#include "json_reader.h"

CBackupTransportSink::CBackupTransportSink(CAWAutoPtr<IDIPCTransport> aTrptId,
    CStarfaceProcess* process)
    :m_aTrptId(aTrptId)
    ,m_pProcess(process)
{
    CAW_INFO_TRACE("CBackupTransportSink::CBackupTransportSink");
}

CBackupTransportSink::~CBackupTransportSink()
{
    CAW_INFO_TRACE("CBackupTransportSink::~CBackupTransportSink");
    
}
void CBackupTransportSink::OnReceive(CAWMessageBlock &data,
                            IDIPCTransport *trptId)
{
    CAW_INFO_TRACE("CBackupTransportSink::OnReceive,"
        " len = " << data.GetChainedLength() <<
        " TrptId=" << trptId);
    Json::Reader reader;
    Json::Value json_object;
    CAWString strcontent=data.FlattenChained();
    std::string str_db(strcontent.c_str(),strcontent.size());
    reader.parse(str_db,json_object);
    CAW_INFO_TRACE("CBackupTransportSink::OnReceive,"
        "str_db" << str_db);
    m_pProcess->GetStarfaceServer().GetControllerMgr().WriteDb(json_object);

}
void CBackupTransportSink::OnDisconnect(CAWResult aReason,
                    IDIPCTransport *aTrptId)
{
    CAW_INFO_TRACE("CBackupTransportSink::OnDisconnect");
    CAW_INFO_TRACE("CBackupTransportSink::OnDisconnect,"
        " Reason = " << aReason <<
        " TrptId = " << aTrptId);
    
    
}
void CBackupTransportSink::OnSend(IDIPCTransport *aTrptId)
{
    CAW_INFO_TRACE("CBackupTransportSink::OnSend");      
}
