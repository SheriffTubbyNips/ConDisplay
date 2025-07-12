#pragma once
#include <windows.h>
#include <list>
#include <cmath>

#define BACKGROUND_WHITE BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED
#define FOREGROUND_WHITE FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED

// List-based image format (better for raster graphics)
struct Plot
{
    //Struct for holding data of single plot-points
    struct Point
    {
        int x = 0;
        int y = 0;
        WORD colour = 0;
        char character = ' ';

        Point() //Default constructor
        {}

        Point(int x,int y,WORD colour,char c) //Initializing constructor
        : x(x)
        , y(y)
        , colour(colour)
        , character(c)
        {}
    };

    std::string name;
    std::list<Point> points;

    Plot(std::string n,std::list<Point> p)
    : name(n)
    , points(p)
    {}

    Plot(std::list<Point> p)
    : name("Name Not Set")
    , points(p)
    {}

    Plot()
    {}

    void SetPoints(std::list<Point> pt)
    {
        points = pt;
    }

    void AddPoints(std::list<Point> pt)
    {
        for(Point temp:pt)
        {
            points.push_back(temp);
        }
    }

    std::list<Point> GetPoints()
    {
        return points;
    }
};


//Array-based image format (Better for buffered display)
struct Image
{
    //Struct for holding data of a single image array-index
    struct Pixel
    {
        WORD colour = 0;
        char character = ' ';
    };

    HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

    int sizeX,sizeY;
    Pixel** data;

    Image(int sX,int sY)
    : sizeX(sX)
    , sizeY(sY)
    {
        data = new Pixel*[sizeY];
        for(int i = 0; i < sizeY; i++)
        {
            data[i] = new Pixel[sizeX];
        }
    }

    //Draw pixel onto image
    void Draw(int x,int y,WORD colour,char c = ' ')
    {
        if(x < sizeX && y < sizeY)
        {
            data[y][x].character = c;
            data[y][x].colour = colour;
        }
    }

    //Insert contents of another image into this image at position x/y.
    void InsertImage(Image img,int x = 0,int y = 0)
    {
        for(int iy = 0; iy < sizeY && iy < img.sizeY; iy++)
        {
            for(int ix = 0; ix < sizeX && ix < img.sizeX; ix++)
            {
                data[iy+y][ix+x] = img.data[iy][ix];
            }
        }
    }

    //Converts plot-data to image-data and inserts it to image at x/y.
    void InsertPlot(Plot pl,int x = 0, int y = 0)
    {
        for(Plot::Point pt:pl.points)
        {
            if(pt.x < sizeX && pt.y < sizeY)
            {
                data[pt.y+y][pt.x+x].colour = pt.colour;
                data[pt.y+y][pt.x+x].character = pt.character;
            }
        }
    }

    //Displays image to console window.
    void Display()
    {
        CONSOLE_SCREEN_BUFFER_INFO oldInfo;
        GetConsoleScreenBufferInfo(consoleOut,&oldInfo);
        SetConsoleCursorPosition(consoleOut,{0,0});
        for(int y = 0; y < sizeY; y++)
        {
            for(int x = 0; x < sizeX; x++)
            {
                SetConsoleTextAttribute(consoleOut,data[y][x].colour);
                std::cout << data[y][x].character;
            }
            std::cout << "\n";
        }
        SetConsoleTextAttribute(consoleOut,oldInfo.wAttributes);
    }
};

void ClearConsole()
{
    static HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbInfo;
    GetConsoleScreenBufferInfo(consoleOut,&csbInfo);
    SetConsoleCursorPosition(consoleOut,{0,0});
    for(int iy = 0; iy < csbInfo.dwSize.Y; iy++)
    {
        for(int ix = 0; ix < csbInfo.dwSize.X; ix++)
        {
            std::cout << ' ';
        }
    }
    SetConsoleCursorPosition(consoleOut,{0,0});
}

