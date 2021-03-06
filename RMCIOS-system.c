﻿/* 
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
 * Generic RMCIOS-system implementation.
 * Changelog: (date,who,description)
 */

#include "RMCIOS-system.h"
#include "stream_search.h"
#include "shared_resource.h"
#include "convert.h"

// Add new class function to the system. return the id.
int add_class_func (const struct context_rmcios *context,
                    class_rmcios class_func);

// Add channel to specified chass fucnction.
int add_channel (const struct context_rmcios *context, int class_function,
                 void *data);
// Update channel to class & data
int update_channel (const struct context_rmcios *context, int channel_id,
                    int class_function, void *data);
// Add channel to already known id
//int   add_static_channel(int class_function,void *data,int id) ;
// Add name for a channel id.
void add_channel_enum (const struct context_rmcios *context, const char *name,
                       int id);
//
int link_channel_param (const struct context_rmcios *context,
                        enum type_rmcios paramtype, int num_params,
                        const union param_rmcios param);
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

// API FUNCTION
void run_channel_ (struct ch_system_data *data,
                   const struct context_rmcios *context,
                   int channel,
                   enum function_rmcios function,
                   enum type_rmcios paramtype,
                   struct combo_rmcios *returnv,
                   int num_params, const union param_rmcios param);

struct context_rmcios funcs = {
   sizeof (struct context_rmcios),
   (class_rmcios)run_channel_,
   0,   // System data NULL
   1,   // id - Channel for reading channel identifier number.
   2,   // name - Channel for reading channel name.
   3,   // mem - Channel for allocating memory.
   4,   // quemem - Channel for allocating temporary memory.
   5,   // errors - Channel for error messages.
   6,   // warning - Channel for warning messages.
   7,   // report - Channel for reporting/logging status messages.
   8,   // control - Channel for text based control.
   9,   // link - Channel for creating links.
   10,  // linked - Channel for interacting with linked channels.
   11,  // create - Channel for creating channels.
   12,  // convert - Channel for converting parameters.
};

struct control_data
{
   struct context_rmcios context;
   int index;                   // Command line
   int direct_channel;
   char command[256];           // Command line buffer
} cdata =
{
   {
   0}, 0, 0,
   {
0}};


// Channel system channel for listing all channels in the system
void channels_class_func (void *data, const struct context_rmcios *context,
                          int id, enum function_rmcios function,
                          enum type_rmcios paramtype,
                          struct combo_rmcios *returnv, 
                          int num_params,
                          const union param_rmcios param);

void as_class_func (void *data, const struct context_rmcios *context, int id,
                    enum function_rmcios function, enum type_rmcios paramtype,
                    struct combo_rmcios *returnv, 
                    int num_params,
                    const union param_rmcios param);

void lock_func (void *data, const struct context_rmcios *context, int id,
                enum function_rmcios function, enum type_rmcios paramtype,
                struct combo_rmcios *returnv, 
                int num_params,
                const union param_rmcios param);

void name_func (void *data, const struct context_rmcios *context, int id,
                enum function_rmcios function, enum type_rmcios paramtype,
                struct combo_rmcios *returnv, 
                int num_params,
                const union param_rmcios param);

void id_func (void *data, const struct context_rmcios *context, int id,
              enum function_rmcios function, enum type_rmcios paramtype,
              struct combo_rmcios *returnv, 
              int num_params,
              const union param_rmcios param);

void control_class_func (struct control_data *this,
                         const struct context_rmcios *context, int id,
                         enum function_rmcios function,
                         enum type_rmcios paramtype,
                         struct combo_rmcios *returnv, 
                         int num_params,
                         const union param_rmcios param);

void link_func (void *data, const struct context_rmcios *context, int id,
                enum function_rmcios function, enum type_rmcios paramtype,
                struct combo_rmcios *returnv, 
                int num_params,
                const union param_rmcios param);

void linked_func (void *data, const struct context_rmcios *context, int id,
                  enum function_rmcios function, enum type_rmcios paramtype,
                  struct combo_rmcios *returnv, 
                  int num_params,
                  const union param_rmcios param);

void create_class_func (void *data, 
                        const struct context_rmcios *context, 
                        int id,
                        enum function_rmcios function, 
                        enum type_rmcios paramtype,
                        struct combo_rmcios *returnv, 
                        int num_params,
                        const union param_rmcios param);

int channels = 0;

char *memcopy (char *dest, const char *src, int count)
{
   int i;
   for (i = 0; i < count; i++)
   {
      dest[i] = src[i];
   }
   return dest;
}

