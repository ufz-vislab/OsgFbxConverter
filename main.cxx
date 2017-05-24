
#include <fbxsdk.h>
#include "Common.h"

#include "OsgFbxConverter.h"
#include <OpenSG/OSGSceneFileHandler.h>

using namespace std;

size_t findLastPathSeparator(std::string const& path)
{
    return path.find_last_of("/\\");
}

size_t findLastDot(std::string const& path)
{
    return path.find_last_of(".");
}

std::string dropFileExtension(std::string const& filename)
{
    // Look for dots in filename.
    const size_t p = findLastDot(filename);
    if (p == std::string::npos)
        return filename;

    // Check position of the last path separator.
    const size_t s = findLastPathSeparator(filename);
    if (s != std::string::npos && p < s)
        return filename;

    return filename.substr(0, p);
}

std::string extractBaseName(std::string const& pathname)
{
    const size_t p = findLastPathSeparator(pathname);
    return pathname.substr(p + 1);
}

std::string extractBaseNameWithoutExtension(std::string const& pathname)
{
    std::string basename = extractBaseName(pathname);
    return dropFileExtension(basename);
}

// First argument: output path
// Second argument: file to convert
int main (int argc, char **argv)
{
    OSG::osgInit(argc,argv);

    // Init fbx
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult = false;
    InitializeSdkObjects(lSdkManager, lScene);

    string outputDirectory = string(argv[1]);
    string filename = string(argv[2]);

    std::cout << "Converting file " << filename << std::endl;
    OSG::NodePtr node = OSG::SceneFileHandler::the().read(filename.c_str());
    if (node == OSG::NullFC)
    {
        std::cout
            << "It was not possible to load all needed models from file"
            << std::endl;
        return 1;
    }

    OsgFbxConverter* converter = new OsgFbxConverter(node, lScene);
    converter->convert();

    // Save the scene.
    string filenameWithoutPath = extractBaseNameWithoutExtension(filename);
    filename = outputDirectory.append(filenameWithoutPath).append(".fbx");
    cout << "Saving to " << filename << " ..." << endl;

    // Use the binary format with embedded media.
    int lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX 6.0 binary (*.fbx)");
    lResult = SaveScene(lSdkManager, lScene, filename.c_str(), lFormat, true);

    delete converter;

    if(!lResult)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, true);
        return 1;
    }

    // Destroy all objects created by the FBX SDK.
    // DestroySdkObjects(lSdkManager); // Crashes??

    return 0;
}
