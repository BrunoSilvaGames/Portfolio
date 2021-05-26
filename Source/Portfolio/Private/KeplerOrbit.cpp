// Copyright Bruno Silva. All rights reserved.


#include "KeplerOrbit.h"

bool FKeplerOrbitConfig::IsValid() const
{
	const bool ArePeriapsisAndApoapsisGreaterThanZero = (Periapsis > 0.0f && Apoapsis > 0.0f);
	const bool IsApoapsisGreaterThanPeriapsis = (Apoapsis >= Periapsis);
	return ArePeriapsisAndApoapsisGreaterThanZero && IsApoapsisGreaterThanPeriapsis;
}

void FKeplerOrbitConfig::FixOrbitConfig()
{
	Periapsis = FMath::Max(1.0f, Periapsis);
	Apoapsis = FMath::Max(1.0f, Apoapsis);
	if (Periapsis > Apoapsis)
	{
		const float OldPeriapsis = Periapsis;
		Periapsis = Apoapsis;
		Apoapsis = OldPeriapsis;
	}
}

void FKeplerOrbitConfig::UpdateOrbitData()
{
	if (!IsValid())
	{
		FixOrbitConfig();
	}

	Eccentricity = 1.0f - (2.0f / ((Apoapsis / Periapsis) + 1.0f));

	SemiMajorAxis = (Periapsis + Apoapsis) / 2.0f;

	SemiMinorAxis = SemiMajorAxis * FMath::Sqrt(1 - (Eccentricity * Eccentricity));

	Period = (360.0f * 2.0f) * FMath::Sqrt(FMath::Pow(SemiMajorAxis, 3.0f));
}

bool FKeplerOrbitConfig::Equals(const FKeplerOrbitConfig& Other) const
{
	const bool bIsPeriapsisEqual = FMath::IsNearlyEqual(Periapsis, Other.Periapsis);
	const bool bIsApoapsisEqual = FMath::IsNearlyEqual(Apoapsis, Other.Apoapsis);
	const bool bIsOrientationEqual = Orientation.Equals(Other.Orientation, 1e-6f);
	const bool bIsAnomalyEqual = FMath::IsNearlyEqual(InitialTrueAnomaly, Other.InitialTrueAnomaly);
	return bIsPeriapsisEqual && bIsApoapsisEqual && bIsOrientationEqual && bIsAnomalyEqual;
}

bool UKeplerLibrary::IsOrbitValid(const FKeplerOrbitConfig& OrbitConfig)
{
	const bool bIsValid = OrbitConfig.IsValid();
	return bIsValid;
}

void UKeplerLibrary::FixOrbitConfig(FKeplerOrbitConfig& OrbitConfig)
{
	OrbitConfig.FixOrbitConfig();
}

void UKeplerLibrary::UpdateOrbitData(FKeplerOrbitConfig& OrbitConfig)
{
	OrbitConfig.UpdateOrbitData();
}

bool UKeplerLibrary::Equals(const FKeplerOrbitConfig& A, const FKeplerOrbitConfig& B)
{
	const bool bIsEqual = A.Equals(B);
	return bIsEqual;
}

float UKeplerLibrary::GetMeanAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float Time)
{
	const float MeanMotion = 360.0f / OrbitConfig.Period;
	const float MeanAnomaly = MeanMotion * Time;
	return MeanAnomaly;
}

float UKeplerLibrary::GetEccentricAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float MeanAnomaly)
{
	const float MeanAnomalyRad = FMath::DegreesToRadians(MeanAnomaly);
	float EccentricAnomaly = MeanAnomalyRad;
	for (int i = 0; i < 18; i++)
	{
		EccentricAnomaly = MeanAnomalyRad + (OrbitConfig.Eccentricity * FMath::Sin(EccentricAnomaly));
	}
	EccentricAnomaly = FMath::RadiansToDegrees(EccentricAnomaly);
	return EccentricAnomaly;
}

float UKeplerLibrary::GetTrueAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float EccentricAnomaly)
{
	const float Eccentricity = OrbitConfig.Eccentricity;
	const float EccentricAnomalyRad = FMath::DegreesToRadians(EccentricAnomaly);

	const float X = FMath::Cos(EccentricAnomalyRad) - Eccentricity;
	const float Y = FMath::Sqrt(1 - FMath::Pow(Eccentricity, 2)) * FMath::Sin(EccentricAnomalyRad);
	float TrueAnomaly = FMath::Atan2(Y, X);

	TrueAnomaly = FMath::RadiansToDegrees(TrueAnomaly);
	return TrueAnomaly;
}

FVector UKeplerLibrary::GetOrbitalPositionTrue(const FKeplerOrbitConfig& OrbitConfig, const float TrueAnomaly)
{
	FQuat OrbitalOrientation = FQuat(OrbitConfig.Orientation);
	FVector OrbitalPosition = OrbitalOrientation.GetForwardVector();
	OrbitalPosition = OrbitalPosition.RotateAngleAxis(TrueAnomaly - 180.0f, OrbitalOrientation.GetUpVector());

	const float Dividend = OrbitConfig.SemiMajorAxis * (1 - FMath::Pow(OrbitConfig.Eccentricity, 2));
	const float Divisor = 1 + OrbitConfig.Eccentricity * FMath::Cos(FMath::DegreesToRadians(TrueAnomaly));
	const float OrbitalDistance = Dividend / Divisor;

	OrbitalPosition *= OrbitalDistance;
	return OrbitalPosition;
}

FVector UKeplerLibrary::GetOrbitalPositionEcc(const FKeplerOrbitConfig& OrbitConfig, const float EccentricAnomaly)
{
	FQuat OrbitalOrientation = FQuat(OrbitConfig.Orientation);
	FVector OrbitalPosition = OrbitalOrientation.GetForwardVector();
	OrbitalPosition = OrbitalPosition.RotateAngleAxis(EccentricAnomaly, OrbitalOrientation.GetUpVector());

	const float EccentricAnomalyRad = FMath::DegreesToRadians(EccentricAnomaly);
	const float OrbitalDistance = OrbitConfig.SemiMajorAxis * (1 - (OrbitConfig.Eccentricity * FMath::Cos(EccentricAnomalyRad)));
	OrbitalPosition *= OrbitalDistance;
	return OrbitalPosition;
}

void UKeplerLibrary::GetOrbitPoints(const FKeplerOrbitConfig& OrbitConfig, const int NumOfPoints, TArray<FVector>& OutPoints)
{
	if (NumOfPoints < 1)
	{
		return;
	}

	const float MeanMotion = 360.0f / (float)NumOfPoints;
	FVector OrbPoint;
	for (int i = 0; i < NumOfPoints; i++)
	{
		const float MeanAnomaly = MeanMotion * i;
		const float EccentricAnomaly = GetEccentricAnomaly(OrbitConfig, MeanAnomaly);
		const float TrueAnomaly = GetTrueAnomaly(OrbitConfig, MeanAnomaly);
		OrbPoint = GetOrbitalPositionTrue(OrbitConfig, TrueAnomaly);
		OutPoints.Add(OrbPoint);
	}
}

FKeplerOrbitConfig UKeplerLibrary::GetOppositeFocus(const FKeplerOrbitConfig& OrbitConfig)
{
	FKeplerOrbitConfig OtherFocus = OrbitConfig;
	OtherFocus.Orientation.Yaw += 180.0f;
	return OtherFocus;
}
