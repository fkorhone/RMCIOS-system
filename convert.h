#ifndef CONVERT_H
#define CONVERT_H

void convert_func (void *data, const struct context_rmcios *context,
                  int channel, enum function_rmcios function,
                  enum type_rmcios paramtype, 
                  struct combo_rmcios *returnv,
                  int num_params, const union param_rmcios param);

#endif
