/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description:      CFileObject.h
 * Others:
 * Version:          V1.0
 * Author:           Zhang XinXiang
 * Date:             2017-06-09
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/
#ifndef CFILEOBJECT_H
#define CFILEOBJECT_H
#include "CAWACEWrapper.h"
#include "IAWHttpManager.h"
#include "IAWSSLUtils.h"

class CFileObject
{
public:
    CFileObject();
    virtual ~CFileObject();
    CAWResult Init();
    unsigned long get_file_size(const char *path);
    CAWResult ReadFile(const std::string &name,CAWMessageBlock &msg);
    CAWResult WriteFile(const std::string &name, CAWMessageBlock &msg);
    CAWResult Mkdir(const std::string &name);
    CAWResult DeleteFile(const std::string &name);
    CAWResult Readdir(const std::string &name, std::vector<std::string>& files);
    bool IsDir(const std::string &name);
};


#endif//CFILEOBJECT_H