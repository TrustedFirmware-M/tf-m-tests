/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <stdbool.h>
#include "psa/client.h"
#include "psa/service.h"
#include "psa_manifest/tfm_ipc_service_test.h"
#include "spm_test_defs.h"
#include "tfm_hal_isolation.h"
#include "tfm_log_unpriv.h"
#include "client_api_test_defs.h"
#include "client_api_test_service.h"

#include "tfm_hal_multi_core.h"
#include "tfm_plat_defs.h"
#include "internal_status_code.h"

#if PSA_FRAMEWORK_HAS_MM_IOVEC
#include "tfm_mmiovec_test_service.h"
#endif

#define IPC_SERVICE_BUFFER_LEN                          32

/* Define the global variable for the IPC_APP_ACCESS_PSA_MEM_SID service. */
uint8_t ipc_servic_data;
uint8_t *ipc_service_data_p = &ipc_servic_data;

static uint8_t srvc_private_data_connection1[2] = {0x11, 0x00};
static uint8_t srvc_private_data_connection2[2] = {0x21, 0xF0};

/*
 * Fixme: Temporarily implement abort as infinite loop,
 * will replace it later.
 */
static void tfm_abort(void)
{
    while (1)
        ;
}

static void ipc_service_basic(void)
{
    psa_msg_t msg;
    psa_status_t status;

    status = psa_get(IPC_SERVICE_TEST_BASIC_SIGNAL, &msg);
    if (status != PSA_SUCCESS) {
        return;
    }

    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    case CLIENT_API_TEST_TYPE_REQUEST_SRV:
        psa_reply(msg.handle, client_api_test_rot_srv(&msg));
        break;
    default:
        psa_reply(msg.handle, PSA_ERROR_NOT_SUPPORTED);
        break;
    }
}

static void ipc_service_psa_access_app_mem(void)
{
    psa_msg_t msg;
    char rec_data;
    uint32_t rec_buf;

    psa_get(IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_SIGNAL, &msg);
    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    case PSA_IPC_CALL:
        /*
         * rec_buf is a pointer pointed to a char type memory in client stack.
         */
        if (msg.in_size[0] != 0) {
            psa_read(msg.handle, 0, &rec_buf, 4);
            rec_data = *(char *)rec_buf;

            /* rec_data is assigned to 'A' by the client side. */
            if (rec_data != 'A') {
                psa_reply(msg.handle, -1);
                break;
            }

            /* Change the char type client stack memory to 'B'. */
            *((char *)rec_buf) = 'B';
        }
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    default:
        /* cannot get here? [broken SPM]. TODO*/
        tfm_abort();
        break;
    }
}

#ifdef TFM_IPC_ISOLATION_2_TEST_READ_ONLY_MEM
/*
 * Fixme: Temporarily using a predefined boundary value.
 * will replace it later.
 */
#define PROT_BOUNDARY   2

static void ipc_service_psa_access_app_readonly_mem(void)
{
    psa_msg_t msg;
    char rec_data;
    uint32_t rec_buf;

    psa_get(IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SIGNAL, &msg);
    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    case PSA_IPC_CALL:
         /*
          * rec_buf is a pointer pointed to a char type memory in client
          * code section. Write the memory will cause MemManage fault.
          */
        if (msg.in_size[0] != 0) {
            psa_read(msg.handle, 0, &rec_buf, 4);
            rec_data = *(char *)rec_buf;

            /* rec_data is assigned to 'A' by the client side. */
            if (rec_data != 'A') {
                psa_reply(msg.handle, -1);
                break;
            }
            if (tfm_hal_memory_check(PROT_BOUNDARY, (uintptr_t)rec_buf, 4,
                         TFM_HAL_ACCESS_READWRITE) != TFM_HAL_ERROR_MEM_FAULT) {
                psa_reply(msg.handle, PSA_ERROR_GENERIC_ERROR);
                break;
            }
        }
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    default:
        /* cannot get here? [broken SPM]. TODO*/
        tfm_abort();
        break;
    }
}
#endif

#if defined TFM_IPC_ISOLATION_2_MEM_CHECK \
    || defined TFM_IPC_ISOLATION_2_APP_ACCESS_PSA_MEM
