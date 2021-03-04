//redbox, 2021

#include "CameraVolumeActor.h"
#include "CameraVolumesCameraComponent.h"
#include "CameraVolumesFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"

ACameraVolumeActor::ACameraVolumeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default root
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->Mobility = EComponentMobility::Static;
	DefaultSceneRoot->SetVisibility(false);
	RootComponent = DefaultSceneRoot;

#if WITH_EDITORONLY_DATA
	// Billboard
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(RootComponent);
	BillboardComponent->bHiddenInGame = true;

	// CameraPreview
	CameraPreview = CreateDefaultSubobject<UCameraVolumesCameraComponent>(TEXT("CameraPreview"));
	CameraPreview->SetupAttachment(RootComponent);
#endif

	// BoxComponent
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Default values
	Priority = 0;

	bUseZeroDepthExtentEditCond = true;
	bUseZeroDepthExtent = false;
	bUse6DOFVolume = false;
	bPerformCameraBlockingEditCond = true;
	bPerformCameraBlocking = true;
	bDisableMainBoxCollision = false;

#if WITH_EDITORONLY_DATA
	CameraProjectionMode = ECameraProjectionMode::Perspective;
#endif

	CameraMobility = ECameraMobility::ECM_Movable;

	bOverrideCameraLocation = false;
	bCameraLocationRelativeToVolume = true;

	bOverrideCameraRotation = false;
	CameraFocalPoint = FVector::ZeroVector;
	CameraRoll = 0.f;
	bFocalPointIsPlayer = false;

	bOverrideCameraFieldOfView = false;
	CameraFieldOfView = 90.f;

	bOverrideCameraOrthoWidth = false;
	CameraOrthoWidth = 512.f;

	CameraSmoothTransitionSpeed = 1.f;

	bOverrideDeadZoneSettings = false;
	DeadZoneExtent = FVector2D::ZeroVector;
	DeadZoneOffset = FVector2D::ZeroVector;

	bUseCameraRotationAxisEditCond = true;
	bUseCameraRotationAxis = false;

#if WITH_EDITORONLY_DATA
	TextSize = 50.f;
#endif

	FrontSide.Side = ESide::ES_Front;
	BackSide.Side = ESide::ES_Back;
	RightSide.Side = ESide::ES_Right;
	LeftSide.Side = ESide::ES_Left;
	TopSide.Side = ESide::ES_Top;
	BottomSide.Side = ESide::ES_Bottom;

#if WITH_EDITOR
	ACameraVolumeActor::CreateSidesIndicators();
#endif

	OpenEdgeOffset = 10000.f;
	VolumeExtentDefault = FVector(500.f);

#if WITH_EDITORONLY_DATA
	BillboardIconPath = TEXT("/CameraVolumes/Icons/CameraVolume");
#endif

	LoadConfig();
	
	VolumeExtent = VolumeExtentDefault;

#if WITH_EDITORONLY_DATA
	static ConstructorHelpers::FObjectFinder<UTexture2D> TextureObj(*BillboardIconPath);
	if (TextureObj.Object)
	{
		BillboardComponent->Sprite = TextureObj.Object;
	}
#endif

	ACameraVolumeActor::UpdateVolume();
}

float ACameraVolumeActor::GetCamVolAspectRatio() const
{
	return CamVolAspectRatio;
}

FVector ACameraVolumeActor::GetCamVolMinCorrected() const
{
	return CamVolMinCorrected;
}

FVector ACameraVolumeActor::GetCamVolMaxCorrected() const
{
	return CamVolMaxCorrected;
}

FVector ACameraVolumeActor::GetCamVolExtentCorrected() const
{
	return CamVolExtentCorrected;
}

