/*$I0 

    This file is part of CWStudio.

    Copyright 2008-2017 Lukasz Komsta, SP8QED

    CWStudio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CWStudio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CWStudio. If not, see <http://www.gnu.org/licenses/>.

 */

#include "cwstudio.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#include <stdio.h>

void cwstudio_getconfigfile(char* filename)
{
#if defined WIN32 && !defined __CYGWIN__
	sprintf(filename, "%s%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"), "\\cwstudio.ini");
#elif defined __DJGPP__
	sprintf(filename, "%s", "cwstudio.cfg");
#else
	if (getenv("HOME") != NULL) sprintf(filename, "%s%s", getenv("HOME"), "/.cwstudio");
#ifdef HAVE_PWD_H
	else if (getpwuid(getuid()) != NULL) sprintf(filename, "%s%s", getpwuid(getuid())->pw_dir, "/.cwstudio");
#endif
	else sprintf(filename, "%s", "./.cwstudio");
#endif
}

