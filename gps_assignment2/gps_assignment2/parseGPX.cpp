#include <sstream>
#include <fstream>
#include <iomanip>

#include "xml/parser.h"

#include "parseGPX.h"
using namespace XML;
using namespace GPS;
using namespace std;

namespace GPX
{

// convert sting in file to element
void parseRoute::convert(string source,bool Filename){
    ostringstream oss,oss2;
    string name;
    if (Filename) {
        ifstream fs(source);
        if (! fs.good()) throw invalid_argument("Error opening source file '" + source + "'.");
        oss << "Source file '" << source << "' opened okay." << endl;
        while (fs.good()) {
            getline(fs, name); // Using name as temporary variable as we don't need it until later
            oss2 << name << endl;
        }
        source = oss2.str();
    }

    Element ele = Parser(source).parseRootElement();

    this-> ele=ele;
}

Element parseRoute::get_ele()
{
    return ele;
}

//check for "gps" on sub element
void parseRoute::check(string subElement)
{

    Element ele=get_ele();
    if (ele.getName() != "gpx") throw domain_error("Missing 'gpx' element.");

    if (! ele.containsSubElement(subElement)) throw domain_error("Missing '"+subElement+"' element.");
    ele=ele.getSubElement(subElement);

    this -> ele= ele;
}

//check sub element
void parseRoute::checkSubElement(string subSubElement)
{
    //Element ele=get_ele();
    if (! ele.containsSubElement(subSubElement)) throw domain_error("Missing '"+subSubElement+"' element.");
    Element temp = ele.getSubElement(subSubElement);

    this->subSubElement=subSubElement;
    this-> temp=temp;

}



// check for lat and lon on sub element
void parseRoute::checkLatLon ()
{

    if (! temp.containsAttribute("lat")) throw domain_error("Missing 'lat' attribute.");
    string lat = temp.getAttribute("lat");
    this -> lat = lat;
    if (! temp.containsAttribute("lon")) throw domain_error("Missing 'lon' attribute.");
   string lon = temp.getAttribute("lon");
    this-> lon=lon;
}




//find position of the element and push it into result vector
void parseRoute::find_position()
{

    ostringstream oss;
    std::vector<RoutePoint>result;
    //int num =0;

    if (temp.containsSubElement("ele"))
    {
        Element temp2 = temp.getSubElement("ele");
        string el = temp2.getLeafContent();
        Position startPos = Position(lat,lon,el);
        result.push_back({startPos,""});
        oss << "Position added: " << endl; // << startPos.toString() << endl; // Need to update since removing toString()

    }
    else
    {
        Position startPos = Position(lat,lon);
        result.push_back({startPos,""});
        oss << "Position added: " << endl; // << startPos.toString() << endl; // Need to update since removing toString()
     }
   this->result=result;
}



std::vector<GPS::RoutePoint> parseRoute::get_result()
{
    return result;
}

void parseRoute::checkName()
{

    int i,j;
    string name;


    if (temp.containsSubElement("name"))
    {
        Element temp2 = temp.getSubElement("name");
        name = temp2.getLeafContent();
        i = name.find_first_not_of(' ');
        j = name.find_last_not_of(' ');
        name = (i == -1) ? "" : name.substr(i,j-i+1);
    }
    else name = ""; // Fixed bug by adding this.

    //result.front().name = name;
    this -> name= name;

}

void parseRoute::pushName()
{
    checkName();
    result.front().name = name;
}

void parseRoute::next_position()
{
    int num =1, skipped=0;
    int total = ele.countSubElements(subSubElement);
    Position prevPos (0,0), nextPos=prevPos;
    prevPos = result.back().position;
    nextPos = result.back().position;
    ostringstream oss;



    while (num+skipped < total)
    {
              temp = ele.getSubElement("rtept",num+skipped);
              checkLatLon();
              if (temp.containsSubElement("ele"))
              {
                  Element temp2 = temp.getSubElement("ele");
                  string el = temp2.getLeafContent();
                  nextPos = Position(lat,lon,el);
              }
              else nextPos = Position(lat,lon);
              checkName();

              result.push_back({nextPos,name});
              oss << "Position added: " << endl; // << nextPos.toString() << endl; // Need to update since removing toString()
              ++num;
              prevPos = nextPos;
    }

}