void ACameraVolumeActor::UpdateVolume()
{
	//Reset actor scale
	SetActorScale3D(FVector::OneVector);

	//Extents
	CalculateVolumeExtents();

	//Components
	BoxComponent->SetBoxExtent(VolumeExtent);
	SetDisableMainBoxCollision(bDisableMainBoxCollision);

#if WITH_EDITORONLY_DATA
	BillboardComponent->SetRelativeLocation(FVector::ZeroVector);
	BillboardComponent->SetRelativeScale3D(FVector(5.f, 1.f, 1.f));
#endif

	if (bUse6DOFVolume)
	{
		bUseZeroDepthExtentEditCond = false;
		bUseZeroDepthExtent = false;
	}
	else
	{
		bUseZeroDepthExtentEditCond = true;
	}

	switch (CameraMobility)
	{
	case ECameraMobility::ECM_Movable:
		bIsCameraStatic = false;
		bFocalPointIsPlayer = true;
		bUseCameraRotationAxisEditCond = true;
		break;
	case ECameraMobility::ECM_Static:
		bIsCameraStatic = true;
		bOverrideCameraLocation = true;
		bPerformCameraBlocking = false;
		bUseCameraRotationAxisEditCond = false;
		bUseCameraRotationAxis = false;
		break;
	}

	if (!bOverrideCameraLocation)
	{
		CameraLocation = FVector(0.f, 1000.f, 0.f);
	}

	if (!bOverrideCameraRotation)
	{
		CameraFocalPoint = FVector::ZeroVector;
		CameraRoll = 0.f;
		CameraRotation = FRotator(0.f, -90.f, 0.f).Quaternion();
	}
	else
	{
		CameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocation, CameraFocalPoint, CameraRoll);
	}

	if (!bOverrideCameraFieldOfView)
	{
		CameraFieldOfView = 90.f;
	}

	if (!bOverrideCameraOrthoWidth)
	{
		CameraOrthoWidth = 512.f;
	}

#if WITH_EDITORONLY_DATA
	CameraPreview->ProjectionMode = CameraProjectionMode;
	CameraPreview->DefaultCameraFieldOfView = CameraFieldOfView;
	CameraPreview->DefaultCameraOrthoWidth = CameraOrthoWidth;
	CameraPreview->DefaultCameraLocation = CameraLocation;
	CameraPreview->DefaultCameraFocalPoint = CameraFocalPoint;
	CameraPreview->DefaultCameraRoll = CameraRoll;

	CameraPreview->bUseDeadZone = bOverrideDeadZoneSettings;
	CameraPreview->DeadZoneExtent = DeadZoneExtent;
	CameraPreview->DeadZoneOffset = DeadZoneOffset;
	CameraPreview->bPreviewDeadZone = bOverrideDeadZoneSettings;
#endif

#if WITH_EDITOR
	CameraPreview->UpdateCameraComponent();
	UpdateSidesIndicators();
	Modify();
#endif
}

void ACameraVolumeActor::CalculateVolumeExtents()
{
	VolumeExtent = VolumeExtent.GetAbs();
	CamVolMinCorrected = -VolumeExtent;
	CamVolMaxCorrected = VolumeExtent;

	if (RightSide.SideType == ESideType::EST_Open)
	{
		CamVolMaxCorrected.X += OpenEdgeOffset;
	}

	if (LeftSide.SideType == ESideType::EST_Open)
	{
		CamVolMinCorrected.X -= OpenEdgeOffset;
	}

	if (TopSide.SideType == ESideType::EST_Open)
	{
		CamVolMaxCorrected.Z += OpenEdgeOffset;
	}

	if (BottomSide.SideType == ESideType::EST_Open)
	{
		CamVolMinCorrected.Z -= OpenEdgeOffset;
	}

	if (bUse6DOFVolume)
	{
		if (FrontSide.SideType == ESideType::EST_Open)
		{
			CamVolMaxCorrected.Y += OpenEdgeOffset;
		}

		if (BackSide.SideType == ESideType::EST_Open)
		{
			CamVolMinCorrected.Y -= OpenEdgeOffset;
		}
	}

	if (bUseZeroDepthExtent)
	{
		CamVolMinCorrected.Y = 0.f;
		CamVolMaxCorrected.Y = 0.f;
	}

	CamVolExtentCorrected = (CamVolMaxCorrected - CamVolMinCorrected) * 0.5f;
	CamVolAspectRatio = CamVolExtentCorrected.X / CamVolExtentCorrected.Z;
}

