/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS, Inc. 
 * Description:     staropenflow.cpp
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

#include <stdlib.h>
#include "dipc.h"
#include "ICOREMain.h"
#include "CStarOpenflow.h"
#include "corejno.h"
#include "starfaceserverjno.h"

int main(int argc, char **argv)
{
    COREProcessInit(argc, argv);
    CStarOpenflow *process=new CStarOpenflow();
    COREProcessLoop(process , STARFACESERVER_JNO_STAROPENFLOW);
    return 0;
}

