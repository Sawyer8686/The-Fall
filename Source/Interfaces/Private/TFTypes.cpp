// Copyright TF Project. All Rights Reserved.

#include "TFTypes.h"
#include "TFContainerInterface.h"

// Define Log Categories
DEFINE_LOG_CATEGORY(LogTFInteraction);
DEFINE_LOG_CATEGORY(LogTFDoor);
DEFINE_LOG_CATEGORY(LogTFItem);
DEFINE_LOG_CATEGORY(LogTFCharacter);
DEFINE_LOG_CATEGORY(LogTFStats);
DEFINE_LOG_CATEGORY(LogTFContainer);

ITFContainerInterface* FTFContainerContext::ActiveContainer = nullptr;
