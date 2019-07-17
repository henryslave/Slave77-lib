/**
 * @file state_machine.h
 * @brief Definition of functions used to create and handle state machines.
 *
 * @author Slave77 <henry.slave77@gmail.com>
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>
#include <stdint.h>



/**
 * @typedef fsm_t
 * @brief Data type used to create the main structure of the state machine.
 */
typedef struct _fsm_t fsm_t;

/**
 * @typedef fsm_state_t
 * @brief Data type used to create a new state of the state machine.
 */
typedef struct _fsm_state_t fsm_state_t;


/**
 * @typedef fsm_run_t
 * @brief Pointer to the function to be called to update the state machine.
 * INFO: This function must be called to execute the state transitions.
 * @param fsm Pointer to the target state machine.
 * @param arg Optional parameters "passed" to the callback function.
 * Example: During a transition this parameter will be accessible from the "enter"
 * callback function.
 * @return The actual state of the state machine.
 */
typedef uint32_t (*fsm_run_t) (fsm_t *fsm, void *par);

/**
 * @typedef fsm_state_run_t
 * @brief Pointer to the callback function called by "fsm_run_t" when no transition
 * are required.
 * i.e. This function is the one called when the state is not changed.
 * @param par Optional parametr "passed" directly from "fsm_run_t" function.
 */
typedef void (*fsm_state_run_t) (void *par);

/**
 * @typedef fsm_state_enter_t
 * @brief Pointer to the callback function called by "fsm_run_t" when a transition
 * is required.
 * i.e. This function is the one called when the state is changed.
 * @param exit_state_id The exit state of the state machine (i.e. the old state).
 * @param par Optional parametr "passed" directly from "fsm_run_t" function.
 */
typedef void (*fsm_state_enter_t) (uint32_t exit_state_id, void *par);

/**
 * @typedef state_machine_add_state_t
 * @brief Add a new state to the given state machine.
 * @param fsm Pointer to the target state machine.
 * @param id The ID of the state to be added.
 * @param run Callback function called when no state changes are planned for the
 * state machine.
 * @param enter Function called when the state machine enter into this state.
 * @return true if the state was successfully created, false if not.
 */
typedef bool (*state_machine_add_state_t) (fsm_t *fsm, uint32_t id, fsm_state_run_t run, fsm_state_enter_t enter);

/**
 * @typedef state_machine_add_transition_t
 * @brief Add a transition to the given state machine.
 * @param fsm Pointer to the given state machine.
 * @param state_id Starting state of the transition.
 * @param target_id Target state of the transition.
 * @return true if the transition was added to the state machine, false if not.
 */
typedef bool (*state_machine_add_transition_t) (fsm_t *fsm, uint32_t state_id, uint32_t target_id);

/**
 * @typedef state_machine_go_to_state_t
 * @brief Plan the execution of a state transition.
 * WARNING: The transition will be executed ONLY after the calling of the callback
 * function "fsm_state_run_t".
 * @return true if the required operation is valid, false if not.
 */
typedef bool (*state_machine_go_to_state_t) (fsm_t *fsm, uint32_t target_id);



/**
 * @struct _fsm_state_t
 * @brief Definition of a new state of a state machine.
 */
struct _fsm_state_t {
    uint32_t id;            /**< The ID of the state */
    uint32_t valid_target;  /**< Mask used to detect valid targets */

    void *private_data;     /**< Private date used by callback functions */
};

/**
 * @struct _fsm_t
 * @brief Main struct used for the definition of a state machine.
 */
struct _fsm_t {
    fsm_state_t *actual_state;  /**< Actual state of the state machine */
    uint32_t target_state;      /**< Target state of the state machine
                                     i.e. if "target state" is different from "actual state",
                                     a transition is executed. */

    fsm_state_t *states;        /**< List of the valid states */
    uint32_t state_nr;          /**< Number of states of the state machine */
    fsm_run_t sm_run;           /**< Update the state machine and execute its transitions */

    uint32_t (*get_state) (fsm_t *fsm); /** Get the ID of the actual state of the state machine */

    state_machine_add_state_t add_state;            /** Function called to add a state to the state machine */
    state_machine_add_transition_t add_transition;  /** Add a valid transition to the state machine */
    state_machine_go_to_state_t go_to_state;        /** Update the state of the given state machine */
};



/**
 * @fn state_machine_init
 * @brief Create and initialize a state machine.
 * @param state_nr Maximum number of states that can be added to the state machine.
 * @param initial_state Initial state of the state machine.
 * @param arg Not used at the moment.
 */
fsm_t* state_machine_init (uint32_t state_nr, uint32_t initial_state, void *arg);



#endif
