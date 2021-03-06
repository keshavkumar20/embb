/*
 * Copyright (c) 2014-2017, Siemens AG. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EMBB_MTAPI_C_MTAPI_EXT_H_
#define EMBB_MTAPI_C_MTAPI_EXT_H_


#include <embb/mtapi/c/mtapi.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup C_MTAPI_EXT MTAPI Extensions
 *
 * \ingroup C_MTAPI
 *
 * Provides extensions to the standard MTAPI API.
 *
 * There is a single extension function defined here to support user defined
 * behavior of an action to allow for actions that are not implemented locally
 * in software but e.g. on a remote node in a network or on an accelerator
 * device like a GPU or FPGA.
 */

/**
 * Represents a callback function that is called when a plugin action is about
 * to start a plugin task.
 * This function should return MTAPI_SUCCESS if the task could be started and
 * the appropriate MTAPI_ERR_* if not.
 *
 * \ingroup C_MTAPI_EXT
 */
typedef void(*mtapi_ext_plugin_task_start_function_t)(
  MTAPI_IN mtapi_task_hndl_t task,
  MTAPI_OUT mtapi_status_t* status
);

/**
 * Represents a callback function that is called when a plugin task is about
 * to be canceled.
 * This function should return MTAPI_SUCCESS if the task could be canceled and
 * the appropriate MTAPI_ERR_* if not.
 *
 * \ingroup C_MTAPI_EXT
 */
typedef void(*mtapi_ext_plugin_task_cancel_function_t)(
  MTAPI_IN mtapi_task_hndl_t task,
  MTAPI_OUT mtapi_status_t* status
);

/**
 * Represents a callback function that is called when a plugin action is about
 * to be finalized.
 * This function should return MTAPI_SUCCESS if the action could be deleted and
 * the appropriate MTAPI_ERR_* if not.
 *
 * \ingroup C_MTAPI_EXT
 */
typedef void(*mtapi_ext_plugin_action_finalize_function_t)(
  MTAPI_IN mtapi_action_hndl_t action,
  MTAPI_OUT mtapi_status_t* status
);

/**
 * This function creates a plugin action.
 *
 * It is called on the node where the plugin action is implemented. A plugin
 * action is an abstract encapsulation of a user defined action that is needed
 * to implement a job that does not represent a software action. A plugin
 * action contains a reference to a job, callback functions to start and cancel
 * tasks and a reference to an callback function to finalize the action.
 * After a plugin action is created, it is referenced by the application using
 * a node-local handle of type \c mtapi_action_hndl_t, or indirectly through a
 * node-local job handle of type \c mtapi_job_hndl_t. A plugin action's
 * life-cycle begins with mtapi_ext_plugin_action_create(), and ends when
 * mtapi_action_delete() or mtapi_finalize() is called.
 *
 * To create an action, the application must supply the domain-wide job ID of
 * the job associated with the action. Job IDs must be predefined in the
 * application and runtime, of type \c mtapi_job_id_t, which is an
 * implementation-defined type. The job ID is unique in the sense that it is
 * unique for the job implemented by the action. However several actions may
 * implement the same job for load balancing purposes.
 *
 * If \c node_local_data_size is not zero, \c node_local_data specifies the
 * start of node local data shared by action functions executed on the same
 * node. \c node_local_data_size can be used by the runtime for cache coherency
 * operations.
 *
 * On success, an action handle is returned and \c *status is set to
 * \c MTAPI_SUCCESS. On error, \c *status is set to the appropriate error
 * defined below. In the case where the action already exists, \c status will
 * be set to \c MTAPI_ERR_ACTION_EXISTS and the handle returned will not be a
 * valid handle.
 * <table>
 *   <tr>
 *     <th>Error code</th>
 *     <th>Description</th>
 *   </tr>
 *   <tr>
 *     <td>\c MTAPI_ERR_JOB_INVALID</td>
 *     <td>The \c job_id is not a valid job ID, i.e., no action was created for
 *         that ID or the action has been deleted.</td>
 *   </tr>
 *   <tr>
 *     <td>\c MTAPI_ERR_ACTION_EXISTS</td>
 *     <td>This action is already created.</td>
 *   </tr>
 *   <tr>
 *     <td>\c MTAPI_ERR_ACTION_LIMIT</td>
 *     <td>Exceeded maximum number of actions allowed.</td>
 *   </tr>
 *   <tr>
 *     <td>\c MTAPI_ERR_NODE_NOTINIT</td>
 *     <td>The calling node is not initialized.</td>
 *   </tr>
 * </table>
 *
 * \see mtapi_action_delete(), mtapi_finalize()
 *
 * \returns Handle to newly created plugin action, invalid handle on error
 * \threadsafe
 * \ingroup C_MTAPI_EXT
 */
mtapi_action_hndl_t mtapi_ext_plugin_action_create(
  MTAPI_IN mtapi_job_id_t job_id, /**< [in] Job id */
  MTAPI_IN mtapi_ext_plugin_task_start_function_t task_start_function,
                              /**< [in] Task start function */
  MTAPI_IN mtapi_ext_plugin_task_cancel_function_t task_cancel_function,
                              /**< [in] Task cancel function */
  MTAPI_IN mtapi_ext_plugin_action_finalize_function_t action_finalize_function,
                              /**< [in] Finalize action function */
  MTAPI_IN void* plugin_data,
                              /**< [in] Pointer to plugin data */
  MTAPI_IN void* node_local_data,
                              /**< [in] Pointer to node local data */
  MTAPI_IN mtapi_size_t node_local_data_size,
                              /**< [in] Size of node local data */
  MTAPI_IN mtapi_action_attributes_t* attributes,
                              /**< [out] Pointer to attributes */
  MTAPI_OUT mtapi_status_t* status/**< [out] Pointer to error code,
                                             may be \c MTAPI_NULL */
);


#ifdef __cplusplus
}
#endif


#endif // EMBB_MTAPI_C_MTAPI_EXT_H_
