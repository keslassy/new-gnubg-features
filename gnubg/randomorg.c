/*
 * randomorg.c
 *
 * by Michael Petch <mpetch@capp-sysware.com>, 2015.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#include "config.h"

#if defined(LIBCURL_PROTOCOL_HTTPS)

#include <ctype.h>
#include <curl/curl.h>

#include "backgammon.h"
#include "randomorg.h"


/*
 * Fetch random numbers from www.random.org
 */

static size_t
RandomOrgCallBack(void *pvRawData, size_t nSize, size_t nNumMemb, void *pvUserData)
{
    size_t nNewDataLen = nSize * nNumMemb;
    RandomData *randomData = (RandomData *) pvUserData;
    int i;
    int iNumRead = 0;
    char *szRawData = (char *) pvRawData;

    for (i = 0; i < nNewDataLen; i++) {
        if ((szRawData[i] >= '0') && (szRawData[i] <= '5')) {
            /* Get a number */
            randomData->anBuf[iNumRead] = 1 + (unsigned int) (szRawData[i] - '0');
            iNumRead++;
        } else if (isspace(szRawData[i])) {
            /* Skip spaces */
            continue;
        } else {
            /* Any other characters suggest invalid data
             * exit with error (0) */
            return 0;
        }
    }

    randomData->nNumRolls += iNumRead;
    return nNewDataLen;
}

static RandomData randomData = { 0, -1 };

unsigned int
getDiceRandomDotOrg(void)
{
    CURL *curl_handle;
    CURLcode nRetVal;
    static char szHTTP[] =
        "https://" RANDOMORGSITE "/integers/?num=" BUFLENGTH_STR "&min=0&max=5&col=1&base=10&format=plain&rnd=new";
    static char szUserAgent[] = "GNUBG/" VERSION " (email: " SUPPORT_EMAIL ")";

    if ((randomData.nCurrent >= 0) && (randomData.nCurrent < randomData.nNumRolls)) {
        return randomData.anBuf[randomData.nCurrent++];
    }

    randomData.nNumRolls = 0;
    outputf(_("Fetching %d random numbers from <%s>\n"), BUFLENGTH, RANDOMORGSITE);
    outputx();

    /* Initialize the curl session */
    curl_handle = curl_easy_init();

#ifdef WIN32
    /* Set location of certificate bundle */
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, ".\\etc\\ssl\\ca-bundle.crt");
#endif

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_URL, szHTTP);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, RandomOrgCallBack);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) &randomData);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, szUserAgent);

    nRetVal = curl_easy_perform(curl_handle);

    /* check if an error condition exists */
    if (nRetVal != CURLE_OK) {
        curl_easy_cleanup(curl_handle);
        outputerrf("curl_easy_perform() failed: %s\n", curl_easy_strerror(nRetVal));
        randomData.nCurrent = -1;
        return 0;
    }

    /* Cleanup curl session */
    curl_easy_cleanup(curl_handle);

    randomData.nCurrent = 1;
    return randomData.anBuf[0];
}

#endif
