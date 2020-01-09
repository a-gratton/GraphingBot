//inclusion of selected standard C++ libraries
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

class Polynomial {
 public:
    //default constructor
    Polynomial() //coefficients stored in even indices, exponents in odd ones.
    {
        //if object created with no arguments, automatically prompts for input.
        //note if a users last input has no exponent value,
        //then it is assumed to be an x^0 term (a constant value).
        double temp = 0;

        cout << "Input the coefficients and exponents of your function," << endl
             << "pressing enter between each, and end your input with a letter."
             << endl << endl << "For example, Y = X^2 + 0.5x + 5 would be entered as "
             << "1 2 0.5 1 5 0 " << endl << "with enter replacing each space and a "
             << "letter at the end" << endl;

        while (cin >> temp) {
            polyFunction.push_back(temp);
            if (polyFunction.size() % 2 == 0) {
                system("cls");
                currentFunction();
                cout << "Enter the next term, or a letter to end." << endl;
            }
        }

        //defining behaviour for coefficient-only last entry
        if (polyFunction.size() % 2 != 0) {
            polyFunction.push_back(0);
        }

        finalFunction();
    }

    void outputFunction() const
    //output readable function to user
    {
        //catching the case of an empty input
        if (polyFunction.size() == 0) {
            cout << "Y = 0" << endl;
        } else {
            //finding constant term to ensure we output it at the end of the function
            double constantTerm = 0;
            if (polyFunction[polyFunction.size() - 1] == 0) {
                constantTerm = polyFunction[polyFunction.size() - 2];
            }

            //output formatting
            cout << " Y = ";
            for (int index = 0; index < polyFunction.size(); index++) {
                if ((polyFunction[index] == constantTerm && polyFunction[index + 1] == 0) ||
                        (polyFunction[index] == 0 && polyFunction[index - 1] == constantTerm)) {}

                else if (index == 0) {
                    cout << polyFunction[index];
                } else if (index % 2 == 0) {

                    if (polyFunction[index] < 0) {
                        cout << " - " << polyFunction[index] * -1;
                    } else {
                        cout << " + " << polyFunction[index];
                    }
                } else {
                    if (polyFunction[index] == 1) {
                        cout << "X";
                    } else {
                        cout << "X^" << polyFunction[index];
                    }
                }
            }
            //constant term output
            if (constantTerm != 0) {
                if (constantTerm < 0) {
                    cout << " - " << constantTerm * -1;
                } else {
                    cout << " + " << constantTerm;
                }
            }
            cout << endl;
        }
    }

    double evalFunction(double x) const //evaluates function at given x value
    {
        double sum = 0;
        for (int index = 0; index < polyFunction.size(); index += 2) {
            sum += polyFunction[index] * pow(x, polyFunction[index + 1]);
        }
        return sum;
    }

    void graph(double const scale) const
    //outputs file for the robot to graph
    //first line is the function (to be read as a string) and the scale factor
    //second line is movement to point where function first intersects plane
    //all other lines are pairs of distances and angles for the robot to draw
    //angles of -100 signal switch to null implement, 100 signals marker.
    {
        const double PRECISION = 0.5;
        double prevX = -10, prevY = -10, newX = 0, newY = 0, angle = 0, dist = 0;
        bool goingOff = 1, write = 0;
        ofstream fout(FILE_PATH.string::c_str());
        //ofstream constructor needs a C type string instead of a C++ one
        //the function c_str returns a pointer to an array of this type
        //which the ofstream constructor then knows how to use.

        //note that this wasnt necessary for our implementation
        //but was built in case we could figure out how to directly upload files
        //to the EV3.

        //string output formatting
        for (int index = 0; index < polyFunction.size(); index++) {
            if (index % 2 == 0) {
                fout << showpos << polyFunction[index];
            } else if (polyFunction[index] == 0) {}
            else {
                fout << noshowpos << "X^" << polyFunction[index];
            }
        }
        fout << noshowpos << " " << scale << endl;

        //main evaluation loop
        for (double value = -10 / scale; value <= 10 / scale; value += PRECISION / scale) {
            //normalizing values back onto -10 to 10 virtual grid
            newX = value * scale;
            newY = evalFunction(value) * scale;

            if (goingOff) //handles initial implement switch instruction
                //and also other cases where the graph goes out of bounds
            {
                fout << 0 << "  " << -100 << endl;
                write = 1;
                goingOff = 0;
            }

            if (newY > 10 || newY < -10) {
                if (!write) //prevents writing implement instruction over and over
                {
                    goingOff = 1; //communicating graph has left the normalized range
                }
            }

                //standard instruction generator (distance and angle)
            else {

                fout << sqrt(pow((newX - prevX), 2) + pow((newY - prevY), 2));

                if (newX - prevX == 0) {
                    fout << " " << 0 << endl;
                } else {
                    fout << "  " << atan((newY - prevY) / (newX - prevX)) << endl;


                    //<< "  " << newX << "  " << newY
                    //<< endl;
                }

                prevX = newX;
                prevY = newY;

                if (write) //catching graph re-entering plane
                {
                    fout << 0 << "  " << 100 << endl;
                    write = 0;
                }
            }
        }

        fout.close();
    }