void setup_channel_system_data (struct context_rmcios * context, int data_size, void *vpdata, int num_classes, int num_channels)
{
    context->data = vpdata;
    struct ch_system_data *p_data = vpdata;
    void *p = (((struct ch_system_data *)vpdata) + 1); 
    if (p - vpdata < data_size)
    {
        p_data->functions = p;
    }

    p = (((class_rmcios *)p) + num_classes);
    if (p - vpdata < data_size)
    {
        p_data->max_classes = num_classes;
        p_data->channel_functions = p; 
    }
    
    p = (((int *)p) + num_channels);
    if (p - vpdata < data_size)
    {
        p_data->channel_datas = p;  
    }

    p = (((void **)p) + num_channels);
    if (p - vpdata < data_size)
    {
        p_data->channel_linked = p;  
    }
  
    p = (((int **)p) + num_channels);
    if (p - vpdata < data_size)
    {
        p_data->share_registers = p;  
    }
    
    p = (((int *)p) + num_channels);
    if (p - vpdata < data_size)
    {
        p_data->exec_queues = p;  
    }

    p = (((struct exec_queue **)p) + num_channels);
    if (p - vpdata < data_size)
    {
        p_data->channel_enum_pattern = p;  
    }

    p = (((char **)p) + num_channels * 2);    
    if ((p - vpdata) < data_size)
    {
        int i;
        for(i = 0; i < num_channels * 2; i++)
        {
            p_data->channel_enum_pattern[i] = "";
        }

        p_data->max_channels = num_channels;
    }

    context->version = sizeof (struct context_rmcios);
    context->run_channel = (class_rmcios)run_channel_;
    context->id = 1;       // id - Channel for reading channel identifier number.
    context->name = 2;     // name - Channel for reading channel name.
    context->mem = 0;
    context->quemem = 0;
    context->errors = 0;
    context->warning = 0;
    context->report = 0;
    context->control = 8;  // control - Channel for text based control.
    context->link = 9;     // link - Channel for creating links.
    context->linked = 10;  // linked - Channel for interacting with linked channels.
    context->create = 11;  // create - Channel for creating channels.
    context->convert = 12;

    // INTERFACE channels : 
    p_data->channel_functions[context->name] = add_class_func (context, (class_rmcios) name_func);
    p_data->channel_functions[context->id] = add_class_func (context, (class_rmcios) id_func);
    p_data->channel_functions[context->control] = add_class_func (context, (class_rmcios) control_class_func);
    p_data->channel_functions[context->link] = add_class_func (context, (class_rmcios) link_func);
    p_data->channel_functions[context->linked] = add_class_func (context, (class_rmcios) linked_func);
    p_data->channel_functions[context->create] = add_class_func (context, (class_rmcios) create_class_func);
    p_data->channel_functions[context->convert] = add_class_func (context, (class_rmcios) convert_func);
    p_data->channel_datas[context->control] = &cdata;

    p_data->share_registers[context->id] = -1;
    // Simultanous calls to linked in the whole system allowed.
    p_data->share_registers[context->linked] = -1;
    p_data->share_registers[context->create] = -1;
    p_data->share_registers[context->link] = -1;
    // Simultanous calls to control allowed (Call from self needs to be working.
    p_data->share_registers[context->control] = -1;
    p_data->share_registers[context->convert] = -1;

    /* p_data->share_registers[context->mem] = -1;
       p_data->share_registers[context->report] = -1;
       p_data->share_registers[context->warning] = -1;
       p_data->share_registers[context->errors] = -1;
     */
    cdata.context = *context;
}

void setup_rmcios_context (struct context_rmcios* context)
{
   add_channel_enum (context, "control", context->control);

   channels = create_channel_str (context, "channels",
                                  (class_rmcios) channels_class_func, 0);
   
   create_channel_str (context, "name", (class_rmcios) name_func, 0);
   create_channel_str (context, "id", (class_rmcios) id_func, 0);
   create_channel_str (context, "as", (class_rmcios) as_class_func, 0);
   create_channel_str (context, "lock", (class_rmcios) lock_func, 0);
}

// *******************************************************************
// API FUNCTION implementation
// *******************************************************************


struct exec_queue *allocate_exec_queue_item (const struct context_rmcios
                                             *context,
                                             enum function_rmcios function,
                                             enum type_rmcios paramtype,
                                             struct combo_rmcios *returnv,
                                             int num_params,
                                             const union param_rmcios param)
{
   int i;
   struct exec_queue *newitem;

   // Determine needed queue item size:
   int size = sizeof (struct exec_queue);

   // Calculate space for the parameters:
   if (paramtype == buffer_rmcios || paramtype == binary_rmcios)
   {
      // Space for array structures 
      size += sizeof (struct buffer_rmcios) * num_params;

      // Space for actual data:
      for (i = 0; i < num_params; i++)
      {
         size += param.bv[i].length;
      }
   }
   else if (paramtype == channel_rmcios)
   {
      size += num_params * sizeof (int);
   }
   else if (paramtype == int_rmcios)
   {
      size += num_params * sizeof (int);
   }
   else if (paramtype == float_rmcios)
   {
      size += num_params * sizeof (float);
   }
   
   // Allocate memory for the new item:
   newitem = allocate_storage (context, size, 0);

   // Save the execute command data
   newitem->next = 0;
   newitem->size = size - sizeof (struct exec_queue *);
   newitem->context = (void *) context;
   newitem->function = function;
   newitem->paramtype = paramtype;
   if (paramtype != buffer_rmcios)
   {
      newitem->returnv = returnv;
   }
   else
   {
      newitem->returnv = 0;
   }
   char *dst = (char *)(newitem) + sizeof(struct exec_queue) ;
   newitem->num_params = num_params;
   newitem->param.p = dst;
   
   // Copy parameter data:
   if (paramtype == buffer_rmcios || paramtype == binary_rmcios)
   {
      int offset = 0;
      // Copy buffer parameters:

      memcopy (dst, param.bv,
               num_params * sizeof (struct buffer_rmcios));

      dst = dst + num_params * sizeof (struct buffer_rmcios);

      struct buffer_rmcios *par = param.bv;
      // Copy data
      for (i = 0; i < num_params; i++)
      {
         memcopy (&dst[offset], par[i].data, par[i].length);
         newitem->param.bv[i].data = &dst[offset];
         newitem->param.bv[i].size = 0;
         newitem->param.bv[i].length = par[i].length;
         newitem->param.bv[i].length = par[i].required_size;
         newitem->param.bv[i].trailing_size = 0;
         offset += par[i].length;
      }
   }
   else if (paramtype == int_rmcios)
   {
      memcopy (newitem->param.p, param.p, num_params * sizeof (int));
   }
   else if (paramtype == float_rmcios)
   {
      memcopy (newitem->param.p, param.p, num_params * sizeof (float));
   }
   else
      write_str (context, context->report, "other param!\r\n", 0);

   return newitem;
}