FSideInfo ACameraVolumeActor::GetNearestVolumeSideInfo(FVector& PlayerPawnLocation)
{
	ESide NearestSide = ESide::ES_Front;
	TMap<ESide, float> Sides;
	const FVector PlayerPawnLocationTransformed = GetActorTransform().InverseTransformPositionNoScale(PlayerPawnLocation);

	if (bUse6DOFVolume)
	{
		Sides.Add(ESide::ES_Front, FMath::Abs(PlayerPawnLocationTransformed.Y - VolumeExtent.Y));
		Sides.Add(ESide::ES_Back, FMath::Abs(PlayerPawnLocationTransformed.Y + VolumeExtent.Y));
		Sides.Add(ESide::ES_Right, FMath::Abs(PlayerPawnLocationTransformed.X - VolumeExtent.X));
		Sides.Add(ESide::ES_Left, FMath::Abs(PlayerPawnLocationTransformed.X + VolumeExtent.X));
		Sides.Add(ESide::ES_Top, FMath::Abs(PlayerPawnLocationTransformed.Z - VolumeExtent.Z));
		Sides.Add(ESide::ES_Bottom, FMath::Abs(PlayerPawnLocationTransformed.Z + VolumeExtent.Z));
	}
	else
	{
		Sides.Add(ESide::ES_Right, FMath::Abs(PlayerPawnLocationTransformed.X - VolumeExtent.X));
		Sides.Add(ESide::ES_Left, FMath::Abs(PlayerPawnLocationTransformed.X + VolumeExtent.X));
		Sides.Add(ESide::ES_Top, FMath::Abs(PlayerPawnLocationTransformed.Z - VolumeExtent.Z));
		Sides.Add(ESide::ES_Bottom, FMath::Abs(PlayerPawnLocationTransformed.Z + VolumeExtent.Z));
	}

	Sides.ValueSort([](float Min, float Max) { return Min < Max; });

	for (auto& Pair : Sides)
	{
		NearestSide = Pair.Key;
		break;
	}

	switch (NearestSide)
	{
	case ESide::ES_Front:
		return FrontSide;
	case ESide::ES_Back:
		return BackSide;
	case ESide::ES_Right:
		return RightSide;
	case ESide::ES_Left:
		return LeftSide;
	case ESide::ES_Top:
		return TopSide;
	case ESide::ES_Bottom:
		return BottomSide;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown side type! Using SideInfo(Front, Open, Normal)"));
		return FSideInfo();
	}
}

#if WITH_EDITOR
void ACameraVolumeActor::CreateSidesIndicators()
{
	/**
	*	Sides indicators
	*	Priority [0]
	*	SideInfo [1-18]
	*/
	Text_Indicators.SetNum(19);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TextMaterialObj(TEXT("/CameraVolumes/Materials/UnlitText.UnlitText"));

	for (uint8 i = 0; i < 19; i++)
	{
		const FName ComponentName = *(FString(TEXT("TextRenderComponent")) + FString::FromInt(i));
		Text_Indicators[i] = CreateDefaultSubobject<UTextRenderComponent>(ComponentName);
		Text_Indicators[i]->SetupAttachment(RootComponent);
		Text_Indicators[i]->bHiddenInGame = true;
		Text_Indicators[i]->bAlwaysRenderAsText = true;
		Text_Indicators[i]->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
		Text_Indicators[i]->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
		Text_Indicators[i]->SetTextRenderColor(FColor::White);

		if (TextMaterialObj.Object)
		{
			Text_Indicators[i]->SetTextMaterial(TextMaterialObj.Object);
		}
	}
}

