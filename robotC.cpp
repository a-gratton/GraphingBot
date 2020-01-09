#include "EV3Servo-lib-UW.c" //library for controlling servo motors
#include "PC_FileIO.c"         //library for file I/O with robotC

/* 
	Note that the majority of the while loops in this code contain
	the condition && SensorValue(ULTRA) < SAFTEY_DIST.

	This is to allow the emergency shutdown ultrasonic sensor
	to immediately stop the program at any point
	instead of waiting for the current loop to finish.
*/

const int SVPORT = 0, ULTRA = 3, TOUCH_VERT = 1, TOUCH_HOR = 2; //sensor ports
const int X_TOP = 2, X_BOT = 3, Y1 = 0, Y2 = 1; //motor ports

const int SAFTEY_DIST = 20, IDLE_TIME = 900000; //15 mins in ms

const float WHEEL_SIZE = 5.4; //in cm

const int SVNUM = 1, SV_MARKER = -65, SV_NEUT = 0, SV_ERASER = 90; //servo constants

const int REFPWR = 20; //reference motor power
const float SIDE_LENGTH = 25.0; //in cm, of plottable area
const float UNIT_SIZE = (SIDE_LENGTH / 20.0); //size of 1 grid square in cm

const int Y_CORRECTION = 2; //corrects mechanical advantage from y pulley
const int ERASE_CORRECTION = 1.2; //corrects friction effecting eraser movement 


const float ERASE_WIDTH = 4; //width of eraser in cm


void setX(int power); //sets x power
void setY(int power); //sets y power
float getDist(int degrees); //returns linear dist from motor encoder degrees
void switchTool(int svinput); //switchs between plotting tool
void plot(TFileHandle &fin); //reads insruction file, commands movement
void move(float distance, float angle); //moves plotter head
void returnToRestPosition(); //returns plotter to rest position
void erase(); //erases whiteboard
void drawGrid(); //draws grid on board

task main() //menu system and overall logic for calling functions
{
    bool exit = 0;
    //configuring sensors
    SensorType[SVPORT] = sensorI2CCustom9V;
    SensorType[ULTRA] = sensorEV3_Ultrasonic;
    SensorType[TOUCH_VERT] = sensorEV3_Touch;
    SensorType[TOUCH_HOR] = sensorEV3_Touch;

    drawGrid();
    time1[T1] = 0;
    int timeMins = IDLE_TIME % 60000;
    while (!exit && SensorValue(ULTRA) < SAFTEY_DIST && time1[T1] < IDLE_TIME) {
        eraseDisplay();
        displayString(1, "Welcome to GraphingBot");
        displayString(2, "Press top button to erase");
        displayString(3, "Press middle button to graph");
        displayString(4, "Press bottom button to exit");
        displayString(6, "Time until auto shutdown:");
        displayString(7, "%d minutes", timeMins);

        while (!getButtonPress(buttonAny) && SensorValue(ULTRA) < SAFTEY_DIST && time1[T1] % 60000 != 0) {}
        timeMins--;

        if (getButtonPress(buttonUp)) {
            time1[T1] = 0;
            timeMins = 5;
            while (getButtonPress(buttonUp) && SensorValue(ULTRA) < SAFTEY_DIST) {}
            erase();
        }

        if (getButtonPress(buttonEnter)) {
            time1[T1] = 0;
            timeMins = 5;
            while (getButtonPress(buttonEnter) && SensorValue(ULTRA) < SAFTEY_DIST) {}

            TFileHandle fin;
            openReadPC(fin, "graph.txt");
            plot(fin);
        }

        if (getButtonPress(buttonDown)) {
            while (getButtonPress(buttonDown)) {}
            exit = 1;
        }
    }
}

void setX(int power) {
    motor[X_TOP] = motor[X_BOT] = -power; //negative to correct motor orientation
}

void setY(int power) //CW is up, CCW is down looking from back
{
    motor[Y1] = motor[Y2] = power * Y_CORRECTION;
}

float getDist(int degrees) {
    return (fabs(((degrees / (1.0 * 360)) * (PI * WHEEL_SIZE))));
    //1.0 to correct for integer division
}

void switchTool(int svinput) //100 is marker, -100 is neutral, 200 is eraser
{
    setY(0);
    setX(0);

    if (svinput == 100) {
        setServoPosition(SVPORT, SVNUM, SV_MARKER);
        wait1Msec(1000);
    } else if (svinput == -100) {
        setServoPosition(SVPORT, SVNUM, SV_NEUT);
        wait1Msec(1000);
    } else if (svinput == 200) {
        //mechanical limitations prevent switching to eraser at bottom of ROM
        if (SensorValue(TOUCH_VERT) == 1) {
            //so we move up slightly first
            move(2, PI / 2);
            setServoPosition(SVPORT, SVNUM, SV_ERASER);
            wait1Msec(1000);
            setY(-REFPWR);
            while (SensorValue(TOUCH_VERT) != 1) {}
            setY(0);
        } else {
            setServoPosition(SVPORT, SVNUM, SV_ERASER);
        }
    }
}