 private:
    //private variables
    vector<double> polyFunction; //function storage container
    string liveFunction; //for outputting function after each term entry


    //not constant because string needs to be modified into a cstring later on
    string FILE_PATH = "C:/Users/gratt/Documents/Final Project/graph.txt";

    //private functions
    void finalFunction()
    //returns a vector with like terms combined, in descending exponent order
    //even entries are coefficients, odd entries are exponents
    {
        vector<double> simplified(0);
        vector<double> exponentOrder(0);

        //for all exponent values of the input
        for (int expIndex = 1; expIndex < polyFunction.size(); expIndex += 2) {
            exponentOrder.push_back(polyFunction[expIndex]);
        }
        makeUnique(exponentOrder);

        for (int ordIndex = 0; ordIndex < exponentOrder.size(); ordIndex++) {
            double tempSum = 0;
            //for all exponent values of the input
            for (int inputIndex = 1; inputIndex < polyFunction.size(); inputIndex += 2) {
                //if the value matches the current exponent we are working on
                if (polyFunction[inputIndex] == exponentOrder[ordIndex])
                    //add the coefficient to the running total
                    tempSum += polyFunction[inputIndex - 1];
            }
            //if the coefficient is non-zero, put in vector "simplified"
            if (tempSum != 0) {
                simplified.push_back(tempSum);
                simplified.push_back(exponentOrder[ordIndex]);
            }
        }

        polyFunction = simplified; //assigning final function to object vector
    }

    void makeUnique(vector<double> &uniqueVec) {
        //had to use iterator since unique returns an iterator value
        vector<double>::iterator iuniqueVec;
        //makes the passed vector have only unique entries
        sort(uniqueVec.begin(), uniqueVec.end(), greater<double>());
        iuniqueVec = unique(uniqueVec.begin(), uniqueVec.end());
        //vector resizing must be done manually aftr unique is run
        uniqueVec.erase(iuniqueVec, uniqueVec.end());
    }

    void currentFunction() {
        //outputs the current function to the console
        //updates dynamically as user enters each term
        {
            //checks for non integer exponent entries
            //main input verification handler
            if (polyFunction[polyFunction.size() - 1] < 0
                    || floor(polyFunction[polyFunction.size() - 1]) != polyFunction[polyFunction.size() - 1]) {
                polyFunction.pop_back();
                polyFunction.pop_back();
                cout << "Invalid Entry: Exponent must be a positive integer" << endl;
            } else {
                liveFunction = "";
                for (int index = 0; index < polyFunction.size(); index++) {
                    string temp = "";
                    stringstream conversion;
                    conversion << polyFunction[index];
                    temp = conversion.str();

                    //output formatting
                    if (index % 2 == 0) //if current index is a coefficient
                    {
                        liveFunction += temp + "X";
                    } else //if current index is an exponent
                    {
                        liveFunction += "^" + temp + " + ";
                    }
                }
            }
            cout << liveFunction << endl << endl;
        }
    }
};

