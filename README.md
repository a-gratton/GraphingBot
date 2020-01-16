# GraphingBot
Code written in C++ and RobotC for a first year design project, which allowed a robot to plot functions on a whiteboard.
This project was created collaboratively with Kyle Hong and Kevin Gu.

# Features
- Can accept, parse, and simplify input of any number of arbitrary polynomial terms in any order that the user desires
- Capable of generating a plotting instruction file for any mathematical function
- Limit switches allowing consistent coordinate tracking
- Movable pulley system to lift the end effector
- Ultrasonic sensor keeps track of the robot's distance from the wall as a safety feature
- Robot can both plot and erase the whiteboard.

# Flowcharts

![alt text](https://github.com/Scotrus/GraphingBot/blob/master/Flowcharts/C%2B%2BFlowChart.png)
C++ input parsing and path finding algorithm flowchart

![alt text](https://github.com/Scotrus/GraphingBot/blob/master/Flowcharts/RobotCFlowChart.png)

RobotC sensor, file, actuator interfacing algorithm flowchart

**Check out Images folder to see the GraphingBot operation video!**

![alt text](https://github.com/Scotrus/GraphingBot/blob/master/Images/IMG_1458.JPG)

Note that some software libraries for the RobotC code have been excluded, as I don't own the rights to distribute them.