void link_func (void *data,
                const struct context_rmcios *context, int id,
                enum function_rmcios function,
                enum type_rmcios paramtype,
                struct combo_rmcios *returnv,
                int num_params, const union param_rmcios param)
{
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "link channel - channel for linking channels\r\n"
                     "read link channel\r\n"
                     " -returns channel link representation channel\r\n"
                     "write link channel to_channel\r\n"
                     "write link channel <from_commmand> to_channel\r\n"
                     "write link channel to_channel <with_commmand>\r\n"
                     "write link channel <from_commmand> to_channel"
                     " <with_commmand>\r\n");
      break;

   case read_rmcios:
      // Negative id is the link representative channel
      return_int (context, returnv,
                  ~param_to_int (context, paramtype, param, 0));
      break;

   case write_rmcios:
      if (num_params < 2)
         break;
      {
         link_channel_param (context, paramtype, num_params, param);
      }
      break;
   }
}

void linked_func (void *data, const struct context_rmcios *context,
                  int channel, enum function_rmcios function,
                  enum type_rmcios paramtype, 
                  struct combo_rmcios *returnv,
                  int num_params, const union param_rmcios param)
{
   switch (function)
   {
   case help_rmcios:
     /* return_string (context, returnv,
                     "linked channel - channel for managing calls to"
                     " linked channels. \r\n"
                     "setup linked channel data...\r\n"
                     "write linked channel data...\r\n"
                     "read linked channel data...\r\n");*/
   case setup_rmcios:
   case read_rmcios:
   case write_rmcios:
      //if(num_params<1) break  ;
      if (channel > ((struct ch_system_data *) (context->data))->max_channels)
      {
         write_str (context, context->warning, "Channel id too big!\r\n", 0);
         return;
      }
      if (((struct ch_system_data *) context->data)->
          channel_functions[channel] == 0)
      {
         return;
      }
      if (((struct ch_system_data *) context->data)->
          channel_functions[channel] >=
          ((struct ch_system_data *) context->data)->max_classes)
      {
         write_str (context, context->errors,
                    "Channel class > max_classes\r\n", 0);
         return;
      }

      int *link =
         ((struct ch_system_data *) context->data)->channel_linked[channel];
      if (link == 0)
         return;        // No links for this channel.
      while ((*link) != 0)
      { // go until 0
         if ((*(link)) == ~0)
         {      // end of array go to next if exists
            link++;
            if ((int *) *link == 0)
               break;   // no more channel arrays
            else
               link = (int *) *link;
         }
         else
         {
            int from_function = ((*link) >> (32 - 3)) & 0x07;
            int to_function = ((*link) >> (32 - 6)) & 0x07;
            int linked_channel = *link & 0x03ff;
            // Filter called functions to execute with this link:
            if (from_function == 0 || from_function == function)
            {
               // run the called function
               if (to_function == 0)
               {
                  run_channel (context, linked_channel,
                                        function, paramtype,
                                        returnv, num_params, param);
               }
               else
               {
                  // redirected function
                  run_channel (context, linked_channel,
                                        to_function, paramtype,
                                        returnv, num_params, param);
               }
            }
            link++;
         }
      }
      break;
   }
}

// Lock channel for an function:
// returns 1 on success. After success the channel MUST be unlocked after use.
int lock_channel (const struct context_rmcios *context, int channel,
                  enum function_rmcios function)
{
	
   // Check if channel is locked:
   if (((struct ch_system_data *) context->data)->share_registers[channel] >= 0)
   {
      if (function == read_rmcios || function == create_rmcios)
      { // SEMI-LOCKING operation
         // Test/Lock the channel for reading:
         if (request_read_resource
             (&((struct ch_system_data *) context->data)->
              share_registers[channel]) == 0)
         {
            return 0;
         }
      }
      else if (function == write_rmcios || function == setup_rmcios)
      { // LOCKING operation
         // Test/Lock the channel for writing:
         if (request_write_resource
             (&((struct ch_system_data *) context->data)->
              share_registers[channel]) == 0)
         {
            return 0;
         }
      }
   }

   // Successfully locked channel for the function
   return 1;
}

// Unlock channel after use:
void unlock_channel (const struct context_rmcios *context, int channel,
                     enum function_rmcios function)
{
   // Unlock the channel
   if (((struct ch_system_data *) context->data)->share_registers[channel] >= 0)
   {
      if (function == read_rmcios || function == create_rmcios)
      { // SEMI-LOCKING operation
         // Unlock the channel from reading
         stop_read_resource (&
                             ((struct ch_system_data *) context->
                              data)->share_registers[channel]);
      }
      else if (function == write_rmcios || function == setup_rmcios)
      {
         // Unlock the channel from writing
         stop_write_resource (&
                              ((struct ch_system_data *) context->
                               data)->share_registers[channel]);
      }
   }
}

void run_exec_queue (const struct context_rmcios *context, int channel)
{
   int func_index =
      ((struct ch_system_data *) context->data)->channel_functions[channel];
   void *channel_data =
      ((struct ch_system_data *) context->data)->channel_datas[channel];

   // Check the execute queue
   if (((struct ch_system_data *) context->data)->exec_queues[channel] == 0)
      return;
   else
   {
      do
      {
         struct exec_queue *que =
            ((struct ch_system_data *) context->data)->exec_queues[channel];

         if (lock_channel (context, channel, que->function) == 0)
         {
            return;
         }
         // Pop the exec queue (with atomic race condition check):
         if (__sync_bool_compare_and_swap(
              (int *) &((struct ch_system_data *) context->data)->
              exec_queues[channel], que, que->next) != 1)
         {
            // Racing condition
            write_str (context, context->report,
                       "ERRROR! exec queue RACE!\r\n", 0);
            // Unlock the channel
            unlock_channel(context, channel, que->function) ;
            return;
         }

         // Execute from queue
         struct ch_system_data *cdata ; 
		 cdata= ((struct ch_system_data *)  context->data);
		 class_rmcios about_to_call=cdata->functions[func_index] ;
		 about_to_call(channel_data, que->context,
                                   channel, que->function,
                                   que->paramtype, que->returnv,
                                   que->num_params, que->param);

         // Unlock the channel
         unlock_channel (context, channel, que->function);

         // free the memory
         free_storage (context, que, 0);
         //free(que) ;
      }
      while (((struct ch_system_data *) context->data)
             ->exec_queues[channel] != 0);

   }
}

