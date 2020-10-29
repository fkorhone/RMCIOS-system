#include "test.h"

#include "convert.c"
enum test_cases {
    TEST_INT_TO_INT = 0,
    TEST_INT_TO_FLOAT,
    TEST_INT_TO_BUFFER_STR,
    TEST_INT_TO_BUFFER_FULL,
    TEST_INT_TO_BUFFER_OVERFLOW,
    TEST_INT_TO_CHANNEL,
    TEST_INT_TO_BINARY,
    TEST_INT_TO_COMBO_INT,

    TEST_FLOAT_TO_INT,
    TEST_FLOAT_TO_FLOAT,
    TEST_FLOAT_TO_BUFFER_STR,
    TEST_FLOAT_TO_BUFFER_FULL,
    TEST_FLOAT_TO_BUFFER_OVERFLOW,
    TEST_FLOAT_TO_CHANNEL,
    TEST_FLOAT_TO_BINARY,
    TEST_FLOAT_TO_COMBO_FLOAT,

    TEST_BUFFER_TO_INT,
    TEST_BUFFER_TO_FLOAT,
    TEST_BUFFER_TO_BUFFER,
    TEST_BUFFER_TO_CHANNEL,
    TEST_BUFFER_TO_BINARY,
    TEST_BUFFER_TO_COMBO_BUFFER,

    TEST_BINARY_TO_INT,
    TEST_BINARY_TO_FLOAT,
    TEST_BINARY_TO_BUFFER,
    TEST_BINARY_TO_CHANNEL,
    TEST_BINARY_TO_BINARY,
    TEST_BINARY_TO_COMBO_BINARY,
    TEST_BINARY_TO_BUFFER_DATA,
    TEST_BINARY_TO_BUFFER_APPEND,
    TEST_BINARY_TO_BUFFER_OVERFLOW,
    TOTAL_TEST_CASES
};

char test_status[TOTAL_TEST_CASES] = {0};


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
        retbuffer.length = 0;
        retbuffer.size = 2;
        
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
        retbuffer.length = 0;
        retbuffer.size = 2;
 
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
        retbuffer.length = 0;
        retbuffer.size = 2;
        
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
        retbuffer.length = 0;
        retbuffer.size = 2;
 
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

    TEST_CASE(TEST_FLOAT_TO_CHANNEL,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_FLOAT_TO_BINARY,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_FLOAT_TO_COMBO_FLOAT,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }


    TEST_CASE(TEST_BUFFER_TO_INT,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BUFFER_TO_FLOAT,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BUFFER_TO_BUFFER,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BUFFER_TO_CHANNEL,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BUFFER_TO_BINARY,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BUFFER_TO_COMBO_BUFFER,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BINARY_TO_INT,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }
    
    TEST_CASE(TEST_BINARY_TO_FLOAT,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }
    
    TEST_CASE(TEST_BINARY_TO_BUFFER,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BINARY_TO_CHANNEL,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BINARY_TO_BINARY,"")
    {

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BINARY_TO_COMBO_BINARY,"")
    {
        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                      1, (const union param_rmcios)&pbuffer);
    }

    TEST_CASE(TEST_BINARY_TO_BUFFER, "Test basic use")
    {
        retbuffer.size = 5;
        retbuffer.length = 0;

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
        retbuffer.size = 0;
        retbuffer.length = 2;

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_CHAR(returnv.param.bv[0].data[2], 'r');
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 5);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
    }

    TEST_CASE(TEST_BINARY_TO_BUFFER_OVERFLOW, "Test to buffer that is not large enough")
    {
        retbuffer.size = 2;
        retbuffer.length = 2;

        convert_func (0, 0, 0, write_rmcios, binary_rmcios, &returnv,
                1, (const union param_rmcios)&pbuffer);

        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].length, 2);
        TEST_ASSERT_EQUAL_INT(returnv.param.bv[0].required_size, 3);
    }

    // Result summary:
    int passed = 0;
    int failed = 0;
    int unexecuted = 0;
    int i;
    for (i = 0; i < TOTAL_TEST_CASES; i++)
    {
        if (test_status[i] == PASS)
        {
            passed++;
        }
        if (test_status[i] == FAIL)
        {
            failed++;
        }
        if (test_status[i] == UNEXECUTED)
        {
            unexecuted++;
        }
    }
    printf("\npassed:%d failed:%d unexecuted:%d total:%d\n", passed, failed, unexecuted, TOTAL_TEST_CASES);
    if (failed > 0 || unexecuted > 0) return EXIT_FAILURE;
    else return EXIT_SUCCESS ;
}

