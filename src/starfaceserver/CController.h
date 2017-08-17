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
#ifndef CCONTROLLER
#define CCONTROLLER
#include "CAWACEWrapper.h"
#include "IAWHttpManager.h"

class CStarfaceServer;

class CController
{
public:
    CController(const std::string controllerip, 
            const std::string controllerport,
            const std::string username,
            const std::string password);
    virtual ~CController();
    std::string GetControllerip(){return m_controllerip;}
    std::string GetControllerport(){return         m_controllerport;}
    std::string GetControllerusername(){return           m_controllerusername;}
    std::string GetControllerpassword(){return          m_controllerpassword;}
private:

    std::string m_controllerip;
    std::string m_controllerport;
    std::string m_controllerusername;
    std::string m_controllerpassword;
    
};


#endif//CSTARTFACEWEBCLIENT
