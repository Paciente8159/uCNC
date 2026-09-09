/*
	This is and example of how to add a custom task extension to µCNC
	Extension tasks can be added simply by adding an event listener to the core main loop
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief	Task extensions depend on the ENABLE_MAIN_LOOP_MODULES option
 * 			Check if this option is defined or not
 */
#ifdef ENABLE_MAIN_LOOP_MODULES

/**
 * @brief	Check if your current module is up to date with the current core version of module 
 */
#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

/**
 * @brief Create a function execute your custom task. All event functions are declared as uint8_t <function>(void* args, bool* handle)
 * 
 * @param args		is a pointer to a set of arguments to be passed to the event handler. In the case of the cnc_dotasks event it's a NULL pointer.
 * @return bool 	a boolean that tells the handler if the event should continue to propagate through additional listeners or is handled by the current listener an should stop propagation
 */
bool mycustom_task(void *args)
{
	// just do whatever you need here
	static uint32_t timeout = 5000;
	uint32_t millis = mcu_millis();
	if(millis>timeout){
		proto_info("System time %llu", millis);
		timeout += 5000;
	}
	// you must return EVENT_CONTINUE to enable other tasks to run or return EVENT_HANDLED to terminate the event handling within this callback
	return EVENT_CONTINUE;
}

/**
 * @brief 	Create an event listener object an attach our custom code parser handler.
 * 			in this case we are adding a listener to the 'cnc_dotasks' EVENT
 * 
 */
CREATE_EVENT_LISTENER(cnc_dotasks, mycustom_task);

#endif


/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_task_module); from inside the core code
 */
DECL_MODULE(mycustom_task_module)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
	// Makes the event handler 'mycustom_task' listen to the event 'cnc_dotasks'
	ADD_EVENT_LISTENER(cnc_dotasks, mycustom_task);
#else
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif
}