void specialtyGraph(int val);

int main() {
    vector<Polynomial> userFunctionList; //stores object for reach function entered
    int const SPECIALTY = 3; //number of specialty functions coded
    int prompt = 100; //ensuring overall menu loop structure runs at least once

    while (prompt) //menu system
    {
        do {
            cout << "Welcome to DesmosBot" << endl;
            cout << "What would you like to do?" << endl;
            cout << "Enter 1 to input a new custom function" << endl;
            cout << "Enter 2 to choose a custom function to graph" << endl;
            cout << "Enter 3 to choose a built in specialty function to graph" << endl;
            cout << "Enter 0 to exit" << endl;
        } while (cin >> prompt && prompt != 1 && prompt != 2 && prompt != 3 && prompt != 0);

        if (prompt == 1) {
            system("cls");
            userFunctionList.push_back(Polynomial()); //calls input constructor
            system("cls");
            userFunctionList[userFunctionList.size() - 1].outputFunction();
            prompt = 100;
            system("pause");
        } else if (prompt == 2) //user choosing which function they want to graph
        {
            if (userFunctionList.size() == 0) //if no functions have been entered
            {
                cout << "There are no functions available to graph" << endl;
                system("pause");
            } else //otherwise display the available functions and go into prompt
            {
                bool functionInvalid = 0, scaleInvalid = 0;
                double function = 0; //uses double to prevent unexpected behaviour
                //for non integer entries
                //verifies int value later using floor()
                double scale = 1;

                do {
                    system("cls");
                    for (int index = 0; index < userFunctionList.size(); index++) {
                        cout << index + 1 << ": ";
                        userFunctionList[index].outputFunction();
                    }

                    if (functionInvalid) //error message (only appears after 1 loop)
                    {
                        cout << "Invalid Entry: Numbers must be positive integers"
                             << " in the range [1, " << userFunctionList.size() << "]."
                             << endl;
                    }
                    cout << "Enter the number of the function you want to graph,"
                         << endl << "or a letter to go back." << endl;
                    functionInvalid = 0;
                }
                    //sets flag for invalid message and accepts the input
                    //while also verifying valid function selection
                while (cin >> function && (functionInvalid = 1) &&
                        (function <= 0 || function > userFunctionList.size()
                                || floor(function) != function));

                if (functionInvalid) //takes advantage of above short circuit evaluation
                {
                    do //getting scale factor
                    {
                        system("cls");
                        if (scaleInvalid) {
                            cout << "Invalid Entry: Scale factor must be a "
                                 << "positive number." << endl;
                            scaleInvalid = 0;
                        }
                        cout << "Enter a scale factor for your graph, "
                             << endl << "or a letter to go back." << endl;
                        scaleInvalid = 1;
                    } while (cin >> scale && scale <= 0);

                    userFunctionList[function - 1].graph(scale);
                }
            }
            prompt = 100;
        } else if (prompt == 3)
            //some hard coded non polynomial functions,
            //to demonstrate robot capabilities.
            //note that parsing user input for these functions is beyond
            //our project scope.
        {
            bool functionInvalid = 0;
            double function = 0;
            //uses double to prevent unexpected behaviour for non integer entries
            //verifies int value later using floor()
            do {
                system("cls");
                cout << "Trig Functions:" << endl;
                cout << "1: sin(X)" << endl;
                cout << "2: cos(X)" << endl;
                cout << endl << endl;
                cout << "3: 1/X" << endl;
                if (functionInvalid) //error message (only appears after 1 loop)
                {
                    cout << "Invalid Entry: Numbers must be positive integers"
                         << " in the range [1, " << SPECIALTY << "]."
                         << endl;
                }
                cout << "Enter the number of the function you want to graph,"
                     << endl << "or a letter to go back." << endl;
                functionInvalid = 0;
            }
                //sets flag for invalid message and accepts the input
                //while also verifying valid function selection
            while (cin >> function && (functionInvalid = 1) &&
                    (function <= 0 || function > SPECIALTY
                            || floor(function) != function));

            if (functionInvalid) //takes advantage of above short circuit evaluation
            {
                specialtyGraph(function);
            }
        }

        cin.clear(); //fixes infinite looping top-level menu
        cin.ignore(1);

        system("cls");
    }
}

