/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_plat_test.h"
#include "spm_test_defs.h"
#include "tfm_log_unpriv.h"
#include "psa/service.h"
#include "psa_manifest/tfm_slih_test_service.h"

static void slih_test_timer_handler(psa_signal_t timer_irq_signal)
{
    tfm_plat_test_secure_timer_stop();
    psa_irq_disable(timer_irq_signal);
    psa_eoi(timer_irq_signal);
}

static void slih_test_case_1(const psa_msg_t *msg, psa_signal_t timer_irq_signal)
{
    psa_irq_enable(timer_irq_signal);

    tfm_plat_test_secure_timer_start();

    if (psa_wait(timer_irq_signal, PSA_BLOCK) != timer_irq_signal) {
        psa_panic();
    }
    slih_test_timer_handler(timer_irq_signal);
}

static psa_status_t slih_test_handle_msg(const psa_msg_t *msg)
{
    psa_status_t status;

    switch (msg->type) {
    case TFM_SLIH_TEST_CASE_1:
        slih_test_case_1(msg, TFM_TIMER0_IRQ_SIGNAL);
        status = PSA_SUCCESS;
        break;
    default:
        ERROR_UNPRIV_RAW("SLIH test service: Invalid message type: 0x%x\n",
            msg->type);
        status = PSA_ERROR_PROGRAMMER_ERROR;
        break;
    }

    return status;
}

#if TFM_SP_SLIH_TEST_MODEL_IPC == 1
static void slih_test_get_msg(psa_signal_t signal, psa_msg_t *msg)
{
    psa_status_t status;

    status = psa_get(signal, msg);
    if (status != PSA_SUCCESS) {
        psa_panic();
    }
}

void tfm_slih_test_service_entry(void)
{
    psa_signal_t signals = 0;
    psa_msg_t msg;

    while (1) {
        signals = psa_wait(PSA_WAIT_ANY, PSA_BLOCK);
        if (signals & TFM_SLIH_TEST_CASE_SIGNAL) {
            slih_test_get_msg(TFM_SLIH_TEST_CASE_SIGNAL, &msg);
            psa_reply(msg.handle, slih_test_handle_msg(&msg));
        }
    }
}
#elif TFM_SP_SLIH_TEST_MODEL_SFN == 1
psa_status_t tfm_slih_test_case_sfn(const psa_msg_t *msg)
{
    return slih_test_handle_msg(msg);
}
#endif
