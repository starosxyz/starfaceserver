/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     CStarOpenflow.h
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

#ifndef CSTAROPENFLOW_H
#define CSTAROPENFLOW_H

#include "dipc.h"
#include "IStarGui.h"
#include "ISYSDBManager.h"
#include "CAWACEWrapper.h"
#include "COpenflowServer.h"

class CStarOpenflow : public IDIPCProcessSink, public IRequestHandler
{
public:
    CStarOpenflow();
    virtual ~CStarOpenflow();

    //IDIPCProcessSink
    virtual void OnBootOK();
    virtual void OnProcessRun(int argc, char** argv, IDIPCProcess *dipcProcess);
    virtual void OnHAProcessConnected(uint32_t clusterid,
                                        uint32_t datacenterid,
                                        CAWAutoPtr<IDIPCTransport> &transport);

    //IRequestHandler
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);
public:
        CAWResult ReadDb();
        CAWResult WriteDb();
        CAWResult ResponseData(Json::Value &response);
        virtual void OnProcessUpdateState(const CDIPCProcess&){}
private:
    CAWAutoPtr<IUiExtension> m_uiext;
    IDIPCProcess *m_pdipcProcess;
    COpenflowServer m_openflowserver;

    std::string m_stropenflowip;
    std::string m_stropenflowport;
    std::string m_stropenflowtype;
};
#endif//CSTAROPENFLOW_H

