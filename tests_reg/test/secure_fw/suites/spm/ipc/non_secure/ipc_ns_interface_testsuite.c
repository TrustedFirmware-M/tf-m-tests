/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "ipc_ns_tests.h"
#include "psa/client.h"
#include "psa/framework_feature.h"
#include "test_framework_helpers.h"
#include "psa_manifest/sid.h"
#include "client_api_tests.h"
#include "spm_test_defs.h"
#if PSA_FRAMEWORK_HAS_MM_IOVEC
#include "mmiovec_test.h"
#endif

/* List of tests */
static void tfm_ipc_test_1001(struct test_result_t *ret);
static void tfm_ipc_test_1002(struct test_result_t *ret);
static void tfm_ipc_test_1003(struct test_result_t *ret);
static void tfm_ipc_test_1004(struct test_result_t *ret);

static void tfm_ipc_test_1010(struct test_result_t *ret);

static void tfm_ipc_test_1012(struct test_result_t *ret);

#if PSA_FRAMEWORK_HAS_MM_IOVEC
static void tfm_ipc_test_1013(struct test_result_t *ret);

static void tfm_ipc_test_1014(struct test_result_t *ret);

static void tfm_ipc_test_1015(struct test_result_t *ret);

static void tfm_ipc_test_1019(struct test_result_t *ret);

static void tfm_ipc_test_1020(struct test_result_t *ret);

static void tfm_ipc_test_1021(struct test_result_t *ret);
#endif

static void tfm_ipc_test_1016(struct test_result_t *ret);

static void tfm_ipc_test_1017(struct test_result_t *ret);

static void tfm_ipc_test_1018(struct test_result_t *ret);

static struct test_t ipc_veneers_tests[] = {
    {&tfm_ipc_test_1001, "TFM_NS_IPC_TEST_1001",
     "Get PSA framework version"},
    {&tfm_ipc_test_1002, "TFM_NS_IPC_TEST_1002",
     "Get version of an RoT Service"},
    {&tfm_ipc_test_1003, "TFM_NS_IPC_TEST_1003",
     "Connect to an RoT Service"},
    {&tfm_ipc_test_1004, "TFM_NS_IPC_TEST_1004",
     "Request connection-based RoT Service"},
    {&tfm_ipc_test_1010, "TFM_NS_IPC_TEST_1010",
     "Test psa_call with the status of PSA_ERROR_PROGRAMMER_ERROR"},
    {&tfm_ipc_test_1012, "TFM_NS_IPC_TEST_1012",
     "Request stateless service"},
#if PSA_FRAMEWORK_HAS_MM_IOVEC
    {&tfm_ipc_test_1013, "TFM_NS_IPC_TEST_1013",
     "Mapping input vectors and unmapping them"},
    {&tfm_ipc_test_1014, "TFM_NS_IPC_TEST_1014",
     "Mapping output vectors and unmapping them"},
    {&tfm_ipc_test_1015, "TFM_NS_IPC_TEST_1015",
     "Mapping output vectors and not unmapping them"},
    {&tfm_ipc_test_1019, "TFM_NS_IPC_TEST_1019",
     "Testing connection based mapping and unmapping of input vectors"},
    {&tfm_ipc_test_1020, "TFM_NS_IPC_TEST_1020",
     "Testing connection based mapping and unmapping of output vectors"},
    {&tfm_ipc_test_1021, "TFM_NS_IPC_TEST_1021",
     "Testing connection based mapping of output vectors without unmapping them"},
#endif

    {&tfm_ipc_test_1016, "TFM_NS_IPC_TEST_1016",
     "Testing Client-Id Translation"},
    {&tfm_ipc_test_1017, "TFM_NS_IPC_TEST_1017",
     "Testing Reverse-Handle rhandle"},
    {&tfm_ipc_test_1018, "TFM_NS_IPC_TEST_1018",
     "Testing Refused connection"},
};

