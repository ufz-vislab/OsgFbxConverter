
#include <fbxsdk.h>
#include "Common.h"

#include "OsgFbxConverter.h"
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <iostream>
#include <vector>

#define SAMPLE_FILENAME "Sample.fbx"

using namespace std;

// No arguments: batch convert all vt* files
// switch argument: batch convert all vt* files into one osb file with a switch
// file argument: convert only the specified file
int main (int argc, char const* argv[])
{
    // Init fbx
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;
    InitializeSdkObjects(lSdkManager, lScene);

    string outputDirectory = "";
    vector<string> filenames;
    if (argc > 2)
    {
        outputDirectory = string(argv[1]);
        filenames.push_back(string(argv[2]));
    }

    for (vector<string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        string filename(*it);
        OSG::NodePtr node = OSG::SceneFileHandler::the().read(filename);

        VtkFbxConverter* converter = new OsgFbxConverter(node, lScene);
        converter->convert();

        // Save the scene.
        string filenameWithoutPath = VtkFbxHelper::extractBaseNameWithoutExtension(filename);
        filename = outputDirectory.append(filenameWithoutPath).append(".fbx");
        cout << "Saving to " << filename << " ..." << endl;

        // Use the binary format with embedded media.
        int lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX 6.0 binary (*.fbx)");
        lResult = SaveScene(lSdkManager, lScene, filename.c_str(), lFormat, true);

        delete converter;

        actor->Delete();
    }

    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, true);
        return 1;
    }

    // Destroy all objects created by the FBX SDK.
    // DestroySdkObjects(lSdkManager); // Crashes??

    return 0;
}