  parseRoute::parseRoute(std::string source, bool isFileName)
  {
       convert(source,isFileName);
       check("rte");
       // rtept element
       checkSubElement("rtept");
       checkLatLon();
       find_position();
       pushName();
       next_position();

  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Element convertSource(string source,bool isFileName)
  {
      ostringstream oss,oss2;
      string name;
      if (isFileName) {
          ifstream fs(source);
          if (! fs.good()) throw invalid_argument("Error opening source file '" + source + "'.");
          oss << "Source file '" << source << "' opened okay." << endl;
          while (fs.good()) {
              getline(fs, name); // Using name as temporary variable as we don't need it until later
              oss2 << name << endl;
          }
          source = oss2.str();
      }
      Element ele = Parser(source).parseRootElement();
      return ele;
  }

  Element check_subElement( Element ele,string subElement)
  {

      if (! ele.containsSubElement(subElement)) throw domain_error("Missing '" +subElement+ "' element.");
      Element element = ele.getSubElement(subElement);
      return element;
  }

  string get_lat(Element temp)
  {
      string lat;
      if (! temp.containsAttribute("lat")) throw domain_error("Missing 'lat' attribute.");
      lat = temp.getAttribute("lat");
      return lat;

  }
string get_lon(Element temp)
{
    string lon;
    if (! temp.containsAttribute("lon")) throw domain_error("Missing 'lon' attribute.");
    lon = temp.getAttribute("lon");
    return lon;

}

//get t element from tag time
//also check for time in temp
tm get_t(Element temp)
{
    string time;
    tm t;
    istringstream iss;

    if (! temp.containsSubElement("time")) throw domain_error("Missing 'time' element.");
    Element temp2 = temp.getSubElement("time");
    time = temp2.getLeafContent();
    iss.str(time);
    iss >> std::get_time(&t,"%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail()) throw std::domain_error("Malformed date/time content: " + time);

    return t;
}

string get_name(Element temp)
{
    string name;
    int i,j;

    //if temp contain element name return name, otherwise return empty name
    if (temp.containsSubElement("name")) {
        Element temp2 = temp.getSubElement("name");
        name = temp2.getLeafContent();
        i = name.find_first_not_of(' ');
        j = name.find_last_not_of(' ');
        name = (i == -1) ? "" : name.substr(i,j-i+1);
    } else name = ""; // Fixed bug by adding this.

    return name;
}


// if temp(trkpt element) contain 'ele', start position will be (lat,lon, el), otherwise star position will be (lat,lon)
//startPos will then be pushed to vector result.
std::vector<GPS::TrackPoint> find_startPos(Element temp,string lat,string lon,string name,tm t)
{
    Position startPos(0,0);
    std::vector<GPS::TrackPoint>result;
    ostringstream oss;
    if (temp.containsSubElement("ele"))
    {

        //element in ele
        Element temp2 = temp.getSubElement("ele");
        string el = temp2.getLeafContent();
        startPos = Position(lat,lon,el);
        result.push_back({startPos,name,t});
        oss << "Position added: " << endl; // << startPos.toString() << endl; // Need to update since removing toString()

    }
    else
    {
        startPos = Position(lat,lon);
        result.push_back({startPos,name,t});
        oss << "Position added: " << endl; // << startPos.toString() << endl; // Need to update since removing toString()

    }

    return result;

}

//find next position
Position find_nextPos(Element temp,string lat,string lon)
{
    Position nextPos(0,0);

    if (temp.containsSubElement("ele")) {
        Element temp2 = temp.getSubElement("ele");
        string el = temp2.getLeafContent();
        nextPos = Position(lat,lon,el);
    }
    else nextPos = Position(lat,lon);
    return nextPos;
}



  std::vector<GPS::TrackPoint> parseTrack(std::string source, bool isFileName)
   {

       int num,total,skipped;
       ostringstream oss;
       string lat,lon,name,time;
       tm t;
       std::vector<TrackPoint> result;
       Element temp = SelfClosingElement("",{}), ele2 = temp ; // Work-around because there's no public constructor in Element.
       Position startPos(0,0), prevPos = startPos, nextPos = startPos; // Same thing but for Position.

       Element ele =convertSource(source,isFileName);
        if (ele.getName() != "gpx") throw domain_error("Missing 'gpx' element.");

        //element in trk
       ele=check_subElement(ele,"trk");

       num = 1;// num start at 1 as i replace if cause with find_startPos funtion

       if (! ele.containsSubElement("trkseg"))
       {

               //Element in trkpt
               temp = check_subElement(ele,"trkpt");
               total = ele.countSubElements("trkpt");
               lat=get_lat(temp);
               lon=get_lon(temp);

               //find startPos anf push to result
               result=find_startPos(temp,lat,lon,name,t);

               //if temp contain element name return name, otherwise return empty name
               name=get_name(temp);
               //push name in to vector result
               result.back().name = name;
               //get t element from tag time
               t=get_t(temp);
               //push t to result
               result.back().dateTime = t;


               prevPos = result.back().position;
               nextPos = result.back().position;
               skipped = 0;


               while (num+skipped < total)
               {
                       temp = ele.getSubElement("trkpt",num+skipped);

                       lat = temp.getAttribute("lat");
                       lon = temp.getAttribute("lon");

                       //find nextPos
                       nextPos=find_nextPos(temp,lat,lon);

                       //get t element from tag time
                       t=get_t(temp);

                       //if temp contain element name return name, otherwise return empty name
                       name=get_name(temp);

                       result.push_back({nextPos,name,t});

                       oss << "Position added: " << endl; // << nextPos.toString() << endl; // Need to update since removing toString()
                       oss << " at time: " << std::put_time(&t,"%c") << endl;
                       ++num;
                       prevPos = nextPos;

               }//end of while loop
       }//end of if (temp.containsSubElement("name"))

       else
       {
           for (unsigned int segNum = 0; segNum < ele.countSubElements("trkseg"); ++segNum)
           {
               ele2 = ele.getSubElement("trkseg",segNum);
               total = ele2.countSubElements("trkpt");
               skipped = -num; // Setting skipped to start at -num (rather than 0) cancels any points accumulated from previous segments
                               // We have to set it here, rather than just before the loop, because num may increment in the next if-statement
               if (segNum == 0)
               {
                   temp = ele2.getSubElement("trkpt");
                   lat = get_lat(temp);
                   lon = get_lon(temp);
                   result=find_startPos(temp,lat,lon,name,t);
                   name=get_name(temp);

                   result.back().name = name;

                   t=get_t(temp);
                   result.back().dateTime = t;

                   ++num;//because replce if cuase with function find_startPos

                }//end of if(segNum==0)

              prevPos = result.back().position;
              nextPos = result.back().position;

              while (num+skipped < total)
              {
                  temp = ele2.getSubElement("trkpt",num+skipped);
                  lat = get_lat(temp);
                  lon = get_lon(temp);
                  //find nextPos

                  nextPos=find_nextPos(temp,lat,lon);

                  t=get_t(temp);

                  name=get_name(temp);


                  result.push_back({nextPos,name,t});
                  oss << "Position added: " << endl; // << nextPos.toString() << endl; // Need to update since removing toString()
                  oss << " at time: " << std::put_time(&t,"%c") << endl;
                  ++num;
                  prevPos = nextPos;
                  // }
              }//end of while loop
           }//end og for loop
       }
       oss << num << " positions added." << endl;
       // cout << oss.str();
       return result;
   }
 }