void register_testsuite_ns_ipc_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

    list_size = (sizeof(ipc_veneers_tests) / sizeof(ipc_veneers_tests[0]));

    set_testsuite("IPC non-secure interface test (TFM_NS_IPC_TEST_1XXX)",
                  ipc_veneers_tests, list_size, p_test_suite);
}

/**
 * \brief Retrieve the version of the PSA Framework API.
 *
 * \note This is a functional test only and doesn't
 *       mean to test all possible combinations of
 *       input parameters and return values.
 */
static void tfm_ipc_test_1001(struct test_result_t *ret)
{
    psa_framework_version_test(ret);
}

/**
 * \brief Retrieve the version of an RoT Service.
 */
static void tfm_ipc_test_1002(struct test_result_t *ret)
{
    psa_version_test(IPC_SERVICE_TEST_BASIC_SID, ret);
}

/**
 * \brief Connect to an RoT Service by its SID.
 */
static void tfm_ipc_test_1003(struct test_result_t *ret)
{
    psa_handle_t handle;

    handle = psa_connect(IPC_SERVICE_TEST_BASIC_SID,
                         IPC_SERVICE_TEST_BASIC_VERSION);
    if (handle > 0) {
        TEST_LOG("Connect success!\r\n");
    } else {
        TEST_FAIL("The RoT Service has refused the connection!\r\n");
        return;
    }
    psa_close(handle);
    ret->val = TEST_PASSED;
}

/**
 * \brief Call a Connection-based RoT Service.
 */
static void tfm_ipc_test_1004(struct test_result_t *ret)
{
    psa_handle_t handle;

    handle = psa_connect(IPC_SERVICE_TEST_BASIC_SID,
                         IPC_SERVICE_TEST_BASIC_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        TEST_FAIL("Connection to service failed!\r\n");
        return;
    }

    request_rot_service_test(handle, ret);

    psa_close(handle);
}

/**
 * \brief Call IPC_SERVICE_TEST_CLIENT_PREGRAMMER_ERROR RoT Service to
 *  test psa_call with the status of PSA_ERROR_PROGRAMMER_ERROR.
 */
static void tfm_ipc_test_1010(struct test_result_t *ret)
{
    psa_handle_t handle;
    psa_status_t status;
    handle = psa_connect(IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_SID,
                         IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_VERSION);
    if (handle > 0) {
        TEST_LOG("Connect success!\r\n");
    } else {
        TEST_LOG("The RoT Service has refused the connection!\r\n");
        ret->val = TEST_FAILED;
        return;
    }
    status = psa_call(handle, PSA_IPC_CALL, NULL, 0, NULL, 0);
    if (status == PSA_ERROR_PROGRAMMER_ERROR) {
        TEST_LOG("The first time call success!\r\n");
    } else {
        TEST_LOG("The first time call failed!\r\n");
        ret->val = TEST_FAILED;
    }
    status = psa_call(handle, PSA_IPC_CALL, NULL, 0, NULL, 0);
    if (status == PSA_ERROR_PROGRAMMER_ERROR) {
        TEST_LOG("The second time call success!\r\n");
    } else {
        TEST_LOG("The second time call failed!\r\n");
        ret->val = TEST_FAILED;
    }

    psa_close(handle);
}

/**
 * \brief Accessing a stateless service
 *
 * \note Accessing stateless service from non-secure client.
 */
static void tfm_ipc_test_1012(struct test_result_t *ret)
{
    psa_handle_t handle;

    /* Connecting to a stateless service should fail. */
    handle = psa_connect(IPC_SERVICE_TEST_STATELESS_ROT_SID,
                         IPC_SERVICE_TEST_STATELESS_ROT_VERSION);
    if (PSA_HANDLE_IS_VALID(handle)) {
        TEST_FAIL("Connecting to stateless service test should fail.\r\n");
        return;
    }

    request_rot_service_test(IPC_SERVICE_TEST_STATELESS_ROT_HANDLE, ret);
}

