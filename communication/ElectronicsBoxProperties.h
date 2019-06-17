#pragma once

#include <string>
#include "../Common/Common.h"

namespace Communication
{
    struct ElectronicsBoxProperties
    {
        ELECTRONICS_BOX version = ELECTRONICS_BOX::BOX_VERSION_1;

        // Simple name of this system
        std::string name = "Axis";

        // File system: The (case sensitive) file extension of the .pak files
        std::string pakFileEnding = "PAK";

        // File system: The location of the .pak files
        std::string pakFileLocation = "/mnt/flash/novac";

        // File system: The location of cfgonce.txt
        std::string configurationFileLocation = "/mnt/flash";

        // File system: The temporary location to which files should be uploaded
        //  only useful if uploadToTemporaryFile is true
        std::string temporaryUploadFileName = "/mnt/flash/XYZ";

        // Commands: The command used to delete a directory
        std::string command_RemoveDirectory = "rmdir";

        // Commands: The command to enter the top level data directory
        std::string command_EnterDataDirectory = "cd /mnt/flash/novac/";

        // Commands: The command to enter the command-file directory
        std::string command_EnterConfigurationFileDirectory = "cd /mnt/flash/";

        // The maximum packet size with downloading over serial.
        long maxChunkSize = 8192;

        // The maximum length of a valid path (folder + filename)
        long maxPathLength = 128;

        // The number of disks, where pak files may be located
        int numberOfDisks = 1;

        // Set to true if the disk-name should be included when downloading .pak files
        bool includeDiskNameInDataDownload = false;

        // Set to true if the uploads needs to be done to a temporary file (which then should be renamed afterwards)
        bool uploadToTemporaryFile = true;

        // Set to true if the device needs a special mode switch command between shell and command mode.
        bool needsToSwitchModes = false;
    };

    void SetupElectronicsBoxProperties(ELECTRONICS_BOX box, ElectronicsBoxProperties& properties);

}
