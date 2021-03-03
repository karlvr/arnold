
static char *Diss_char(char *pString, const char ch)
{
        pString[0] = ' ';
        ++pString;
        return pString;
}

char *Diss_space(char *pString)
{
	return Diss_char(pString,' ');
}

char *Diss_comma(char *pString)
{
	return Diss_char(pString,',');
}

char *Diss_colon(char *pString)
{
	return Diss_char(pString,':');
}

char *Diss_endstring(char *pString)
{
    pString[0] = '\0';
    ++pString;
    return pString;
}

char *Diss_strcat(char *pString, const char *pToken)
{
    int nTokenLength = strlen(pToken);
    strncpy(pString, pToken, nTokenLength);
    pString += nTokenLength;
    return pString;
}
