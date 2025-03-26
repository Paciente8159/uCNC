#include "../../cnc.h"

extern void ethernet_init();

DECL_MODULE(w5500_ethernet)
{
	ethernet_init();
}