##############################################################################
#
#    file                 : Makefile
#    created              : 2016年 07月 28日 星期四 16:41:58 CST
#    copyright            : (C) 2002 xxs
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = test
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = ${ROBOT}.cpp

SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml car1-stock1.rgb logo.rgb
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-test_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-test_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
