/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description:     SessionManager.cpp
 * Others:
 * Version:          V1.0
 * Author:           hu peng
 * Date:             2017-04-4
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/


#ifndef CCONTROLLERMANAGER_H
#define CCONTROLLERMANAGER_H
#include "CAWACEWrapper.h"
#include "IStarGui.h"
#include "ISYSDBManager.h"
#include <vector>
#include "CController.h"

class CStarfaceServer;

class CControllerManager:IRequestHandler
{
public:
    CControllerManager(CStarfaceServer *pserver);
    virtual ~CControllerManager();
    void Init();

    //IRequestHandler
    void process(long sid, Json::Value &payload, IUiExtension *pUi);

    CAWResult ReadDb(Json::Value &json_object);
    CAWResult WriteDb(Json::Value jsonvalue,bool isDeleteOther=false);
    CAWResult DeleteDbByID(std::string id);
    CAWResult SaveDataToList();
    CAWResult GetControllerList(std::vector<CController> & vec);
private:
    CStarfaceServer *m_pserver;
    CAWAutoPtr<IUiExtension> m_uiext;
    int m_databaseid;
    std::vector<CController> m_vec;
};


#endif//CCONTROLLERMANAGER_H


