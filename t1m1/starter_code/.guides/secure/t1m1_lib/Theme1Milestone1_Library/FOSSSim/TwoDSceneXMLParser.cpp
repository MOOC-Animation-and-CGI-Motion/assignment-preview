#include "TwoDSceneXMLParser.h"

void TwoDSceneXMLParser::loadSceneFromXML( const std::string& filename, TwoDScene& twodscene, SceneStepper** scenestepper, scalar& dt, scalar& max_t, scalar& maxfreq, std::vector<renderingutils::Color>& particle_colors, std::vector<renderingutils::Color>& edge_colors, std::vector<renderingutils::ParticlePath>& particle_paths, renderingutils::Color& bgcolor )
{
    assert( *scenestepper == NULL );
    //std::cout << "Loading scene: " << filename << std::endl;
    
    // Load the xml document
    std::vector<char> xmlchars;
    rapidxml::xml_document<> doc;
    loadXMLFile( filename, xmlchars, doc );
    
    // Attempt to locate the root node
    rapidxml::xml_node<>* node = doc.first_node("scene");
    if( node == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse xml scene file. Failed to locate root <scene> node. Exiting." << std::endl;
        exit(1);
    }    
    
    // TODO: Clear old state
    
    // Scene
    loadParticles( node, twodscene );
    loadEdges( node, twodscene );
    // Forces
    //loadSpringForces( node, twodscene );
    loadSimpleGravityForces( node, twodscene );
    //loadGravitationalForces( node, twodscene );
    //loadDragDampingForces( node, twodscene );
    // Integrator/solver
    loadIntegrator( node, scenestepper, dt );
    loadMaxTime( node, max_t );
    // UI
    loadMaxSimFrequency( node, maxfreq );  
    // Rendering state
    particle_colors.resize(twodscene.getNumParticles(),renderingutils::Color(0.650980392156863,0.294117647058824,0.0));
    loadParticleColors( node, particle_colors );
    edge_colors.resize(twodscene.getNumEdges(),renderingutils::Color(0.0,0.388235294117647,0.388235294117647));
    loadEdgeColors( node, edge_colors );
    loadBackgroundColor( node, bgcolor );
    loadParticlePaths( node, dt, particle_paths );
}

void TwoDSceneXMLParser::loadXMLFile( const std::string& filename, std::vector<char>& xmlchars, rapidxml::xml_document<>& doc )
{
    // Attempt to read the text from the user-specified xml file
    std::string filecontents;
    if( !loadTextFileIntoString(filename,filecontents) )
    {
        std::cerr << "\033[31;1mERROR IN TWODSCENEXMLPARSER:\033[m XML scene file " << filename << ". Failed to read file." << std::endl;
        exit(1);
    }
    
    // Copy string into an array of characters for the xml parser
    for( int i = 0; i < (int) filecontents.size(); ++i ) xmlchars.push_back(filecontents[i]);
    xmlchars.push_back('\0');
    
    // Initialize the xml parser with the character vector
    doc.parse<0>(&xmlchars[0]);
}

bool TwoDSceneXMLParser::loadTextFileIntoString( const std::string& filename, std::string& filecontents )
{
    // Attempt to open the text file for reading
    std::ifstream textfile(filename.c_str(),std::ifstream::in);
    if(!textfile) return false;
    
    // Read the entire file into a single string
    std::string line;
    while(getline(textfile,line)) filecontents.append(line);
    
    textfile.close();
    
    return true;
}

