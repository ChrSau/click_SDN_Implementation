#pragma once

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
#pragma message("Your System Runs in Big Endian")
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
#pragma message("Your System Runs in Little Endian")
#else
#error "Could't detect the Endian of your System by Click Modular Router."
#endif


#include "OFConstants.hh"
#include "OFFunctions.hh"
