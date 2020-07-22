# synchro_sim_for_c - A simple framework for simulating CPUs designs in C.

This framework implements much of the 'delta cycle' of HDLs, allowing
rapid prototyping of CPU designs.

All modules calculate the next state of the system in the xyz_next_state()
function (based on the current state and any data on the bus's data_in signal),
and then the new_state is copied over by existing state in the xyz_clock() function.

If this convention is followed the design should easily transfer to a HDL.

The only dodgy bits are if modules have async outputs, whihc may 
require xyz_next_state() to be called multiple times.

## Components are:

* __Makefile__        : The make file
* __main.c__          : Top level design
* __config.h__        : Configuration macros
* __io.c   & io.h__   : An I/O device - currently just a RX/TX UART
* __mem.c  & mem.h__  : A memory device - contents is loaded on startup
* __cpu.c  & cpu.h__  : Where the CPU logic goes
* __bus.c  & bus.h__  : The definition of the CPU's external bus
* __term.c & term.h__ : Helper functions to create a UART to the console

![System diagram](systems_diagram.png)

## Running

Just run ./sim with the name of the file that has the memory contents.

The current CPU design just copies the data from the memory and write it 
to the simulated serial port.