void ACameraVolumeActor::UpdateSidesIndicators()
{
	//Priority
	Text_Indicators[0]->SetText(FText::FromString(FString::FromInt(Priority)));
	Text_Indicators[0]->SetWorldSize(2.f * TextSize);
	Text_Indicators[0]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f));

	//Sides
	for (uint8 i = 1; i <= 18; i = i + 3)
	{
		Text_Indicators[i]->SetWorldSize(TextSize);
		Text_Indicators[i + 1]->SetWorldSize(TextSize);
		Text_Indicators[i + 2]->SetWorldSize(TextSize);

		FSideInfo SideInfo;
		if (i == 1)
		{
			SideInfo = FrontSide;
			Text_Indicators[i]->SetText(Text_Front);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 2.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 1.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 0.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				Text_Indicators[i]->SetVisibility(false);
				Text_Indicators[i + 1]->SetVisibility(false);
				Text_Indicators[i + 2]->SetVisibility(false);
			}
		}
		else if (i == 4)
		{
			SideInfo = BackSide;
			Text_Indicators[i]->SetText(Text_Back);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 0.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 1.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 2.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				Text_Indicators[i]->SetVisibility(false);
				Text_Indicators[i + 1]->SetVisibility(false);
				Text_Indicators[i + 2]->SetVisibility(false);
			}
		}
		else if (i == 7)
		{
			SideInfo = RightSide;
			Text_Indicators[i]->SetText(Text_Right);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, 0.f), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, -TextSize), FRotator(0.f, 90.f, 0.f));
		}
		else if (i == 10)
		{
			SideInfo = LeftSide;
			Text_Indicators[i]->SetText(Text_Left);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, 0.f), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, -TextSize), FRotator(0.f, 90.f, 0.f));
		}
		else if (i == 13)
		{
			SideInfo = TopSide;
			Text_Indicators[i]->SetText(Text_Top);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 0.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 1.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 2.5f * TextSize), FRotator(0.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
		}
		else if (i == 16)
		{
			SideInfo = BottomSide;
			Text_Indicators[i]->SetText(Text_Bottom);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 2.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 1.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 0.5f * TextSize), FRotator(0.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
		}

		if (SideInfo.SideType == ESideType::EST_Closed)
		{
			Text_Indicators[i + 1]->SetText(Text_Closed);
			Text_Indicators[i + 1]->SetTextRenderColor(FColor::Red);
		}
		else
		{
			Text_Indicators[i + 1]->SetText(Text_Open);
			Text_Indicators[i + 1]->SetTextRenderColor(FColor::Green);
		}

		switch (SideInfo.SideTransitionType)
		{
		case ESideTransitionType::ESTT_Normal:
			Text_Indicators[i + 2]->SetText(Text_Normal);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Yellow);
			break;
		case ESideTransitionType::ESTT_Smooth:
			Text_Indicators[i + 2]->SetText(Text_Smooth);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Green);
			break;
		case ESideTransitionType::ESTT_Cut:
			Text_Indicators[i + 2]->SetText(Text_Cut);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Red);
			break;
		}
	}
}
#endif

//Update with changed property
#if WITH_EDITOR
void ACameraVolumeActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("Priority") || TEXT("VolumeExtent")
		|| TEXT("bUseZeroDepthExtent") || TEXT("bUse6DOFVolume")
		|| TEXT("bDisableMainBoxCollision")
		|| TEXT("CameraProjectionMode") || TEXT("CameraMobility")
		|| TEXT("bOverrideCameraLocation") || TEXT("CameraLocation")
		|| TEXT("bOverrideCameraRotation") || TEXT("CameraFocalPoint") || TEXT("CameraRoll")
		|| TEXT("bOverrideCameraFieldOfView") || TEXT("CameraFieldOfView")
		|| TEXT("bOverrideCameraOrthoWidth") || TEXT("CameraOrthoWidth")
		|| TEXT("FrontSide") || TEXT("BackSide") || TEXT("RightSide") || TEXT("LeftSide") || TEXT("TopSide") || TEXT("BottomSide")
		|| TEXT("TextSize")
		|| TEXT("bUseCameraRotationAxis")
		|| TEXT("bOverrideDeadZoneSettings") || TEXT("DeadZoneExtent") || TEXT("DeadZoneOffset"))
	{
		UpdateVolume();
	}
}

void ACameraVolumeActor::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyScale(DeltaScale, PivotLocation, bAltDown, bShiftDown, bCtrlDown);

	FVector ScaleToApply = VolumeExtent / VolumeExtentDefault;

	if (bUsePercentageBasedScaling)
	{
		ScaleToApply *= FVector::OneVector + DeltaScale;
	}
	else
	{
		ScaleToApply += DeltaScale;
	}

	VolumeExtent = VolumeExtentDefault * ScaleToApply;
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyMirror(const FVector& MirrorScale, const FVector& PivotLocation)
{
	Super::EditorApplyMirror(MirrorScale, PivotLocation);
	SetActorScale3D(FVector::OneVector);
}
#endif

// Runtime setters
void ACameraVolumeActor::SetVolumeExtent(FVector NewVolumeExtent)
{
	VolumeExtent = NewVolumeExtent;
	UpdateVolume();
}

void ACameraVolumeActor::SetUseZeroDepthExtent(bool bNewUseZeroDepthExtent)
{
	bUseZeroDepthExtent = bNewUseZeroDepthExtent;
	UpdateVolume();
}

