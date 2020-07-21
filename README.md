synchro_sim_for_c - A simple framework for simulating CPUs designs in C.

== This framework implements much of the 'delta cycle' of HDLs.

All modules calculate the next state of the system in the xyz_next_state()
function (based on the current state and any data on the bus's data_in signal),
and then the new_state is copied over by existing state in the xyz_clock() function.

If this convention is followed the design should easily transfer to a HDL.

The only dodgy bits are if modules have async outputs. 

== Components are:

Makefile        : The make file
main.c          : Top level design
config.h        : Configuration macros
io.c   & io.h   : An I/O device - currently just a RX/TX UART
mem.c  & mem.h  : A memory device - contents is loaded on startup
cpu.c  & cpu.h  : Where the CPU logic goes
bus.c  & bus.h  : The definition of the CPU's external bus
term.c & term.h : Helper functions to create a UART to the console

== Running

Just run ./sim and the name of the file that has the memory contents.

The current CPU design just copies the data from the memory and write it 
to the simulated serial port.
