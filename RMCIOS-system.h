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

// ****************************************************************
// Channelsystem public Interface:
// ****************************************************************

    struct exec_queue
    {
        struct exec_queue *next;
        int size;               // Size of payload (exclude next pointer)
        struct context_rmcios *context;
        enum function_rmcios function;
        enum type_rmcios paramtype;
        struct combo_rmcios *returnv;
        int num_params;
        union param_rmcios param;
        // param_data_offset start
        // ....
        // ....
        // ....
        // int param3_offset ;
        // int param2_offset ;
    };

///////////////////////////////////////
// Channel system data pointers      //
// -Function pointers                //
// -Channel member data pointers     //
// -Pointers to linked channel lists //
///////////////////////////////////////
    struct ch_system_data
    {
        class_rmcios *functions;     
        // Channel implementation functions
        unsigned int *channel_functions; 
        // Indices to channel implementation functions
        void **channel_datas;  
        // Pointers to channel member data
        int **channel_linked;  
        // Channel linked channels list 
        // terminated list of channel indices. 
        // inverted 0 represents last allocated memory block for indices
        int *share_registers;  
        // Registers for multithreading handling
        struct exec_queue **exec_queues;        
        // Linked lists of execution queues (next, pointer to data)  
        const char **channel_enum_pattern;
        unsigned int max_channels;
        unsigned int max_classes;
    };

    // memory allocation channel.
    // To be defined outside of channel system operating scope
    void mem_func (void *data, 
                   const struct context_rmcios *context, 
                   int id,
                   enum function_rmcios function, 
                   enum type_rmcios paramtype,
                   struct combo_rmcios *returnv,
                   int num_params,
                   const union param_rmcios param);

// Macro for creating/allocating static channelsystem data 
// CREATE_STATIC_CHANNEL_SYSTEM_DATA(DATANAME, MAX_CLASSES, MAX_CHANNELS)
#define CREATE_STATIC_CHANNEL_SYSTEM_DATA(DATANAME, MAX_CLASSES, MAX_CHANNELS) \
	struct DATANAME ## _channel_system_data { \
		class_rmcios *functions ; \
		int *channel_functions ; \
		void **channel_datas ; \
		int **channel_linked ; \
		int *share_registers ; \
		struct exec_queue **exec_queues ; \
		const char **channel_enum_pattern ; \
		unsigned int max_channels ; \
		unsigned int max_classes ; \
		class_rmcios functions_d[MAX_CLASSES] ; \
		int channel_functions_d[MAX_CHANNELS] ;  \
		void *channel_datas_d[MAX_CHANNELS] ; \
		int *channel_linked_d[MAX_CHANNELS] ; \
		int share_registers_d[MAX_CHANNELS] ; \
		struct exec_queue *exec_queues_d[MAX_CHANNELS] ;  \
		const char *channel_enum_pattern_d[2*MAX_CHANNELS] ;}  \
		DATANAME = {DATANAME.functions_d, \
       		DATANAME.channel_functions_d,  \
		DATANAME.channel_datas_d, \
		DATANAME.channel_linked_d, \
		DATANAME.share_registers_d, \
		DATANAME.exec_queues_d, \
		DATANAME.channel_enum_pattern_d, \
		MAX_CHANNELS, \
		MAX_CLASSES,\
		{NULL},{0},{NULL},{NULL},{0},{NULL},{""} } ;


#define INIT_CHANNEL_SYSTEM(MAX_CLASSES, MAX_CHANNELS) \
   (set_channel_api( get_channel_api_functions() ))  ; \
	CREATE_STATIC_CHANNEL_SYSTEM_DATA(ch_sys_dat,MAX_CLASSES,MAX_CHANNELS) ; \
	set_channel_api( get_channel_api_functions() )  ; \
	set_channel_system_data( (struct ch_system_data *)&ch_sys_dat ) ;

// Set channel system data.
    void set_channel_system_data (struct ch_system_data *p_data);

// Get api interface function pointers.
    const struct context_rmcios *get_rmios_context (void);

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