// API FUNCTION
void run_channel_ (struct ch_system_data *data,
                   const struct context_rmcios *context,
                   int channel_orig,
                   enum function_rmcios function,
                   enum type_rmcios paramtype,
                   struct combo_rmcios *returnv,
                   int num_params, const union param_rmcios param)
{
   int channel = channel_orig;

   int func_index;
   unsigned int call_channel;
   void *channel_data;
   int execute_later = 0;

   // check for linked calls (chnnel id<0)
   if (channel < 0)
   {
      call_channel = ~channel;
      channel = context->linked;
   }
   else
      call_channel = channel;

   if (call_channel > data->max_channels)
   {
      write_str (context, context->warning, "Channel id too big!\r\n", 0);
      return;
   }
   if (data->channel_functions[channel] == 0)
   {
      return;
   }
   if (data->channel_functions[channel] >= data->max_classes)
   {
      write_str (context, context->errors,
                 "Channel class > max_classes\r\n", 0);
      return;
   }

   func_index = data->channel_functions[channel];
   channel_data = data->channel_datas[channel];

   // Check and run pending operations from execution queue:
   run_exec_queue (context, channel);

   execute_later = !lock_channel (context, channel, function);

   // Channel was locked -> Add call to queue
   if (execute_later == 1)
   {
      if (data->exec_queues[channel] == 0)
      {
         struct exec_queue *newque ;
         newque= allocate_exec_queue_item (context, function, paramtype,
                                           returnv, num_params, param);
         if (__sync_bool_compare_and_swap
             ( (int *)&(data->exec_queues[channel]), 
             0, newque) == 0)
         {
            write_str (context, context->report, "EXEC RACE!\r\n", 0);
         }
      }
      else
      {
         struct exec_queue *newque;
         struct exec_queue *que = data->exec_queues[channel];
         // Locate end of the que
         while (que->next != 0)
            que = que->next;
         newque =
            allocate_exec_queue_item (context, function, paramtype,
                                      returnv, num_params, param);
         // Add the item to end of queue
         if (__sync_bool_compare_and_swap (&que->next, 0, newque) == 0)
            write_str (context, context->report, "EXEC RACE!\r\n", 0);
      }
   }
   else
   {
      // Call channel
      struct ch_system_data * cdata ;
      cdata = data;
      cdata->functions[func_index] (channel_data, context, call_channel,
                                function, paramtype, returnv, num_params,
                                param);

      // Unlock the channel
      unlock_channel (context, channel, function);

      // Check and run pending operations from execution queue:
      run_exec_queue (context, channel);
   }
}

// returns identifier for the link
int link_channel_param (const struct context_rmcios *context,
                        enum type_rmcios paramtype, int num_params,
                        const union param_rmcios param)
{

   // Parameter conversion :
   if (num_params < 2)
      return 0; // No  channel&destination channel specified
   int channel = param_to_int (context, paramtype, param, 0);
   if (channel == 0)
      return 0;
   int to_channel = param_to_int (context, paramtype, param, 1);
   int from_function = 0;
   int to_function = 0;
   if (num_params == 3) // Single function specified
   {
      if (to_channel == 0)
         to_channel = param_to_int (context, paramtype, param, 2);
      from_function = param_to_function (context, paramtype, param, 1);
      to_function = param_to_function (context, paramtype, param, 2);
   }
   if (num_params == 4) // Both functions specified
   {
      from_function = param_to_function (context, paramtype, param, 1);
      to_channel = param_to_int (context, paramtype, param, 2);
      to_function = param_to_function (context, paramtype, param, 3);
   }

   /// ACTUAL LINKAGE:
   int channel_linked = ~channel;       // convert to link id from channel id
   int *link =
      ((struct ch_system_data *) context->data)->channel_linked[channel];

   if (link == 0)       // allocate initial link space
   {
      int i;
      // 2 links + end of array + pointer to next array:
      link =
         (int *) allocate_storage (context, 3 * sizeof (int) + sizeof (int), 0);
      if (link == 0)
      {
         write_str (context, context->errors,
                    "FATAL ERROR! Could not allocate memory for links!\n", 0);
         return 0;
      }
      for (i = 0; i < 2; i++)
         link[i] = 0;
      link[i++] = ~0;   // end of array
      *(int *) (link + i) = (int) 0;    // pointer to next array
      ((struct ch_system_data *) context->data)->channel_linked[channel] = link;
   }

   if (to_channel == 0)
      *link = 0;        // detach all channels
   int *start = link;
   while ((*link) != 0) // go to first free slot
   {
      if (*link == to_channel)
         return 0;      // already linked -> nothing to do ; 
      if (*link++ == (~0))      // last allocated space in the array
      {
         if ((int *) *link != 0)
         {
            // follow to next array of linked channels   
            link = (int *) *link;
            start = link;
         }
         else
         {      // allocate new data
            int i;
            int olditems = ((link - start) / sizeof (int) + 1);

            // allocate more memory for links (2x older)
            int *newlinks = (int *) allocate_storage (context,
                                                      2 * sizeof (int) *
                                                      olditems + sizeof (int) +
                                                      sizeof (int), 0);
            if (newlinks == 0)
            {
               write_str (context, context->errors,
                          "FATAL ERROR! Could not allocate more memory for links!\n",
                          0);
               return 0;
            }
            for (i = 0; i < 2 * olditems; i++)
               newlinks[i] = 0; //
            newlinks[i++] = ~0; // end of array
            *(int *) (newlinks + i) = 0;        // pointer to next array
            *(int *) (link) = (int) newlinks;   // chain to new array

            link = newlinks;
         }
      }
   }
   // add the new linked channel 
   // embed the from and to link information to most significant bits.
   *link = to_channel | (from_function << ((sizeof (int) * 8) - 3))
      | (to_function << ((sizeof (int) * 8) - 6));

   return channel_linked;
}

