/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "fih_s_tests.h"
#include "fih.h"
#include "cmsis.h"

#define FIH_ERROR_CODE 0x5

#ifdef FIH_ENABLE_DOUBLE_VARS
extern fih_int _fih_cfi_ctr;
#endif

/*
 * From CoreSight version 3.0 onwards, implementation of the Software lock
 * mechanism that is controlled by LAR and LSR is deprecated and their
 * definitions may not be available.
 * Explicitly define them here.
 */
#define DWT_LAR (uint32_t *)(DWT_BASE + 0xFB0UL)
#define DWT_LSR (uint32_t *)(DWT_BASE + 0xFB4UL)
#define LSR_SLI_Pos  0U /*!< LSR, SLI: Software Lock Implemented */
#define LSR_SLI_Mask (1UL /* << LSR_SLI_Pos */)

#ifdef FIH_ENABLE_DELAY
/* If you intend to change the number of trials, also update the chi square critical
 * value from the chi square distribution table. */
#define NUM_TRIALS 50

/* Critical value from chi squared distribution table for a significance level of 5%  */
#define CHI_SQUARED_CRITICAL_VAL 67.505f
/*!
 * \brief Perform a chi-square test on the given integer array.
 *
 * This function calculates the chi-square statistic to evaluate the
 * uniformity of the distribution of the integer array. We don't
 * perform the binning of the data, so we assume the data is uniformly
 * distributed.
 *
 * \param data Pointer to the integer array.
 * \param size Size of the integer array.
 * \return The chi-square statistic.
 */
double chi_square_test(const unsigned int *data, int size)
{
    int expected = 0;
    double chi_stat = 0.0;

    /* Take the average of the data as the expected value */
    for (int i = 0; i < size; i++) {
        expected += data[i];
    }

    expected /= (size + 1);

    /* Calculate chi-square statistic */
    for (int i = 0; i < size; i++) {
        unsigned int diff = data[i] - expected;
        chi_stat += (double)(diff * diff) / (double)expected;
    }

    return chi_stat;
}

/*!
 * \brief Test the randomness of an integer array.
 *
 * This function performs statistical tests to evaluate the randomness
 * of the given integer array. It calculates the chi-square statistic
 * and performs a runs test to determine randomness.
 *
 * \param data Pointer to the integer array.
 * \param size Size of the integer array.
 * \param ret Pointer to the test result structure to store the result.
 */
void test_randomness(const unsigned int *data, int size, struct test_result_t *ret)
{
    double chi_stat = chi_square_test(data, size);

    TEST_LOG("Chi-Square Statistic: %d\r\n", (int)((double)chi_stat + 0.5f));

    /* Compare against the critical value */
    if (chi_stat >  CHI_SQUARED_CRITICAL_VAL) {
        TEST_LOG("The data appears to be reasonably random.\n");
        ret->val = TEST_PASSED;
    } else {
        TEST_FAIL("The data may not be random.\n");
    }
}
#endif /* FIH_ENABLE_DELAY */

/*!
 * \brief Executes a test function based on the provided mode and validates
 *        the Control Flow Integrity (CFI) counter if double variables are enabled.
 *
 * \param mode         The mode of operation for the test function:
 *                     - 0: Returns FIH_SUCCESS.
 *                     - 1: Returns FIH_FAILURE.
 *                     - 2: Returns an encoded error code using fih_ret_encode_zero_equality.
 *                     - Default: Fails the test with an invalid mode error.
 * \param cfi_counter  The expected value of the CFI counter to validate against
 *                     when FIH_ENABLE_DOUBLE_VARS is defined.
 * \param ret          Pointer to the test result structure.
 *
 * \return fih_ret     Encoded return value based on the mode or failure conditions.
 *
 */
static fih_ret fih_call_test_fn(int mode, int cfi_counter, struct test_result_t *ret)
{
#ifdef FIH_ENABLE_DOUBLE_VARS
    if (FIH_NOT_EQ(fih_int_decode(_fih_cfi_ctr), cfi_counter + 1)) {
        TEST_FAIL("The CFI counter does not match expected value.");
        FIH_RET(FIH_FAILURE);
    }
#endif
    switch (mode) {
    case 0:
        FIH_RET(FIH_SUCCESS);
        break;
    case 1:
        FIH_RET(FIH_FAILURE);
        break;
    case 2:
        FIH_RET(fih_ret_encode_zero_equality(FIH_ERROR_CODE));
        break;
    default:
        TEST_FAIL("Invalid mode for fih_call_test_fn.");
        FIH_RET(FIH_FAILURE);
    }
}

/*!
 * \brief Test FIH equality macros.
 *
 * This function tests the FIH_EQ and FIH_NOT_EQ macros by setting two
 * FIH variables and checking their equality and inequality.
 *
 * \param ret Pointer to the test result structure.
 */
