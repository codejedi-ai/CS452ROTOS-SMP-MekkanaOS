## Overview

This code represents a simple name server implemented in C for managing named processes. It allows processes to register, deregister, and query for other processes using human-readable names.

## Code Structure

The code is divided into several sections:

- `nameserver`: The main function serving as the name server. It handles registration, deregistration, and query requests from processes. The name server maintains an array of process names and their associated PIDs.
    
- Utility Functions:
    
    - `RegisterAs`: A function for a process to register itself with the name server using a human-readable name.
    - `Deregister`: A function for a process to deregister itself from the name server.
    - `WhoIs`: A function for a process to query the PID of a process with a given name from the name server.
- Debugging and Terminal Control: The code includes terminal control sequences to improve the user interface. Debugging features are also integrated for monitoring and managing registered processes.
    

## Usage

1. To register a process with a name, use the `RegisterAs` function. Example: `RegisterAs("my_process_name");`
    
2. To deregister a process, use the `Deregister` function. Example: `Deregister();`
    
3. To query the PID of a process by its name, use the `WhoIs` function. Example: `WhoIs("process_name_to_query");`
    
4. The `nameserver` function handles all the registration, deregistration, and query operations.
