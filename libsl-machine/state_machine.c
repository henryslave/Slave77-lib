/**
 * @file state_machine.c
 */

#include <stdlib.h>

#include "state_machine.h"



/**
 * @typedef state_private_t
 * @brief Private data of the state machine. This data are used to call the callback functions related
 * to the given state.
 */
typedef struct _state_private_t state_private_t;

/**
 * @struct _state_private_t
 * @brief See "state_private_t" for details.
 */
struct _state_private_t {
    bool enabled;               /**< Check if the given state is enabled.
                                     INFO: states are enabled by "state_machine_add_state" function. */
    fsm_state_run_t run;        /**< Standard callback function: it is called when no transitions are planned
                                     for the actual state */
    fsm_state_enter_t enter;    /**< Callback function executed during a state transition */
};



/**
 * @fn state_machine_run
 * @brief Function to be called to update the state or execute the transitions
 * of the given state machine.
 * @param fsm Pointer to the target state machine.
 * @param arg Optional parameters "passed" to the callback function.
 * Example: During a transition this parameter will be accessible from the "enter"
 * callback function.
 * @return The actual state of the state machine.
 */
static uint32_t state_machine_run (fsm_t *fsm, void *arg);

/**
 * @fn state_machine_add_state
 * @brief See "state_machine_add_state_t" for details.
 */
static bool state_machine_add_state (fsm_t *fsm, uint32_t id, fsm_state_run_t run, fsm_state_enter_t enter);

/**
 * @fn state_machine_add_transition
 * @brief See "state_machine_add_transition_t" for details.
 */
static bool state_machine_add_transition (fsm_t *fsm, uint32_t state_id, uint32_t target_id);

/**
 * @fn state_machine_go_to_state
 * @brief See "state_machine_go_to_state_t" for details.
 */
static bool state_machine_go_to_state (fsm_t *fsm, uint32_t target_id);

/**
 * @fn state_machine_get_state
 * @brief Get the ID of the actual state of the state machine.
 * @param fsm The target state machine.
 * @return The ID of the actual state.
 */
static uint32_t state_machine_get_state (fsm_t *fsm);



fsm_t* state_machine_init (uint32_t state_nr, uint32_t initial_state, void *arg)
{
    uint32_t cntr;
    fsm_t *fsm;
    state_private_t *private_data;

    /* Allocate the memory needed by state machine */
    fsm = (fsm_t *)malloc(sizeof(fsm_t));

    /* Set the number of states of the state machine */
    fsm->state_nr = state_nr;

    /* Set the space needed by states array */
    fsm->states = (fsm_state_t*)malloc(fsm->state_nr * sizeof(fsm_state_t));

    /* Initialize the state machine */
    for (cntr = 0; cntr <state_nr; cntr++)
    {
        fsm->states[cntr].id = cntr;
        fsm->states[cntr].valid_target = 0;

        private_data = (state_private_t*)malloc(sizeof(state_private_t));
        private_data->enabled = false;

        fsm->states[cntr].private_data = (state_private_t*)private_data;
    }

    /* Set the value of  initial state */
    fsm->actual_state = &fsm->states[initial_state];
    fsm->target_state = initial_state;

    /* Set the default function to be called to run the state machine */
    fsm->sm_run = state_machine_run;
    fsm->get_state = state_machine_get_state;

    /* Set the functions used to configure the state machine */
    fsm->add_state = state_machine_add_state;
    fsm->add_transition = state_machine_add_transition;

    /* Set the function used to require a transition of the state machine */
    fsm->go_to_state = state_machine_go_to_state;

    /* Return the pointer to the state machine */
    return(fsm);
}



void state_machine_deinit (fsm_t *fsm)
{
    free(fsm->states);
    fsm->states = NULL;

    free(fsm);
    fsm = NULL;
}



static bool state_machine_add_state (fsm_t *fsm, uint32_t id, fsm_state_run_t run, fsm_state_enter_t enter)
{
    state_private_t *private_data;

    /* Check for valid state machine */
    if (fsm == NULL)
    {
        return(-1);
    }

    /* Check if the required target state for the transition is valid */
    if (id >= fsm->state_nr)
    {
        return(false);
    }

    /* Set the pointer to the private fields of the structure */
    private_data = (state_private_t*)fsm->states[id].private_data;

    /* Check if the state has already been enabled */
    if (private_data->enabled == false)
    {
        private_data->enabled = true;
        private_data->run = run;
        private_data->enter = enter;

        return(true);
    }

    return(false);
}



static bool state_machine_add_transition (fsm_t *fsm, uint32_t state_id, uint32_t target_id)
{
    uint32_t target_reg;

    /* Check if both states are valid */
    if ((state_id >= fsm->state_nr) || (target_id >= fsm->state_nr))
    {
        return(false);
    }

    /* Update the "Valid Targes" register of the state */
    target_reg = fsm->states[state_id].valid_target;
    target_reg |= (0x1 << target_id);
    fsm->states[state_id].valid_target = target_reg;

    return(true);
}



static uint32_t state_machine_run (fsm_t *fsm, void *arg)
{
    fsm_t *sm;
    state_private_t *private_data;
    uint32_t id;

    /* Check for valid state machine */
    if (fsm == NULL)
    {
        return(-1);
    }

    /* Set the pointer to the state machine to be handled */
    sm = (fsm_t *)fsm;

    /*
     Check for the callback function to be called. Available options are:
     - standard callback: the state is not changed.
     - exit callback: the state is changed and the exit callback must be executed.
     */
    if (sm->actual_state->id == sm->target_state)
    {
        /* Set the pointer to the private data of the state */
        private_data = (state_private_t*)sm->actual_state->private_data;

        if (private_data->run != NULL)
        {
            private_data->run(arg);
        }
    }
    else
    {
        id = fsm->get_state(fsm);

        fsm->actual_state = &fsm->states[fsm->target_state];

        /* Set the pointer to the private data of the state */
        private_data = (state_private_t*)sm->actual_state->private_data;

        if (private_data->enter != NULL)
        {
            private_data->enter(id, arg);
        }
    }


    return(sm->actual_state->id);
}



static bool state_machine_go_to_state (fsm_t *fsm, uint32_t target_id)
{
    fsm_state_t *state;
    uint32_t state_mask;

    /* Check if the state machine is valid */
    if (fsm == NULL)
    {
        return(-1);
    }

    /* Check for valid state */
    if (target_id >= fsm->state_nr)
    {
        return(false);
    }

    /* Set the pointer to the state "in use" */
    state = fsm->actual_state;
    state_mask = state->valid_target;

    if ((state_mask & (0x1 << target_id)) != 0)
    {
        /* Update the target state */
        fsm->target_state = target_id;
        return(true);
    }

    return(false);
}



static uint32_t state_machine_get_state (fsm_t *fsm)
{
    return(fsm->actual_state->id);
}
