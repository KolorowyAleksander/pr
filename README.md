### tugboats
A classroom project - 
simple distributed programming application in c which simulates a bunch of ships  entering/leaving a port through a harbour.
There is a limited number of tugboats which can assist the ships while docking.
Each ship needs to know if it can enter or leave the port and use the tugboats. 
This is done in a completely distributed way, meaning, that no mechanism of synchronization besides FIFO messaging is used.
Implemented using the Parallel Virtual Machine platform (PVM).