// *******************************************************
// IMPLEMENTATION FUNCTIONS:
// *******************************************************
int add_class_func (const struct context_rmcios *context,
                    class_rmcios class_func)
{
   unsigned int i;
   // Search if already exists :
   for (i = 1; i < ((struct ch_system_data *) (context->data))->max_classes;
        i++)
   {
      if (((struct ch_system_data *) (context->data))->functions[i] ==
          class_func)
         return i;
   }
   // Create new
   for (i = 1; i < ((struct ch_system_data *) (context->data))->max_classes;
        i++)
   {
      if (((struct ch_system_data *) (context->data))->functions[i] == 0)
         break;
   }
   if (i >= ((struct ch_system_data *) (context->data))->max_classes - 1)
   {
      write_str (&funcs, funcs.errors, "Too many channel classes!\r\n", 0);
      return 0;
   }
   ((struct ch_system_data *) (context->data))->functions[i] = class_func;
   return i;
}

int add_channel (const struct context_rmcios *context, int class_function,
                 void *data)
{
   unsigned int i;
   if (class_function == 0)
      return 0;
   for (i = 1; i < ((struct ch_system_data *) context->data)->max_channels; i++)
   {
      if (((struct ch_system_data *) context->data)->channel_functions[i] == 0)
         break;
   }
   if (i == ((struct ch_system_data *) context->data)->max_channels - 1)
   {
      write_str (&funcs, funcs.errors, "ERROR! too many channels!\r\n", 0);
      return 0;
   }

   ((struct ch_system_data *) context->data)->channel_functions[i] =
      class_function;
   ((struct ch_system_data *) context->data)->channel_datas[i] = data;
   return i;
}

int update_channel (const struct context_rmcios *context, int channel_id,
                    int class_function, void *data)
{
   if (class_function == 0)
      return 0;
   if (channel_id >=
       ((struct ch_system_data *) context->data)->max_channels - 1)
   {
      write_str (&funcs, funcs.errors,
                 "ERROR! too big channel id. Unable to update!r\n", 0);
      return 0;
   }
   ((struct ch_system_data *) context->data)->channel_functions[channel_id] =
      class_function;
   ((struct ch_system_data *) context->data)->channel_datas[channel_id] = data;
   return channel_id;
}

void add_channel_enum (const struct context_rmcios *context, const char *name,
                       int id)
{
   add_search_word (((struct ch_system_data *) context->data)->
                    channel_enum_pattern, name, (const char *) id);
}

/////////////////////////////////////////////////////
//! Channel for listing all channels in the system //
/////////////////////////////////////////////////////
void channels_class_func (void *data,
                          const struct context_rmcios *context, int id,
                          enum function_rmcios function,
                          enum type_rmcios paramtype,
                          struct combo_rmcios *returnv,
                          int num_params, const union param_rmcios param)
{
   switch (function)
   {
   case help_rmcios:
      {
         const char **s;
         s = ((struct ch_system_data *) context->data)->channel_enum_pattern;
         while ((*s)[0] != 0)
         {      // Iterate through channels
            if (s[1] != 0)
            {
               return_string (context, returnv, *s);
               return_string (context, returnv, "\n");
               int ch = channel_enum (context, *s);
               if (ch != channels)
                  run_channel (context, ch, help_rmcios,
                                        paramtype, returnv, 0,
                                        (const union param_rmcios) 0);
               return_string (context, returnv, "\n");
            }
            s += 2;
         }

      }
      break;
   case read_rmcios:
      {
         const char **s;
         s = ((struct ch_system_data *) context->data)->channel_enum_pattern;
         while ((*s)[0] != 0)
         {
            if (s[1] != 0)
            {
               return_string (context, returnv, *s);
               return_string (context, returnv, " ");
            }
            s += 2;
         }
         return_string (context, returnv, "total=");
         return_int (context, returnv,
                     ((int)
                      (s -
                       ((struct ch_system_data *) context->data)->
                       channel_enum_pattern)) / 2);
         return_string (context, returnv, " ");
      }
      break;
   }
}

/////////////////////////////////////////////////////////
// Channel for redirecting channel input to an channel //
/////////////////////////////////////////////////////////
void as_class_func (void *data,
                    const struct context_rmcios *context, int id,
                    enum function_rmcios function,
                    enum type_rmcios paramtype,
                    struct combo_rmcios *returnv,
                    int num_params, const union param_rmcios param)
{
   if (num_params < 2)
   {
      if (function == help_rmcios)
      {
         return_string (context, returnv,
                        "as channel - redirect call return data\r\n"
                        "read as destination channel params...\r\n");
      }
      return;
   }

   struct combo_rmcios rvalue= {
      .paramtype = channel_rmcios,
      .num_params = 1,
      .param.channel = param_to_channel (context, paramtype, param, 0)
   };

   int execute_channel = param_to_channel (context, paramtype, param, 1);
   
   run_param_subset (context, 
                     execute_channel, 
                     function,
                     paramtype,
                     &rvalue,
                     num_params - 2,
                     param, 2);
}

