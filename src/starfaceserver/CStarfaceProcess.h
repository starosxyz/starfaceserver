/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd  
 * Description:      CStarfaceProcess.h
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

#ifndef CSTARFACEPROCESS_H
#define CSTARFACEPROCESS_H

#include "dipc.h"
#include "IStarGui.h"
#include "ISYSDBManager.h"
#include "CAWACEWrapper.h"
#include "CStarfaceServer.h"
#include "CFileObject.h"

class CStarfaceProcess : public IDIPCProcessSink
    , public IRequestHandler
{
public:
    CStarfaceProcess();
    virtual ~CStarfaceProcess();

    //IDIPCProcessSink
    virtual void OnBootOK();
    virtual void OnProcessRun(int argc, char** argv, IDIPCProcess *dipcProcess);
    virtual void OnHAProcessConnected(uint32_t clusterid,
                                        uint32_t datacenterid,
                                        CAWAutoPtr<IDIPCTransport> &transport);

    //IRequestHandler
    virtual void process(long sid, Json::Value &payload, IUiExtension *pUi);
	CStarfaceServer& GetStarfaceServer();
public:
    CAWResult ReadDb();
    bool IsCertValite(const std::string& m_cerfile);
    bool IsKeyValite(const std::string& m_keyfile);
    CAWResult WriteDb();
     void writefile();
    /*void GetWebInfo(std::string &webip,std::string &webhttpport);
    void GetControlerInfo(std::string &controlerip,std::string &controlerport);*/
    CAWResult ResponseData(Json::Value &response);
    void getPortFromList(std::string      & strcontrolerport);
    virtual void OnProcessUpdateState(const CDIPCProcess&){}
private:
    IDIPCProcess *m_dipcProcess;
    std::string m_strwebip;
    std::string m_strwebhttpport;
    std::string m_strwebhttpsport;
    std::string m_strcontrolerip;
    std::string m_strcontrolerport;
    std::string m_cerfile;
    std::string m_keyfile;
    CAWAutoPtr<IUiExtension> m_uiext;
    CStarfaceServer m_server;
    std::string m_controllerusername;
    std::string m_controllerpassword;
};
#endif//CSTARFACEPROCESS_H

