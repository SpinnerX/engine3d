#pragma once
#include <string>


/*

What we could do is for world and scenes

Is having a hash tag that is associated to the world.
- This can be used to allow users not have to worry about selecting which world they want to register their scene to
- SystemRegistry::GetWorld()->Register(this)
- Where GetWorld() fetches the world_hashID where this can be stored in a bitset
    - Where we can probably set the first 8-bits in an integer to the hash is the ID of the world the scene is associated with
    - While the last 8-bits tell what the actual scene's hash ID itself
    - These hashes are something added to the scene when they get added to the world
    - When the world is constructed, the SystemRegistry will associate a unique hash to that world
*/

namespace engine3d{
    struct ImageSampler{
    };

    class Image{
    public:
        Image(const std::string& p_Filepath);
    private:
    };
};