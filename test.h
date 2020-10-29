#ifndef test_h
#define test_h

#include <stdio.h>
#define UNEXECUTED 0
#define PASS 1
#define FAIL -1

int test_index = 0;
const char *test_description = "";
const char *test_label = "";
int do_test=0;
int test_case=-1;
int test_stage=0;

#define TEST_CASE(TEST_ENUM, DESCRIPTION) test_index=TEST_ENUM;test_label=#TEST_ENUM; test_description=DESCRIPTION;\


#define TEST_ASSERT_EQUAL(EXPECTED, GOT) if (EXPECTED != GOT){ test_status[test_index] = FAIL; \
                                               printf( "FAILED: %s:%d %s "  #EXPECTED " != " #GOT " %s\n",__FILE__ , __LINE__, test_label, test_description); } \
                                         else { if(test_status[test_index] != FAIL) test_status[test_index] = PASS; \
                                               printf( "PASSED: %s:%d %s\n",__FILE__ , __LINE__, test_label); } 

#define TEST_ASSERT_EQUAL_EX(EXPECTED, GOT, CAST_TO, PRINT_TYPE) if ((CAST_TO)EXPECTED != (CAST_TO)GOT){ test_status[test_index] = FAIL; \
                                               printf( "FAILED: %s:%d %s "  #EXPECTED " = " PRINT_TYPE " != " #GOT " %s\n",__FILE__ , __LINE__, test_label, (CAST_TO)EXPECTED, test_description); } \
                                         else { if(test_status[test_index] != FAIL) test_status[test_index] = PASS; \
                                               printf( "PASSED: %s:%d %s\n",__FILE__ , __LINE__, test_label); } 

#define TEST_ASSERT_EQUAL_INT(EXPECTED, GOT)  TEST_ASSERT_EQUAL_EX(EXPECTED, GOT, int, "%d")
#define TEST_ASSERT_EQUAL_CHAR(EXPECTED, GOT)  TEST_ASSERT_EQUAL_EX(EXPECTED, GOT, char, "'%c'")
#define TEST_ASSERT_EQUAL_FLOAT(EXPECTED, GOT)  TEST_ASSERT_EQUAL_EX(EXPECTED, GOT, float, "%f")

/*if ((int)EXPECTED != (int)GOT){ test_status[test_index] = FAIL; \
                                               printf( "FAILED: %s:%d %s "  #EXPECTED " = %d != " #GOT " %s\n",__FILE__ , __LINE__, test_label, (int)EXPECTED, test_description); } \
                                             else { if(test_status[test_index] != FAIL) test_status[test_index] = PASS; \
                                               printf( "PASSED: %s:%d %s\n",__FILE__ , __LINE__, test_label); } 
*/
#endif // test_h

