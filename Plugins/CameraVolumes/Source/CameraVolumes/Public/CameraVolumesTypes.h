//redbox, 2019

#pragma once
#include "CameraVolumesTypes.generated.h"

// Camera mobility
UENUM(BlueprintType)
enum class ECameraMobility : uint8
{
	ECM_Movable	UMETA(DisplayName = "Movable"),
	ECM_Static	UMETA(DisplayName = "Static")
};

// Sides
UENUM(BlueprintType)
enum class ESide : uint8
{
	ES_Front	UMETA(DisplayName = "Front"),
	ES_Back		UMETA(DisplayName = "Back"),
	ES_Right	UMETA(DisplayName = "Right"),
	ES_Left		UMETA(DisplayName = "Left"),
	ES_Top		UMETA(DisplayName = "Top"),
	ES_Bottom	UMETA(DisplayName = "Bottom")
};

// Side type (Open or Closed)
UENUM(BlueprintType)
enum class ESideType : uint8
{
	EST_Open	UMETA(DisplayName = "Open"),
	EST_Closed	UMETA(DisplayName = "Closed")
};

// Side Transition Type (Smooth or Cut)
UENUM(BlueprintType)
enum class ESideTransitionType : uint8
{
	ESTT_Normal	UMETA(DisplayName = "Normal"),
	ESTT_Smooth	UMETA(DisplayName = "Smooth"),
	ESTT_Cut	UMETA(DisplayName = "Cut")
};

// Side info
USTRUCT(BlueprintType)
struct FSideInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = VolumeSides)
		ESide Side;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VolumeSides)
		ESideType SideType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VolumeSides)
		ESideTransitionType SideTransitionType;

	FSideInfo()
	{
		Side = ESide::ES_Front;
		SideType = ESideType::EST_Open;
		SideTransitionType = ESideTransitionType::ESTT_Normal;
	}
};
