// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// API macro for exporting symbols
#ifdef INTERFACES_EXPORTS
#define INTERFACES_API DLLEXPORT
#else
#define INTERFACES_API DLLIMPORT
#endif

class FInterfacesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
