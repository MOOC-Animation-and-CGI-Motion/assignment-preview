#include "TT.h"
DEFINE_TESTS;

#include <Eigen/StdVector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <tclap/CmdLine.h>

#include "TwoDScene.h"
#include "Force.h"
#include "ExplicitEuler.h"
#include "TwoDimensionalDisplayController.h"
#include "TwoDSceneRenderer.h"
#include "TwoDSceneXMLParser.h"
#include "TwoDSceneSerializer.h"
#include "StringUtilities.h"
#include "MathDefs.h"
#include "TimingUtilities.h"
#include "RenderingUtilities.h"
#include "YImage.h"
#include "Clogs.h" // logging utilities

#include "Macros.h" // for IGNORE_UNUSED

///////////////////////////////////////////////////////////////////////////////
// Rendering State
TwoDimensionalDisplayController g_display_controller(512,512);
TwoDSceneRenderer* g_scene_renderer = NULL;
renderingutils::Color g_bgcolor(1.0,1.0,1.0);
bool g_rendering_enabled = true;

// if true, we enter testmain instead of anything else.
bool g_testmode = false;
bool g_suppress_logs = false;
bool g_use_pngs = false;

double g_sec_per_frame;
double g_last_time = timingutils::seconds();

///////////////////////////////////////////////////////////////////////////////
// Parser state
std::string g_xml_scene_file;
TwoDSceneXMLParser g_xml_scene_parser;

///////////////////////////////////////////////////////////////////////////////
// Scene input/output/comparison state
TwoDSceneSerializer g_scene_serializer;

bool g_save_to_binary = false;
std::string g_binary_file_name;
std::ofstream g_binary_output;

///////////////////////////////////////////////////////////////////////////////
// Simulation state
bool g_paused = true;
scalar g_dt = 0.0;
int g_num_steps = 0;
int g_current_step = 0;
TwoDScene g_scene;
SceneStepper* g_scene_stepper = NULL;



///////////////////////////////////////////////////////////////////////////////
// Simulation functions

void miscOutputCallback();
void dumpPNG(const std::string &filename);

void stepSystem()
{
    assert( g_scene_stepper != NULL );
    
    // Determine if the simulation is complete
    if( g_current_step >= g_num_steps )
    {
        std::cout << std::endl;
        std::cout << outputmod::startpink << "FOSSSim message: " << outputmod::endpink << "Simulation complete at time " << g_current_step*g_dt << ". Exiting." << std::endl;
        exit(0);
    }
    
    // Step the simulated scene forward
    g_scene_stepper->stepScene( g_scene, g_dt );
    
    // Check for obvious problems in the simulated scene
#ifdef DEBUG
    g_scene.checkConsistency();
#endif
    
    //g_t += g_dt;
    g_current_step++;
    
    // If saving the simulation output, do it!
    if( g_save_to_binary ) g_scene_serializer.serializeScene( g_scene, g_binary_output );
    
  // If the user wants to generate a PNG movie
  if(g_use_pngs) {
    std::stringstream oss;
    oss << "pngs/frame" << std::setw(5) << std::setfill('0') << g_current_step << ".png";
    dumpPNG(oss.str());
  }

  miscOutputCallback();
}

void headlessSimLoop()
{
    scalar nextpercent = 0.02;
    std::cout << outputmod::startpink << "Progress: " << outputmod::endpink;
    for( int i = 0; i < 50; ++i ) std::cout << "-";
    //std::cout << "]" << std::endl;
    std::cout << std::endl;
    std::cout << "          ";
    while( true )
    {
        scalar percent_done = ((double)g_current_step)/((double)g_num_steps);
        if( percent_done >= nextpercent )
        {
            nextpercent += 0.02;
            std::cout << "." << std::flush;
        }
        stepSystem();
    }
}




///////////////////////////////////////////////////////////////////////////////
// Rendering and UI functions

void dumpPNG(const std::string &filename)
{

  YImage image;
  image.resize(g_display_controller.getWindowWidth(), g_display_controller.getWindowHeight());
  
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glReadBuffer(GL_BACK);
  
  glFinish();
  glReadPixels(0, 0, g_display_controller.getWindowWidth(), g_display_controller.getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
  image.flip();
  
  image.save(filename.c_str());

}

void reshape( int w, int h ) 
{
    g_display_controller.reshape(w,h);
    
    assert( renderingutils::checkGLErrors() );
}

// TODO: Move these functions to scene renderer?
void setOrthographicProjection() 
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	gluOrtho2D(0, g_display_controller.getWindowWidth(), 0, g_display_controller.getWindowHeight());
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    assert( renderingutils::checkGLErrors() );
}

