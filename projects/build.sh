#!/bin/bash
#
# Copyright (C) 2013-2014, Nanjing WFNEX Technology Co., Ltd
#
source ./env.sh

help()
{
echo ===============================================================================
echo build usage:
echo ===============================================================================
echo "./build.sh [type]"
echo "[type]:"
echo "          help    		: help info" 
echo "          debug     		: build a debug"
echo "          release		    : build a release"
echo "          rpm 		    : build a rpm"
echo "          all 		    : build a debug and release"
echo ===============================================================================
}

# Build an RPM package
function build_rpm() {

    sudo rm -fr $STARFACESERVER_RPM_ROOT

    mkdir -p $STARFACESERVER_RPM_ROOT/{BUILD,RPMS,SOURCES/starfaceserver-$STARFACESERVER_RPM_VERSION/opt/staros.xyz,SPECS,SRPMS}

    cp -r $STARFACESERVER_STAGE $STARFACESERVER_RPM_ROOT/SOURCES/starfaceserver-$STARFACESERVER_RPM_VERSION/opt/staros.xyz/starfaceserver

    cd $STARFACESERVER_RPM_ROOT/SOURCES
    COPYFILE_DISABLE=1 tar zcf starfaceserver-$STARFACESERVER_RPM_VERSION.tar.gz starfaceserver-$STARFACESERVER_RPM_VERSION

    cp $CURRENT/rpm/starfaceserver.spec $STARFACESERVER_RPM_ROOT/SPECS/
    sed -i'' -E "s/@STARFACESERVER_RPM_VERSION/$STARFACESERVER_RPM_VERSION/g" $STARFACESERVER_RPM_ROOT/SPECS/starfaceserver.spec

    rpmbuild --define "_topdir $STARFACESERVER_RPM_ROOT" -bb $STARFACESERVER_RPM_ROOT/SPECS/starfaceserver.spec

    cp $STARFACESERVER_RPM_ROOT/RPMS/x86_64/starfaceserver-$STARFACESERVER_RPM_VERSION-1.x86_64.rpm $CURRENT && ls -l $CURRENT/starfaceserver-$STARFACESERVER_RPM_VERSION-1.x86_64.rpm
}

function clean_stage_dir() {
    [ -d "$STARFACESERVER_RPM_ROOT" ] && rm -r $STARFACESERVER_RPM_ROOT || :
}

if [ "$1" == "help" ] || [ "$1" == "" ]
then
	help
	exit 1
fi

if [ "$1" == "debug" ]
then
	make -f Makefile debug=1 optimize=0
elif [ "$1" == "release" ]
then
	make -f Makefile debug=0 optimize=1
elif [ "$1" == "rpm" ]
then
	build_rpm
	clean_stage_dir
elif [ "$1" == "all" ]
then
	make -f Makefile cleanall
	make -f Makefile debug=0 optimize=1
	build_rpm
	clean_stage_dir
else
	help
	exit 1
fi

