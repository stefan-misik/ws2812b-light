# Start the linked application directly, including when it is offset in flash.
# The debug server must reset the target before this script is sourced.
monitor halt
set $sp = &_estack
set $pc = &Reset_Handler
