#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include "Application.h"
#include "Paths.h"
#include "core/Random.h"

int main(int argc, char** argv)
{
    try
    {
        AppOptions options;
        options.shaderDir = FindShaderDir(argv[0]);
        
        Random::Init();

        Application app(options);
        app.Run();
    }
    catch (const std::exception& e)
    {
        spdlog::critical("{}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
