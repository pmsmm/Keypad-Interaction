# Keypad-Interaction

The Keypad Interaction is a mini game that was developed for my Master Thesis "Developing a Support Infrastructure for an Escape The Room and Peddy Paper Games".

This mini game, or interaction as I so call it, was developed to be part of an Escape the Room game that was also developed as part of my thesis. Since this interaction can work as a standalone mini game I decided to place it here for anyone interested in using it.

## List of Components

- Otemu Mechanical Switch (16x);
- Diode 1n4148 (16x);
- Display LCD 16x2 1602 I2C IIC (1x);
- Red LED (1x);
- Green LED (1x);
- 220Ω Resistor (2x);
- Arduino Nano (1x);

![Wiring Diagram](images/KeypadElectricalDiagram_bb.jpg)

In order to fully assemble the Interaction, so as to look like the picture below, you will also need to 3D Print the enclosure which is divided into two parts that can be found [here](enclosure/). Additionally, you will also need to 3D Print the keycaps which **were not** designed by me but can be found in [Thingiverse](https://www.thingiverse.com/thing:2783650).

![Assembled Interaction](images/AssembledInteraction.jpeg)

## The Purpose of The Game

This game was intended to be played as part of an Escape the Room but it can still be used as a fun project with other purposes besides a game. As many might have guessed, the purpose of this game is to introduce a secret code using the 4x4 keypad and if the code is correct the green LED will light up.

## Instructions

First start by uploading the code to your Arduino Nano (This is the one I used so I can only guarantee proper working with this micro-controller).

### Starting the Game

To start the game you must first type in the Arduino IDE serial monitor the following:

- `COM:START;ID:123456789`

This command will then print in the Serial Monitor the solution of the game which then needs to be introduced using the keypad. If the introduced code is correct then the following message will appear in the serial monitor:

- > COM:INTERACTION_SOLVED;MSG:User Guessed Code Correctly;PNT:2000

## Warning

The source code for this interaction contains a lot of logic that was made to communicate with the infrastructure that was developed for my thesis and therefor it is not as clean as it could be when compared to a mini game that is developed with the intent of being used by the Arduino IDE directly.