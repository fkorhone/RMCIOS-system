#include "test.h"

// Module to test
#include "convert.c"

enum test_cases {
    TEST_INT_TO_INT = 0,
    TEST_INT_TO_FLOAT,
    TEST_INT_TO_BUFFER_STR,
    TEST_INT_TO_BUFFER_FULL,
    TEST_INT_TO_BUFFER_OVERFLOW,
    TEST_INT_TO_BUFFER_REQUIRED_SIZE,
    TEST_INT_TO_CHANNEL,
    TEST_INT_TO_BINARY,
    TEST_INT_TO_COMBO_INT,

    TEST_FLOAT_TO_INT,
    TEST_FLOAT_TO_FLOAT,
    TEST_FLOAT_TO_BUFFER_STR,
    TEST_FLOAT_TO_BUFFER_FULL,
    TEST_FLOAT_TO_BUFFER_OVERFLOW,
    TEST_FLOAT_TO_BUFFER_REQUIRED_SIZE,
    TEST_FLOAT_TO_CHANNEL,
    TEST_FLOAT_TO_BINARY,
    TEST_FLOAT_TO_COMBO_FLOAT,

    TEST_BUFFER_TO_INT,
    TEST_BUFFER_TO_FLOAT,
    TEST_BUFFER_TO_BUFFER,
    TEST_BUFFER_TO_BUFFER_STR,
    TEST_BUFFER_TO_BUFFER_FULL,
    TEST_BUFFER_TO_BUFFER_STR_OVERFLOW,
    TEST_BUFFER_TO_BUFFER_STR_APPEND,
    TEST_BUFFER_TO_BUFFER_REQUIRED_SIZE,

    TEST_BUFFER_TO_CHANNEL,
    TEST_BUFFER_TO_BINARY,
    TEST_BUFFER_TO_COMBO_BUFFER,

    TEST_BINARY_TO_INT,
    TEST_BINARY_TO_FLOAT,
    TEST_BINARY_TO_CHANNEL,
    TEST_BINARY_TO_BINARY,
    TEST_BINARY_TO_BUFFER,
    TEST_BINARY_TO_COMBO_BINARY,
    TEST_BINARY_TO_BUFFER_APPEND,
    TEST_BINARY_TO_BUFFER_OVERFLOW,
    TEST_BINARY_TO_BUFFER_REQUIRED_SIZE,

    TEST_COMBO_TO_INT,
    TEST_COMBO_TO_FLOAT,
    TEST_COMBO_TO_CHANNEL,
    TEST_COMBO_TO_BINARY,
    TEST_COMBO_TO_BUFFER,
    TEST_COMBO_TO_COMBO_BUFFER,
    TEST_COMBO_INT_TO_BUFFER,
    TEST_COMBO_MULTIPLE_VALUES,
    TEST_COMBO_MULTIPLE_VALUES_SECOND_SET,

    TOTAL_TEST_CASES
};

struct call_data {
    const struct context_rmcios * context;
    int id;
    enum function_rmcios function ;
    enum type_rmcios paramtype;
    struct combo_rmcios *returnv;
    int num_params;
    union param_rmcios param;
} last_call;

void run_mock(void *data,
        const struct context_rmcios * context,
        int id,
        enum function_rmcios function,
        enum type_rmcios paramtype,
        struct combo_rmcios *returnv,
        int num_params,
        const union param_rmcios param)
{
        last_call.context = context;
        last_call.id = id;
        last_call.function = function;
        last_call.paramtype = paramtype;
        last_call.returnv = returnv;
        last_call.num_params = num_params;
        last_call.param = param;
}

struct context_rmcios context_mock =
{
        .run_channel = run_mock
};

