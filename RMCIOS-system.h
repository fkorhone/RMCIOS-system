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

RMCIOS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
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
 * Generic channel measurement system implementation header for host programs.
 *
 */
#ifndef channel_system_h
#define channel_system_h

#ifdef __cplusplus
extern "C"
{
#endif

//#include <stdio.h>

#include "RMCIOS-functions.h"

/// @brief Function for initializing dynamic module channels
    void API_ENTRY_FUNC init_channels (const struct context_rmcios *context);

// Set channel system memory pointer.
    void setup_channel_system_data (struct context_rmcios * context, int data_size, void *p_data, int max_classes, int max_channels);

// Get api interface function pointers.
    void setup_rmcios_context (struct context_rmcios * context);

/// @brief Prints channel call parameters to returnv parameter as string. 
///
/// Formatting is similar as in printf.
/// Helper function for implementing channels.
/// @param format character string with same format as c standard library printf
/// @param paramtype type of parameters given to the channel
/// @param returnv pointer to return parameter. destination of the print string.
/// @param num_params number of parameters in param array
/// @param param array of parameters given to the channel
///
/// @snippet examples.c print_param
    void print_param (char *format, enum type_rmcios paramtype,
                      union param_rmcios returnv, int num_params,
                      const union param_rmcios param);

    int execute (const struct context_rmcios *context, const char *input,
                 struct combo_rmcios *returnv);

#ifdef __cplusplus
}
#endif

#endif
