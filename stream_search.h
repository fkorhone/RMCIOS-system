/*
RMCIOS - Reactive Multipurpose Control Input Output System
Copyright (c) 2018 Frans Korhonen

RMCIOS was originally developed at Institute for Atmospheric 
and Earth System Research / Physics, Faculty of Science, 
University of Helsinki, Finland

Assistance, experience and feedback from following persons have been 
critical for development of RMCIOS: Erkki Siivola, Juha Kangasluoma, 
Lauri Ahonen, Ella Häkkinen, Pasi Aalto, Joonas Enroth, Runlong Cai, 
Markku Kulmala and Tuukka Petäjä.

This file is part of RMCIOS. This notice was encoded using utf-8.

RMCIOS is free software: you can redistribute this file and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RMCIOS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public Licenses
along with RMCIOS.  If not, see <http://www.gnu.org/licenses/>.
*/

/* 
 * Search tool for matching multiple words from character based stream.
 * 
 * Changelog data,who,what
 */
#ifndef stream_search_h
#define stream_search_h
/*
9.2.2013 Frans Korhonen
Example usage:

void main(void)
{
    // table containing whole search pattern
    const char *pattern[32]={
                        "one" ,(const char *)1,
                        "ones",(const char *)2,
                        "tree",(const char *)3,
                        "four",(const char *)4,
                        "five",(const char *)5,
                        "six",(const char *)6,
                        "" } ; // empty marks end of search pattern

    const char *str="ones" ; // Will match on last character of "one" and "ones"
    unsigned int states ;
    const char *result ;
    states=~(0) ;
    int i=0 ;
    while( states>0 ) 
    {
        if(str[i]==0) break ; 
        if((result=search(i,&states,pattern,str[i]))!=0 ) 
        {
            printf("found:%d\n",((int)result) ) ;
        }
        i++;
    }
}
*/

/* Compare strings (from glibc)*/
static inline int string_compare (const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;

    do
      {
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0')
              return c1 - c2;
      }
    while (c1 == c2);

    return c1 - c2;
}

// Search function
// iteration is the number of character in the stream to be matched
// states is the register containing the results for the 
static inline const char *search (unsigned int iteration,
                                  unsigned int *states, const char **pattern,
                                  char c)
{
   unsigned int i = 0;
   const char *match = 0;
   while (pattern[i][0] != 0)
   {
      // States span over multiple unsigned integers.
      if ((i >> 1) >= (sizeof (unsigned int) << 3)) 
      {
         states++;
         pattern += (sizeof (unsigned int) << 3) * 2;
         i -= (sizeof (unsigned int) << 3) * 2;
      }

      // word is active
      if ((((*states) >> (i >> 1)) & 1) == 1)       
      {
         // last char of word
         if (pattern[i][iteration + 1] == 0)     
         {
            // remove word from active words.
            *states &= (~(1 << (i >> 1)));    
            if (c == pattern[i][iteration])
            {
               // MATCH!
               match = pattern[i + 1];     
            }
         }
         else if (c != pattern[i][iteration])
         {
            // remove word from active words.
            *states &= (~(1 << (i >> 1)));      
         }
      }
      i += 2;
   }
   return match;
}

static inline void add_search_word (const char **pattern, const char *keyword,
                                    const char *result)
{
    int i = 0;
    while (pattern[i][0] != 0)
      {
          if (string_compare (pattern[i], keyword) == 0)
            {
                pattern[i + 1] = result;
                // keyword already in pattern -> only update the result
                return;  
            }
          i += 2;              
      }
    pattern[i] = keyword;
    pattern[i + 1] = result;
    pattern[i + 2] = "";
}

static inline void add_search_word_synonym (const char **pattern,
                                            const char *keyword,
                                            const char *synonym)
{
    int i = 0;
    while (pattern[i][0] != 0)  
      // Find result for the keyword.
      {
          if (string_compare (pattern[i], keyword) == 0)
            {
                // Add the synonym.
                add_search_word (pattern, synonym, pattern[i + 1]);     
                break;
            }
          i += 2;               
      }
}

static inline void remove_search_word (const char **pattern,
                                       const char *keyword)
{
    int i = 0;
    while (pattern[i][0] != 0)
      {
          if (string_compare (pattern[i], keyword) == 0)
            {
                // keyword found in pattern -> remove the result
                pattern[i + 1] = 0;     
                break;
            }
          i += 2;               
      }
}

static inline int count_search_words(const char **pattern)
{
   int words = 0;
   int i = 0;
   while (pattern[i][0] != 0)
   {
      words++;
      i += 2;               
   }
   return words;
}

#endif