void TwoDSceneXMLParser::loadParticles( rapidxml::xml_node<>* node, TwoDScene& twodscene )
{
    assert( node != NULL );
    
    // Count the number of particles
    int numparticles = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("particle"); nd; nd = nd->next_sibling("particle") ) ++numparticles;
    
    twodscene.resizeSystem(numparticles);
    
    //std::cout << "Num particles " << numparticles << std::endl;
    
    int particle = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("particle"); nd; nd = nd->next_sibling("particle") )
    {
        // Extract the particle's initial position
        Vector2s pos;
        if( nd->first_attribute("px") ) 
        {
            std::string attribute(nd->first_attribute("px")->value());
            if( !stringutils::extractFromString(attribute,pos.x()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of px attribute for particle " << particle << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse px attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( nd->first_attribute("py") ) 
        {
            std::string attribute(nd->first_attribute("py")->value());
            if( !stringutils::extractFromString(attribute,pos.y()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of py attribute for particle " << particle << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse py attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        twodscene.setPosition( particle, pos );
        
        // Extract the particle's initial velocity
        Vector2s vel;
        if( nd->first_attribute("vx") ) 
        {
            std::string attribute(nd->first_attribute("vx")->value());
            if( !stringutils::extractFromString(attribute,vel.x()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of vx attribute for particle " << particle << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse vx attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( nd->first_attribute("vy") ) 
        {
            std::string attribute(nd->first_attribute("vy")->value());
            if( !stringutils::extractFromString(attribute,vel.y()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of vy attribute for particle " << particle << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse vy attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        twodscene.setVelocity( particle, vel );
        
        // Extract the particle's mass
        scalar mass = std::numeric_limits<scalar>::signaling_NaN();
        if( nd->first_attribute("m") ) 
        {
            std::string attribute(nd->first_attribute("m")->value());
            if( !stringutils::extractFromString(attribute,mass) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of m attribute for particle " << particle << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse m attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        twodscene.setMass( particle, mass );
        
        // Determine if the particle is fixed
        bool fixed;
        if( nd->first_attribute("fixed") ) 
        {
            std::string attribute(nd->first_attribute("fixed")->value());
            if( !stringutils::extractFromString(attribute,fixed) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of fixed attribute for particle " << particle << ". Value must be boolean. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse fixed attribute for particle " << particle << ". Exiting." << std::endl;
            exit(1);
        }
        twodscene.setFixed( particle, fixed );
        
        // Extract the particle's radius, if present
        scalar radius = 0.1;
        if( nd->first_attribute("radius") )
        {
            std::string attribute(nd->first_attribute("radius")->value());
            if( !stringutils::extractFromString(attribute,radius) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse radius attribute for particle " << particle << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }
        }    
        twodscene.setRadius( particle, radius );
        
        //std::cout << "Particle: " << particle << "    x: " << pos.transpose() << "   v: " << vel.transpose() << "   m: " << mass << "   fixed: " << fixed << std::endl;
        
        ++particle;
    }
}

void TwoDSceneXMLParser::loadEdges( rapidxml::xml_node<>* node, TwoDScene& twodscene )
{
    assert( node != NULL );
    
    twodscene.clearEdges();
    
    int edge = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("edge"); nd; nd = nd->next_sibling("edge") )
    {
        std::pair<int,int> newedge;
        if( nd->first_attribute("i") )
        {
            std::string attribute(nd->first_attribute("i")->value());
            if( !stringutils::extractFromString(attribute,newedge.first) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for edge " << edge << ". Value must be integer. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for edge " << edge << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( nd->first_attribute("j") )
        {
            std::string attribute(nd->first_attribute("j")->value());
            if( !stringutils::extractFromString(attribute,newedge.second) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of j attribute for edge " << edge << ". Value must be integer. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of j attribute for edge " << edge << ". Exiting." << std::endl;
            exit(1);
        }
        
        scalar radius = 0.015;
        if( nd->first_attribute("radius") )
        {
            std::string attribute(nd->first_attribute("radius")->value());
            if( !stringutils::extractFromString(attribute,radius) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse radius attribute for edge " << edge << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }
        }
        
        //std::cout << "Edge: " << edge << "    i: " << newedge.first << "   j: " << newedge.second << std::endl;
        
        twodscene.insertEdge( newedge, radius );
        
        ++edge;
    }
}

//void TwoDSceneXMLParser::loadSpringForces( rapidxml::xml_node<>* node, TwoDScene& twodscene )
//{
//  assert( node != NULL );
//  
//  // Extract the edge the force acts across
//  int forcenum = 0;
//  for( rapidxml::xml_node<>* nd = node->first_node("springforce"); nd; nd = nd->next_sibling("springforce") )
//  {
//    int edge = -1;
//
//    if( nd->first_attribute("edge") )
//    {
//      std::string attribute(nd->first_attribute("edge")->value());
//      if( !stringutils::extractFromString(attribute,edge) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of edge attribute for springforce " << forcenum << ". Value must be integer. Exiting." << std::endl;
//        exit(1);
//      }
//    }
//    else
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of edge attribute for springforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }      
//  
//    std::pair<int,int> newedge(twodscene.getEdge(edge));
//    
//    // Extract the spring stiffness
//    scalar k = std::numeric_limits<scalar>::signaling_NaN();
//    if( nd->first_attribute("k") ) 
//    {
//      std::string attribute(nd->first_attribute("k")->value());
//      if( !stringutils::extractFromString(attribute,k) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of k attribute for springforce " << forcenum << ". Value must be numeric. Exiting." << std::endl;
//        exit(1);
//      }
//    }
//    else 
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse k attribute for springforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//    
//    // Extract the spring rest length
//    scalar l0 = std::numeric_limits<scalar>::signaling_NaN();
//    if( nd->first_attribute("l0") ) 
//    {
//      std::string attribute(nd->first_attribute("l0")->value());
//      if( !stringutils::extractFromString(attribute,l0) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of l0 attribute for springforce " << forcenum << ". Value must be numeric. Exiting." << std::endl;
//        exit(1);
//      }
//    }
//    else 
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse l0 attribute for springforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//    
//    //std::cout << "Springforce: " << forcenum << "    i: " << newedge.first << "   j: " << newedge.second << "   k: " << k << "   l0: " << l0 << std::endl;
//    
//    twodscene.insertForce(new SpringForce(newedge,k,l0));
//    
//    ++forcenum;
//  }
//
//  //SpringForce( const std::pair<int,int>& endpoints, const scalar& k, const scalar& l0 )
//}

//void TwoDSceneXMLParser::loadGravitationalForces( rapidxml::xml_node<>* node, TwoDScene& twodscene )
//{
//  assert( node != NULL );
//  
//  // Extract the edge the force acts across
//  int forcenum = 0;
//  for( rapidxml::xml_node<>* nd = node->first_node("gravitationalforce"); nd; nd = nd->next_sibling("gravitationalforce") )
//  {
//    std::pair<int,int> newedge;
//    if( nd->first_attribute("i") )
//    {
//      std::string attribute(nd->first_attribute("i")->value());
//      if( !stringutils::extractFromString(attribute,newedge.first) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for gravitationalforce " << forcenum << ". Value must be integer. Exiting." << std::endl;
//        exit(1);
//      }        
//    }
//    else
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for gravitationalforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//    
//    if( nd->first_attribute("j") )
//    {
//      std::string attribute(nd->first_attribute("j")->value());
//      if( !stringutils::extractFromString(attribute,newedge.second) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of j attribute for gravitationalforce " << forcenum << ". Value must be integer. Exiting." << std::endl;
//        exit(1);
//      }        
//    }
//    else
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of j attribute for gravitationalforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//    
//    // Extract the gravitational constant
//    scalar G = std::numeric_limits<scalar>::signaling_NaN();
//    if( nd->first_attribute("G") )
//    {
//      std::string attribute(nd->first_attribute("G")->value());
//      if( !stringutils::extractFromString(attribute,G) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of G attribute for gravitationalforce " << forcenum << ". Value must be numeric. Exiting." << std::endl;
//        exit(1);
//      }
//    }
//    else 
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse G attribute for gravitationalforce " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//
//    //std::cout << "GravitationalForce: " << forcenum << "    i: " << newedge.first << "   j: " << newedge.second << "   G: " << G << std::endl;
//
//    twodscene.insertForce(new GravitationalForce(newedge,G));
//    
//    ++forcenum;
//  }
//  
//  //SpringForce( const std::pair<int,int>& endpoints, const scalar& k, const scalar& l0 )  
//}

//void TwoDSceneXMLParser::loadDragDampingForces( rapidxml::xml_node<>* node, TwoDScene& twodscene )
//{
//  assert( node != NULL );
//  
//  int forcenum = 0;
//  for( rapidxml::xml_node<>* nd = node->first_node("dragdamping"); nd; nd = nd->next_sibling("dragdamping") )
//  {
//    Vector2s constforce;
//    constforce.setConstant(std::numeric_limits<scalar>::signaling_NaN());
//    
//    // Extract the linear damping coefficient
//    scalar b = std::numeric_limits<scalar>::signaling_NaN();
//    if( nd->first_attribute("b") )
//    {
//      std::string attribute(nd->first_attribute("b")->value());
//      if( !stringutils::extractFromString(attribute,b) )
//      {
//        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for dragdamping " << forcenum << ". Value must be numeric. Exiting." << std::endl;
//        exit(1);
//      }
//    }
//    else 
//    {
//      std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse b attribute for dragdamping " << forcenum << ". Exiting." << std::endl;
//      exit(1);
//    }
//    
//    //std::cout << "x: " << constforce.transpose() << std::endl;
//    
//    twodscene.insertForce(new DragDampingForce(b));
//    
//    ++forcenum;
//  }  
//}

void TwoDSceneXMLParser::loadSimpleGravityForces( rapidxml::xml_node<>* node, TwoDScene& twodscene )
{
    assert( node != NULL );
    
    // Load each constant force
    int forcenum = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("simplegravity"); nd; nd = nd->next_sibling("simplegravity") )
    {
        Vector2s constforce;
        constforce.setConstant(std::numeric_limits<scalar>::signaling_NaN());
        
        // Extract the x component of the force
        if( nd->first_attribute("fx") ) 
        {
            std::string attribute(nd->first_attribute("fx")->value());
            if( !stringutils::extractFromString(attribute,constforce.x()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of fx attribute for constantforce " << forcenum << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse fx attribute for constantforce " << forcenum << ". Exiting." << std::endl;
            exit(1);
        }
        
        // Extract the y component of the force
        if( nd->first_attribute("fy") ) 
        {
            std::string attribute(nd->first_attribute("fy")->value());
            if( !stringutils::extractFromString(attribute,constforce.y()) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of fy attribute for constantforce " << forcenum << ". Value must be numeric. Exiting." << std::endl;
                exit(1);
            }
        }
        else 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse fy attribute for constantforce " << forcenum << ". Exiting." << std::endl;
            exit(1);
        }
        
        //std::cout << "x: " << constforce.transpose() << std::endl;
        
        twodscene.insertForce(new SimpleGravityForce(constforce));
        
        ++forcenum;
    }  
}


void TwoDSceneXMLParser::loadIntegrator( rapidxml::xml_node<>* node, SceneStepper** scenestepper, scalar& dt )
{
    assert( node != NULL );
    
    dt = -1.0;
    
    // Attempt to locate the integrator node
    rapidxml::xml_node<>* nd = node->first_node("integrator");
    if( nd == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No integrator specified. Exiting." << std::endl;
        exit(1);
    }
    
    // Attempt to load the integrator type
    rapidxml::xml_attribute<>* typend = nd->first_attribute("type"); 
    if( typend == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No integrator 'type' attribute specified. Exiting." << std::endl;
        exit(1);
    }
    std::string integratortype(typend->value());
    
    if( integratortype == "explicit-euler" ) *scenestepper = new ExplicitEuler;
    //else if( integratortype == "semi-implicit-euler" ) *scenestepper = new SemiImplicitEuler;
    //else if( integratortype == "implicit-euler" ) *scenestepper = new ImplicitEuler;
    //else if( integratortype == "linearized-implicit-euler" ) *scenestepper = new LinearizedImplicitEuler;
    else
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Invalid integrator 'type' attribute specified. Exiting." << std::endl;
        exit(1);
    }
    
    // Attempt to load the timestep
    rapidxml::xml_attribute<>* dtnd = nd->first_attribute("dt"); 
    if( dtnd == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No integrator 'dt' attribute specified. Exiting." << std::endl;
        exit(1);
    }
    
    dt = std::numeric_limits<scalar>::signaling_NaN();
    if( !stringutils::extractFromString(std::string(dtnd->value()),dt) )
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse 'dt' attribute for integrator. Value must be numeric. Exiting." << std::endl;
        exit(1);
    }
    
    //std::cout << "Integrator: " << (*scenestepper)->getName() << "   dt: " << dt << std::endl;
}

void TwoDSceneXMLParser::loadMaxTime( rapidxml::xml_node<>* node, scalar& max_t )
{
    assert( node != NULL );
    
    // Attempt to locate the duraiton node
    rapidxml::xml_node<>* nd = node->first_node("duration");
    if( nd == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No duration specified. Exiting." << std::endl;
        exit(1);
    }
    
    // Attempt to load the duration value
    rapidxml::xml_attribute<>* timend = nd->first_attribute("time"); 
    if( timend == NULL ) 
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No duration 'time' attribute specified. Exiting." << std::endl;
        exit(1);
    }
    
    max_t = std::numeric_limits<scalar>::signaling_NaN();
    if( !stringutils::extractFromString(std::string(timend->value()),max_t) )
    {
        std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse 'time' attribute for duration. Value must be numeric. Exiting." << std::endl;
        exit(1);
    }
}

void TwoDSceneXMLParser::loadMaxSimFrequency( rapidxml::xml_node<>* node, scalar& max_freq )
{
    assert( node != NULL );
    
    // Attempt to locate the duraiton node
    if( node->first_node("maxsimfreq") ) 
    {
        // Attempt to load the duration value
        rapidxml::xml_attribute<>* atrbnde = node->first_node("maxsimfreq")->first_attribute("max"); 
        if( atrbnde == NULL ) 
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m No maxsimfreq 'max' attribute specified. Exiting." << std::endl;
            exit(1);
        }
        
        if( !stringutils::extractFromString(std::string(atrbnde->value()),max_freq) )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse 'max' attribute for maxsimfreq. Value must be scalar. Exiting." << std::endl;
            exit(1);
        }
    }
}




void TwoDSceneXMLParser::loadBackgroundColor( rapidxml::xml_node<>* node, renderingutils::Color& color )
{
    if( rapidxml::xml_node<>* nd = node->first_node("backgroundcolor") )
    {
        // Read in the red color channel 
        double red = -1.0;
        if( nd->first_attribute("r") )
        {
            std::string attribute(nd->first_attribute("r")->value());
            if( !stringutils::extractFromString(attribute,red) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for backgroundcolor. Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for backgroundcolor. Exiting." << std::endl;
            exit(1);
        }
        
        if( red < 0.0 || red > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for backgroundcolor. Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the green color channel 
        double green = -1.0;
        if( nd->first_attribute("g") )
        {
            std::string attribute(nd->first_attribute("g")->value());
            if( !stringutils::extractFromString(attribute,green) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for backgroundcolor. Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for backgroundcolor. Exiting." << std::endl;
            exit(1);
        }
        
        if( green < 0.0 || green > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for backgroundcolor. Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the blue color channel 
        double blue = -1.0;
        if( nd->first_attribute("b") )
        {
            std::string attribute(nd->first_attribute("b")->value());
            if( !stringutils::extractFromString(attribute,blue) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for backgroundcolor. Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for backgroundcolor. Exiting." << std::endl;
            exit(1);
        }
        
        if( blue < 0.0 || blue > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for backgroundcolor. Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        //std::cout << red << "   " << green << "   " << blue << std::endl;
        
        color.r = red;
        color.g = green;
        color.b = blue;  
    }
}

void TwoDSceneXMLParser::loadParticleColors( rapidxml::xml_node<>* node, std::vector<renderingutils::Color>& particle_colors )
{
    int particlecolornum = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("particlecolor"); nd; nd = nd->next_sibling("particlecolor") )
    {
        // Determine which particle this color corresponds to
        int particle = -1;
        if( nd->first_attribute("i") )
        {
            std::string attribute(nd->first_attribute("i")->value());
            if( !stringutils::extractFromString(attribute,particle) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for particlecolor " << particlecolornum << ". Value must be integer. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for particlecolor " << particlecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( particle < 0 || particle > (int) particle_colors.size() )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for particlecolor " << particlecolornum << ". Invalid particle specified. Valid range is " << 0 << "..." << particle_colors.size()-1 << std::endl;
            exit(1);
        }
        
        
        // Read in the red color channel 
        double red = -1.0;
        if( nd->first_attribute("r") )
        {
            std::string attribute(nd->first_attribute("r")->value());
            if( !stringutils::extractFromString(attribute,red) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlecolor " << particlecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlecolor " << particlecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( red < 0.0 || red > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlecolor " << particlecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the green color channel 
        double green = -1.0;
        if( nd->first_attribute("g") )
        {
            std::string attribute(nd->first_attribute("g")->value());
            if( !stringutils::extractFromString(attribute,green) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlecolor " << particlecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlecolor " << particlecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( green < 0.0 || green > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlecolor " << particlecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the blue color channel 
        double blue = -1.0;
        if( nd->first_attribute("b") )
        {
            std::string attribute(nd->first_attribute("b")->value());
            if( !stringutils::extractFromString(attribute,blue) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlecolor " << particlecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlecolor " << particlecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( blue < 0.0 || blue > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlecolor " << particlecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        particle_colors[particle] = renderingutils::Color(red,green,blue);
        
        ++particlecolornum;
    }
}

void TwoDSceneXMLParser::loadEdgeColors( rapidxml::xml_node<>* node, std::vector<renderingutils::Color>& edge_colors )
{
    int edgecolornum = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("edgecolor"); nd; nd = nd->next_sibling("edgecolor") )
    {
        // Determine which particle this color corresponds to
        int edge = -1;
        if( nd->first_attribute("i") )
        {
            std::string attribute(nd->first_attribute("i")->value());
            if( !stringutils::extractFromString(attribute,edge) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for edgecolor " << edgecolornum << ". Value must be integer. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for edgecolor " << edgecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( edge < 0 || edge > (int) edge_colors.size() )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for edgecolor " << edgecolornum << ". Invalid edge specified. Valid range is " << 0 << "..." << edge_colors.size()-1 << std::endl;
            exit(1);
        }
        
        
        // Read in the red color channel 
        double red = -1.0;
        if( nd->first_attribute("r") )
        {
            std::string attribute(nd->first_attribute("r")->value());
            if( !stringutils::extractFromString(attribute,red) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for edgecolor " << edgecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for edgecolor " << edgecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( red < 0.0 || red > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for edgecolor " << edgecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the green color channel 
        double green = -1.0;
        if( nd->first_attribute("g") )
        {
            std::string attribute(nd->first_attribute("g")->value());
            if( !stringutils::extractFromString(attribute,green) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for edgecolor " << edgecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for edgecolor " << edgecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( green < 0.0 || green > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for edgecolor " << edgecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the blue color channel 
        double blue = -1.0;
        if( nd->first_attribute("b") )
        {
            std::string attribute(nd->first_attribute("b")->value());
            if( !stringutils::extractFromString(attribute,blue) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for edgecolor " << edgecolornum << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for edgecolor " << edgecolornum << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( blue < 0.0 || blue > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for edgecolor " << edgecolornum << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        //std::cout << "edge: " << edge << " r: " << red << " g: " << green << " b: " << blue << std::endl;
        
        edge_colors[edge] = renderingutils::Color(red,green,blue);
        
        ++edgecolornum;
    }  
}

void TwoDSceneXMLParser::loadParticlePaths( rapidxml::xml_node<>* node, double dt, std::vector<renderingutils::ParticlePath>& particle_paths )
{
    int numpaths = 0;
    for( rapidxml::xml_node<>* nd = node->first_node("particlepath"); nd; nd = nd->next_sibling("particlepath") )
    {
        // Determine which particle this color corresponds to
        int particle = -1;
        if( nd->first_attribute("i") )
        {
            std::string attribute(nd->first_attribute("i")->value());
            if( !stringutils::extractFromString(attribute,particle) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for particlepath " << numpaths << ". Value must be integer. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of i attribute for particlepath " << numpaths << ". Exiting." << std::endl;
            exit(1);
        }
        
        // How long the path should be buffered for
        double duration = -1.0;
        if( nd->first_attribute("duration") )
        {
            std::string attribute(nd->first_attribute("duration")->value());
            if( !stringutils::extractFromString(attribute,duration) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of duration attribute for particlepath " << numpaths << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of duration attribute for particlepath " << numpaths << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( duration < 0.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of duration attribute for particlepath " << numpaths << ". Value must be positive scalar." << std::endl;
            exit(1);
        }
        
        
        // Read in the red color channel 
        double red = -1.0;
        if( nd->first_attribute("r") )
        {
            std::string attribute(nd->first_attribute("r")->value());
            if( !stringutils::extractFromString(attribute,red) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlepath " << numpaths << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlepath " << numpaths << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( red < 0.0 || red > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of r attribute for particlepath " << numpaths << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the green color channel 
        double green = -1.0;
        if( nd->first_attribute("g") )
        {
            std::string attribute(nd->first_attribute("g")->value());
            if( !stringutils::extractFromString(attribute,green) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlepath " << numpaths << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlepath " << numpaths << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( green < 0.0 || green > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of g attribute for particlepath " << numpaths << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        
        // Read in the blue color channel 
        double blue = -1.0;
        if( nd->first_attribute("b") )
        {
            std::string attribute(nd->first_attribute("b")->value());
            if( !stringutils::extractFromString(attribute,blue) )
            {
                std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlepath " << numpaths << ". Value must be scalar. Exiting." << std::endl;
                exit(1);
            }        
        }
        else
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlepath " << numpaths << ". Exiting." << std::endl;
            exit(1);
        }
        
        if( blue < 0.0 || blue > 1.0 )
        {
            std::cerr << "\033[31;1mERROR IN XMLSCENEPARSER:\033[m Failed to parse value of b attribute for particlepath " << numpaths << ". Invalid color specified. Valid range is " << 0.0 << "..." << 1.0 << std::endl;
            exit(1);
        }
        
        particle_paths.push_back(renderingutils::ParticlePath( particle, ceil(duration/dt), renderingutils::Color(red,green,blue) ));
        
        //std::cout << particle << " " << duration << " " << red << " " << green << " " << blue << std::endl;
        
        ++numpaths;
    }  
}





