#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"

uint8_t MY_HEAP[SIZE_HEAP];

void test_simple(void){
    // Je test la fonction init
    init();
    uint16_t index = read_2bytes(0);
    CU_ASSERT_EQUAL(index, 2);

    // Je test la fonction get_first_free et si la heap a été bien préparer
    BeginBlock* bloc = get_first_free();
    CU_ASSERT_EQUAL(bloc->size, 63992)


    uint8_t* p1 = my_malloc(1);
    CU_ASSERT_EQUAL(p1, &MY_HEAP[6]);

    uint8_t* p2 = my_malloc(17);
    CU_ASSERT_EQUAL(p2, &MY_HEAP[13]);

    uint8_t* p3 = my_malloc(13);
    CU_ASSERT_EQUAL(p3, &MY_HEAP[36]);
    
    index = read_2bytes(0);
    CU_ASSERT_EQUAL(index, 51);

    bloc = get_first_free();
    CU_ASSERT_EQUAL(bloc->size, 63943);

    my_free(p2);
    index = read_2bytes(0);
    BeginBlock* p22 = ((BeginBlock*) &MY_HEAP[13]) - 1;
    CU_ASSERT_EQUAL(index, get_index(p22));

    bloc = get_first_free();
    CU_ASSERT_EQUAL(bloc->size, 17);

    printf("\n");
}


int main(int argc, char** argv) {
    srand(time(NULL));
    if (CUE_SUCCESS != CU_initialize_registry()) {return CU_get_error();}
    CU_pSuite test_malloc = CU_add_suite("Test malloc", NULL, NULL);
    if (test_malloc == NULL) {CU_cleanup_registry(); return CU_get_error();}
    if (CU_add_test(test_malloc, "test_simple", test_simple) == NULL) {
            CU_cleanup_registry(); return CU_get_error();
    }
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_basic_show_failures(CU_get_failure_list());
    int failed_tests = CU_get_number_of_failures();
    CU_cleanup_registry();
    return (failed_tests > 0) ? 1 : 0;
}