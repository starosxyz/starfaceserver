/***********************************************************************
 Copyright (C) 2016-2017 Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef STARFACESERVERJNO_H
#define STARFACESERVERJNO_H
#include "corejno.h"
#define STARFACESERVER_DIPC_JNO_START                (uint16_t)(CORE_DIPC_JNO_END+1001)
#define STARFACESERVER_JNO_STARFACESERVER            (uint16_t)(STARFACESERVER_DIPC_JNO_START+1)
#define STARFACESERVER_JNO_STAROPENFLOW            (uint16_t)(STARFACESERVER_DIPC_JNO_START+2)
#define STARFACESERVER_DIPC_JNO_END                  (uint16_t)(STARFACESERVER_DIPC_JNO_START+1000)
#endif//DIPCJNO_H

