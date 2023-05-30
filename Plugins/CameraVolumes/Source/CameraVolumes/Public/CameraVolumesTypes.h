// redbox, 2021

#pragma once

#include "CoreMinimal.h"
#include "CameraVolumesTypes.generated.h"

#define DEAD_ZONES 0
#define DRAW_DEBUG 0

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

#if 0 //DEAD_ZONES
// Dead zone transform
USTRUCT(BlueprintType)
struct FDeadZoneTransform
{
	GENERATED_BODY()

	// Dead zone extent (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone)
	FVector2D DeadZoneExtent;

	// Dead zone offset from the center of the screen (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone)
	FVector2D DeadZoneOffset;

	// Dead zone roll (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone)
	float DeadZoneRoll;

	FDeadZoneTransform()
	{
		DeadZoneExtent = FVector2D::ZeroVector;
		DeadZoneOffset = FVector2D::ZeroVector;
		DeadZoneRoll = 0.f;
	}

	FDeadZoneTransform(FVector2D InDeadZoneExtent, FVector2D InDeadZoneOffset, float InDeadZoneRoll)
	{
		DeadZoneExtent = InDeadZoneExtent;
		DeadZoneOffset = InDeadZoneOffset;
		DeadZoneRoll = InDeadZoneRoll;
	}
};
#endif