static void ipc_service_app_access_psa_mem(void)
{
    psa_msg_t msg;

    psa_get(IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_SIGNAL, &msg);
    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    case PSA_IPC_CALL:
        if (msg.out_size[0] != 0) {

            /*
             * Write a pointer to outvec. The pointer points to uint8_t
             * memory in ipc servive partition.
             */
            psa_write(msg.handle, 0, &ipc_service_data_p,
                      sizeof(ipc_service_data_p));
        }

        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    default:
        /* cannot get here? [broken SPM]. TODO*/
        tfm_abort();
        break;
    }
}
#endif

static void ipc_service_programmer_error(void)
{
    psa_msg_t msg;

    psa_get(IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_SIGNAL, &msg);
    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        psa_reply(msg.handle, PSA_SUCCESS);
        break;
    case PSA_IPC_CALL:
        psa_reply(msg.handle, PSA_ERROR_PROGRAMMER_ERROR);
        break;
    default:
        /* cannot get here? [broken SPM]. TODO*/
        tfm_abort();
        break;
    }
}

static void ipc_service_stateless_rot(void)
{
    psa_status_t status;
    psa_msg_t msg;

    /* Retrieve the message corresponding to the example service signal */
    status = psa_get(IPC_SERVICE_TEST_STATELESS_ROT_SIGNAL, &msg);
    if (status != PSA_SUCCESS) {
        return;
    }

    /* Decode the message */
    switch (msg.type) {
    case CLIENT_API_TEST_TYPE_REQUEST_SRV:
        status = client_api_test_rot_srv(&msg);
        break;
    default:
        /* Invalid message type */
        status = PSA_ERROR_PROGRAMMER_ERROR;
        break;
    }

    /* Reply with the message result status to unblock the client */
    psa_reply(msg.handle, status);
}

#if PSA_FRAMEWORK_HAS_MM_IOVEC

static void ipc_service_mmiovec_test_handle(uint32_t signals)
{
    psa_status_t status = TFM_MMIOVEC_TEST_ERROR;
    psa_msg_t msg;

    /* Retrieve the message corresponding to the MMIO test service signal */
    status = psa_get(signals, &msg);
    if (status != PSA_SUCCESS) {
        psa_panic();
    }

    /* Decode the message */
    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:

        /* Only entertain cases when the call is from a connection-based test context */
        if (signal & IPC_SERVICE_TEST_MMIOVEC_STATEFUL_SIGNAL) {
            status = PSA_SUCCESS;
        }
        break;
    case INVEC_MAP_AND_UNMAP:
        status = test_service_mmiovec_invec(&msg);
        break;
    case OUTVEC_MAP_AND_UNMAP:
        status = test_service_mmiovec_outvec(&msg);
        break;
    case OUTVEC_MAP_NOT_UNMAP:
        status = test_service_outvec_not_unmap(&msg);
        break;
    default:
        /* Invalid message type */
        status = TFM_MMIOVEC_TEST_ERROR;
    }
    /* Reply with the message result status to unblock the client */
    psa_reply(msg.handle, status);
}

#endif

static void ipc_service_client_id_translate(void)
{
    psa_status_t status = PSA_ERROR_INVALID_ARGUMENT;
    psa_msg_t msg;

    status = psa_get(IPC_SERVICE_TEST_CLIENT_ID_TRANSLATE_SIGNAL, &msg);
    if (status != PSA_SUCCESS) {
        psa_panic();
    }

    switch (msg.type) {
    case PSA_IPC_CONNECT:
    case PSA_IPC_DISCONNECT:
        status = PSA_SUCCESS;
        break;

    case CLIENT_ID_TRANSLATE_TEST_TYPE_REQUEST_SRVC:
        if (msg.client_id == 0xC4000000) { /* client_id_limit - 1 */
            status = PSA_SUCCESS;
        }
        break;

    default:
        tfm_abort();
        break;
    }

    psa_reply(msg.handle, status);
}

