#ifndef __TWO_D_SCENE_SERIALIZER_H__
#define __TWO_D_SCENE_SERIALIZER_H__

#include <fstream>
#include <iostream>

#include "TwoDScene.h"
#include "StringUtilities.h"

class TwoDSceneSerializer
{
public:
    void serializeScene( TwoDScene& scene, std::ofstream& outputstream ) const;
    
    void loadScene( TwoDScene& scene, std::ifstream& inputstream ) const;
};

#endif
