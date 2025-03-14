#include <stdint.h>

typedef int32_t i32;

#ifdef __cplusplus
extern "C" {
#endif
extern void testing_started_cb(void);
extern void test_file_cb(const char *file);
extern i32 test_function_cb(i32 (*f)(void), const char *fname);
extern void testing_finished_cb(i32 passed, i32 failed);
#ifdef __cplusplus
}
#endif

// From ./tests/vfpu.cpp
extern i32 __VFPU_init_test(void);
extern i32 VFPU_ASSERT_ptr_is_NULL_test(void);
extern i32 VFPU_ALIGNED_ASSERT_test(void);
extern i32 VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test(void);
extern i32 __VFPU_deinit_test(void);

int main(void) {
	i32 passed = 0;
	const i32 N_TESTS = 5;

	testing_started_cb();

	test_file_cb("./tests/vfpu.cpp");
	passed += test_function_cb(__VFPU_init_test, "__VFPU_init_test");
	passed += test_function_cb(VFPU_ASSERT_ptr_is_NULL_test, "VFPU_ASSERT_ptr_is_NULL_test");
	passed += test_function_cb(VFPU_ALIGNED_ASSERT_test, "VFPU_ALIGNED_ASSERT_test");
	passed += test_function_cb(VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test, "VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test");
	passed += test_function_cb(__VFPU_deinit_test, "__VFPU_deinit_test");

	testing_finished_cb(passed, N_TESTS - passed);

	return (passed == N_TESTS) ? 0 : 1;
}
