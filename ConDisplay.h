#pragma once

#include <cmath>
#include <windows.h>
#include <fstream>
#include <list>

#define BGWHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE
#define FGWHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define SETCURSOR(x,y) SetConsoleCursorPosition(hConsole,{(short)(x),(short)(y)})

#define RadToDeg(x) 180/3.1415926*x
#define DegToRad(x) 3.1415926/180*x

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
DWORD mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ~ENABLE_QUICK_EDIT_MODE;

int width = 2;
std::fstream fs;

struct coord
{
    float x = 0;
    float y = 0;
};

struct plot
{
    std::string id = "default";
    WORD colour = BGWHITE;
    char graphic = ' '; //Defaults display to a filled in character-block. Advisable to use BG colours for this case.
    std::list<coord> coords;

    void displayPlot()
    {
        for(coord c:coords){
            SETCURSOR(round(c.x)*width,round(c.y));
            SetConsoleTextAttribute(hConsole,colour);
            for(int a=1;a<=width;a++){std::cout << graphic;}
            SetConsoleTextAttribute(hConsole,FGWHITE);
        }
    }

    void createPoint(float x,float y)
    {
        coords.push_back({x,y});
    }

    void createLine(float X1,float Y1,float X2,float Y2)
    {
        //Get Delta for X and Y
        float dX = X2-X1;
        float dY = Y2-Y1;
        //Kind of a specific feature
        //Obtains a value of 1 or -1 to step/advance coordinates along the known-axis
        int incX = dX != 0 ? abs(dX)/dX : 0;
        int incY = dY != 0 ? abs(dY)/dY : 0;
        float slope;
        bool slopeUndef = false; //Useful so point logs for vertical lines do not get messed up
        std::isinf(dY/dX) ? slopeUndef = true : slope = dY/dX; //Checks for undefined slopes (vertical lines)
        float yInt = Y1-(slope*X1); //Obtains Y intercept
        /*Ties back into incX/incY this checks delta Y against delta X and assigns the length of the line
        to be the larger of the two values. The larger value can simply be advanced by incX/incY for each loop cycle and
        subsequently solve for the other value (x/y)

        If a line has a DX of 5 and a DY of 3 then you would assume the length of the line is 5
        Though in reality this plots 6 points because your first coordinate is treated as a 0 value
        EG a line from 2,2 to 7,2 has a DX of 5 (7-2=5) but you start at and include 2,2 then move through to 7,2
        2,2 3,2 4,2 5,2 6,2 7,2 <--- You end up with 6 points even though the "length" is 5
        So to get from 2 to 7 the line has to step horizontally 5 times so the X of each point is basically already known
        Subsequently to fully and accurately plot the line you have to solve for the Y of each X coordinate
        Or vice versa if DY was larger

        Mostly gotta do it this way because the computer doesn't have a ruler and a pencil... or hands

        EG if DX is larger then step X by 1/-1 each loop cycle and then solve for Y
        perhaps not an amazing system but mostly tends to work*/
        int length = abs(dX)>abs(dY) ? abs(dX) : abs(dY);
        if(slopeUndef == true){ //Case for vertical lines.
            for(int a = 0; a <= length; a++){
                this->createPoint(a*incX+X1,a*incY+Y1);
            }
        }
        else if(abs(dY)>abs(dX)){
            float y;
            for(int a = 0; a <= length; a++){
                //DY is the large value so you can just increment it and solve for X
                y = a*incY+Y1;
                this->createPoint( (y-yInt)/slope,y );
            }
        }
        else{
            float x;
            for(int a = 0; a <= length; a++){
                //DX is the large value so you can just increment it and solve for Y
                x = a*incX+X1;
                this->createPoint(x,slope*x+yInt);
            }
        }
    }

    void createBox(float X,float Y,float W,float L)
    {
        this->createLine(X,Y,X+W,Y);
        this->createLine(X+W,Y,X+W,Y+L);
        this->createLine(X+W,Y+L,X,Y+L);
        this->createLine(X,Y+L,X,Y);
    }

    void createFilledBox(float X,float Y,float W,float L)
    {
        for(int a = 0; a <= L; a++){
            this->createLine(X,Y+a,X+W,Y+a);
        }
    }

    void createCircle(float x,float y,float r)
    {
        std::list<coord> rasterCoords;
        coords.push_back({(float)x,(float)(y+r)});
        for(int a = 0; a < 360; a++){
            if(round(coords.back().x) != round(rasterCoords.back().x) || round(coords.back().y) != round(rasterCoords.back().y)){
                rasterCoords.push_back(coords.back());
            }
            this->rotatePlot(-1,x,y);
        }
        rasterCoords.pop_back(); //Revolving creates a redundant start-point. Remove it here.
        coords = rasterCoords;
    }

    void translatePlot(float x,float y)
    {
        for(std::list<coord>::iterator it = coords.begin();it != coords.end(); it++)
            {
                it->x += x;
                it->y += y;
            }
    }

    void rotatePlot(int deg,int originX = 0,int originY = 0)
    {
        float x,y;
        std::list<coord> newCoords;
        for(coord c:coords){
            x = ( (c.x-originX)*cos( DegToRad(deg) ) - (c.y-originY)*sin( DegToRad(deg) ) ) + originX;
            y = ( (c.x-originX)*sin( DegToRad(deg) ) + (c.y-originY)*cos( DegToRad(deg) ) ) + originY;
            newCoords.push_back({x,y});
        }
        coords = newCoords;
    }

    void savePlot(std::string path = "default")
    {
        if(path == "default"){
            path = id + ".succ";
        }
        fs.open(path,std::fstream::out|std::fstream::trunc);
        fs << id << "\n" << colour << "\n";
        for(coord c:coords)
        {
            fs << c.x << "," << c.y << ",";
        }
        fs.close();
    }

    void loadPlot(std::string path)
    {
        coord c;
        std::string output;
        fs.open(path,std::fstream::in);
        fs >> id >> colour;
        while(!fs.eof())
        {
            fs >> c.x;
            fs.ignore(10,',');
            fs >> c.y;
            fs.ignore(10,',');
            coords.push_back(c);
        }

        fs.close();
    }

};
