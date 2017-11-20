# $OpenBSD: Makefile.template,v 1.75 2016/03/20 17:19:49 naddy Exp $

COMMENT =	fast, lightweight and portable CW signals generator

DISTNAME =	cwstudio-0.9.5
CATEGORIES =	comms hamradio

HOMEPAGE =	http://cwstudio.sourceforge.net/

MAINTAINER =		Lukasz Komsta <luqqe@users.sourceforge.net>

# GPLv3+
PERMIT_PACKAGE_CDROM =	Yes
PERMIT_PACKAGE_FTP = Yes

PKG_ARCH = 	*
CONFIGURE_STYLE = 	gnu

WANTLIB += ncursesw pthread sndio 
WANTLIB += c m ossaudio stdc++
WANTLIB += wx_base wx_base_net wx_base_odbc wx_base_xml wx_gtk2_adv
WANTLIB += wx_gtk2_aui wx_gtk2_core wx_gtk2_dbgrid wx_gtk2_html
WANTLIB += wx_gtk2_qa wx_gtk2_richtext wx_gtk2_xrc

LIB_DEPENDS = 		x11/wxWidgets
MASTER_SITES =		${MASTER_SITE_SOURCEFORGE:=cwstudio/}

.include <bsd.port.mk>