int main(void)
{
    INIT_TESTING(TOTAL_TEST_CASES)
    
    struct buffer_rmcios pbuffer = {
        .data = (char *)"red",
        .length = 3,
        .size = 0,
        .required_size = 3,
        .trailing_size = 0
    };
    char rbuffer[10] = {};
    struct buffer_rmcios retbuffer = {
        .data = rbuffer,
        .length = 0,
        .size = sizeof(rbuffer),
        .required_size = 0,
        .trailing_size = 0
    } ;

    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = (const union param_rmcios)(&retbuffer)
    };

    TEST_CASE(TEST_INT_TO_INT,"")
    {
        int value = 74;
        int retval = 0;

        struct combo_rmcios returnv = {
            .paramtype = int_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };

        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT(retval, 74);

        int values[2] = {13, 14};
        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                      2, (const union param_rmcios)values);

        TEST_ASSERT_EQUAL_INT(retval, 14);

    }
    TEST_CASE(TEST_INT_TO_FLOAT,"")
    {
        int value = 88;
        float retval = 0;

        struct combo_rmcios returnv = {
            .paramtype = float_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };

        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_FLOAT(retval, 88);

        int values[2] = {55, 110};
        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                      2, (const union param_rmcios)values);

        TEST_ASSERT_EQUAL_FLOAT(retval, 110);
    }

    TEST_CASE(TEST_INT_TO_BUFFER_STR,"Convert int to buffer (string compatible)")
    {
        int value = 34;

        char rbuffer[10] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '3');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '4');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 1);       
    }

    TEST_CASE(TEST_INT_TO_BUFFER_FULL,"Convert int to buffer (terminating str NULL does not fit)")
    {
        int value = 67;
        
        char rbuffer[10] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = 2,
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);
 
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '6');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '7');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);       
    }
    TEST_CASE(TEST_INT_TO_BUFFER_OVERFLOW,"Convert int to buffer (Buffer overflow)")
    {
        int value = 820;
        
        char rbuffer[10] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = 2,
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);
 
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '8');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '2');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);       
    }

    TEST_CASE(TEST_INT_TO_BUFFER_REQUIRED_SIZE,"")
    {
        int value = 1460;
        struct buffer_rmcios retbuffer = {
            .data = 0,
            .length = 0,
            .size = 0,
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };

        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 4);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].size, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].data, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);
    }

    TEST_CASE(TEST_INT_TO_CHANNEL,"")
    { 
        int value = 6782;
        struct combo_rmcios returnv = {
            .paramtype = channel_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(5)
        };

        convert_func (0, &context_mock, 0, write_rmcios, int_rmcios, &returnv,
                      1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT( last_call.paramtype, int_rmcios );
        TEST_ASSERT_EQUAL_INT( last_call.function, write_rmcios );
        TEST_ASSERT_EQUAL_INT( last_call.returnv, 0 );
        TEST_ASSERT_EQUAL_INT( last_call.num_params, 1 );
        TEST_ASSERT_EQUAL_INT( last_call.param.iv[0], 6782 );
        TEST_ASSERT_EQUAL_INT( last_call.id, 5 );
    }

    TEST_CASE(TEST_INT_TO_BINARY,"")
    {
        int ireturn = 0;
        int value = 6921;

        struct buffer_rmcios retbuffer = {
            .data = (char *)&ireturn,
            .length = 0,
            .size = sizeof(ireturn),
            .required_size = 0,
            .trailing_size = 0
        } ;
        struct combo_rmcios returnv = {
            .paramtype = binary_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };

        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                      1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT( ireturn, 6921 );
        TEST_ASSERT_EQUAL_INT( retbuffer.required_size, sizeof(ireturn) );

        value = 1132;
        retbuffer.length = 0; 
        retbuffer.size = 0; 
        retbuffer.required_size = 0; 
        convert_func (0, 0, 0, read_rmcios, int_rmcios, &returnv,
                      1, (const union param_rmcios)&value);
        TEST_ASSERT_EQUAL_INT( retbuffer.required_size, sizeof(ireturn) );
        TEST_ASSERT_EQUAL_INT( ireturn, 6921 ); // Should not change
    }

    TEST_CASE(TEST_INT_TO_COMBO_INT,"")
    {
        int value = 4512;
    
        int retval = 0;

        struct combo_rmcios int_returnv = {
            .paramtype = int_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };
 
        struct combo_rmcios returnv = {
            .paramtype = combo_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&int_returnv)
        };

        convert_func (0, 0, 0, write_rmcios, int_rmcios, &returnv,
                      1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT(retval, 4512);
    }

    TEST_CASE(TEST_FLOAT_TO_INT,"")
    {
        float value = 74;
        int retval = 0;

        struct combo_rmcios returnv = {
            .paramtype = int_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };

        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT(retval, 74);

        float values[2] = {33, 15.3};
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                      2, (const union param_rmcios)values);

        TEST_ASSERT_EQUAL_INT(retval, 15);
    }
    
    TEST_CASE(TEST_FLOAT_TO_FLOAT,"")
    {

        float value = 13;
        float retval = 0;

        struct combo_rmcios returnv = {
            .paramtype = float_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };

        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_FLOAT(retval, 13);

        float values[2] = {104, 44};
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                      2, (const union param_rmcios)values);

        TEST_ASSERT_EQUAL_FLOAT(retval, 44);
    }
    
    TEST_CASE(TEST_FLOAT_TO_BUFFER_STR,"Convert float to buffer (string compatible)")
    {
        float value = 64;
        struct buffer_rmcios pbuffer = {
            .data = (char *)"red",
            .length = 3,
            .size = 0,
            .required_size = 3,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '6');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '4');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 1);       
    }

    TEST_CASE(TEST_FLOAT_TO_BUFFER_FULL,"Convert float to buffer (terminating str NULL does not fit)")
    {
        float value = 89;
        char rbuffer[10] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = 2,
            .required_size = 0,
            .trailing_size = 0
        };
        
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);
 
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '8');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '9');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);       
    }

    TEST_CASE(TEST_FLOAT_TO_BUFFER_OVERFLOW,"Convert float to buffer (Buffer overflow)")
    {
        float value = 460;
        char rbuffer[2] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = 2,
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);
 
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         '4');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[1],         '6');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);       
    }

    TEST_CASE(TEST_FLOAT_TO_BUFFER_REQUIRED_SIZE,"")
    {
        float value = 460;
        struct buffer_rmcios retbuffer = {
            .data = 0,
            .length = 0,
            .size = 0,
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };

        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].size, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].data, 0);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);
    }

    TEST_CASE(TEST_FLOAT_TO_CHANNEL,"")
    {
        float value = 10312;
        struct combo_rmcios returnv = {
            .paramtype = channel_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(15)
        };

        convert_func (0, &context_mock, 0, write_rmcios, float_rmcios, &returnv,
                      1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_INT( last_call.paramtype, float_rmcios );
        TEST_ASSERT_EQUAL_INT( last_call.function, write_rmcios );
        TEST_ASSERT_EQUAL_INT( last_call.returnv, 0 );
        TEST_ASSERT_EQUAL_INT( last_call.num_params, 1 );
        TEST_ASSERT_EQUAL_FLOAT( last_call.param.fv[0], 10312 );
        TEST_ASSERT_EQUAL_INT( last_call.id, 15 );
    }

    TEST_CASE(TEST_FLOAT_TO_BINARY,"")
    {
        float freturn = 0;
        float value = 312;

        struct buffer_rmcios retbuffer = {
            .data = (char *)&freturn,
            .length = 0,
            .size = sizeof(freturn),
            .required_size = 0,
            .trailing_size = 0
        };
        struct combo_rmcios returnv = {
            .paramtype = binary_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retbuffer)
        };

        TEST_ASSERT_EQUAL_FLOAT( freturn, 0);
        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                      1, (const union param_rmcios)&value);
        
        TEST_ASSERT_EQUAL_FLOAT(freturn, 312);
    }

    TEST_CASE(TEST_FLOAT_TO_COMBO_FLOAT,"")
    {
        float value = 2104;
        float retval = 0;
        struct combo_rmcios float_returnv = {
            .paramtype = float_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&retval)
        };
 
        struct combo_rmcios returnv = {
            .paramtype = combo_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&float_returnv)
        };

        convert_func (0, 0, 0, write_rmcios, float_rmcios, &returnv,
                      1, (const union param_rmcios)&value);

        TEST_ASSERT_EQUAL_FLOAT(retval, 2104);
    }

    TEST_CASE(TEST_BUFFER_TO_INT,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"213",
            .length = 3,
            .size = 0,
            .required_size = 3,
            .trailing_size = 0
        };
        int ireturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = int_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&ireturnv)
        };
        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);
        
        TEST_ASSERT_EQUAL_INT(ireturnv, 213);

        pbuffer.data = (char *)"413";
        pbuffer.trailing_size = 1;
        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);
        
        TEST_ASSERT_EQUAL_INT(ireturnv, 413);
    }

    TEST_CASE(TEST_BUFFER_TO_FLOAT,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"441",
            .length = 3,
            .size = 0,
            .required_size = 3,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = float_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&freturnv)
        };

        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);
        
        TEST_ASSERT_EQUAL_FLOAT(freturnv, 441);

        pbuffer.data = (char *)"123";
        pbuffer.trailing_size = 1;
        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);
        
        TEST_ASSERT_EQUAL_FLOAT(freturnv, 123);
    }

    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"abc",
            .length = 3,
            .size = 0,
            .required_size = 3,
            .trailing_size = 0
        };

        char rbuffer[10] = {0};
        struct buffer_rmcios breturnv = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&breturnv)
        };

        TEST_CASE(TEST_BUFFER_TO_BUFFER,"Test writing buffer to another buffer")
        {

            convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                    1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR(rbuffer[0], 'a');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[1], 'b');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[2], 'c');

            TEST_ASSERT_EQUAL_INT(breturnv.length, 3);
        }
 
        TEST_CASE(TEST_BUFFER_TO_BUFFER_STR,"Test writing null-terminated string to another buffer")
        {
            rbuffer[3] = 'X';
            pbuffer.trailing_size = 1;
            breturnv.length = 0;

            convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                    1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR(rbuffer[0], 'a');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[1], 'b');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[2], 'c');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[3], 0);

            TEST_ASSERT_EQUAL_INT(breturnv.trailing_size, 1);
            TEST_ASSERT_EQUAL_INT(breturnv.length, 3);
        }
 
        TEST_CASE(TEST_BUFFER_TO_BUFFER_FULL, "Test writing null-terminated string to another buffer that gets full")
        {
            rbuffer[0] = '1';
            rbuffer[1] = '2';
            rbuffer[2] = '3';
            rbuffer[3] = 'X';
            pbuffer.trailing_size = 1;
            breturnv.trailing_size = 0; // Data is preferred over trailing byte
            breturnv.size = 3;
            breturnv.length = 0;


            convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                          1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR(rbuffer[0], 'a');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[1], 'b');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[2], 'c');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[3], 'X');

            TEST_ASSERT_EQUAL_INT(breturnv.trailing_size, 0);
            TEST_ASSERT_EQUAL_INT(breturnv.length, 3);
        }

        TEST_CASE(TEST_BUFFER_TO_BUFFER_STR_OVERFLOW, "Test writing null-terminated string to another buffer 1 byte too short")
        {
            rbuffer[0] = '1';
            rbuffer[1] = '2';
            rbuffer[2] = '3';
            rbuffer[3] = 'X';
            pbuffer.trailing_size = 1;
            breturnv.trailing_size = 1; // Trailing byte is preferred over data
            breturnv.size = 3;
            breturnv.length = 0;


            convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                          1, (const union param_rmcios)&pbuffer);
 
            TEST_ASSERT_EQUAL_CHAR(rbuffer[0], 'a');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[1], 'b');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[2], 0);
            TEST_ASSERT_EQUAL_CHAR(rbuffer[3], 'X');
            TEST_ASSERT_EQUAL_INT(breturnv.trailing_size, 1);
            TEST_ASSERT_EQUAL_INT(breturnv.length, 2);
        }

        TEST_CASE(TEST_BUFFER_TO_BUFFER_STR_APPEND, "Test appending null-terminated string to another buffer")
        {
            rbuffer[3] = 'X';
            pbuffer.trailing_size = 1;
            breturnv.length = 2;
            breturnv.size = sizeof(rbuffer);
            breturnv.data[0] = '1';           
            breturnv.data[1] = '2';           
            breturnv.data[2] = 'Y';           
            breturnv.data[3] = 'Z';           

            convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                    1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR(rbuffer[0], '1');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[1], '2');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[2], 'a');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[3], 'b');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[4], 'c');
            TEST_ASSERT_EQUAL_CHAR(rbuffer[5], 0);

            TEST_ASSERT_EQUAL_INT(breturnv.trailing_size, 1);
            TEST_ASSERT_EQUAL_INT(breturnv.length, 5);
        }
    }

    TEST_CASE(TEST_BUFFER_TO_CHANNEL,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"abc",
            .length = 3,
            .size = 0,
            .required_size = 3,
            .trailing_size = 0
        };

        int channel = 42;
        struct combo_rmcios returnv = {
            .paramtype = channel_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(channel)
        };

        convert_func (0, &context_mock, 0, write_rmcios, buffer_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
        
        TEST_ASSERT_EQUAL_INT( last_call.id, 42);
        TEST_ASSERT_EQUAL_INT( last_call.function, write_rmcios);
        TEST_ASSERT_EQUAL_INT( last_call.paramtype, buffer_rmcios);
        TEST_ASSERT_EQUAL_INT( last_call.returnv, 0);
        TEST_ASSERT_EQUAL_INT( last_call.num_params, 1);
        TEST_ASSERT_EQUAL_CHAR( last_call.param.bv->data[0], 'a' );
        TEST_ASSERT_EQUAL_CHAR( last_call.param.bv->data[1], 'b' );
        TEST_ASSERT_EQUAL_CHAR( last_call.param.bv->data[2], 'c' );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->length, 3 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->size, 0 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->required_size, 3 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->trailing_size, 0 );
    }

    TEST_CASE(TEST_BUFFER_TO_BINARY,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"a\0c",
            .length = 4,
            .size = 0,
            .required_size = 4,
            .trailing_size = 0
        };

        char rbuffer[10] = {0};
        struct buffer_rmcios breturnv = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = binary_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&breturnv)
        };

        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR( breturnv.data[0], 'a');
        TEST_ASSERT_EQUAL_INT( breturnv.data[1], 0);
        TEST_ASSERT_EQUAL_CHAR( breturnv.data[2], 'c');
        TEST_ASSERT_EQUAL_INT( breturnv.data[3], 0);
        TEST_ASSERT_EQUAL_INT( breturnv.length, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.required_size, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.trailing_size, 0);
    }

    TEST_CASE(TEST_BUFFER_TO_COMBO_BUFFER,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"a\0c",
            .length = 4,
            .size = 0,
            .required_size = 4,
            .trailing_size = 0
        };

        char rbuffer[10] = {0};
        struct buffer_rmcios breturnv = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios creturnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&breturnv)
        };

        struct combo_rmcios returnv = {
            .paramtype = combo_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&creturnv)
        };

        convert_func (0, 0, 0, write_rmcios, buffer_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR( breturnv.data[0], 'a');
        TEST_ASSERT_EQUAL_INT( breturnv.data[1], 0);
        TEST_ASSERT_EQUAL_CHAR( breturnv.data[2], 'c');
        TEST_ASSERT_EQUAL_INT( breturnv.data[3], 0);
        TEST_ASSERT_EQUAL_INT( breturnv.length, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.required_size, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.trailing_size, 0);
    }

    TEST_CASE(TEST_BINARY_TO_INT,"")
    {   
        // Integer as binary param
        int value = 1023;
        struct buffer_rmcios binary_param = {
            .data = (char *)&value,
            .length = sizeof(value),
            .size = 0,
            .required_size = sizeof(value),
            .trailing_size = 0
        };

        // Return to integer
        int ireturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = int_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&ireturnv)
        };

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&binary_param);

        TEST_ASSERT_EQUAL_INT(value, 1023);
    }
    
    TEST_CASE(TEST_BINARY_TO_FLOAT,"")
    {
        // float as binary param
        float value = 423;
        struct buffer_rmcios binary_param = {
            .data = (char *)&value,
            .length = sizeof(value),
            .size = 0,
            .required_size = sizeof(value),
            .trailing_size = 0
        };

        // Return to float
        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = float_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&freturnv)
        };

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&binary_param);

        TEST_ASSERT_EQUAL_FLOAT(value, 423);
    }
    
    TEST_CASE(TEST_BINARY_TO_BUFFER,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"a\0c",
            .length = 4,
            .size = 0,
            .required_size = 4,
            .trailing_size = 0
        };

        char rbuffer[10] = {0};
        struct buffer_rmcios breturnv = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = buffer_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&breturnv)
        };

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR( breturnv.data[0], 'a');
        TEST_ASSERT_EQUAL_INT( breturnv.data[1], 0);
        TEST_ASSERT_EQUAL_CHAR( breturnv.data[2], 'c');
        TEST_ASSERT_EQUAL_INT( breturnv.data[3], 0);
        TEST_ASSERT_EQUAL_INT( breturnv.length, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.required_size, 4);
        TEST_ASSERT_EQUAL_INT( breturnv.trailing_size, 0);
    }

    TEST_CASE(TEST_BINARY_TO_CHANNEL,"")
    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"a\0c",
            .length = 4,
            .size = 0,
            .required_size = 4,
            .trailing_size = 0
        };
  
        int channel = 312;
        struct combo_rmcios returnv = {
            .paramtype = channel_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(channel)
        };

        convert_func (0, &context_mock, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_INT( last_call.id, 312);
        TEST_ASSERT_EQUAL_INT( last_call.function, write_rmcios);
        TEST_ASSERT_EQUAL_INT( last_call.paramtype, buffer_rmcios);
        TEST_ASSERT_EQUAL_INT( last_call.returnv, 0);
        TEST_ASSERT_EQUAL_INT( last_call.num_params, 1);
        TEST_ASSERT_EQUAL_CHAR( last_call.param.bv->data[0], 'a' );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->data[1], 0 );
        TEST_ASSERT_EQUAL_CHAR( last_call.param.bv->data[2], 'c' );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->length, 4 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->size, 0 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->required_size, 4 );
        TEST_ASSERT_EQUAL_INT( last_call.param.bv->trailing_size, 0 );
    }

    {
        struct buffer_rmcios pbuffer = {
            .data = (char *)"a\0c",
            .length = 4,
            .size = 0,
            .required_size = 4,
            .trailing_size = 0
        };

        char rbuffer[10] = {0};
        struct buffer_rmcios breturnv = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };

        float freturnv = 0;
        struct combo_rmcios returnv = {
            .paramtype = binary_rmcios,
            .num_params = 1,
            .param = (const union param_rmcios)(&breturnv)
        };

        TEST_CASE(TEST_BINARY_TO_BINARY,"")
        {

            convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                    1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR( breturnv.data[0], 'a');
            TEST_ASSERT_EQUAL_INT( breturnv.data[1], 0);
            TEST_ASSERT_EQUAL_CHAR( breturnv.data[2], 'c');
            TEST_ASSERT_EQUAL_INT( breturnv.data[3], 0);
            TEST_ASSERT_EQUAL_INT( breturnv.length, 4);
            TEST_ASSERT_EQUAL_INT( breturnv.required_size, 4);
            TEST_ASSERT_EQUAL_INT( breturnv.trailing_size, 0);
        }

        TEST_CASE(TEST_BINARY_TO_COMBO_BINARY,"")
        { 
            struct combo_rmcios creturnv = {
                .paramtype = combo_rmcios,
                .num_params = 1,
                .param = (const union param_rmcios)(&returnv)
            };

            convert_func (0, 0, 0, write_rmcios, combo_rmcios, &creturnv,
                    1, (const union param_rmcios)&pbuffer);

            TEST_ASSERT_EQUAL_CHAR( breturnv.data[0], 'a');
            TEST_ASSERT_EQUAL_INT( breturnv.data[1], 0);
            TEST_ASSERT_EQUAL_CHAR( breturnv.data[2], 'c');
            TEST_ASSERT_EQUAL_INT( breturnv.data[3], 0);
            TEST_ASSERT_EQUAL_INT( breturnv.length, 4);
            TEST_ASSERT_EQUAL_INT( breturnv.required_size, 4);
            TEST_ASSERT_EQUAL_INT( breturnv.trailing_size, 0);
        }
    }

    TEST_CASE(TEST_BINARY_TO_BUFFER, "Test basic use")
    {
        char rbuffer[5] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 0,
            .size = sizeof(rbuffer),
            .required_size = 0,
            .trailing_size = 0
        };
        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[0],         'r');
        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[2],         'd');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length,        3);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].trailing_size, 0);                 
    }

    TEST_CASE(TEST_BINARY_TO_BUFFER_APPEND, "Test append to buffer with data")
    {
        char rbuffer[10] = {};
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 2,
            .size = 0,
            .required_size = 0,
            .trailing_size = 0
        };
        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[2], 'r');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 5);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
    }

    TEST_CASE(TEST_BINARY_TO_BUFFER_OVERFLOW, "Test to buffer that is not large enough")
    {
        struct buffer_rmcios retbuffer = {
            .data = rbuffer,
            .length = 2,
            .size = 2,
            .required_size = 0,
            .trailing_size = 0
        };
        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
    }
    
    return test_results();
}

