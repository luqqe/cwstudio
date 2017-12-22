/*$T /morse.c GC 1.150 2017-12-22 21:30:10 */

/*$I0 

    This file is part of CWStudio.

    Copyright 2008-2016 Lukasz Komsta, SP8QED

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
	for(i = 0; i < length; i++)
	{
		switch((char) *(text + i))
		{

		/*$3- International Morse Code ===============================================================================*/

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

		case '@':
			strcat(decoded, ".--.-. ");
			break;

		/*$3- Additional four element combinations ===================================================================*/

		case '\xa5':	/* Accented a */
		case '\xb9':
		case '\xa1':
		case '\xb1':
			strcat(decoded, ".-.- ");
			break;

		case '\xd9':	/* Accented u */
		case '\xda':
		case '\xdb':
		case '\xdc':
		case '\xf9':
		case '\xfa':
		case '\xfb':
		case '\xfc':
			strcat(decoded, "..-- ");
			break;

		case '\xd2':	/* Accented o */
		case '\xd3':
		case '\xd4':
		case '\xd5':
		case '\xd6':
		case '\xf2':
		case '\xf3':
		case '\xf4':
		case '\xf5':
		case '\xf6':
			strcat(decoded, "---. ");
			break;

		case '\xaa':	/* S with Cedilla */
		case '\xba':
			strcat(decoded, "---- ");
			break;

		/*$3- Additional five element combinations ===================================================================*/

		case '\x8a':	/* S with circumflex */
		case '\x9a':
			strcat(decoded, "...-. ");
			break;

		case '\xc9':	/* E with acute */
		case '\xe9':
			strcat(decoded, "..-.. ");
			break;

		case '\xa3':	/* L with stroke (Polish) */
		case '\xb3':
		case '\xc8':	/* E with grave */
		case '\xe8':
			strcat(decoded, ".-..- ");
			break;

		case '\xc1':	/* A with grave */
		case '\xe1':
			strcat(decoded, ".--.- ");
			break;

		/*$F    case '\xac': J with circumflex - conflict with acute Z
		case '\xbc':
			strcat(decoded, ".---. ");
			break;
		*/
		case '\xc6':	/* C with circumflex */
		case '\xe6':
			strcat(decoded, "-.-.. ");
			break;

		case '\xa6':	/* H with circumflex */
		case '\xb6':
			strcat(decoded, "-.--. ");
			break;

		case '\x8f':	/* Z with acute (Polish) */
		case '\x9f':
		case '\xac':
		case '\xbc':
			strcat(decoded, "--..- ");
			break;

		case '\xab':	/* G with circumflex */
		case '\xbb':
			strcat(decoded, "--.-. ");
			break;

		case '\xd1':	/* Spanish N with tilde and Polich N with acute */
		case '\xf1':
			strcat(decoded, "--.-- ");
			break;

		/*$3- Additional six element combinations ====================================================================*/

		case '_':
			strcat(decoded, "..--.- ");
			break;

		case '"':
			strcat(decoded, ".-..-. ");
			break;

		case '.':
			strcat(decoded, ".-.-.- ");
			break;

		case '\'':
			strcat(decoded, ".----. ");
			break;

		case '-':
			strcat(decoded, "-....- ");
			break;

		case ';':
			strcat(decoded, "-.-.-. ");
			break;

		case '\xaf':	/* Z with dot (Polish */
		case '\xbf':
			strcat(decoded, "--..-. ");
			break;

		case '(':
		case ')':
			strcat(decoded, "-.--.- ");
			break;

		case ':':
			strcat(decoded, "---... ");
			break;

		/*$3- Seven elements: Polish accented S ======================================================================*/

		case '\x8c':
		case '\x9c':
			strcat(decoded, "...-... ");
			break;

		/*$3- Word space =============================================================================================*/

		case ' ':
		case '\n':
			strcat(decoded, "|");
			break;
		}
	}

	strcat(decoded, "   ");
	return(decoded);
}
