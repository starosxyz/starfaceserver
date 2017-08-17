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
#ifndef  CBACKUPTRANSPORTSINK
#define  CBACKUPTRANSPORTSINK
#include "dipc.h"
#include "IStarGui.h"
#include "ISYSDBManager.h"
#include "IAWHttpManager.h"
#include "CStarfaceProcess.h"
#include <vector>
#include <cstring>
class CStarfaceProcess;
class IDIPCTransport;
class CBackupTransportSink: public IDIPCTransportSink
{
public:
	CBackupTransportSink(CAWAutoPtr<IDIPCTransport> trptId,CStarfaceProcess* process);
	virtual ~CBackupTransportSink();
	// IDIPCTransportSink
	void OnReceive(CAWMessageBlock &aData,
                            IDIPCTransport *aTrptId);
	void OnDisconnect(CAWResult aReason,
                    IDIPCTransport *aTrptId);
	void OnSend(IDIPCTransport *aTrptId);
	
private:
	 CAWAutoPtr<IDIPCTransport> m_aTrptId;
	 CStarfaceProcess* m_pProcess;
};

#endif //CBackupTransportSink