void lock_func (void *data,
                const struct context_rmcios *context, int id,
                enum function_rmcios function,
                enum type_rmcios paramtype,
                struct combo_rmcios *returnv,
                int num_params, const union param_rmcios param)
{
   int ch;
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "lock channel -" 
                     " read and write channel multiaccess register\r\n"
                     "read lock channel # Read current lock status\r\n"
                     "setup lock channel # Read current lock status\r\n"
                     "setup lock channel -1 "
                     "# Access without restrictions.\r\n"
                     "setup lock channel 0 # (read=semi-locking) "
                     "(setup,write,link=locking)\r\n");
      break;

   case read_rmcios:
      if (num_params < 1)
         break;
      ch = param_to_int (context, paramtype, param, 0);
      return_int (context, returnv,
                  ((struct ch_system_data *) context->data)->
                  share_registers[ch]);
      break;

   case setup_rmcios:
      if (num_params == 1)
      {
         ch = param_to_int (context, paramtype, param, 0);
         return_int (context, returnv,
                     ((struct ch_system_data *) context->data)->
                     share_registers[ch]);
      }
      if (num_params < 2)
         break;
      ch = param_to_int (context, paramtype, param, 0);
      // No restrictions
      ((struct ch_system_data *) context->data)->share_registers[ch]
         = param_to_int (context, paramtype, param, 1);

      break;

   }
}

// NAME INTERFACE:
// read name // read list of channel names
// read name channel // read name(s) of channel
// write name channel the_name // register name for an channel
// write name the_name // remove the name for an channel
// read id channel // read identifier number for a channel
void name_func (void *data,
                const struct context_rmcios *context, int id,
                enum function_rmcios function,
                enum type_rmcios paramtype,
                struct combo_rmcios *returnv,
                int num_params, const union param_rmcios param)
{
   int p0len, p1len;
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "name channel - Rename, and read channel names\r\n"
                     "write name channel newname\r\n"
                     " -Adds a new name for channel\r\n"
                     " -Channel can be a name or id\r\n"
                     "read name channel\r\n" "-read name(s) of channel\r\n");
      break;
      // setup - Rename
   case write_rmcios:
      if (num_params < 2)
      {
         break;
      }

      p1len = param_string_length (context, paramtype, param, 1) + 1;
      {
         int id = param_to_channel(context, paramtype, param, 0);
         // Allocate memory for the new name
         char *newname = allocate_storage (context, p1len, 0);
         param_to_string (context, paramtype, param, 1, p1len, newname);
         
         // Add the name
         add_search_word (((struct ch_system_data *) context->data)->channel_enum_pattern, newname, (const char *) id);
      }
      break;
   case read_rmcios:
      if (num_params < 1)
      {
         break;
      }
      {
         int channel_enum = param_to_int (context, paramtype, param, 0);
         if (channel_enum == 0)
            channel_enum =
               param_to_integer (context, paramtype,
                                 (const union param_rmcios) param, 0);
         if (channel_enum == 0)
            break;

         int i = 0;
         while (((struct ch_system_data *) context->data)->
                channel_enum_pattern[i][0] != 0)
         {
            // look for channel
            if (((struct ch_system_data *) context->data)->
                channel_enum_pattern[i + 1] == ((const char *) channel_enum))
            {
               // Return the name
               return_string (context, returnv,
                              ((struct ch_system_data *) context->data)->
                              channel_enum_pattern[i]);
            }
            i += 2;
         }
      }
      break;
   }
}

void id_func (void *data,
              const struct context_rmcios *context, int id,
              enum function_rmcios function,
              enum type_rmcios paramtype,
              struct combo_rmcios *returnv,
              int num_params, const union param_rmcios param)
{
   int i;
   int ireturn = 0;
   int channel_state_registers =
      (((struct ch_system_data *) context->data)->max_channels / 8 /
       sizeof (unsigned int) + 1);
   int blen;
   int p0len, p1len;
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "id channel - Channel for getting id for a channel\r\n"
                     //"setup id new_id\r\n" // Set id for a channel and remove old)
                     //"write id new_id\r\n" // Set new id for a channel)
                     "read id ... channel\r\n"
                     "  -read identifier number for a channel\r\n"
                     "   when multiple parameters given id for the last is returned\r\n"
                     "   When parameter is not a channel name converts the parameter direcly into number \r\n");
      break;

   case write_rmcios:
      if (num_params < 2)
      {
         break;
      }

   case read_rmcios:
      if (num_params < 1)
      {
         break;
      }
      blen = param_string_alloc_size (context, paramtype, param, num_params - 1);
      {
         char buffer[blen];
         unsigned int states[channel_state_registers];
         const char *ch_name;
         ch_name = param_to_string (context, paramtype, param, num_params - 1,
                                    blen, buffer);
         for (i = 0; i < channel_state_registers; i++)
         {
            states[i] = ~(0);   // reset state registers
         }
         i = 0;

         
         while (states > (unsigned int *)0)
         {
            if (ch_name[i] == 0 || ch_name[i] == ' ')
            {
               break;
            }
            ireturn = 0;
            ireturn = (int) search (i, states,
                                    ((struct ch_system_data *) 
                                    context->data)->channel_enum_pattern, 
                                    ch_name[i]);
            i++;
         }
         if (ireturn == 0) 
         {
            ireturn = param_to_integer(context, paramtype, param, num_params - 1);
         }
      }
      return_int (context, returnv, ireturn);
      break;
   }
}



