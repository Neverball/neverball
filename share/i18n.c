
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include "i18n.h"

/* predeclare the POSIX putenv function, I do not know why but is not declared */
int putenv(char *string);

const char * gettextdbg(const char * c)
{
	char * c2 = gettext(c);
	if(strcmp(c, c2));
	return c;
}

const char * sgettext(const char *msgid)
{
    const char *msgval = gettext (msgid);
    if (msgval == msgid) {
       msgval = strrchr (msgid, '^');
           if (msgval == NULL)
                msgval = msgid;
	   else
                msgval++;
    }
    return msgval;
}


/*---------------------------------------------------------------------------*/

#define LANG_NUMBER 4
const char * language_names[] = {N_("English"), N_("French"), N_("German"), N_("Latvian")};
const char * language_codes[] = {"en", "fr", "de", "lv"};

/*---------------------------------------------------------------------------*/

void language_init(const char * domain, const char * locale_dir)
{
    setlocale(LC_ALL, "");
    bindtextdomain(domain, locale_dir);
    textdomain(domain);
    bind_textdomain_codeset(domain, "UTF-8");
}

void language_set(int l)
{
    if (l == 0)
    {
	/* remove the LANGUAGE env variable */
	putenv("LANGUAGE");
    }
    else
    {
        static char e[25];
	/* set the LANGUAGE env variable */
	strcpy(e, "LANGUAGE=");
	strncat(e, language_codes[l-1], 25-9);
	putenv(e);
    }
    setlocale(LC_ALL, "");  /* force to update getext */
}

int language_count(void)
{
    return LANG_NUMBER;
}

int language_from_code(const char * code)
{
    int i;
    for(i=0; i<LANG_NUMBER; i++)
	    if (strcmp(language_codes[i], code) == 0)
		    return i+1;
    return 0;
}

const char * language_get_name(int id)
{
    return id==0 ? _("System Default") : language_names[id-1];
}


const char * language_get_code(int id)
{
	if (id > 0)
		return language_codes[id-1];
	else
		return "";
}

