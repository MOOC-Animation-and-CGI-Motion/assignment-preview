#include "TwoDSceneSerializer.h"

void TwoDSceneSerializer::serializeScene( TwoDScene& scene, std::ofstream& outputstream ) const
{
    assert( outputstream.is_open() );
    
    int ndof = 2*scene.getNumParticles();
    scalar* xdata = scene.getX().data();
    outputstream.write((char*)xdata,ndof*sizeof(scalar));
    scalar* vdata = scene.getV().data();
    outputstream.write((char*)vdata,ndof*sizeof(scalar));
}

void TwoDSceneSerializer::loadScene( TwoDScene& scene, std::ifstream& inputstream ) const
{
    assert( inputstream.is_open() );
    assert( !inputstream.eof() );
    
    int ndof = 2*scene.getNumParticles();
    scalar* xdata = scene.getX().data();
    inputstream.read((char*)xdata,ndof*sizeof(scalar));
    scalar* vdata = scene.getV().data();
    inputstream.read((char*)vdata,ndof*sizeof(scalar));
    
    if( inputstream.fail() )
    {
        std::cout << outputmod::startred << "Error in TwoDSceneSerialized: " << outputmod::endred << "Failed to load timestep. Exiting." << std::endl;
        exit(1);
    }  
}
