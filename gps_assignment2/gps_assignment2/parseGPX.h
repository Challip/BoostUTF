#ifndef PARSEGPX_H_201220
#define PARSEGPX_H_201220

#include <string>
#include <vector>

#include "points.h"
#include "xml/parser.h"

using namespace XML;
using namespace std;

namespace GPX
{
  /*  Parse GPX data containing a route.
   *  The source data can be provided as a string, or from a file; which one is determined by the bool parameter.
   */


class parseRoute

{
    public:

        parseRoute(std::string source, bool isFileName);//Constructor
        Element get_ele();

        std::vector<GPS::RoutePoint> get_result();


        private:

        Element ele = SelfClosingElement("",{});
        Element temp =SelfClosingElement("",{});
        string subSubElement;
        string lat;
        string lon;
        string name;
        std::vector<GPS::RoutePoint>result;


        void convert(string ,bool);
        void check(string);
        void checkSubElement(string);
        void checkLatLon();
        void find_position();
        void checkName();
        void pushName();
        void next_position();



};


  /*  Parse GPX data containing a track.
   *  The source data can be provided as a string, or from a file; which one is determined by the bool parameter.
   */
  std::vector<GPS::TrackPoint> parseTrack(std::string source, bool isFileName);
}

#endif
