#pragma once

#include <cmath>
#include <windows.h>
#include <fstream>
#include <list>
#include <string>


#define TEXTDEFAULT FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED
#define BGWHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE
#define FGWHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define SETCURSOR(x,y) SetConsoleCursorPosition(hConsole,{(short)(x),(short)(y)})

#define PI 3.1415926
#define RadToDeg(x) 180/PI*x
#define DegToRad(x) PI/180*x

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
DWORD mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ~ENABLE_QUICK_EDIT_MODE;

int widthDefault = 2;
bool newLog = true;
std::ofstream writeLog; //Probably should set this up to be less shit PROBABLY
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
};

void updateLog(std::string info,plot* p = NULL)
{
    int counter = 0;
    if (newLog == true) {writeLog.open("log.txt",std::ofstream::trunc); newLog = false;}
    else {writeLog.open("log.txt",std::ofstream::app);}
    writeLog << info << "\n";
    if(p != NULL)
    {
        for(coord c:p->coords)
        {
            writeLog << p->id << "-" << counter << " [" << c.x << "," << c.y << "]\n";
            counter++;
        }
    }
    writeLog.close();
}

void savePlot(plot* p)
{
    std::string path = p->id + ".succ";
    const char* cPath = path.c_str();
    fs.open(cPath,std::fstream::out|std::fstream::trunc);
    fs << p->id << "\n" << p->colour << "\n";
    for(coord c:p->coords)
    {
        fs << c.x << "," << c.y << ",";
    }
    fs.close();
}

void loadPlot(plot* p,std::string path)
{
    coord c;
    std::string output;
    path = path + ".succ";
    fs.open(path,std::fstream::in);
    fs >> p->id >> p->colour;
    while(!fs.eof())
    {
        fs >> c.x;
        fs.ignore(10,',');
        fs >> c.y;
        fs.ignore(10,',');
        p->coords.push_back(c);
    }

    fs.close();


}

void translatePlot(plot* p,int x,int y)
{
    updateLog("Plot translated\nOld coordinates:",p);
    std::list<coord> newCoords;
    for(coord c:p->coords){
        c.x += x;
        c.y += y;
        newCoords.push_back(c);
    }
    p->coords = newCoords;
    updateLog("New coordinates:",p);
}

void displayPlot(plot* p,int width = widthDefault)
{
    for(coord c:p->coords){
        SETCURSOR(round(c.x)*width,round(c.y));
        SetConsoleTextAttribute(hConsole,p->colour);
        for(int a=1;a<=width;a++){std::cout << p->graphic;}
        SetConsoleTextAttribute(hConsole,TEXTDEFAULT);
    }
}

void rotatePlot(plot* p,int deg,int originX = 0,int originY = 0)
{
    float x,y;
    std::list<coord> newCoords;
    for(coord c:p->coords){
        x = ( (c.x-originX)*cos( DegToRad(deg) ) - (c.y-originY)*sin( DegToRad(deg) ) ) + originX;
        y = ( (c.x-originX)*sin( DegToRad(deg) ) + (c.y-originY)*cos( DegToRad(deg) ) ) + originY;
        newCoords.push_back({x,y});
    }
    p->coords = newCoords;
}

void createPoint(plot* p,int x,int y){
    p->coords.push_back({x,y});
}

void createLine(plot* p,int X1,int Y1,int X2,int Y2)
{
    //Get Delta for X and Y
    float dX = X2-X1;
    float dY = Y2-Y1;
    //Kind of a specific feature
    //Obtains a value of 1 or -1 to step/advance the coordinate which does not need to be solved for
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
    coord c;
    if(slopeUndef == true){ //Case for vertical lines.
        for(int a = 0; a <= length; a++){
            c.x = a*incX+X1;
            c.y = a*incY+Y1;
            p->coords.push_back(c);
        }
    }
    else if(abs(dY)>abs(dX)){
        for(int a = 0; a <= length; a++){
            c.y = a*incY+Y1; //DY is the large value so you can just increment it and solve for X
            c.x = (c.y-yInt)/slope;
            p->coords.push_back(c);
        }
    }
    else{
        for(int a = 0; a <= length; a++){
            c.x = a*incX+X1; //DX is the large value so you can just increment it and solve for Y
            c.y = slope*c.x + yInt;
            p->coords.push_back(c);
        }
    }
    updateLog("Line created.\nCoordinates:",p);
}

void createBox(plot* p,int x1,int y1,int x2,int y2)
{
    createLine(p,x1,y1,x2,y1);
    createLine(p,x1,y2,x2,y2);
    createLine(p,x1,y1,x1,y2);
    createLine(p,x2,y1,x2,y2);
}

void createFilledBox(plot* p,int x1,int y1,int x2,int y2)
{
    for(int a = 0; a <= y2-y1; a++){
        createLine(p,x1,y1+a,x2,y1+a);
    }
}

void createCircle(plot* p,int x,int y,int r)
{
    std::list<coord> rasterCoords;
    coord point = {(float)x,(float)(y+r)};
    p->coords.push_back(point);
    for(int a = 0; a < 360; a++){
        if(round(p->coords.back().x) != round(rasterCoords.back().x) || round(p->coords.back().y) != round(rasterCoords.back().y)){
            rasterCoords.push_back(p->coords.back());
        }
        rotatePlot(p,-1,x,y);
    }
    rasterCoords.pop_back(); //Revolving creates a redundant start-point. Remove it here.
    p->coords = rasterCoords;
}