static void tfm_ipc_test_1016(struct test_result_t *ret)
{
    psa_handle_t handle;
    psa_status_t status;

    handle = psa_connect(IPC_SERVICE_TEST_CLIENT_ID_TRANSLATE_SID,
                         IPC_SERVICE_TEST_CLIENT_ID_TRANSLATE_VERSION);
    if (handle < 0) {
        TEST_FAIL("The RoT Service refused the connection!\r\n");
        return;
    }

    status = psa_call(handle, CLIENT_ID_TRANSLATE_TEST_TYPE_REQUEST_SRVC,
                      NULL, 0, NULL, 0);
    if (status >= 0) {
        ret->val = TEST_PASSED;
    } else {
        TEST_FAIL("Call failed!\r\n");
        ret->val = TEST_FAILED;
    }

    psa_close(handle);
}

/**
 * \brief Test the reverse handle "rhandle" by creating two connections and
 *  performing two calls on each connection. The target service will increment
 *  its own client-bound private data on each call and will return the
 *  incremented value in the outvec buffer for validation.
 */
static void tfm_ipc_test_1017(struct test_result_t *ret)
{
    psa_handle_t handle, handle2;
    psa_status_t status;

    uint8_t srvc_cnt = 0;
    struct psa_outvec outvecs[1] = {{&srvc_cnt, sizeof(srvc_cnt)}};
    uint32_t failed = 0;

    ret->val = TEST_FAILED;

    /* First connection handle, connect and call twice */
    handle = psa_connect(IPC_SERVICE_TEST_RHANDLE_CHECK_SID,
                         IPC_SERVICE_TEST_RHANDLE_CHECK_VERSION);
    if (handle < 0) {
        TEST_FAIL("The RoT Service refused the connection!\r\n");
        return;
    }

    /*
     * Perform 2 rounds of calls, the service is expected to write in outvec the
     * incremented value of its private data upon each call.
     */
    status = psa_call(handle, IPC_SERVICE_RHANDLE_CHECK_TYPE_REQUEST_SRVC,
                      NULL, 0, &outvecs[0], 1);
    if (status < 0) {
        TEST_FAIL("Call 1.0 failed!\r\n");
        return;
    }

    status = psa_call(handle, IPC_SERVICE_RHANDLE_CHECK_TYPE_REQUEST_SRVC,
                      NULL, 0, &outvecs[0], 1);
    if (status < 0) {
        TEST_FAIL("Call 1.1 failed!\r\n");
        return;
    }

    /*
     * At each call the service will have incremented a counter, starting from
     * 0x00.
     */
    failed |= (srvc_cnt != 0x02) ? 1 : 0;

    /* Second connection handle, connect and call twice */
    handle2 = psa_connect(IPC_SERVICE_TEST_RHANDLE_CHECK_SID,
                         IPC_SERVICE_TEST_RHANDLE_CHECK_VERSION);
    if (handle2 < 0) {
        TEST_FAIL("The RoT Service refused the connection!\r\n");
        return;
    }

    /*
     * Perform 2 rounds of calls, the service is expected to write in outvec the
     * incremented value of its private data upon each call.
     */
    srvc_cnt = 0;
    status = psa_call(handle2, IPC_SERVICE_RHANDLE_CHECK_TYPE_REQUEST_SRVC,
                      NULL, 0, &outvecs[0], 1);
    if (status < 0) {
        TEST_FAIL("Call 2.0 failed!\r\n");
        return;
    }

    status = psa_call(handle2, IPC_SERVICE_RHANDLE_CHECK_TYPE_REQUEST_SRVC,
                      NULL, 0, &outvecs[0], 1);
    if (status < 0) {
        TEST_FAIL("Call 2.1 failed!\r\n");
        return;
    }

    /*
     * At each call the service will have incremented a counter, starting from
     * 0xF0.
     */
    failed |= (srvc_cnt != 0xF2) ? 1 : 0;

    if (failed > 0) {
        TEST_FAIL("Test failed!\r\n");
    } else {
        ret->val = TEST_PASSED;
    }

    psa_close(handle);
    psa_close(handle2);
}