///////////////////////////////////////////////////////////////////////
// channel for controlling the system with channel command interface //
///////////////////////////////////////////////////////////////////////

// executes one line from input text.
// "link ch 0 #detaches channel from all linked channels"
// "link ch linkch #adds linked to linked channels for specific channel" 
// "link ch notexisting # must not detach channels"
// Only acceptable types are : channel_rmcios buffer_rmcios
// returns 0 on normal execute line.
// On direct write command returns the channel identifier.
//
int execute (const struct context_rmcios *context,
             const char *input,
             struct combo_rmcios *returnv)
{
   int linelen;
   int i, param_i = -1;
   int start_i = 0;
   enum function_rmcios function = 0;
   int channel = 0;
   int params = 1;
   int c;
   char prev = 0;
   char direct = 0;
   int hex = -2;
   char bstr = 0;
   
   for (linelen = 0; input[linelen] != 0; linelen++)
   {
      if (prev != '\\' && input[linelen] == '\"')
      {
         if (bstr == 0)
            bstr = 1;
         else
            bstr = 0;
      }
      if (input[linelen] == ' ' && bstr == 0)
         params++;      // count parameters       
      prev = input[linelen];
   }
   prev = 0;
   bstr = 0;

   {
      // compund statement for dynamic size array
      char line_string[linelen + 1];
      //char *sparam[params] ;
      struct buffer_rmcios sparam[params];
      for (i = 0; i < params; i++)
      {
         sparam[i] = (struct buffer_rmcios){0};
      }
      if (params < 1)
      {
         return 0;      // no parameters
      }

      for (i = 0; i < linelen + 1; i++)
      {
         c = *input;
         input++;

         if (c == '\n' || c == '\r' || c == 0
             || (c == '#' && prev != '\\') || c == -1)
            // End of command line
         {
            if (prev == ' ')
               line_string[i - 1] = 0;
            else
               line_string[i] = 0;
            {
               if (param_i > -1)
               {
                  // param_length= index - start index
                  sparam[param_i].length = i - start_i;
                  sparam[param_i].required_size = sparam[param_i].length;
               }
            }
            break;
         }
         line_string[i] = c;
         if (c == ' ' && param_i > -1 && bstr == 0)
         {
            //param_length=  index - start index
            sparam[param_i].length = i - start_i;
            sparam[param_i].required_size = sparam[param_i].length;
         }
         if (prev == ' ' && bstr == 0)
            // New parameter
         {

            line_string[i - 1] = 0;
            param_i++;
            // Set pointer to start of data
            sparam[param_i].data = line_string + i;
            // Size of remaining buffer
            sparam[param_i].size = linelen - i + 1;
            // Start index for calculating length later
            start_i = i;

         }
         // Special escape character
         if (prev == '\\')
         {
            i--;
            if (c == 'a')
               line_string[i] = '\a';
            if (c == 'b')
               line_string[i] = '\b';
            if (c == 'f')
               line_string[i] = '\f';
            if (c == 'n')
               line_string[i] = '\n';
            if (c == 'r')
               line_string[i] = '\r';
            if (c == 's')
               line_string[i] = ' ';
            if (c == 't')
               line_string[i] = '\t';
            if (c == 'v')
               line_string[i] = '\v';
            if (c == '\\')
               line_string[i] = '\\';
            if (c == '\'')
               line_string[i] = '\'';
            if (c == '\"')
               line_string[i] = '\"';
            if (c == '{')
               line_string[i] = '{';
            if (c == '?')
               line_string[i] = '?';
            if (c == '#')
               line_string[i] = '#';
            if (c == 'x')
            {
               i--;
               hex = -1;
            }
            prev = 0;
         }
         else if (prev != '\\' && c == '\"')
         {
            i--;
            if (bstr == 0)
            {
               bstr = 1;
            }
            else
            {
               bstr = 0;
            }
            prev = 0;
         }
         else if (prev != '\\' && c == '{')
         {
            direct = 1;
         }
         else if (hex != -2)
         {

            if (line_string[i] <= 'F' && line_string[i] >= 'A')
            {
               if (hex == -1)
                  hex = (line_string[i--] - 'A' + 10) << 4;
               else
               {
                  hex |= line_string[i] - 'A' + 10;
                  line_string[i] = hex;
                  hex = -1;
               }
            }
            else if (line_string[i] <= 'f' && line_string[i] >= 'a')
            {

               if (hex == -1)
                  hex = (line_string[i--] - 'a' + 10) << 4;
               else
               {
                  hex |= line_string[i] - 'a' + 10;
                  line_string[i] = hex;
                  hex = -1;
               }
            }
            else if (line_string[i] <= '9' && line_string[i] >= '0')
            {
               if (hex == -1)
                  hex = (line_string[i--] - '0') << 4;
               else
               {
                  hex |= line_string[i] - '0';
                  line_string[i] = hex;
                  hex = -1;
               }
            }
            else
               hex = -2;
         }
         else
            prev = c;
      }
      param_i++;
      if (param_i < 1)
         return 0;      // No channel specified
      function = function_enum (line_string);
      channel = channel_enum (context, sparam[0].data);
      if (channel == 0)
      {
         write_str (context, context->warning, "No channel on name:", 0);
         write_str (context, context->warning, sparam[0].data, 0);
         write_str (context, context->warning, "\r\n", 0);
         return 0;
      }
      // Direct write command
      if (function == write_rmcios && direct == 1)     
      {
         return channel;
      }
      // Convert link channel to > write link channel to
      else if (function == link_rmcios)
      {
         run_channel (context, 
                               context->link,
                               write_rmcios, 
                               buffer_rmcios,
                               returnv,
                               param_i, 
                               (const union param_rmcios) sparam);
      }
      else
      {
         run_channel (context, 
                               channel,
                               function, 
                               buffer_rmcios,
                               returnv,
                               param_i - 1,
                               (const union param_rmcios) (sparam + 1));

      }

      return_string (context, returnv, "\r\n");
      // return void to signal buffer flushing
      return_void (context, returnv);      
   }
   return 0;
}

