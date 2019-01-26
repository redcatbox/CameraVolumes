//Dmitriy Barannik aka redbox, 2019

/**
* Camera volume actor stores desired camera parameters and volume information.
* Calculates camera preview, side indicators and volume extents by itself when updated in Editor.
* This actor and it's parameters are static in context of this algorithm.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Components/BillboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "CameraVolumeActor.generated.h"

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

UENUM(BlueprintType)
enum class ECameraMobility : uint8
{
	ECM_Movable	UMETA(DisplayName = "Movable"),
	ECM_Static	UMETA(DisplayName = "Static")
};

UCLASS(AutoExpandCategories = (Volume, Camera))
class CAMERAVOLUMES_API ACameraVolumeActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACameraVolumeActor();

protected:
	//Components
	UPROPERTY()
		USceneComponent* DefaultSceneRoot;

	UPROPERTY()
		UBillboardComponent* BillboardComponent;

	UPROPERTY()
		UBoxComponent* BoxComponent;

	UPROPERTY()
		UCameraComponent* CameraComponent;
	//--------------------------------------------------

public:
	//Parameters
	/** Priority of camera volume in case of few overlapped volumes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume", Meta = (ClampMin = "-100", ClampMax = "100", UIMin = "-100", UIMax = "100"))
		int32 Priority;

	/** Volume extent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume", Meta = (MakeEditWidget = true))
		FVector VolumeExtent;

	/** Camera mobility */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		ECameraMobility CameraMobility;

	/** Should override camera location? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		bool bOverrideCameraLocation;

	/** New camera offset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (EditCondition = "bOverrideCameraLocation"))
		FVector CameraLocation;

	/** Should camera location be relative to volume? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (EditCondition = "bOverrideCameraLocation"))
		bool bCameraLocationRelativeToVolume;

	/** Should override camera focal point? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		bool bOverrideCameraFocalPoint;

	/** Location that fixed camera look at */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (EditCondition = "bOverrideCameraFocalPoint", MakeEditWidget = true))
		FVector CameraFocalPoint;

protected:
	UPROPERTY()
		bool bIsCameraStatic;

public:
	/** Returns is volume uses static camera settings */
	FORCEINLINE bool GetIsCameraStatic() const { return bIsCameraStatic; }

	/** Should fixed camera look at player character pivot? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (EditCondition = "bIsCameraStatic"))
		bool bFocalPointIsPlayer;
	
	UPROPERTY()
		FQuat CameraRotation;

		/** Should override camera FOV? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		bool bOverrideCameraFieldOfView;

	/** New camera FOV */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (EditCondition = "bOverrideCameraFieldOfView", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
		float CameraFieldOfView;
	//--------------------------------------------------

	// Sides info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo FrontSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo BackSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo RightSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo LeftSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo TopSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SidesInfo", Meta = (ShowOnlyInnerProperties))
		FSideInfo BottomSide;

	/** Time of smooth camera transition */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0"))
		float CameraSmoothTransitionTime;
	//--------------------------------------------------

	UPROPERTY()
		float CamVolAspectRatio;

	UPROPERTY()
		FVector CamVolWorldMin;

	UPROPERTY()
		FVector CamVolWorldMax;

	UPROPERTY()
		FVector CamVolWorldMinCorrected;

	UPROPERTY()
		FVector CamVolWorldMaxCorrected;

	UPROPERTY()
		FVector CamVolExtentCorrected;

	/** Use this to update volume after made changes in editor, if they are not applied automatically */
	UFUNCTION(CallInEditor, Category = "Volume")
		virtual void UpdateVolume();

	/** Update volume extents. Can be called for dynamic camera volume */
	UFUNCTION(BlueprintCallable, Category = "Volume")
		virtual void UpdateVolumeExtents();

	/** Get side info */
	UFUNCTION(BlueprintCallable)
		virtual FSideInfo GetSideInfo(ESide Side);

	/** Get volume side nearest to player location */
	UFUNCTION(BlueprintCallable)
		virtual ESide GetNearestVolumeSide(FVector& PlayerPawnLocation);

protected:
	//Sides indicators
	UPROPERTY()
		TArray<UTextRenderComponent*> Text_Indicators;

	const float Text_Size = 50.f;
	const FText Text_Open = FText::FromString("OPEN");
	const FText Text_Closed = FText::FromString("CLOSED");
	const FText Text_Normal = FText::FromString("NORMAL");
	const FText Text_Smooth = FText::FromString("SMOOTH");
	const FText Text_Cut = FText::FromString("CUT");
	//--------------------------------------------------

	const FVector DefaultCameraLocation = FVector(1000.f, 0.f, 0.f); // Side-scroller
//const FVector DefaultCameraLocation = FVector(0.f, 0.f, 1000.f); // Top-down
	const FVector DefaultCameraFocalPoint = FVector::ZeroVector;
	const float DefaultCameraFOV = 90.f;
	const float OpenEdgeOffset = 10000.f;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};