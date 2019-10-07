#ifndef __SPRING_UTILITIES_H__
#define __SPRING_UTILITIES_H__

#include <string>
#include <sstream>

namespace outputmod
{
    
    std::ostream& startred( std::ostream& stream );
    std::ostream& endred( std::ostream& stream );
    
    std::ostream& startgreen( std::ostream& stream );
    std::ostream& endgreen( std::ostream& stream );
    
    std::ostream& startpink( std::ostream& stream );
    std::ostream& endpink( std::ostream& stream );
    
    std::ostream& startblue( std::ostream& stream );
    std::ostream& endblue( std::ostream& stream );
    
}

namespace stringutils 
{
    
    template<class T>
    std::string convertToString( const T& tostring )
    {
        std::string out_string;
        std::stringstream ss;
        
        ss << tostring;
        ss >> out_string;
        
        return out_string;
    }
    
    template<class T>
    bool extractFromString( const std::string& in_string, T& output )
    {
        return (std::stringstream(in_string) >> output);
    }
    
}

#endif
