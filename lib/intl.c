/*
** $Id$
**
** intl.c: Internationalize interface for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: liupeng .
*/

#include <string.h>

#undef _DEBUG_MSG
#include "pmp.h"
#include "text_id.h"

extern PMP_TEXT_DOMAIN_INFO __pmp_text_domain_en;
extern PMP_TEXT_DOMAIN_INFO __pmp_text_domain_zh_cn;

static PMP_TEXT_DOMAIN_INFO *all_domains [] = {
    &__pmp_text_domain_en,
    &__pmp_text_domain_zh_cn,
};

static PMP_TEXT_DOMAIN_INFO *cur_text_domain = &__pmp_text_domain_en;

/*
 * text_domain: The domain name  to be set such as 'en', 'zh_cn', 'zh_tw',etc
 * Return TRUE when the domain name has been selected as current text domain,
 * FALSE when the default text domain selected.
 */ 
BOOL pmp_select_text_domain (const char *text_domain)
{
    int i;

    for (i = 0; i< TABLESIZE (all_domains); i++) {
        if (strcmp (all_domains[i]->name, text_domain) == 0) {
            cur_text_domain = all_domains[i];
            return TRUE;
        }
    }

    /* set the first text domain as the default domain  */
    cur_text_domain = all_domains[0];
    return FALSE;
}


/*
 * id: The identifier to text which will be translated.
 * Return the pointer to the text string in current text domain.
 */
const char * pmp_get_string_from_id (int id)
{
    _MY_PRINTF ("id = %d, id_base = %d \n", id, cur_text_domain->id_base);
    
    _MY_PRINTF ("str = %s\n", cur_text_domain->text_array[0]);
    
    return cur_text_domain->text_array[ id - cur_text_domain->id_base];
}


