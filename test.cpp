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

// From C-CPP-CodeBase/tests/c.c
#ifdef __cplusplus
extern "C" {
#endif
extern i32 MIN_test(void);
extern i32 MAX_test(void);
extern i32 C_ARR_LEN_test(void);
extern i32 C_ARR_LEN_NULL_test(void);
extern i32 C_ARR_LEN_arg_ptr_test(void);
extern i32 MEMBER_OFFSET_test(void);
extern i32 c_arr_check_test(void);
extern i32 c_arr_idx_check_test(void);
extern i32 c_arr_2d_idx_test(void);
extern i32 c_arr_2d_check_test(void);
extern i32 c_arr_2d_idx_check_test(void);
#ifdef __cplusplus
}
#endif

// From C-CPP-CodeBase/tests/cpp.cpp
extern i32 SWAP_test(void);
extern i32 Buf_end_test(void);
extern i32 Buf_operator_indirect_test(void);
extern i32 Buf_operator_subscript_test(void);
extern i32 Buf_operator_add_test(void);
extern i32 Buf2D_get_test(void);
extern i32 Buf2D_len_test(void);
extern i32 Buf2D_operator_subscript_test(void);
extern i32 Arr_len_test(void);
extern i32 Arr_mag_test(void);
extern i32 Arr_norm_test(void);
extern i32 Arr_x_test(void);
extern i32 Arr_y_test(void);
extern i32 Arr_z_test(void);
extern i32 Arr_w_test(void);
extern i32 Arr_xy_test(void);
extern i32 Arr_xyz_test(void);
extern i32 Arr_cast_test(void);
extern i32 Arr_ones_test(void);
extern i32 Arr_zeros_test(void);
extern i32 Arr_dot_test(void);
extern i32 Arr_cross_test(void);
extern i32 Arr_mix_test(void);
extern i32 Arr_bary_test(void);
extern i32 Arr_operator_subscript_test(void);
extern i32 Arr_operator_add_i32_test(void);
extern i32 Arr_operator_neg_test(void);
extern i32 Arr_operator_add_Arr_test(void);
extern i32 Arr_operator_add_assign_test(void);
extern i32 Arr_operator_div_test(void);
extern i32 Arr_operator_sub_test(void);
extern i32 Arr_operator_eq_test(void);
extern i32 Arr_operator_mul_test(void);
extern i32 Arr2D_rows_test(void);
extern i32 Arr2D_cols_test(void);
extern i32 Arr2D_buf2d_test(void);
extern i32 Mat_n_test(void);
extern i32 Mat_len_test(void);
extern i32 Mat_det_test(void);
extern i32 Mat_trans_test(void);
extern i32 Mat_inverse_test(void);
extern i32 Mat_cofactor_test(void);
extern i32 Mat_minor_test(void);
extern i32 Mat_get_test(void);
extern i32 Mat_getp_test(void);
extern i32 Mat_I_test(void);
extern i32 Mat_ones_test(void);
extern i32 Mat_zeros_test(void);
extern i32 Mat_operator_mul_Arr_test(void);
extern i32 Mat_operator_mul_Mat_test(void);
extern i32 Mat_operator_eq_test(void);
extern i32 Mat_operator_mul_test(void);
extern i32 min_test(void);
extern i32 max_test(void);
extern i32 clamp_test(void);
extern i32 eq_test(void);
extern i32 eq__float_test(void);
extern i32 map_range_test(void);
extern i32 mag_v_test(void);
extern i32 cast_v_test(void);
extern i32 fill_v_test(void);
extern i32 neg_v_test(void);
extern i32 norm_v_test(void);
extern i32 dot_v_test(void);
extern i32 eq_v_test(void);
extern i32 add_v_test(void);
extern i32 sub_v_test(void);
extern i32 mul_vs_test(void);
extern i32 div_vs_test(void);
extern i32 mix_v_test(void);
extern i32 bary_v_test(void);
extern i32 I_m_test(void);
extern i32 fill_m_test(void);
extern i32 det_m_test(void);
extern i32 det_m__0_test(void);
extern i32 trans_m_test(void);
extern i32 inverse_m_test(void);
extern i32 cofactor_m_test(void);
extern i32 minor_m_test(void);
extern i32 mul_mv_test(void);
extern i32 mul_mm_test(void);
extern i32 mul_mm_many_test(void);

