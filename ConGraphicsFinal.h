#pragma once
#include <windows.h>
#include <list>
#include <cmath>

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

    //Displays image to console window.
    void Display()
    {
        CONSOLE_SCREEN_BUFFER_INFO oldInfo;
        GetConsoleScreenBufferInfo(consoleOut,&oldInfo);

        for(int y = 0; y < sizeY; y++)
        {
            for(int x = 0; x < sizeX; x++)
            {
                SetConsoleTextAttribute(consoleOut,data[y][x].colour);
                SetConsoleCursorPosition(consoleOut,{(short)x,(short)y});
                std::cout << data[y][x].character;
            }
        }
        SetConsoleTextAttribute(consoleOut,oldInfo.wAttributes);
    }
};


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
};

//Buffers several plots into a single image.
struct PlotBuffer
{
    std::list<Plot> buffer; //list-based buffer for raster graphic plots

    Image* bufferedImage; //Image to act as container for buffered display

    HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

    PlotBuffer(int sX,int sY)
    {
        bufferedImage = new Image(sX,sY);
    }

    //Returns the currently buffered image.
    Image GetImage()
    {
        return *bufferedImage;
    }

    void AddPlot(Plot pl)
    {
        buffer.push_back(pl);
    }

    void RemovePlot(std::string name)
    {
        for(std::list<Plot>::iterator it = buffer.begin(); it != buffer.end(); it++)
        {
            if(it->name == name){
                buffer.erase(it);
                break;
            }
        }
    }

    //Reassigns the point-list of the named plot in the buffer-list.
    void EditPlot(std::string name, std::list<Plot::Point> pt)
    {
        for(std::list<Plot>::iterator it = buffer.begin(); it != buffer.end(); it++)
        {
            if(it->name == name)
            {
                it->points = pt;
                break;
            }
        }
    }

    std::list<Plot::Point> GetPlotPoints(std::string n)
    {
        for(Plot pl:buffer)
        {
            if(pl.name == n)
            {
                return pl.points;
            }
        }
        std::list<Plot::Point> error;
        return error;
    }

    void ClearBuffer()
    {
        for(int y = 0; y < bufferedImage->sizeY; y++)
        {
            for(int x = 0; x < bufferedImage->sizeX; x++)
            {
                bufferedImage->data[y][x].character = ' ';
                bufferedImage->data[y][x].colour = 0;
            }
        }
    }

    void BufferPlots()//Merge raster-plots into image
    {
        ClearBuffer();
        for(Plot pl:buffer)
        {
            for(Plot::Point pt:pl.points)
            {
                bufferedImage->data[pt.y][pt.x].character = pt.character;
                bufferedImage->data[pt.y][pt.x].colour = pt.colour;
            }
        }
    }

    //Displays image buffered from plot list. But you should probably use the image buffer for this.
    //I'm not your mom though.
    void Display()
    {
        bufferedImage->Display();
    }
};

//Buffers several images into one for simultaneous display.
struct ImageBuffer
{
    std::list<Image> buffer;
    Image* bufferedImage;

    ImageBuffer(int sX,int sY)
    {
        bufferedImage = new Image(sX,sY);
    }

    void BufferImages()//Merge images in buffer list to a single output image.
    {
        for(Image img:buffer)
        {
            for(int y = 0; y < img.sizeY && y < bufferedImage->sizeY; y++)
            {
                for(int x = 0; x < img.sizeX && x < bufferedImage->sizeX; x++)
                {
                    bufferedImage->data[y][x] = img.data[y][x];
                }
            }
        }
    }

    void Display()//100% doesn't display anything
    {
        bufferedImage->Display();
    }

    void AddImage(Image img)
    {
        buffer.push_back(img);
    }
};

std::list<Plot::Point> GenerateLine(int x1,int y1,int x2,int y2,WORD colour,char c = ' ')
{
    //Create list for line points (will be returned)
    std::list<Plot::Point> line;

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