static void tfm_ipc_test_1018(struct test_result_t *ret)
{
    psa_handle_t handle;

    handle = psa_connect(IPC_SERVICE_TEST_CONNECTION_REFUSED_SID,
                         IPC_SERVICE_TEST_CONNECTION_REFUSED_VERSION);
    if (handle != PSA_ERROR_CONNECTION_REFUSED) {
        TEST_FAIL("The RoT Service was expected to refuse the connection!\r\n");
        ret->val = TEST_FAILED;
        return;
    }

    ret->val = TEST_PASSED;
}

#if PSA_FRAMEWORK_HAS_MM_IOVEC

/**
 * \brief Mapping input vectors and unmapping them.
 *
 * \note Test psa_map_invec() and psa_unmap_invec() functionality.
 */
static void tfm_ipc_test_1013(struct test_result_t *ret)
{
    invec_map_unmap_test(ret, IPC_SERVICE_TEST_MMIOVEC_HANDLE);
}

/**
 * \brief Mapping output vectors and unmapping them.
 *
 * \note Test psa_map_outvec() and psa_unmap_outvec() functionality.
 */
static void tfm_ipc_test_1014(struct test_result_t *ret)
{
    outvec_map_unmap_test(ret, IPC_SERVICE_TEST_MMIOVEC_HANDLE);
}

/**
 * \brief Mapping output vectors and not unmapping them.
 *
 * \note RoT services map outvecs and does not unmap outvecs, the service caller
 *       should get a zero out length.
 */
static void tfm_ipc_test_1015(struct test_result_t *ret)
{
    outvec_map_only_test(ret, IPC_SERVICE_TEST_MMIOVEC_HANDLE);
}

/**
 * \brief Connection-based test for mapping input vectors and unmapping them.
 *
 * \note Test psa_map_invec() and psa_unmap_invec() functionality.
 */
static void tfm_ipc_test_1019(struct test_result_t *ret)
{
    psa_handle_t handle;
    ret->val = TEST_FAILED;

    handle = psa_connect(IPC_SERVICE_TEST_MMIOVEC_STATEFUL_SID,
                         IPC_SERVICE_TEST_MMIOVEC_STATEFUL_VERSION);

    if (handle > 0) {
        invec_map_unmap_test(ret, handle);
        psa_close(handle);
        ret->val = TEST_PASSED;
    } else {
        TEST_FAIL("Connection to the service failed!\r\n");
    }
}

/**
 * \brief Connection-based test for mapping output vectors and unmapping them.
 *
 * \note Test psa_map_outvec() and psa_unmap_outvec() functionality.
 */
static void tfm_ipc_test_1020(struct test_result_t *ret)
{
    psa_handle_t handle;
    ret->val = TEST_FAILED;

    handle = psa_connect(IPC_SERVICE_TEST_MMIOVEC_STATEFUL_SID,
                         IPC_SERVICE_TEST_MMIOVEC_STATEFUL_VERSION);

    if (handle > 0) {
        outvec_map_unmap_test(ret, handle);
        psa_close(handle);
        ret->val = TEST_PASSED;
    } else {
        TEST_FAIL("Connection to the service failed!\r\n");
    }
}

/**
 * \brief Connection-based test for mapping of output vectors without unmapping them.
 *
 * \note RoT services map outvecs and does not unmap outvecs, the service caller
 *       should get a zero out length.
 */
static void tfm_ipc_test_1021(struct test_result_t *ret)
{
    psa_handle_t handle;
    ret->val = TEST_FAILED;

    handle = psa_connect(IPC_SERVICE_TEST_MMIOVEC_STATEFUL_SID,
                         IPC_SERVICE_TEST_MMIOVEC_STATEFUL_VERSION);

    if (handle > 0) {
        outvec_map_only_test(ret, handle);
        psa_close(handle);
        ret->val = TEST_PASSED;
    } else {
        TEST_FAIL("Connection to the service failed!\r\n");
    }
}
#endif
