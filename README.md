# PolicyAgainstStarvation
Policy Against Starvation for Resource Manager

Based on a printer manager about Message Passing with a set of N daemon resources LPDs under the management of the Manager and several Clients requesting to allocate up to M = N − 1 resources.
In order to simulate a worst case scenario, a random of m (m ≤ M) requests from a Client are continuously developed by sessions separated in random inter-vals of time. A few modifications in the code of the M anager allow to allocate the requests according to FIFO policy. The allocation stands for sending the port names of available LP Ds to the requester Client, once this gets the port names and sends the information to the LP Ds, it requests for deallocation of the m used resources.