void specialtyGraph(int val) {
    //modified version of Polynomial::graph()
    //to generate graph files for some specialty functions.
    //essentially separate from rest of the code
    string FILE_PATH = "C:/Users/gratt/Documents/Final Project/graph.txt";
    const double PRECISION = 0.5;
    double prevX = -10, prevY = -10, newX = 0, newY = 0, angle = 0, dist = 0;
    bool goingOff = 1, write = 0;
    ofstream fout(FILE_PATH.string::c_str());
    //ofstream constructor needs a C type string instead of a C++ one
    //the function c_str returns a pointer to an array of this type
    //which the ofstream constructor then knows how to use

    if (val == 1) //"scale factor 5" == 5trig(X)
    {
        fout << "sin(X)" << "  " << 5 << endl;
        for (double value = -10; value <= 10; value += PRECISION) {
            newX = value;
            newY = 5 * (sin(value));

            if (goingOff) {
                fout << 0 << "  " << -100 << endl;
                write = 1;
                goingOff = 0;
            }

            if (newY > 10 || newY < -10) {
                if (!write) {
                    goingOff = 1;
                }
            } else if (newY - prevY == 0) {
                fout << "  " << M_PI / 2 << endl;
            } else {

                fout << sqrt(pow((newX - prevX), 2) + pow((newY - prevY), 2));

                if (newX - prevX == 0) {
                    fout << " " << M_PI / 2 << endl;
                } else {
                    fout << "  " << atan((newY - prevY) / (newX - prevX)) << endl;
                }

                prevX = newX;
                prevY = newY;

                if (write) {
                    fout << 0 << "  " << 100 << endl;
                    write = 0;
                }
            }
        }
        fout.close();
    } else if (val == 2) {
        fout << "cos(X)" << "  " << 5 << endl;
        for (double value = -10; value <= 10; value += PRECISION) {
            newX = value;
            newY = cos(value);

            if (goingOff) {
                fout << 0 << "  " << -100 << endl;
                write = 1;
                goingOff = 0;
            }

            if (newY > 10 || newY < -10) {
                if (!write) {
                    goingOff = 1;
                }
            } else {

                fout << sqrt(pow((newX - prevX), 2) + pow((newY - prevY), 2));

                if (newX - prevX == 0) {
                    fout << " " << M_PI / 2 << endl;
                } else if (newY - prevY == 0) {
                    fout << "  " << M_PI / 2 << endl;
                } else {
                    fout << "  " << atan((newY - prevY) / (newX - prevX)) << endl;
                }

                prevX = newX;
                prevY = newY;

                if (write) {
                    fout << 0 << "  " << 100 << endl;
                    write = 0;
                }
            }
        }
        fout.close();
    } else if (val == 3) {
        bool first = 1;
        fout << "1/X" << "  " << 5 << endl; //scale factor 5 == (1/X) * 5
        for (double value = -10; value <= 10; value += PRECISION) {
            newX = value;
            newY = 5 * (1 / value);

            if (goingOff) {
                fout << 0 << "  " << -100 << endl;
                write = 1;
                goingOff = 0;
            }

            if (newY > 10 || newY < -10) {
                if (!write) {
                    goingOff = 1;
                }
            } else {

                fout << sqrt(pow((newX - prevX), 2) + pow((newY - prevY), 2));

                if (first) {
                    fout << "  " << M_PI / 2 << endl;
                    first = 0;
                } else if (newX - prevX == 0) {
                    fout << " " << 0 << endl;
                } else {
                    fout << "  " << atan((newY - prevY) / (newX - prevX)) << endl;
                }

                prevX = newX;
                prevY = newY;

                if (write) {
                    fout << 0 << "  " << 100 << endl;
                    write = 0;
                }
            }
        }
        fout.close();
    }
}
