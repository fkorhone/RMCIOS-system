#include <stdlib.h>
#include <stdio.h>

/* String to float converter 
 * @param str String to convert from
 * returns the converted value. Returns 0 on failure. */
int string_to_integer (const char *str)
{
   return strtol (str, NULL, 0);
}

/* String to float converter 
 * @param str String to convert from
 * returns the converted value. Returns 0 on failure. */
double string_to_float (const char *str)
{
   return strtof (str, NULL);
}

/* Integer to string converter 
 * @param buffer buffer to write to
 * @param len size of buffer
 * @param value number to convert
 * @return number of characters the full string representation needs. */
int integer_to_string (char *buffer, int len, int value)
{
   return snprintf (buffer, len, "%d", value);
}

/* Float to string converter 
 * @param buffer buffer to write to
 * @param len size of buffer
 * @param value Number to convert
 * @return number of characters the full string representation needs.*/
int float_to_string (char *buffer, int len, double value)
{
   return snprintf (buffer, len, "%g", value);
}

#define COMBINE_SHFT 4

void convert_func (void *data, const struct context_rmcios *context,
                  int channel, enum function_rmcios function,
                  enum type_rmcios paramtype, 
                  struct combo_rmcios *returnv,
                  int num_params, const union param_rmcios param)
{
    if (returnv == NULL || returnv->num_params==0 || num_params == 0) break;

    // Convert last parameter from param:
    int index = num_params - 1;

    switch (paramtype | (returnv->paramtype << COMBINE_SHFT))
    {
        // From | to (return)

        // Integer params:
        case int_rmcios | (int_rmcios << COMBINE_SHFT):
            returnv->param.iv[0] = param.iv[index];
            break;

        case int_rmcios | (float_rmcios << COMBINE_SHFT):
            returnv->param.fv[0] = param.iv[index];
            break;

        case int_rmcios | (buffer_rmcios << COMBINE_SHFT):
           {
             struct buffer_rmcios *sreturn = returnv->param.bv;
             int n = sreturn->size - sreturn->length;
             int rsize; 
             if (n > 0)
             {
                char *end = sreturn->data + sreturn->length ;
                rsize= integer_to_string (end, n, param.iv[index]) ;
                for (n = 0; end[n] != 0; n++);
                sreturn->length += n;
             }
             else rsize=integer_to_string(0, 0, param.iv[index]) ;
             sreturn->required_size = rsize;
             sreturn->trailing_size = 1 ;
             break;
           }

        case int_rmcios | (channel_rmcios << COMBINE_SHFT):
           if (function == write_rmcios){
               run_channel (context, returnv->param.channel, write_rmcios,
                       int_rmcios, 0, 1,
                       (param.iv) + index);
           }
           else if(function == read_rmcios) {
               returnv
           }
           break;

        case int_rmcios | (binary_rmcios << COMBINE_SHFT):
           if(function == write_rmcios)
           {
             int value = param.iv[index];
             struct buffer_rmcios *sreturn = returnv->param.bv;
             if ((sreturn->size - sreturn->length) >= sizeof (value))
             {
                *((int *) (sreturn->data + sreturn->length)) = value;
                sreturn->length += sizeof (value);
             }
             sreturn->required_size = sizeof (value);
           }
           break;

        case int_rmcios | (combo_rmcios << COMBINE_SHFT):
            {
                struct combo_rmcios *creturn = returnv->param.cv;
                convert_func (data, context, channel, function, paramtype, creturn, num_params, param);
            }
            break;
 
        // Float params:
        case float_rmcios | (int_rmcios << COMBINE_SHFT):
            returnv->param.iv[0] = param.fv[index];
            break;

        case float_rmcios | (float_rmcios << COMBINE_SHFT):
            returnv->param.fv[0] = param.fv[index];
            break;

        case float_rmcios | (buffer_rmcios << COMBINE_SHFT):
            {
                 struct buffer_rmcios *sreturn = returnv->param.bv;
                 int n = sreturn->size - sreturn->length;
                 int rsize ;
                 value = param.fv[index];
                 if (n > 0)
                 {
                    char *end = sreturn->data + sreturn->length;
                    rsize=float_to_string (sreturn->data + sreturn->length, n, value);
                    for (n = 0; end[n] != 0; n++);
                    sreturn->length += n;
                 }
                 else rsize=float_to_string(0,0,value) ;

                 sreturn->required_size = rsize ;
                 // Tell that is space for terminating null after data.
                 sreturn->trailing_size = 1 ;
            }
            break;
        
        case float_rmcios | (channel_rmcios << COMBINE_SHFT):
            run_channel (context, returnv->param.channel, write_rmcios,
                         float_rmcios, 0, 1,
                         (param.fv) + index);
            break;
        
        case float_rmcios | (binary_rmcios << COMBINE_SHFT):
           {
             float value = param.fv[index];
             struct buffer_rmcios *sreturn = returnv->param.bv;
             if ((sreturn->size - sreturn->length) > sizeof (value))
             {
                *((float *) (sreturn->data + sreturn->length)) = value;
                sreturn->length += sizeof (value);
             }
             sreturn->required_size = sizeof (value);
           }
           break;

        case float_rmcios | (combo_rmcios << COMBINE_SHFT):
            {
                struct combo_rmcios *creturn = returnv->param.cv;
                convert_func (data, context, channel, function, paramtype, creturn, num_params, param);
            }
            break;

        // Buffer params:
        case buffer_rmcios | (int_rmcios << COMBINE_SHFT):
            {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                int length = pbuffer->length;
                char string[length + 1];
                for (i = 0; i < length; i++)
                {
                    string[i] = buffer[i];
                }
                string[length] = 0;
                returnv->param.iv[0] = string_to_integer (string);
            }
            break;    
    
        case buffer_rmcios | (float_rmcios << COMBINE_SHFT):
            {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                int length = pbuffer->length;
                char string[length + 1];
                for (i = 0; i < length; i++)
                {
                    string[i] = buffer[i];
                }
                string[length] = 0;
                returnv->param.fv[0] = string_to_float (string);
            }
            break;
        
        case buffer_rmcios | (buffer_rmcios << COMBINE_SHFT):
        case buffer_rmcios | (binary_rmcios << COMBINE_SHFT):
            if(function == write_rmcios)
            {
                 int i = 0; // index
                 int si = 0; // Source index   
                 struct buffer_rmcios * pbuffer = (param.bv) + index;
                 int length = pbuffer->length;
                 char * buffer = pbuffer->data;
                 struct buffer_rmcios *sreturn = returnv->param.bv;
                 for (i = sreturn->length; i < sreturn->size && si < length; i++)
                 {
                    // append data to buffer
                    sreturn->data[i] = buffer[si++];    
                 }
                 sreturn->length = i;
                 sreturn->trailing_size = 0;
                 sreturn->required_size = length;
            }
            else if(function == read_rmcios)
            {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                sreturn->data = pbuffer->data;
                sreturn->length = pbuffer->length;
                sreturn->size = pbuffer->size;
                sreturn->required_size = pbuffer->required_size;
                sreturn->trailing_size = pbuffer->required_size;
            }
            break;

        case buffer_rmcios | (channel_rmcios << COMBINE_SHFT):
            {
                if (function == write_rmcios) {
                    run_channel (context, returnv->param.channel, write_rmcios, 
                                 buffer_rmcios, 0, 1, (param.bv) + index);       
                }
                else if (function == read_rmcios) {
                    // get space needed for string
                    int len = param_string_alloc_size (context, paramtype, params, index);        
                    {
                        // allocate space for string
                        char buffer[len];      
                        const char *s;
                        // convert to string
                        s = param_to_string (context, paramtype, params, index, len, buffer);  
                        ch_enum = channel_enum (context, s);   
                        // set return value to channel enum
                        returnv = ch_enum;  
                    }
                    returnv.param.channel = 
                }
            }
            break;
        
        case buffer_rmcios | (combo_rmcios << COMBINE_SHFT):
            {
                struct combo_rmcios *creturn = returnv->param.cv;
                convert_func (data, context, channel, function, paramtype, creturn, num_params, param);
            }
            break;
/*        
        // Channel params:
        case channel_rmcios | (int_rmcios << COMBINE_SHFT):

            break;

        case channel_rmcios | (float_rmcios << COMBINE_SHFT):
            break;

        case channel_rmcios | (buffer_rmcios << COMBINE_SHFT):
            break;

        case channel_rmcios | (channel_rmcios << COMBINE_SHFT):
            break;

        case channel_rmcios | (binary_rmcios << COMBINE_SHFT):
            break;

//        case channel_rmcios | (combo_rmcios << COMBINE_SHFT):
//            break;
*/
        // binary params:
        case binary_rmcios | (int_rmcios << COMBINE_SHFT):
           {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                int length = pbuffer->length;
                char * buffer = pbuffer->data;
                char * dest = (char *) returnv->param.p;

                for (i = 0; i < sizeof (int) && i < length; i++)
                {
                    dest[i] = buffer[i];
                }
           }
           break;

        case binary_rmcios | (float_rmcios << COMBINE_SHFT):
            {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                int length = pbuffer->length;
                char * buffer = pbuffer->data;
                char * dest = (char *) returnv->param.p;
                for (i = 0; i < sizeof (float) && i < length; i++)
                {
                    dest[i] = buffer[i];
                }
            }
            break;

        case binary_rmcios | (buffer_rmcios << COMBINE_SHFT):
        case binary_rmcios | (binary_rmcios << COMBINE_SHFT):
             if (function == write_rmcios)
             {
                 struct buffer_rmcios * pbuffer = (param.bv) + index;
                 int length = pbuffer->length;
                 char * buffer = pbuffer->data;
                 struct buffer_rmcios *sreturn = returnv->param.bv;
                 for (i = sreturn->length; i < sreturn->size && si < length; i++)
                 {
                    // append data to buffer
                    sreturn->data[i] = buffer[si++];    
                 }
                 sreturn->length = i;
                 sreturn->required_size = length;
                 sreturn->trailing_size = 0;
            }
            else if(function == read_rmcios)
            {
                struct buffer_rmcios * pbuffer = (param.bv) + index;
                sreturn->data = pbuffer->data;
                sreturn->length = pbuffer->length;
                sreturn->size = pbuffer->size;
                sreturn->required_size = pbuffer->required_size;
                sreturn->trailing_size = pbuffer->required_size;
            }
            break;

        case binary_rmcios | (channel_rmcios << COMBINE_SHFT):
            {
                struct buffer_rmcios sreturn = {
                    .length = length,
                    .size = length,
                    .required_size = length,
                    .trailing_size = 0,
                    .data = (char *) buffer
                };
                
                // execute write to return channel
                run_channel (context, returnv->param.channel, write_rmcios, 
                             binary_rmcios, 0, 1, (param.bv) + index);        
            }
            break;

        case binary_rmcios | (combo_rmcios << COMBINE_SHFT):
            {
                struct combo_rmcios *creturn = returnv->param.cv;
                convert_func (data, context, channel, function, paramtype, creturn, num_params, param);
            }
            break;
    }
}
