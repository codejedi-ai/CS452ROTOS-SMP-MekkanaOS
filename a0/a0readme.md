# Model Train Control Program

## Overview

The Model Train Control Program is a software application designed for enthusiasts and hobbyists in the model train community. It provides a comprehensive solution for managing and controlling model trains, switches, and solenoids on a layout. This program is intended to enhance the user's experience by offering a user-friendly interface and advanced control features.

## Features

### 1. Train Control

- **Train Movement**: The program allows users to control the speed and direction of individual model trains. Users can issue commands such as `tr <train number> <train speed 1 - 14>` to control the speed of a specific train and `rv <train number>` to reverse the direction of a train.
- To set attributes to the train use `tr <train number> <attribute>`
    
    ```
    code functions on
    64 all off
    65 #1
    66 #2
    67 #1,#2
    68 #3
    69 #3,#1
    70 #3,#2
    71 #3,#2,#1
    72 #4
    73 #4,#1
    74 #4,#2
    75 #4,#2,#1
    76 #4,#3
    77 #4,#3,#1
    78 #4,#3,#2
    79 #4,#3,#2,#1
    ```
    

### 2. Switch Control

- **Switch Management**: Users can control switches on the model train layout. Commands like `sw <switch number> <switch direction 'C' or 'S'>` enable users to change the position of switches, facilitating train routing.

### 3. Data Structures

- **Marklin** **Command Queue**: To ensure efficient command processing, the program employs a command queue. This queue prevents overloading the Marklin controller, which has a processing speed limitation of 100ms per command. I used a list as a queue that would send data to the marklin controller .

### 4. Solenoid Protection

- **Solenoid Safety**: The program includes solenoid protection logic to prevent conflicting switch configurations. It ensures that solenoids are set appropriately based on switch states.

### 5. Initialization

- **Layout Initialization**: The program offers a feature to initialize the layout, setting the initial state of curves and turnouts. This ensures a consistent starting point for operations.

## Usage

The program accepts commands via a command-line interface (CLI). Users can input commands to control trains, switches, and solenoids as described in the "Features" section.

## Requirements

- Compatible model train layout with a Marklin controller.
- Appropriate hardware interface for communication with the layout components.

### Prerequisites

Before running the Model Train Control Program, please ensure the following prerequisites are met:

1. **Marklin System**: The Marklin train control system must be up and running. Ensure that the Marklin hardware, including the tracks, switches, and trains, is set up correctly.
2. **Marklin Reset**: It is essential to reset the Marklin system to its initial state. This ensures that the program can establish communication and control the trains effectively.

### Startup Procedure

Follow these steps to start the Model Train Control Program:

1. **Marklin System**: Make sure your Marklin system is powered on and functioning correctly. Check that all components are connected and operational.
2. **Marklin Reset**: Perform a reset of the Marklin system if necessary. Refer to the Marklin documentation for instructions on how to reset the system to its initial state.
3. **Raspberry Pi**: Power on your Raspberry Pi device and ensure it is connected to the same network as the Marklin system.
4. **Program Execution**: Execute the Model Train Control Program on the Raspberry Pi as per the provided instructions. The program will establish communication with the Marklin system.

By following these steps, you can ensure that the Model Train Control Program operates seamlessly with your Marklin setup. It is essential to have the Marklin system in a known state to achieve accurate and reliable train control.

### Program Execution

Once the Model Train Control Program is initiated on the Raspberry Pi, it operates in an autonomous mode where it continuously processes and executes commands without accepting manual inputs from the Pi. This is designed to maintain the integrity and consistency of train operations. In this mode the tracks are initially set to straight then to curved. Such can be used as a hardware debugging mechanism to see and hear which solenoids are burnt. 

During program execution, please refrain from manual input or adjustments directly through the Raspberry Pi interface, as these inputs may not be processed until the program completes its current task or enters a receptive state.

To stop the program or make any adjustments, follow the provided instructions for program termination or modification, ensuring a safe and controlled transition.

On the bottom of the GUI, there exist 

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/02788782-ec33-4cd7-81c6-26b72be47a78/33f29ba6-3ab2-4250-9c25-5be45a2daf75/Untitled.png)

The LIVE SENSOR interface displays the real time information of the sensors on the track. The ith bit is the ith sensor. 

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/02788782-ec33-4cd7-81c6-26b72be47a78/66e4bd1f-2643-441c-9907-ccfc79a9e875/Untitled.png)

The ACTIVATED SENSOR interface displays which sensor recently underwent a rising edge. This suggest a train have passed over the sensor

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/02788782-ec33-4cd7-81c6-26b72be47a78/c971899d-e74d-4e27-9259-81c3859849a8/Untitled.png)

The recently activated sensors interface displays a list in real time. A new sensor is added to the top when it is triggered.

The SW column displayes the rail switches. S if it is straight and C if it is curved. 

## Queue