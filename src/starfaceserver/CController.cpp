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
#include "CController.h"
#include <string>

CController::CController(
            const std::string controllerip, 
            const std::string controllerport,
            const std::string controllerusername,
            const std::string controllerpassword)
    :m_controllerip(controllerip)
    ,m_controllerport(controllerport)
    ,m_controllerusername(controllerusername)
    ,m_controllerpassword(controllerpassword)
{
    CAW_INFO_TRACE("CController::CController");
}
CController::~CController()
{
    CAW_INFO_TRACE("CController::~CController");
}