void plot(TFileHandle &fin) {
    eraseDisplay();
    string function;
    float scale = 0;
    readTextPC(fin, function);
    readFloatPC(fin, scale);

    if (scale == 0) //verifying file has been read from
    {
        eraseDisplay();
        displayString(1, "Error: File Not Found");
        displayString(2, "Press a button");
        displayString(3, "to continue");
        while (!getButtonPress(buttonAny) && SensorValue(ULTRA) < SAFTEY_DIST) {}
        while (getButtonPress(buttonAny) && SensorValue(ULTRA) < SAFTEY_DIST) {}
    } else {
        displayString(1, "%s", function);
        displayString(2, "Scale: %f", scale);

        float distance = 0;
        float angle = 0;

        while (readFloatPC(fin, distance) && readFloatPC(fin, angle) &&
                SensorValue(ULTRA) < SAFTEY_DIST) {
            distance *= UNIT_SIZE;
            move(distance, angle);
        }
        setX(0);
        setY(0);

        returnToRestPosition();
    }
}

void move(float distance, float angle) {
    if (angle < -2 * PI || angle > 2 * PI) {
        switchTool(angle); //-100 is neutral, 100 is marker, 200 is eraser
    } else {
        nMotorEncoder[X_BOT] = 0;
        nMotorEncoder[Y2] = 0;
        setX(cos(angle) * REFPWR);
        setY(sin(angle) * REFPWR);
        //UNIT_SIZE translates -10 to 10 virtual grid into real distances
        while (((pow(getDist(nMotorEncoder[X_BOT]), 2) + pow((getDist(nMotorEncoder[Y2]) / Y_CORRECTION), 2))
                < pow(distance, 2))
                && SensorValue(ULTRA) < SAFTEY_DIST) {}
    }
}

void returnToRestPosition() {
    switchTool(-100);

    setY(-REFPWR);
    while (SensorValue(TOUCH_VERT) == 0 && SensorValue(ULTRA) < SAFTEY_DIST) {}
    setY(0);

    setX(-REFPWR);
    while (SensorValue(TOUCH_HOR) == 0 && SensorValue(ULTRA) < SAFTEY_DIST) {}
    setX(0);

    //We have changed from moving (left, down) to moving (down, left)
    //in order to make cable management easier
}

void erase() //hard coded s pattern based on constant eraser measurements
//and size of plotting plane
{
    switchTool(200);
    for (int count = 0; count < SIDE_LENGTH / (ERASE_WIDTH); count++) {
        nMotorEncoder[X_BOT] = 0;
        nMotorEncoder[Y2] = 0;

        setY(REFPWR * ERASE_CORRECTION);
        while (fabs(getDist(nMotorEncoder[Y2] / Y_CORRECTION)) < SIDE_LENGTH + 5 && SensorValue(ULTRA) < SAFTEY_DIST) {}
        setY(0);

        setY(-REFPWR * ERASE_CORRECTION);
        while (SensorValue(TOUCH_VERT) != 1 && SensorValue(ULTRA) < SAFTEY_DIST) {}
        setY(0);

        setX(REFPWR * ERASE_CORRECTION);
        while (fabs(getDist(nMotorEncoder[X_BOT])) < (ERASE_WIDTH * 1.0) && SensorValue(ULTRA) < SAFTEY_DIST) {}
        setX(0);
    }

    returnToRestPosition();
    drawGrid();
}

void drawGrid() //Always called at the rest position
{
    returnToRestPosition();
    //Drawing a square outline of the graphing window.
    switchTool(100);
    //Drawing right, up, left, down sides of the square in that order
    for (int count = 1; count <= 4; count++) {
        if (count % 2 == 0) {
            move(20 * UNIT_SIZE, PI / 2 * (count == 2 ? 1 : -1));
        } else {
            move(20 * UNIT_SIZE, PI * (count == 1 ? 0 : 1));
        }
    }
    returnToRestPosition();

    //Drawing x and y axes
    move(10 * UNIT_SIZE, PI / 2);
    switchTool(100);
    move(20 * UNIT_SIZE, 0);
    switchTool(-100);
    move(10 * UNIT_SIZE, PI / 2);
    move(10 * UNIT_SIZE, PI);
    switchTool(100);
    move(20 * UNIT_SIZE, -1 * PI / 2);

    returnToRestPosition();
}
