// Copyright Bruno Silva. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KeplerOrbit.generated.h"

USTRUCT(BlueprintType)
struct FKeplerOrbitConfig
{
	GENERATED_BODY()

public:

	/** Distance of the nearest point of the orbit to the center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kepler Orbit")
	float Periapsis;

	/** Distance of the furthest point of the orbit to the center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kepler Orbit")
	float Apoapsis;

	/**
	 * Orientation of the orbit, where:
	 *	- X is the inclination;
	 *	- Y is the longitude of the ascending node;
	 *	- Z is the argument of the periapsis;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kepler Orbit")
	FRotator Orientation;

	/** True anomaly of the orbiting body at the start of the simulation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kepler Orbit")
	float InitialTrueAnomaly;

public:

	/** Half of the longest diameter of the orbit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kepler Orbit")
	float SemiMajorAxis;

	/** Half of the shortest diameter of the orbit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kepler Orbit")
	float SemiMinorAxis;

	/** Determines the amount by which the orbit deviates from a perfect circle. */
	UPROPERTY(BlueprintReadOnly, Category = "Kepler Orbit")
	float Eccentricity;

	/** How long the orbiting body takes to complete one orbit. */
	UPROPERTY(BlueprintReadOnly, Category = "Kepler Orbit")
	float Period;

public:

	FKeplerOrbitConfig() {};

	FKeplerOrbitConfig(float NewPeriapsis, float NewApoapsis, FRotator NewOrientation = FRotator::ZeroRotator, float NewTrueAnomaly = 0.0f)
	{
		Periapsis = NewPeriapsis;
		Apoapsis = NewApoapsis;
		Orientation = NewOrientation;
		InitialTrueAnomaly = NewTrueAnomaly;

		UpdateOrbitData();
	}

	FKeplerOrbitConfig(const FKeplerOrbitConfig& Other)
	{
		Periapsis = Other.Periapsis;
		Apoapsis = Other.Apoapsis;
		Orientation = Other.Orientation;
		InitialTrueAnomaly = Other.InitialTrueAnomaly;

		UpdateOrbitData();
	}

	bool IsValid() const;

	void FixOrbitConfig();

	void UpdateOrbitData();

	bool Equals(const FKeplerOrbitConfig& Other) const;

public:

	bool operator==(const FKeplerOrbitConfig& Other) const
	{
		const bool bIsEqual = Equals(Other);
		return bIsEqual;
	}

	bool operator!=(const FKeplerOrbitConfig& Other) const
	{
		const bool bIsEqual = Equals(Other);
		return !bIsEqual;
	}

	uint32 GetTypeHash(const FKeplerOrbitConfig& Other)
	{
		uint32 Hash = FCrc::MemCrc32(&Other, sizeof(FKeplerOrbitConfig));
		return Hash;
	}
};

UCLASS()
class PORTFOLIO_API UKeplerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kepler Orbit", meta = (DisplayName = "Is Orbit Valid"))
	static bool IsOrbitValid(const FKeplerOrbitConfig& OrbitConfig);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Fix Orbit Config"))
	static void FixOrbitConfig(UPARAM(ref) FKeplerOrbitConfig& OrbitConfig);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Update Orbit Data"))
	static void UpdateOrbitData(UPARAM(ref) FKeplerOrbitConfig& OrbitConfig);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kepler Orbit", meta = (DisplayName = "Equals"))
	static bool Equals(const FKeplerOrbitConfig& A, const FKeplerOrbitConfig& B);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Mean Anomaly"))
	static float GetMeanAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float Time);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Eccentric Anomaly"))
	static float GetEccentricAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float MeanAnomaly);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get True Anomaly"))
	static float GetTrueAnomaly(const FKeplerOrbitConfig& OrbitConfig, const float EccentricAnomaly);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Orbital Position w/ True Anomaly"))
	static FVector GetOrbitalPositionTrue(const FKeplerOrbitConfig& OrbitConfig, const float TrueAnomaly);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Orbital Position w/ Eccentric Anomaly"))
	static FVector GetOrbitalPositionEcc(const FKeplerOrbitConfig& OrbitConfig, const float EccentricAnomaly);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Orbit Points"))
	static void GetOrbitPoints(const FKeplerOrbitConfig& OrbitConfig, const int NumOfPoints, TArray<FVector>& OutPoints);

	UFUNCTION(BlueprintCallable, Category = "Kepler Orbit", meta = (DisplayName = "Get Opposite Focus"))
	static FKeplerOrbitConfig GetOppositeFocus(const FKeplerOrbitConfig& OrbitConfig);
};