void renderBitmapString( float x, float y, float z, void *font, std::string s ) 
{
	glRasterPos3f(x, y, z);
	for( std::string::iterator i = s.begin(); i != s.end(); ++i )
	{
		char c = *i;
		glutBitmapCharacter(font, c);
	}
    
    assert( renderingutils::checkGLErrors() );
}

void drawHUD()
{
    setOrthographicProjection();
    glColor3f(1.0-g_bgcolor.r,1.0-g_bgcolor.g,1.0-g_bgcolor.b);
    renderBitmapString( 4, g_display_controller.getWindowHeight()-20, 0.0, GLUT_BITMAP_HELVETICA_18, stringutils::convertToString(g_current_step*g_dt) ); 
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    
    assert( renderingutils::checkGLErrors() );
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
	glMatrixMode(GL_MODELVIEW);
    
    g_scene_renderer->renderScene();
    
    drawHUD();
    
    glutSwapBuffers();
    
    assert( renderingutils::checkGLErrors() );
}

void centerCamera()
{
    const VectorXs& x = g_scene.getX();
    assert( (int) x.size() == 2*g_scene.getNumParticles() );
    
    // Compute the bounds on all particle positions
    scalar max_x = -std::numeric_limits<scalar>::infinity();
    scalar min_x =  std::numeric_limits<scalar>::infinity();
    scalar max_y = -std::numeric_limits<scalar>::infinity();
    scalar min_y =  std::numeric_limits<scalar>::infinity();
    for( int i = 0; i < g_scene.getNumParticles(); ++i )
    {
        if( x(2*i) > max_x ) max_x = x(2*i);
        if( x(2*i) < min_x ) min_x = x(2*i);
        if( x(2*i+1) > max_y ) max_y = x(2*i+1);
        if( x(2*i+1) < min_y ) min_y = x(2*i+1);
    }
    
    // Set center of view to center of bounding box
    g_display_controller.setCenterX(0.5*(max_x+min_x));
    g_display_controller.setCenterY(0.5*(max_y+min_y));
    
    // Set the zoom such that all particles are in view
    scalar radius_x = 0.5*(max_x-min_x);
    if( radius_x == 0.0 ) radius_x = 1.0;
    scalar radius_y = 0.5*(max_y-min_y);
    if( radius_y == 0.0 ) radius_y = 1.0;
    scalar ratio = ((scalar)g_display_controller.getWindowHeight())/((scalar)g_display_controller.getWindowWidth());
    
    g_display_controller.setScaleFactor( 1.2*std::max(ratio*radius_x,radius_y) );
    
    // OpenGL must be initialized before calling this function
    //assert( checkGLErrors() );
}

void keyboard( unsigned char key, int x, int y )
{
    g_display_controller.keyboard(key,x,y);
    
    if( key == 27 || key == 'q' )
    {
        exit(0);
    }
    else if( key == 's' || key =='S' )
    {
        stepSystem();
        g_scene_renderer->updateState();
        glutPostRedisplay();
    }
    else if( key == ' ' )
    {
        g_paused = !g_paused;
    }
    else if( key == 'c' || key == 'C' )
    {
        centerCamera();    
        g_display_controller.reshape(g_display_controller.getWindowWidth(),g_display_controller.getWindowHeight());
        glutPostRedisplay();
    }
    
    assert( renderingutils::checkGLErrors() );
}

// Proccess 'special' keys
void special( int key, int x, int y )
{
    g_display_controller.special(key,x,y);
    
    assert( renderingutils::checkGLErrors() );
}

void mouse( int button, int state, int x, int y )
{
    g_display_controller.mouse(button,state,x,y);
    
    assert( renderingutils::checkGLErrors() );
}

void motion( int x, int y ) 
{
    g_display_controller.motion(x,y);
    
    assert( renderingutils::checkGLErrors() );
}

void idle()
{
    //std::cout << "g_last_time: " << g_last_time << std::endl;
    // Trigger the next timestep
    double current_time = timingutils::seconds();
    //std::cout << "current_time: " << current_time << std::endl;
    //std::cout << "g_sec_per_frame: " << g_sec_per_frame << std::endl;
    if( !g_paused && current_time-g_last_time >= g_sec_per_frame ) 
    {
        g_last_time = current_time;
        stepSystem();
        g_scene_renderer->updateState();
        glutPostRedisplay();
    }
    
    assert( renderingutils::checkGLErrors() );
}

