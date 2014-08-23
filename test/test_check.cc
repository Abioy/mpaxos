
#include <stdlib.h>
#include <check.h>

#include "test_hostname.cc"
#include "test_mpr_hash.cc"
#include "test_protobuf.cc"
#include "test_zfec.cc"
#include "test_hello.cc"
#include "test_rpc.cc"

//#include "test_rpc.c"


START_TEST (lalala)
{
    ck_assert_int_lt(1, 2);
    ck_assert_int_eq(3, 3);
}
END_TEST

START_TEST(hahaha) {
    char a[] = "hello";
    ck_assert_str_eq(a, "hello");
}END_TEST


Suite *check_suite (void) {
    Suite *s = suite_create("MPaxos");

    TCase *tc_core = tcase_create("Demo");
    tcase_add_test(tc_core, lalala);
    tcase_add_test(tc_core, hahaha);
    suite_add_tcase(s, tc_core);

    TCase *tc_util = tcase_create("Util");
    tcase_add_test(tc_util, hostname);
    suite_add_tcase(s, tc_util);
    
    TCase *tc_mpr = tcase_create("MPR");
    tcase_add_test(tc_mpr, mpr_hash);
    tcase_set_timeout(tc_mpr, 0);
    suite_add_tcase(s, tc_mpr);

    //TCase *tc_proto = tcase_create("Protobuf");
    //tcase_add_test(tc_proto, protobuf);
    //suite_add_tcase(s, tc_proto);

    TCase *tc_zfec = tcase_create("zfec");
    tcase_add_test(tc_zfec, zfec);
    suite_add_tcase(s, tc_zfec);

	/* wrong now
    TCase *tc_hello = tcase_create("hello");
    tcase_add_test(tc_hello, hello);
    suite_add_tcase(s, tc_hello);
*/
    TCase *tc_rpc = tcase_create("RPC");
    tcase_add_test(tc_rpc, rpc);
    suite_add_tcase(s, tc_rpc);

    return s;
}

int main() {
    int number_failed;
    Suite *s = NULL;
    SRunner *sr = NULL;

    s = check_suite();
    sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
