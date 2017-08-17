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
#include "CFileObject.h"\
#include <stdio.h>
#include <sys/stat.h>   

CFileObject::CFileObject(){
    CAW_INFO_TRACE("CFileObject::CFileObject");
}
CFileObject::~CFileObject(){
    CAW_INFO_TRACE("CFileObject::~CFileObject");
}

CAWResult CFileObject::Init(){
    CAW_INFO_TRACE("CFileObject::Init");
    return CAW_OK;
}

unsigned long CFileObject::get_file_size(const char *path){    
    CAW_INFO_TRACE("CFileObject::get_file_size path="<<path);
    unsigned long filesize = -1;        
    struct stat statbuff;    
    if(stat(path, &statbuff) < 0){    
        return filesize;    
    }else{    
        filesize = statbuff.st_size;    
    }    
    CAW_INFO_TRACE("CFileObject::get_file_size filesize="<<filesize);
    return filesize;    
}    

CAWResult CFileObject::ReadFile(const std::string &name,CAWMessageBlock &msg){
    CAW_INFO_TRACE("CFileObject::ReadFile name="<<name);

    FILE *fp;
    if((fp=fopen(name.c_str(),"r"))==NULL){
        CAW_INFO_TRACE("CFileObject::ReadFile file is cannot opened!");
        return CAW_ERROR_FAILURE;
    }else{
        int buffer_len=get_file_size(name.c_str());
        char *buffer=new char(buffer_len); 
        fread(buffer,buffer_len,1,fp);
        msg.Write(buffer,buffer_len);
        //CAW_INFO_TRACE("CFileObject::ReadFile msg="<<msg);
        fclose(fp);
        delete[] buffer;
        return CAW_OK;
    }
}

CAWResult CFileObject::WriteFile(const std::string &name, CAWMessageBlock &msg){
    CAW_INFO_TRACE("CFileObject::WriteFile name="<<name);
    
    FILE *fp;
    if((fp=fopen(name.c_str(),"a+"))==NULL){
        CAW_INFO_TRACE("CFileObject::ReadFile this file is not opened!");
        return CAW_ERROR_FAILURE;
    }else{
        fwrite((char *)msg.GetTopLevelReadPtr(),msg.GetTopLevelLength(),1,fp);
        fclose(fp);
        return CAW_OK;
    }
}

CAWResult CFileObject::Mkdir(const std::string &name){
    CAW_INFO_TRACE("CFileObject::Mkdir name="<<name);
    if(mkdir(name.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO)==-1){
        CAW_INFO_TRACE("CFileObject::Mkdir this dir is not create!");
        return CAW_ERROR_FAILURE;
    }else{
        return CAW_OK;
    }
}

CAWResult CFileObject::DeleteFile(const std::string &name){
    CAW_INFO_TRACE("CFileObject::DeleteFile name="<<name);
    if(remove(name.c_str())==-1){
        CAW_INFO_TRACE("CFileObject::DeleteFile this file is not exist!");
        return CAW_ERROR_FAILURE;
    }else{
        return CAW_OK;
    }
}

CAWResult CFileObject::Readdir(const std::string &name, std::vector<std::string>& files){
    CAW_INFO_TRACE("CFileObject::Readdir name="<<name);
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(name.c_str())) == NULL)  
    {  
        CAW_INFO_TRACE("CFileObject::Readdir this dir is not exist!");
        return CAW_ERROR_FAILURE;
    }  
    
    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
            continue;
        }else{     
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
    return CAW_OK;
}

bool CFileObject::IsDir(const std::string &name){
    CAW_INFO_TRACE("CFileObject::IsDir name="<<name);
    if(opendir(name.c_str())==NULL){
        CAW_INFO_TRACE("CFileObject::IsDir this dir is not exist!");
        return false;
    }else{
        return true;
    }
}
