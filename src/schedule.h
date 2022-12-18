// make header file
#ifndef SCHEDULE_H
#define SCHEDULE_H

// include libraries
#include <bitmap.h>
#include "icalendar.h"

// define class
class Class
{
public:
    Class();
    Class(String name, String stime, String etime, String location, unsigned char *icon);
    String name;
    String stime;
    String etime;
    String location;
    unsigned char *icon;
};

class Day
{
public:
    Day();
    Day(Class p1, Class p2, Class p3, Class p4);
    Class p1;
    Class p2;
    Class p3;
    Class p4;
}

#endif