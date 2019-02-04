//Dmitriy Barannik aka redbox, 2019

#pragma once
#include "CameraVolumesTypes.generated.h"

//UENUM(BlueprintType)
//enum class ECameraOrientation : uint8
//{
//	ECO_SideScroller	UMETA(DisplayName = "SideScroller"), // Perform calculations in YZ plane
//	ECO_TopDown			UMETA(DisplayName = "TopDown") // Perform calculations in YX plane
//};

UENUM(BlueprintType)
enum class ECameraMobility : uint8
{
	ECM_Movable	UMETA(DisplayName = "Movable"),
	ECM_Static	UMETA(DisplayName = "Static")
};

UENUM(BlueprintType)
enum class ESide : uint8
{
	ES_Unknown	UMETA(DisplayName = "Unknown"),
	ES_Front	UMETA(DisplayName = "Front"),
	ES_Back		UMETA(DisplayName = "Back"),
	ES_Right	UMETA(DisplayName = "Right"),
	ES_Left		UMETA(DisplayName = "Left"),
	ES_Top		UMETA(DisplayName = "Top"),
	ES_Bottom	UMETA(DisplayName = "Bottom")
};

//Side can be Open or Closed
UENUM(BlueprintType)
enum class ESideType : uint8
{
	EST_Open	UMETA(DisplayName = "Open"),
	EST_Closed	UMETA(DisplayName = "Closed")
};

//Side Transition can be Smooth or Cut
UENUM(BlueprintType)
enum class ESideTransitionType : uint8
{
	ESTT_Normal	UMETA(DisplayName = "Normal"),
	ESTT_Smooth	UMETA(DisplayName = "Smooth"),
	ESTT_Cut	UMETA(DisplayName = "Cut")
};

USTRUCT(BlueprintType)
struct FSideInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ESideType SideType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ESideTransitionType SideTransitionType;

	FSideInfo()
	{
		SideType = ESideType::EST_Open;
		SideTransitionType = ESideTransitionType::ESTT_Normal;
	}

	FSideInfo(ESideType SideType, ESideTransitionType SideTransitionType)
	{
		this->SideType = SideType;
		this->SideTransitionType = SideTransitionType;
	}
};