void initializeOpenGLandGLUT( int argc, char** argv )
{
    // Center the camera on the scene
    centerCamera();
    
    // Initialize GLUT
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(g_display_controller.getWindowWidth(),g_display_controller.getWindowHeight());
    glutCreateWindow("Forty One Sixty Seven Sim");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    
    // Initialize OpenGL
	reshape(g_display_controller.getWindowWidth(),g_display_controller.getWindowHeight());
    glClearColor(g_bgcolor.r, g_bgcolor.g, g_bgcolor.b, 0.0);
    
    assert( renderingutils::checkGLErrors() );
}


///////////////////////////////////////////////////////////////////////////////
// Parser functions

void loadScene( const std::string& file_name )
{
    assert( g_scene_stepper == NULL );
    
    // TODO: Just pass scene renderer to xml scene parser
    scalar max_time;
    std::vector<renderingutils::Color> particle_colors;
    std::vector<renderingutils::Color> edge_colors;
    std::vector<renderingutils::ParticlePath> particle_paths;
    scalar steps_per_sec_cap = 100;
    g_xml_scene_parser.loadSceneFromXML( file_name, g_scene, &g_scene_stepper, g_dt, max_time, steps_per_sec_cap, particle_colors, edge_colors, particle_paths, g_bgcolor );
    
    g_sec_per_frame = 1.0/steps_per_sec_cap;
    
    if( g_rendering_enabled )
    {
        g_scene_renderer = new TwoDSceneRenderer(g_scene,particle_colors,edge_colors,particle_paths);
        g_scene_renderer->updateState();
    }
    
    g_num_steps = ceil(max_time/g_dt);
    g_current_step = 0;
    
    assert( g_scene_stepper != NULL );
    assert( g_dt > 0.0 );
}

bool isTestMode(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-t" || string(argv[i]) == "--testmode") {
            return true;
        }
    }
    return false;
}

void parseCommandLine( int argc, char** argv )
{
    try 
    {
        if (isTestMode(argc, argv)) {
          g_testmode = true;
          return;
        }

        TCLAP::CmdLine cmd("Forty One Sixty Seven Sim");
        
        // Will be caught above, but just so that the documentation is there...
        TCLAP::ValueArg<bool> testmode("t", "testmode", "If present, runs unit tests instead of executing FOSSSim", false, g_testmode, "", cmd);

        // XML scene file to load
        TCLAP::ValueArg<std::string> scene("s", "scene", "Simulation to run; an xml scene file", true, "", "string", cmd);
        
        // Begin the scene paused or running
        TCLAP::ValueArg<bool> paused("p", "paused", "Begin the simulation paused if 1, running if 0", false, true, "boolean", cmd);
        
        // Run the simulation with rendering enabled or disabled
        TCLAP::ValueArg<bool> display("d", "display", "Run the simulation with display enabled if 1, without if 0", false, true, "boolean", cmd);

        // Record PNGs
        TCLAP::ValueArg<bool> generate("g", "generate", "Record PNGs if 1, otherwise if 0", false, false, "boolean", cmd);
      
        // If true, suppresses log output to all clogs logs.
        TCLAP::ValueArg<bool> suppress_logs("l",
            "suppress-logs", 
            "If 1, prevent clogs from writing to any log files.", 
            false,            // required?
            g_suppress_logs,  // default value, if none is provided.
            "boolean", 
            cmd);

        // File to save output to
        TCLAP::ValueArg<std::string> output("o", "outputfile", "Binary file to save simulation state to", false, "", "string", cmd);
        
        cmd.parse(argc, argv);
        
        assert( scene.isSet() );
        g_xml_scene_file = scene.getValue();
        g_paused = paused.getValue();
        g_rendering_enabled = display.getValue();
        g_suppress_logs = suppress_logs.getValue();
        g_use_pngs = generate.getValue();
      
        if( output.isSet() )
        {
            g_save_to_binary = true;
            g_binary_file_name = output.getValue();
        }
    } 
    catch (TCLAP::ArgException& e) 
    {
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }
}



///////////////////////////////////////////////////////////////////////////////
// Various support functions

void miscOutputFinalization();

void cleanupAtExit()
{
    if( g_scene_renderer != NULL )
    {
        delete g_scene_renderer;
        g_scene_renderer = NULL;
    }
    
    if( g_scene_stepper != NULL )
    {
        delete g_scene_stepper;
        g_scene_stepper = NULL;
    }
    
    if( g_binary_output.is_open() )
    {
        std::cout << outputmod::startpink << "FOSSSim message: " << outputmod::endpink << "Saved simulation to file." << std::endl;
        g_binary_output.close();
    }
    
    miscOutputFinalization();
}