//Generate line by slope intercept.
std::list<Plot::Point> GenerateLine(int x1,int y1,int x2,int y2,WORD colour,std::string text = " ")
{
    //Create list for line points (will be returned)
    std::list<Plot::Point> line;
    char c;

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
            if(a < (int)text.length())
            {
                c = text[a];
            }
            else
            {
                c = ' ';
            }
            line.push_back( {a*stepX+x1,a*stepY+y1,colour,c} );
        }
    }

    else if(abs(deltaY)>abs(deltaX)){
        float x,y;
        for(int a = 0; a <= length; a++){
            if(a < (int)text.length())
            {
                c = text[a];
            }
            else
            {
                c = ' ';
            }
            //deltaY is the large value so you can just increment it and solve for X.
            y = a*stepY+y1;
            x = (y-yIntercept)/slope;
            //Round x for rasterizing graphics.
            std::round(x);
            //Cast x/y as int for rasterized point list.
            line.push_back( {(int)x,(int)y,colour,c} );
        }
    }

    else{
        float x,y;
        for(int a = 0; a <= length; a++){
            if(a < (int)text.length())
            {
                c = text[a];
            }
            else
            {
                c = ' ';
            }
            //deltaX is the large value so you can just increment it and solve for Y.
            x = a*stepX+x1;
            y = slope*x+yIntercept;
            //Round y for rasterizing graphics.
            std::round(y);
            //Cast x/y as int for rasterized point list.
            line.push_back( {(int)x,(int)y,colour,c} );
        }
    }

    return line;
}

std::list<Plot::Point> GenerateSquare(int x1,int y1,int x2,int y2,WORD colour)
{
    Plot temp;
    temp.AddPoints(GenerateLine(x1,y1,x2,y1,colour));
    temp.AddPoints(GenerateLine(x2,y1,x2,y2,colour));
    temp.AddPoints(GenerateLine(x2,y2,x1,y2,colour));
    temp.AddPoints(GenerateLine(x1,y2,x1,y1,colour));
    return temp.points;
}

//Generate square by line-extrusion
std::list<Plot::Point> GenerateFilledSquare(int x1,int y1,int x2,int y2,WORD colour)
{
    std::list<Plot::Point> square,temp;

    for(int y = 0; y <= y2-y1; y++)
    {
        temp = GenerateLine(x1,y1+y,x2,y1+y,colour);
        for(Plot::Point pt:temp)
        {
            square.push_back(pt);
        }
    }
    return square;
}

std::list<Plot::Point> Translate(std::list<Plot::Point> oldPos,int x,int y)
{
    std::list<Plot::Point> newPos;

    for(Plot::Point p:oldPos)
    {
        newPos.push_back( {p.x+x,p.y+y,p.colour,p.character} );
    }

    return newPos;
}


//Rotate while assuming the origin lies at the first point.
std::list<Plot::Point> Rotate(std::list<Plot::Point> oldPos,int degrees)
{
    std::list<Plot::Point> newPos;

    std::list<Plot::Point>::iterator it = oldPos.begin();
    int originX = it->x;
    int originY = it->y;

    for(Plot::Point p:oldPos)
    {
        float x = ( (p.x-originX)*std::cos(3.1415926/180*degrees) - (p.y-originY)*std::sin(3.1415926/180*degrees) ) + originX;
        float y = ( (p.x-originX)*std::sin(3.1415926/180*degrees) + (p.y-originY)*std::cos(3.1415926/180*degrees) ) + originY;

        std::round(x);
        std::round(y);

        newPos.push_back( {(int)x,(int)y,p.colour,p.character} );
    }
    return newPos;
}

//Rotate from a set origin
std::list<Plot::Point> Rotate(std::list<Plot::Point> oldPos,int degrees,int originX,int originY)
{
    std::list<Plot::Point> newPos;

    for(Plot::Point p:oldPos)
    {
        float x = ( (p.x-originX)*std::cos(3.1415926/180*degrees) - (p.y-originY)*std::sin(3.1415926/180*degrees) ) + originX;
        float y = ( (p.x-originX)*std::sin(3.1415926/180*degrees) + (p.y-originY)*std::cos(3.1415926/180*degrees) ) + originY;

        std::round(x);
        std::round(y);

        newPos.push_back( {(int)x,(int)y,p.colour,p.character} );
    }
    return newPos;
}
