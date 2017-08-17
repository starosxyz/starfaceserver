/***********************************************************************
 * Copyright (C) 2016-2017, Nanjing StarOS Network Technology Co., Ltd 
 * Description: 
 * Others:
 * Version:          V1.0
 * Author:           Yi Jian <yijian@stros.xyz>
 * Date:             2017-02-02
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
#include "CStarfaceProcess.h"
#include "corejno.h"
#include "starfaceserverjno.h"

int main(int argc, char **argv)
{
    COREProcessInit(argc, argv);
    CStarfaceProcess *process=new CStarfaceProcess();
    COREProcessLoop(process , STARFACESERVER_JNO_STARFACESERVER);
    return 0;
}
