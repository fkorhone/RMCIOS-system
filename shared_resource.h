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

/* ************************************************
 * 2016 Frans Korhonen. University of Helsinki. 
 * 
 * Functions for reading and writing safely to a resource.
 * Works from multiple processes/threads. (memory, hardware etc...)
 * Functions allow multiple simultanous readers. 
 * Only one writer is allowed alone without readers reading.
 * 
 *
 * Example usage for safely reading from a resource:
 * 
 * int share_register ; 
 * char shared_message[] ;
 *
 * while(request_read_resource( &share_register )==0 ) ; 
 * printf("The message in the buffer is: %s",shared_message) ;
 * stop_read_resource(&share_register) ;
 *
 * Example for safely writing to the same resource from elsewhere:
 *
 * while(request_write_resource( &share_register )==0 ) ; 
 * strcpy(shared_message, "Safely written message.\n") ;
 * stop_write_resource(&share_register) ;
 *
 * ***********************************************/

#ifndef shared_resource_h
#define shared_resource_h

// share_register states:
// 0 Resource is free. Permissions can be granted for both read and write.
// >0 <MSB contains number of readers working on the resource. 
//              New readers are granted permissions to read.
// ==MSB resource is being written
// ==MSB+1 resource is waiting for an writer to take over
// >MSB+1 Readers still reading. New readers are no longer allowed to start. 
// Waiting for existing readers to finnish.

#define RES_MSB (1<<((sizeof(int)<<3)-2))

// Request permission to read a shared resource. 
// Returns 0 on decline. 1 on granted permission. 
// granted permission MUST be released by calling stop_reading_shared().
// Returns 0 on
static inline int request_read_resource (int *share_register)
{
    int state = *share_register;
    if (state >= 0 && state < (RES_MSB - 1))    
      // Resource free for reading state
      {
          // increment state, permission denied on race condition.
          return __sync_bool_compare_and_swap (share_register, state, state + 1);       
      }
    else
        return 0;               // permission denied
}

// Stop reading of shared resource 
// (Must be called only after permission has been granted
// Releases the resource from being written.
// Allows readers to access the resource)
static inline void stop_read_resource (int *share_register)
{
    __sync_sub_and_fetch (share_register, 1);   // Decrease reader count
}

// Request permission to write to shared resource. 
// Once request_write_shared() is called no more new read permissions given. 
// Function grants permission when all readers have stopped reading resource.
// MUST call stop_write_shared() afer granted permission!
static inline int request_write_resource (int *share_register)
{
    int state = *share_register;
    if (state > 0 && state < RES_MSB - 1)       // Resource is being read
      {
          // Mark the resource to be scheduled for write operations.
          // done by incrementing and setting the MSB to 1. 
          // (No new readers allowed)
          __sync_bool_compare_and_swap (share_register, state,
                                        (state + 1) | RES_MSB);
          return 0;
      }
    else if (state == (1 | RES_MSB) || state == 0) 
      {
          int success;
          // Reserve shared resource for writing == MBS(1) 
          // (returns false on race condition)
          success = __sync_bool_compare_and_swap (share_register, state, RES_MSB);
          return success;
      }
    return 0;                   // Readers or a writer working on the resource
}

// Stop writing to shared resource 
// (Writing task is done, release resource for the readers and other writers)
static inline void stop_write_resource (int *share_register)
{
    *share_register = 0;
}

#endif
