/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_secure_client_service.h"

#include "secure_suites.h"

#include "psa/service.h"
#include "psa_manifest/tfm_secure_client_service.h"
/**
 * \brief Service initialisation function. No special initialisation is
 *        required.
 *
 * \return Returns 0 on success
 */
int32_t tfm_secure_client_service_init(void)
{
    s_reg_test_start();

#if TFM_SP_SECURE_TEST_PARTITION_MODEL_IPC == 1
    psa_signal_t signals;
    signals = psa_wait(PSA_WAIT_ANY, PSA_BLOCK);
    if (signals & TFM_SECURE_CLIENT_SRV_DUMMY_SIGNAL) {
        psa_panic();
    }
#endif /* TFM_SP_SECURE_TEST_PARTITION_MODEL_IPC == 1 */
    return 0;
}

#if TFM_SP_SECURE_TEST_PARTITION_MODEL_SFN == 1
psa_status_t tfm_secure_client_srv_dummy_sfn(const psa_msg_t *msg)
{
    /* Not expected to be called by anyone */
    psa_panic();

    return 0;
}
#endif /* TFM_SP_SECURE_TEST_PARTITION_MODEL_SFN == 1 */
