// PropertiesSingleton.cpp - Singleton wrapper for ApplicationProperties.
//
// This file is part of Pedalboard3, an audio plugin host.
// Copyright (c) 2011 Niall Moody.
// Copyright (c) 2026 Pedalboard3 Project.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "PropertiesSingleton.h"

//------------------------------------------------------------------------------

PropertiesSingleton PropertiesSingleton::singletonInstance;

//------------------------------------------------------------------------------

PropertiesSingleton::PropertiesSingleton()
{
    instance = std::make_unique<ApplicationProperties>();

    PropertiesFile::Options options;
    options.applicationName = "Pedalboard3";
    options.folderName = "Pedalboard3";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";

    instance->setStorageParameters(options);
}

//------------------------------------------------------------------------------

PropertiesSingleton::~PropertiesSingleton()
{
    if (instance)
        instance->closeFiles();
}

//------------------------------------------------------------------------------

ApplicationProperties& PropertiesSingleton::getInstance()
{
    return *singletonInstance.instance;
}

//------------------------------------------------------------------------------

void PropertiesSingleton::killInstance()
{
    singletonInstance.instance.reset();
}
