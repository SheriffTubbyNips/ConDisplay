//Turns out the plot system was good after all
//Should actually upgrade that and keep using it
//Bake the buffer into the bottom-level code and stop using the default display code for consoles
//Try to make this the last time this has to be redone
//(we know it wont be but try)

#pragma once
#include <windows.h>
#include <list>
#include <cmath>

struct Point
{
    int x,y;
    WORD colour;
    char character;

    Point() //Default constructor
    {
        x = 0;
        y = 0;
        colour = 0;
        character = ' ';
    }

    Point(int x,int y,WORD colour,char c) //Initializing constructor
    : x(x)
    , y(y)
    , colour(colour)
    , character(c)
    {}
};

struct Plot
{
    std::string name;
    std::list<Point> points;

    Plot(std::string n,std::list<Point> p)
    : name(n)
    , points(p)
    {}
};

struct DisplayHandler
{
    std::list<Plot> buffer;

    Point** finalFrame;
    int sizeX,sizeY;

    HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DisplayHandler(int sX,int sY)
    : sizeX(sX)
    , sizeY(sY)
    {
        finalFrame = new Point*[sizeY];
        for(int i = 0; i < sizeY; i++)
        {
            finalFrame[i] = new Point[sizeX];
        }
    }

    void AddPlot(Plot p)
    {
        buffer.push_back(p);
    }

    void RemovePlot(std::string n)
    {
        for(std::list<Plot>::iterator it = buffer.begin(); it != buffer.end(); it++)
        {
            if(it->name == n){
                buffer.erase(it);
                break;
            }
        }
    }

    void EditPlot(std::string n, std::list<Point> p)
    {
        for(std::list<Plot>::iterator it = buffer.begin(); it != buffer.end(); it++)
        {
            if(it->name == n)
            {
                it->points = p;
                break;
            }
        }
    }

    std::list<Point> GetPlotPoints(std::string n)
    {
        for(Plot pl:buffer)
        {
            if(pl.name == n)
            {
                return pl.points;
                break;
            }
        }
    }

    void Display()
    {
        CONSOLE_SCREEN_BUFFER_INFO oldInfo;
        GetConsoleScreenBufferInfo(consoleOut,&oldInfo);

        for(Plot pl:buffer)
        {
            for(Point pt:pl.points)
            {
                finalFrame[pt.y][pt.x] = pt;
            }
        }

        for(int y = 0; y < sizeY; y++)
        {
            for(int x = 0; x < sizeX; x++)
            {
                SetConsoleTextAttribute(consoleOut,finalFrame[y][x].colour);
                SetConsoleCursorPosition(consoleOut,{x,y});
                std::cout << finalFrame[y][x].character;
            }
        }
        SetConsoleTextAttribute(consoleOut,oldInfo.wAttributes);
    }



};

std::list<Point> GenerateLine(int x1,int y1,int x2,int y2,WORD colour,char c = ' ')
{
    //Create list for line points (will be returned)
    std::list<Point> line;

    //Calculate delta x/y
    float deltaX = x2-x1;
    float deltaY = y2-y1;

    bool slopeUndefined = false;
    //Calculates slope or sets slopeUndefined to true if slope calculation would divide by 0
    float slope = std::isinf(deltaY/deltaX) ? slopeUndefined = true : slope = deltaY/deltaX;

    //Calculate y Intercept
    float yIntercept = y1-(slope*x1);

    /*
    The long axis of the line does not need to be solved for in a rasterized integer-grid.
    The line has to extend from x1/y1 to x2/y2. You can step 1 over for each point then solve
    for the other coordinate. This determines if the line is stepping left or right and will
    assign a value of either 1 or -1 depending on the direction.
    */
    int stepX = deltaX != 0 ? abs(deltaX)/deltaX : 0;
    int stepY = deltaY != 0 ? abs(deltaY)/deltaY : 0;

    /*
    Determines the length of the line for the sake of running the point-plotting loop.
    Note that rasterized lengths will not match to expected euclidian distances. Attempting
    to calculate the line-length normally would produce a line too long. Because integer-pixels
    do not accurately represent diagonal distances.
    */
    int length = abs(deltaX)>abs(deltaY) ? abs(deltaX) : abs(deltaY);

    if(slopeUndefined == true){ //Case for vertical lines.
        for(int a = 0; a <= length; a++){
            line.push_back(
                {
                    a*stepX+x1
                    ,a*stepY+y1
                    ,colour
                    ,c
                }
            );

        }
    }

    else if(abs(deltaY)>abs(deltaX)){
        float x,y;
        for(int a = 0; a <= length; a++){
            //deltaY is the large value so you can just increment it and solve for X.
            y = a*stepY+y1;
            x = (y-yIntercept)/slope;
            //Round x for rasterizing graphics.
            std::round(x);
            //Cast x/y as int for rasterized point list.
            line.push_back(
                {
                     (int)x
                    ,(int)y
                    ,colour
                    ,c
                }
            );
        }
    }
    else{
        float x,y;
        for(int a = 0; a <= length; a++){
            //deltaX is the large value so you can just increment it and solve for Y.
            x = a*stepX+x1;
            y = slope*x+yIntercept;
            //Round y for rasterizing graphics.
            std::round(y);
            //Cast x/y as int for rasterized point list.
            line.push_back(
                {
                     (int)x
                    ,(int)y
                    ,colour
                    ,c
                }
            );
        }
    }

    return line;
}

std::list<Point> Rotate(std::list<Point> oldPos,int degrees,int originX,int originY)
{
    std::list<Point> newPos;

    for(Point p:oldPos)
    {
        //float x = (p.x-originX)*std::cos()
    }
}