std::ostream& fosssim_header( std::ostream& stream )
{
    stream << outputmod::startgreen << 
    "------------------------------------------    " << std::endl <<
    "  _____ ___  ____ ____ ____  _                " << std::endl <<
    " |  ___/ _ \\/ ___/ ___/ ___|(_)_ __ ___      " << std::endl <<
    " | |_ | | | \\___ \\___ \\___ \\| | '_ ` _ \\ " << std::endl <<
    " |  _|| |_| |___) |__) |__) | | | | | | |     " << std::endl << 
    " |_|   \\___/|____/____/____/|_|_| |_| |_|    " << std::endl <<
    "------------------------------------------    " 
    << outputmod::endgreen << std::endl;
    
    return stream;
}

std::ofstream g_debugoutput;

void miscOutputInitialization()
{
    //g_debugoutput.open("debugoutput.txt");
    //g_debugoutput << "# Time   PotentialEnergy   KineticEnergy   TotalEnergy" << std::endl;
    //g_debugoutput << g_current_step*g_dt << "\t" << g_scene.computePotentialEnergy() << "\t" << g_scene.computeKineticEnergy() << "\t" << g_scene.computeTotalEnergy() << std::endl;
}

void miscOutputCallback()
{
    //g_debugoutput << g_current_step*g_dt << "\t" << g_scene.computePotentialEnergy() << "\t" << g_scene.computeKineticEnergy() << "\t" << g_scene.computeTotalEnergy() << std::endl;
}

void miscOutputFinalization()
{
    //g_debugoutput.close();
}

// sets up clogs so that logging can occur.
CLOGS_OPEN;

int main( int argc, char** argv )
{
    // Parse command line arguments
    parseCommandLine( argc, argv );

    if (g_testmode) {
      std::cout << "Running tests instead of simulation..." << std::endl;
      return tt::Test::main();
    }
    
    // Function to cleanup at progarm exit
    atexit(cleanupAtExit);
    
    if (g_suppress_logs) {
      // ensures that clogs will do nothing.
      SUPPRESS_ALL_CLOGS();
    }

    // Load the user-specified scene
    loadScene(g_xml_scene_file);
    
    // If requested, open the binary output file
    if( g_save_to_binary )
    {
        g_binary_output.open(g_binary_file_name.c_str());
        if( g_binary_output.fail() ) 
        {
            std::cerr << outputmod::startred << "ERROR IN INITIALIZATION: "  << outputmod::endred << "Failed to open binary output file: " << " `" << g_binary_file_name << "`   Exiting." << std::endl;
            exit(1);
        }
        // Save the initial conditions
        g_scene_serializer.serializeScene( g_scene, g_binary_output );
    }
    
    // Initialization for OpenGL and GLUT
    if( g_rendering_enabled ) initializeOpenGLandGLUT(argc,argv);
    
    // Print a header
    std::cout << fosssim_header << std::endl;
    
    // Print some status info about this FOSSSim build
#ifdef FOSSSIM_VERSION
    std::cout << outputmod::startblue << "FOSSSim Version: "  << outputmod::endblue << FOSSSIM_VERSION << std::endl;
#endif
#ifdef CMAKE_BUILD_TYPE
    std::cout << outputmod::startblue << "Build type: " << outputmod::endblue << CMAKE_BUILD_TYPE << std::endl;
#endif
#ifdef EIGEN_VECTORIZE
    std::cout << outputmod::startblue << "Vectorization: " << outputmod::endblue << "Enabled" << std::endl;
#else
    std::cout << outputmod::startblue << "Vectorization: " << outputmod::endblue << "Disabled" << std::endl;
#endif
    
    std::cout << outputmod::startblue << "Scene: " << outputmod::endblue << g_xml_scene_file << std::endl;
    std::cout << outputmod::startblue << "Integrator: " << outputmod::endblue << g_scene_stepper->getName() << std::endl;
    
    if( g_save_to_binary ) std::cout << outputmod::startpink << "FOSSSim message: "  << outputmod::endpink << "Saving simulation to: " << g_binary_file_name << std::endl;
    
    miscOutputInitialization();
    
    if( g_rendering_enabled ) glutMainLoop();
    else headlessSimLoop();
    
    return 0;
}
