#include "StdAfx.h"
#include "ElectronicsBoxProperties.h"

#include <exception>

namespace Communication
{
    void SetupBoxVersion1(ElectronicsBoxProperties& box)
    {
        box.pakFileEnding = ".PAK";
        box.pakFileLocation = "B:";
        box.configurationFileLocation = "A:";
        box.temporaryUploadFileName = "A:\\tempUpload"; // shouldn't be used

        box.command_RemoveDirectory = "rd";
        box.command_EnterDataDirectory = "B:";
        box.command_EnterConfigurationFileDirectory = "A:";

        box.maxChunkSize = 8192;
        box.maxPathLength = 16;

        box.numberOfDisks = 2;

        box.includeDiskNameInDataDownload = true;
        box.uploadToTemporaryFile = false;
        box.needsToSwitchModes = true;
    }

    void SetupBoxVersion2(ElectronicsBoxProperties& box)
    {
        box.pakFileEnding = ".pak";
        box.pakFileLocation = "/mnt/flash/novac";
        box.configurationFileLocation = "/mnt/flash";
        box.temporaryUploadFileName = "/mnt/flash/XYZ";

        box.command_RemoveDirectory = "rmdir";
        box.command_EnterDataDirectory = "cd /mnt/flash/novac/";
        box.command_EnterConfigurationFileDirectory = "cd /mnt/flash/";

        box.maxChunkSize = 2048;
        box.maxPathLength = 128;

        box.numberOfDisks = 1;

        box.includeDiskNameInDataDownload = false;
        box.uploadToTemporaryFile = true;
        box.needsToSwitchModes = false;
    }

    void SetupElectronicsBoxProperties(ELECTRONICS_BOX box, ElectronicsBoxProperties& properties)
    {
        switch (box)
        {
        case ELECTRONICS_BOX::BOX_VERSION_1: return SetupBoxVersion1(properties);
        case ELECTRONICS_BOX::BOX_VERSION_2: return SetupBoxVersion2(properties);
        }

        throw std::invalid_argument("Invalid type of electronics box encountered.");
    }
}