static void ipc_service_rhandle_check(void)
{
    psa_status_t status = PSA_ERROR_INVALID_ARGUMENT;
    psa_msg_t msg;
    uint8_t *data;
    static uint32_t connect_cnt = 0;
    static psa_handle_t first_handle;

    status = psa_get(IPC_SERVICE_TEST_RHANDLE_CHECK_SIGNAL, &msg);
    if (status != PSA_SUCCESS) {
        psa_panic();
    }

    switch (msg.type) {
    case PSA_IPC_CONNECT:
        /* Only two connections expected from the test client */
        if (connect_cnt == 0) {
            psa_set_rhandle(msg.handle,
                            (void *)&srvc_private_data_connection1[0]);
            connect_cnt++;
            first_handle = msg.handle;
        } else if (connect_cnt == 1) {
            if (first_handle == msg.handle) {
                /*
                 * This second connection request comes with the same handle as
                 * the first one, not valid!
                 */
                tfm_abort();
            }

            psa_set_rhandle(msg.handle,
                            (void *)&srvc_private_data_connection2[0]);
            connect_cnt++;
        } else {
            tfm_abort();
        }
        break;

    case PSA_IPC_DISCONNECT:
        /* Check that the data has correct magic and incremented value */
        data = (uint8_t *)msg.rhandle;
        if ((data[0] == 0x11) && (data[1] == 0x02)) {
            status = PSA_SUCCESS;
        }

        if ((data[0] == 0x21) && (data[1] == 0xF2)) {
            status = PSA_SUCCESS;
        }
        break;

    case IPC_SERVICE_RHANDLE_CHECK_TYPE_REQUEST_SRVC:
        /*
         * The associated data is incremented and then written to the output
         * buffer.
         */
        data = (uint8_t *)msg.rhandle;
        data[1]++;

        psa_write(msg.handle, 0,
                  &data[1],
                  sizeof(*data));

        status = PSA_SUCCESS;
        break;

    default:
        tfm_abort();
        break;
    }

    psa_reply(msg.handle, status);
}

static void ipc_service_connection_refused(void)
{
    psa_status_t status = PSA_ERROR_INVALID_ARGUMENT;
    psa_msg_t msg;

    status = psa_get(IPC_SERVICE_TEST_CONNECTION_REFUSED_SIGNAL, &msg);
    if (status != PSA_SUCCESS) {
        psa_panic();
    }

    switch (msg.type) {
    case PSA_IPC_CONNECT:
        break;
    default:
        tfm_abort();
        break;
    }

    psa_reply(msg.handle, PSA_ERROR_CONNECTION_REFUSED);
}

/* Test thread */
void ipc_service_test_main(void *param)
{
    uint32_t signals = 0;

    while (1) {
        signals = psa_wait(PSA_WAIT_ANY, PSA_BLOCK);
        if (signals & IPC_SERVICE_TEST_BASIC_SIGNAL) {
            ipc_service_basic();
        } else if (signals & IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_SIGNAL) {
            ipc_service_psa_access_app_mem();
#ifdef TFM_IPC_ISOLATION_2_TEST_READ_ONLY_MEM
        } else if (signals
                   & IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SIGNAL) {
            ipc_service_psa_access_app_readonly_mem();
#endif
#if defined TFM_IPC_ISOLATION_2_MEM_CHECK \
    || defined TFM_IPC_ISOLATION_2_APP_ACCESS_PSA_MEM
        } else if (signals & IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_SIGNAL) {
            ipc_service_app_access_psa_mem();
#endif
        } else if (signals & IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_SIGNAL) {
            ipc_service_programmer_error();
        } else if (signals & IPC_SERVICE_TEST_STATELESS_ROT_SIGNAL) {
            ipc_service_stateless_rot();
#if PSA_FRAMEWORK_HAS_MM_IOVEC
        } else if ((signals & IPC_SERVICE_TEST_MMIOVEC_SIGNAL) ||
                   (signals & IPC_SERVICE_TEST_MMIOVEC_STATEFUL_SIGNAL)) {
            ipc_service_mmiovec_test_handle(signals);
#endif
        } else if (signals & IPC_SERVICE_TEST_CLIENT_ID_TRANSLATE_SIGNAL) {
            ipc_service_client_id_translate();
        } else if (signals & IPC_SERVICE_TEST_RHANDLE_CHECK_SIGNAL) {
            ipc_service_rhandle_check();
        } else if (signals & IPC_SERVICE_TEST_CONNECTION_REFUSED_SIGNAL) {
            ipc_service_connection_refused();
        } else {
            /* Should not come here */
            tfm_abort();
        }
    }
}
