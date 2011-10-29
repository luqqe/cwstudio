/*$T src/morse.c GC 1.140 10/28/11 20:48:37 */

/*$I0

    This file is part of CWStudio.

    Copyright 2008-2011 Lukasz Komsta, SP8QED

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
#include "cwgen.h"

/*
 =======================================================================================================================
    Convert a text to its Morse code
 =======================================================================================================================
 */
char *cw_encode(const char *text)
{
	/*~~~~~~~~~~~~~~*/
	char	*decoded;
	int		i, length;
	/*~~~~~~~~~~~~~~*/

	length = strlen(text);
	if((decoded = (char *) cw_malloc(10 * length * sizeof(char))) == NULL) return(NULL);
	strncpy(decoded, "\0", 1);
	strcat(decoded, "   ");
	for(i = 0; i < length; i++) {
		switch((char) *(text + i))
		{
		case 'a':
		case 'A':
			strcat(decoded, ".- ");
			break;

		case 'b':
		case 'B':
			strcat(decoded, "-... ");
			break;

		case 'c':
		case 'C':
			strcat(decoded, "-.-. ");
			break;

		case 'd':
		case 'D':
			strcat(decoded, "-.. ");
			break;

		case 'e':
		case 'E':
			strcat(decoded, ". ");
			break;

		case 'f':
		case 'F':
			strcat(decoded, "..-. ");
			break;

		case 'g':
		case 'G':
			strcat(decoded, "--. ");
			break;

		case 'h':
		case 'H':
			strcat(decoded, ".... ");
			break;

		case 'i':
		case 'I':
			strcat(decoded, ".. ");
			break;

		case 'j':
		case 'J':
			strcat(decoded, ".--- ");
			break;

		case 'k':
		case 'K':
			strcat(decoded, "-.- ");
			break;

		case 'l':
		case 'L':
			strcat(decoded, ".-.. ");
			break;

		case 'm':
		case 'M':
			strcat(decoded, "-- ");
			break;

		case 'n':
		case 'N':
			strcat(decoded, "-. ");
			break;

		case 'o':
		case 'O':
			strcat(decoded, "--- ");
			break;

		case 'p':
		case 'P':
			strcat(decoded, ".--. ");
			break;

		case 'q':
		case 'Q':
			strcat(decoded, "--.- ");
			break;

		case 'r':
		case 'R':
			strcat(decoded, ".-. ");
			break;

		case 's':
		case 'S':
			strcat(decoded, "... ");
			break;

		case 't':
		case 'T':
			strcat(decoded, "- ");
			break;

		case 'u':
		case 'U':
			strcat(decoded, "..- ");
			break;

		case 'v':
		case 'V':
			strcat(decoded, "...- ");
			break;

		case 'w':
		case 'W':
			strcat(decoded, ".-- ");
			break;

		case 'x':
		case 'X':
			strcat(decoded, "-..- ");
			break;

		case 'y':
		case 'Y':
			strcat(decoded, "-.-- ");
			break;

		case 'z':
		case 'Z':
			strcat(decoded, "--.. ");
			break;

		case '0':
			strcat(decoded, "----- ");
			break;

		case '1':
			strcat(decoded, ".---- ");
			break;

		case '2':
			strcat(decoded, "..--- ");
			break;

		case '3':
			strcat(decoded, "...-- ");
			break;

		case '4':
			strcat(decoded, "....- ");
			break;

		case '5':
			strcat(decoded, "..... ");
			break;

		case '6':
			strcat(decoded, "-.... ");
			break;

		case '7':
			strcat(decoded, "--... ");
			break;

		case '8':
			strcat(decoded, "---.. ");
			break;

		case '9':
			strcat(decoded, "----. ");
			break;

		case '!':
			strcat(decoded, "--..-- ");
			break;

		case '?':
			strcat(decoded, "..--.. ");
			break;

		case '/':
			strcat(decoded, "-..-. ");
			break;

		case '=':
			strcat(decoded, "-...- ");
			break;

		case '#':
			strcat(decoded, "---- ");
			break;

		case '%':
			strcat(decoded, ".-.-. ");
			break;

		case '|':
			strcat(decoded, "-.--. ");
			break;

		case '$':
			strcat(decoded, "----.- ");
			break;

		case '&':
			strcat(decoded, "-.-..-.. ");
			break;

		case '^':
			strcat(decoded, "..-- ");
			break;

		case '*':
			strcat(decoded, ".-.- ");
			break;

		case ' ':
		case '\n':
			strcat(decoded, "|");
			break;
		}
	}

	strcat(decoded, "   ");
	return(decoded);
}