void ACameraVolumeActor::SetUse6DOFVolume(bool bNewUse6DOFVolume)
{
	bUse6DOFVolume = bNewUse6DOFVolume;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraMobility(ECameraMobility NewCameraMobility)
{
	CameraMobility = NewCameraMobility;
	UpdateVolume();
}

bool ACameraVolumeActor::GetIsCameraStatic() const
{
	return bIsCameraStatic;
}

void ACameraVolumeActor::SetOverrideCameraLocation(bool bNewOverrideCameraLocation)
{
	bOverrideCameraLocation = bNewOverrideCameraLocation;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraLocation(FVector NewCameraLocation)
{
	CameraLocation = NewCameraLocation;
	UpdateVolume();
}

void ACameraVolumeActor::SetOverrideCameraRotation(bool bNewOverrideCameraRotation)
{
	bOverrideCameraRotation = bNewOverrideCameraRotation;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraFocalPoint(FVector NewCameraFocalPoint)
{
	CameraFocalPoint = NewCameraFocalPoint;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraRoll(float NewCameraRoll)
{
	CameraRoll = NewCameraRoll;
	UpdateVolume();
}

void ACameraVolumeActor::SetRightSide(FSideInfo NewRightSide)
{
	RightSide = NewRightSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetLeftSide(FSideInfo NewLeftSide)
{
	LeftSide = NewLeftSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetTopSide(FSideInfo NewTopSide)
{
	TopSide = NewTopSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetBottomSide(FSideInfo NewBottomSide)
{
	BottomSide = NewBottomSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetFrontSide(FSideInfo NewFrontSide)
{
	FrontSide = NewFrontSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetBackSide(FSideInfo NewBackSide)
{
	BackSide = NewBackSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetDisableMainBoxCollision(bool bNewDisableMainBoxCollision)
{
	bDisableMainBoxCollision = bNewDisableMainBoxCollision;

	if (bDisableMainBoxCollision)
	{
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BoxComponent->ShapeColor = FColor(127, 127, 127, 255);
	}
	else
	{
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BoxComponent->ShapeColor = FColor(223, 149, 157, 255);
	}
}

#if WITH_EDITOR
void ACameraVolumeActor::SetAllOpen()
{
	FrontSide.SideType = ESideType::EST_Open;
	BackSide.SideType = ESideType::EST_Open;
	RightSide.SideType = ESideType::EST_Open;
	LeftSide.SideType = ESideType::EST_Open;
	TopSide.SideType = ESideType::EST_Open;
	BottomSide.SideType = ESideType::EST_Open;

	UpdateVolume();
}

void ACameraVolumeActor::SetAllClosed()
{
	FrontSide.SideType = ESideType::EST_Closed;
	BackSide.SideType = ESideType::EST_Closed;
	RightSide.SideType = ESideType::EST_Closed;
	LeftSide.SideType = ESideType::EST_Closed;
	TopSide.SideType = ESideType::EST_Closed;
	BottomSide.SideType = ESideType::EST_Closed;

	UpdateVolume();
}

void ACameraVolumeActor::SetAllNormal()
{
	FrontSide.SideTransitionType = ESideTransitionType::ESTT_Normal;
	BackSide.SideTransitionType = ESideTransitionType::ESTT_Normal;
	RightSide.SideTransitionType = ESideTransitionType::ESTT_Normal;
	LeftSide.SideTransitionType = ESideTransitionType::ESTT_Normal;
	TopSide.SideTransitionType = ESideTransitionType::ESTT_Normal;
	BottomSide.SideTransitionType = ESideTransitionType::ESTT_Normal;

	UpdateVolume();
}

void ACameraVolumeActor::SetAllSmooth()
{
	FrontSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;
	BackSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;
	RightSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;
	LeftSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;
	TopSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;
	BottomSide.SideTransitionType = ESideTransitionType::ESTT_Smooth;

	UpdateVolume();
}

void ACameraVolumeActor::SetAllCut()
{
	FrontSide.SideTransitionType = ESideTransitionType::ESTT_Cut;
	BackSide.SideTransitionType = ESideTransitionType::ESTT_Cut;
	RightSide.SideTransitionType = ESideTransitionType::ESTT_Cut;
	LeftSide.SideTransitionType = ESideTransitionType::ESTT_Cut;
	TopSide.SideTransitionType = ESideTransitionType::ESTT_Cut;
	BottomSide.SideTransitionType = ESideTransitionType::ESTT_Cut;

	UpdateVolume();
}
#endif
