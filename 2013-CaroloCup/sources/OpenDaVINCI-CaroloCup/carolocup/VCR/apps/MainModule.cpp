/*
 * CaroloCup.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#include "VCR.h"

int32_t main(int32_t argc, char **argv) {
    carolocup::VCR vcr(argc, argv);
    return vcr.runModule();
}