static void tfm_fih_test_common_001(struct test_result_t *ret)
{
    FIH_DECLARE(a, FIH_SUCCESS);
    FIH_DECLARE(b, FIH_SUCCESS);

    if (FIH_NOT_EQ(a, b)) {
        TEST_FAIL("FIH_NOT_EQ failed to detect equality");
        return;
    }

    FIH_SET(b, FIH_FAILURE);
    if (FIH_EQ(a, b)) {
        TEST_FAIL("FIH_EQ failed to detect inequality");
        return;
    }

    ret->val = TEST_PASSED;
}

/*!
 * \brief Test FIH_CALL interface.
 *
 * This function tests the FIH_CALL interface by calling a test function
 * with different modes and validating the return values.
 *
 * \param ret Pointer to the test result structure.
 */
static void tfm_fih_test_common_002(struct test_result_t *ret)
{
    FIH_DECLARE(fih_ret, FIH_FAILURE);
    fih_int curr_cfi_ctr = fih_int_encode(0);

#ifdef FIH_ENABLE_DOUBLE_VARS
    curr_cfi_ctr = _fih_cfi_ctr;
#endif

    /* Call with mode 0 for success */
    FIH_CALL(fih_call_test_fn, fih_ret, 0, fih_int_decode(curr_cfi_ctr), ret);
    if (FIH_NOT_EQ(fih_ret, FIH_SUCCESS)) {
        TEST_FAIL("FIH_CALL failed");
        return;
    }

    /* Call with mode 1 for failure */
    FIH_CALL(fih_call_test_fn, fih_ret, 1, fih_int_decode(curr_cfi_ctr), ret);
    if (FIH_NOT_EQ(fih_ret, FIH_FAILURE)) {
        TEST_FAIL("FIH_CALL failed");
        return;
    }

    /* Call with mode 2 for error code. This ensures that the FIH is
     * properly encoding the error code that can be recovered back using
     * the decode function */
    FIH_CALL(fih_call_test_fn, fih_ret, 2, fih_int_decode(curr_cfi_ctr), ret);
    if (FIH_NOT_EQ(fih_ret_decode_zero_equality(fih_ret), FIH_ERROR_CODE)) {
        TEST_FAIL("FIH_CALL failed");
        return;
    }

    ret->val = TEST_PASSED;
}

#ifdef FIH_ENABLE_DELAY
/*!
 * \brief Common test function for FIH delay functionality.
 *
 * This function tests the FIH delay functionality by calling the fih_delay
 * function multiple times and measuring the cycles taken for each call.
 * It then performs a randomness test on the collected cycle data.
 *
 * \param ret Pointer to the test result structure.
 */
static void tfm_fih_test_common_003(struct test_result_t *ret)
{
#ifdef DWT
    unsigned int cycles;
    unsigned int data[NUM_TRIALS];

    for (int i = 0; i < NUM_TRIALS; i++) {
        cycles = DWT->CYCCNT;

        /* Call the FIH delay function and note the cycles everytime */
        fih_delay();

        data[i] = DWT->CYCCNT - cycles;
    }

    /* Perform randomness test on FIH delay cycles */
    test_randomness(data, NUM_TRIALS, ret);
#else
    TEST_LOG("Skipped because Data Watchpoint and Trace (DWT) unit is not available.\n");
    ret->val = TEST_SKIPPED;
#endif /* DWT */
}
#endif /* FIH_ENABLE_DELAY */

static struct test_t fih_tests[] =
{
    {&tfm_fih_test_common_001, "TFM_S_FIH_TEST_1001",
     "Test FIH equality macros"},
    {&tfm_fih_test_common_002, "TFM_S_FIH_TEST_1002",
     "Test FIH_CALL interface"},
#ifdef FIH_ENABLE_DELAY
    {&tfm_fih_test_common_003, "TFM_S_FIH_TEST_1003",
     "Test FIH delay functionality"},
#endif
};

/*!
 * \brief Registers the FIH Secure interface test suite.
 *
 * This function initializes and registers the test suite for the Fault Injection
 * Hardening (FIH) secure interface tests. It configures the Data Watchpoint
 * and Trace (DWT) unit to enable cycle counting and prepares the test suite
 * with the provided test cases.
 *
 * \param[in] p_test_suite Pointer to the test suite structure to be initialized.
 */
void register_testsuite_s_fih_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

#if defined(FIH_ENABLE_DELAY) && defined(DWT)
    DCB->DEMCR = DCB_DEMCR_TRCENA_Msk;

    if((*DWT_LSR & LSR_SLI_Mask) > 0)
    {
        /* Software Lock is implemented, enable write access to DWT unit */
        *DWT_LAR = 0xC5ACCE55;
    }

    /* Enable DWT unit for cycle counting and reset cycle counter */
    DWT->CTRL = DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0x0;
#endif /* FIH_ENABLE_DELAY && DWT */

    list_size = (sizeof(fih_tests) /
                 sizeof(fih_tests[0]));

    set_testsuite("FIH Secure interface tests"
                  "(TFM_S_FIH_TEST_1XXX)",
                  fih_tests, list_size, p_test_suite);
}