void control_class_func (struct control_data *this,
                         const struct context_rmcios *context, int id,
                         enum function_rmcios function,
                         enum type_rmcios paramtype,
                         struct combo_rmcios *returnv,
                         int num_params, const union param_rmcios param)
{
   int plen;
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "help for system control channel\r\n"
                     "   create control newname\r\n"
                     "   setup control namespace\r\n"
                     "   write newname command #execute channel command \r\n"
                     "   read control filename #execute commands from file"
                     " (legacy function)\r\n"
                     " Execute commands from file using following synopsis:\r\n"
                     "   read as control file filename\r\n");
      break;
   case create_rmcios:
      if (num_params < 1)
         break;


      // allocate new data
      this = (struct control_data *) 
             allocate_storage (context, sizeof (struct control_data), 0);           

      //default values :
      this->context = *context;
      this->command[0] = 0;
      this->index = 0;
      this->direct_channel = 0;
      // create channel:
      create_channel_param (context, paramtype, param, 0,
                            (class_rmcios) control_class_func, this);

      break;

   case setup_rmcios:
      if (this == 0)
         break;
      if (num_params < 1)
         break;
      // Get id of new channel id space.
      int newspace = param_to_int (context, paramtype, param, 0);

      if (newspace != 0)
         this->context.id = newspace;
      break;

   case read_rmcios:
      if (num_params < 1)
         break;
      // get size of possibly needed buffer.
      plen = param_string_alloc_size (context, paramtype, param, 0);
      {
         // create buffer if needed
         char buffer[plen];
         const char *filename;
         // get/form the filename string
         filename = param_to_string (context, paramtype, param, 
                                     0, plen, buffer);
         struct buffer_rmcios p = {0};
         p.data = (char *) filename;
         p.size = 0;
         int slen;
         for (slen = 0; filename[slen] != 0; slen++);   // strlen(filename)
         p.length = slen;

         struct combo_rmcios rvalue = {
            .paramtype = channel_rmcios,
            .num_params = 1,
            .param.channel = channel_enum (context, "control")
         };

         // Read file to control channel: 
         run_channel (context, 
                               channel_enum (context, "file"),
                               read_rmcios, 
                               buffer_rmcios,
                               &rvalue, 
                               1,
                               (const union param_rmcios) &p);
      }
      break;

   case write_rmcios:
      if (this == 0)
         break;
      if (num_params < 1)
         break;
      // get size of possibly needed buffer.
      plen = param_buffer_alloc_size (context, paramtype, param, 0);
      {
         char buffer[plen];     // create buffer if needed
         int n, i;
         struct buffer_rmcios buf;
         const char *receive;
         buf = param_to_buffer (context, paramtype, param, 0, plen, buffer);
         receive = buf.data;
         n = buf.length;
         int direct_start = 0;

         for (i = 0; i < n; i++)
         {
            // Normal line execution mode :
            if (this->direct_channel == 0)
            {
               this->command[this->index++] = receive[i];
               this->command[this->index] = 0;
               if (this->index >= (sizeof (this->command) - 1))
                  // overflow protection
                  this->index = sizeof (this->command) - 2;
               if (receive[i] == '\n' || receive[i] == '{')
               {
                  this->command[this->index] = 0;       // NULL character
                  this->index = 0;      // record new command
                  this->direct_channel = execute (context, this->command, returnv);
                  if (this->direct_channel != 0)
                     direct_start = i + 1;
               }
            }
            else
            {
               // Direct write to channel:
               if (receive[i] == '}')
               {
                  // End of direct write
                  int len = i - direct_start;
                  if (len > 1)
                     write_buffer (&this->context,
                                   this->direct_channel,
                                   receive + direct_start, len - 1, 0);
                  this->direct_channel = 0;
               }

            }
         }
         // whole end part of buffer is direct data:
         if (this->direct_channel != 0)
         {
            int len = i - direct_start;
            write_buffer (&this->context, this->direct_channel,
                          receive + direct_start, len, 0);
         }
      }
      break;
   }
}

void create_class_func (void *this,
                        const struct context_rmcios *context, 
                        int id,
                        enum function_rmcios function,
                        enum type_rmcios paramtype,
                        struct combo_rmcios *returnv,
                        int num_params, 
                        const union param_rmcios param)
{
   int bufflen;
   switch (function)
   {
   case help_rmcios:
      return_string (context, returnv,
                     "Channel for creating new channels\r\n"
                     "This channel is not to be called directly!"
                     " parameters: function_pointer data_pointer\r\n");
      break;

   case create_rmcios:
      if(num_params < 2) 
      {
         break;
      }
      else
      {
         class_rmcios class_func;
         void *data;
         int class_ptr_size = param_binary_length(context, paramtype, param, 0);
         int data_ptr_size = param_binary_length(context, paramtype, param, 1);
         int ID;
         int function_id;

         if(class_ptr_size != sizeof(class_func) ||  data_ptr_size != sizeof(void *) )
         {
            write_str (context, context->errors,
                  "ERROR! Attemp of creating channel with incompatible "
                  "pointer parameter size\r\n", 0);
         }

         param_to_binary (context, paramtype, param, 0, sizeof(class_func), &class_func);
         param_to_binary (context, paramtype, param, 1, sizeof(void *), &data);

         function_id = add_class_func (context, class_func);
         ID = add_channel (context, function_id, data);
         return_int(context, returnv, ID);
      }
      break;
   }
}