// From tests/vfpu.cpp
extern i32 __VFPU_init_test(void);
extern i32 VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test(void);
extern i32 VFPU_LOAD_M4__and__VFPU_STORE_M4_test(void);
extern i32 Mat_operator_mul_Mat__4_float_test(void);
extern i32 vfpu_check_test(void);
extern i32 vfpu_aligned_check_test(void);
extern i32 mul_mm__4_float_test(void);
extern i32 __VFPU_deinit_test(void);

int main(void) {
	i32 passed = 0;
	const i32 N_TESTS = 101;

	testing_started_cb();

	test_file_cb("C-CPP-CodeBase/tests/c.c");
	passed += test_function_cb(MIN_test, "MIN_test");
	passed += test_function_cb(MAX_test, "MAX_test");
	passed += test_function_cb(C_ARR_LEN_test, "C_ARR_LEN_test");
	passed += test_function_cb(C_ARR_LEN_NULL_test, "C_ARR_LEN_NULL_test");
	passed += test_function_cb(C_ARR_LEN_arg_ptr_test, "C_ARR_LEN_arg_ptr_test");
	passed += test_function_cb(MEMBER_OFFSET_test, "MEMBER_OFFSET_test");
	passed += test_function_cb(c_arr_check_test, "c_arr_check_test");
	passed += test_function_cb(c_arr_idx_check_test, "c_arr_idx_check_test");
	passed += test_function_cb(c_arr_2d_idx_test, "c_arr_2d_idx_test");
	passed += test_function_cb(c_arr_2d_check_test, "c_arr_2d_check_test");
	passed += test_function_cb(c_arr_2d_idx_check_test, "c_arr_2d_idx_check_test");

	test_file_cb("C-CPP-CodeBase/tests/cpp.cpp");
	passed += test_function_cb(SWAP_test, "SWAP_test");
	passed += test_function_cb(Buf_end_test, "Buf_end_test");
	passed += test_function_cb(Buf_operator_indirect_test, "Buf_operator_indirect_test");
	passed += test_function_cb(Buf_operator_subscript_test, "Buf_operator_subscript_test");
	passed += test_function_cb(Buf_operator_add_test, "Buf_operator_add_test");
	passed += test_function_cb(Buf2D_get_test, "Buf2D_get_test");
	passed += test_function_cb(Buf2D_len_test, "Buf2D_len_test");
	passed += test_function_cb(Buf2D_operator_subscript_test, "Buf2D_operator_subscript_test");
	passed += test_function_cb(Arr_len_test, "Arr_len_test");
	passed += test_function_cb(Arr_mag_test, "Arr_mag_test");
	passed += test_function_cb(Arr_norm_test, "Arr_norm_test");
	passed += test_function_cb(Arr_x_test, "Arr_x_test");
	passed += test_function_cb(Arr_y_test, "Arr_y_test");
	passed += test_function_cb(Arr_z_test, "Arr_z_test");
	passed += test_function_cb(Arr_w_test, "Arr_w_test");
	passed += test_function_cb(Arr_xy_test, "Arr_xy_test");
	passed += test_function_cb(Arr_xyz_test, "Arr_xyz_test");
	passed += test_function_cb(Arr_cast_test, "Arr_cast_test");
	passed += test_function_cb(Arr_ones_test, "Arr_ones_test");
	passed += test_function_cb(Arr_zeros_test, "Arr_zeros_test");
	passed += test_function_cb(Arr_dot_test, "Arr_dot_test");
	passed += test_function_cb(Arr_cross_test, "Arr_cross_test");
	passed += test_function_cb(Arr_mix_test, "Arr_mix_test");
	passed += test_function_cb(Arr_bary_test, "Arr_bary_test");
	passed += test_function_cb(Arr_operator_subscript_test, "Arr_operator_subscript_test");
	passed += test_function_cb(Arr_operator_add_i32_test, "Arr_operator_add_i32_test");
	passed += test_function_cb(Arr_operator_neg_test, "Arr_operator_neg_test");
	passed += test_function_cb(Arr_operator_add_Arr_test, "Arr_operator_add_Arr_test");
	passed += test_function_cb(Arr_operator_add_assign_test, "Arr_operator_add_assign_test");
	passed += test_function_cb(Arr_operator_div_test, "Arr_operator_div_test");
	passed += test_function_cb(Arr_operator_sub_test, "Arr_operator_sub_test");
	passed += test_function_cb(Arr_operator_eq_test, "Arr_operator_eq_test");
	passed += test_function_cb(Arr_operator_mul_test, "Arr_operator_mul_test");
	passed += test_function_cb(Arr2D_rows_test, "Arr2D_rows_test");
	passed += test_function_cb(Arr2D_cols_test, "Arr2D_cols_test");
	passed += test_function_cb(Arr2D_buf2d_test, "Arr2D_buf2d_test");
	passed += test_function_cb(Mat_n_test, "Mat_n_test");
	passed += test_function_cb(Mat_len_test, "Mat_len_test");
	passed += test_function_cb(Mat_det_test, "Mat_det_test");
	passed += test_function_cb(Mat_trans_test, "Mat_trans_test");
	passed += test_function_cb(Mat_inverse_test, "Mat_inverse_test");
	passed += test_function_cb(Mat_cofactor_test, "Mat_cofactor_test");
	passed += test_function_cb(Mat_minor_test, "Mat_minor_test");
	passed += test_function_cb(Mat_get_test, "Mat_get_test");
	passed += test_function_cb(Mat_getp_test, "Mat_getp_test");
	passed += test_function_cb(Mat_I_test, "Mat_I_test");
	passed += test_function_cb(Mat_ones_test, "Mat_ones_test");
	passed += test_function_cb(Mat_zeros_test, "Mat_zeros_test");
	passed += test_function_cb(Mat_operator_mul_Arr_test, "Mat_operator_mul_Arr_test");
	passed += test_function_cb(Mat_operator_mul_Mat_test, "Mat_operator_mul_Mat_test");
	passed += test_function_cb(Mat_operator_eq_test, "Mat_operator_eq_test");
	passed += test_function_cb(Mat_operator_mul_test, "Mat_operator_mul_test");
	passed += test_function_cb(min_test, "min_test");
	passed += test_function_cb(max_test, "max_test");
	passed += test_function_cb(clamp_test, "clamp_test");
	passed += test_function_cb(eq_test, "eq_test");
	passed += test_function_cb(eq__float_test, "eq__float_test");
	passed += test_function_cb(map_range_test, "map_range_test");
	passed += test_function_cb(mag_v_test, "mag_v_test");
	passed += test_function_cb(cast_v_test, "cast_v_test");
	passed += test_function_cb(fill_v_test, "fill_v_test");
	passed += test_function_cb(neg_v_test, "neg_v_test");
	passed += test_function_cb(norm_v_test, "norm_v_test");
	passed += test_function_cb(dot_v_test, "dot_v_test");
	passed += test_function_cb(eq_v_test, "eq_v_test");
	passed += test_function_cb(add_v_test, "add_v_test");
	passed += test_function_cb(sub_v_test, "sub_v_test");
	passed += test_function_cb(mul_vs_test, "mul_vs_test");
	passed += test_function_cb(div_vs_test, "div_vs_test");
	passed += test_function_cb(mix_v_test, "mix_v_test");
	passed += test_function_cb(bary_v_test, "bary_v_test");
	passed += test_function_cb(I_m_test, "I_m_test");
	passed += test_function_cb(fill_m_test, "fill_m_test");
	passed += test_function_cb(det_m_test, "det_m_test");
	passed += test_function_cb(det_m__0_test, "det_m__0_test");
	passed += test_function_cb(trans_m_test, "trans_m_test");
	passed += test_function_cb(inverse_m_test, "inverse_m_test");
	passed += test_function_cb(cofactor_m_test, "cofactor_m_test");
	passed += test_function_cb(minor_m_test, "minor_m_test");
	passed += test_function_cb(mul_mv_test, "mul_mv_test");
	passed += test_function_cb(mul_mm_test, "mul_mm_test");
	passed += test_function_cb(mul_mm_many_test, "mul_mm_many_test");

	test_file_cb("tests/vfpu.cpp");
	passed += test_function_cb(__VFPU_init_test, "__VFPU_init_test");
	passed += test_function_cb(VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test, "VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test");
	passed += test_function_cb(VFPU_LOAD_M4__and__VFPU_STORE_M4_test, "VFPU_LOAD_M4__and__VFPU_STORE_M4_test");
	passed += test_function_cb(Mat_operator_mul_Mat__4_float_test, "Mat_operator_mul_Mat__4_float_test");
	passed += test_function_cb(vfpu_check_test, "vfpu_check_test");
	passed += test_function_cb(vfpu_aligned_check_test, "vfpu_aligned_check_test");
	passed += test_function_cb(mul_mm__4_float_test, "mul_mm__4_float_test");
	passed += test_function_cb(__VFPU_deinit_test, "__VFPU_deinit_test");

	testing_finished_cb(passed, N_TESTS - passed);

	return (passed == N_TESTS) ? 0 : 1;
}
