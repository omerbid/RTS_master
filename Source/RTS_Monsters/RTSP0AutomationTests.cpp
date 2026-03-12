// Copyright RTS_Monsters. P0 automation tests.
// Run from Editor: Session Frontend -> Automation tab, filter "RTS_Monsters.P0"

#include "Misc/AutomationTest.h"
#include "RTSCommandAuthorityComponent.h"
#include "RTSDataTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#define RTS_MONSTERS_P0_TEST_CATEGORY "RTS_Monsters.P0"

// P0.4.1 - Hero default command radius is 2500 UU (GDD)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRTSP0_CommandRadiusDefault,
	RTS_MONSTERS_P0_TEST_CATEGORY ".CommandAuthority.HeroDefaultRadius",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
)

bool FRTSP0_CommandRadiusDefault::RunTest(const FString& Parameters)
{
	const URTSCommandAuthorityComponent* CDO = GetDefault<URTSCommandAuthorityComponent>();
	if (!TestNotNull(TEXT("CommandAuthorityComponent CDO"), CDO))
	{
		return false;
	}
	const float ExpectedRadius = 2500.f;
	TestEqual(TEXT("Hero default CommandRadius (GDD)"), CDO->CommandRadius, ExpectedRadius);
	return true;
}

// Sanity: P0 data types exist (EFactionId has expected count)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRTSP0_DataTypesFactionId,
	RTS_MONSTERS_P0_TEST_CATEGORY ".DataTypes.EFactionId",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
)

bool FRTSP0_DataTypesFactionId::RunTest(const FString& Parameters)
{
	// GDD: 3 factions - Humans, Vampires, Werewolves
	const UEnum* FactionEnum = StaticEnum<EFactionId>();
	if (!TestNotNull(TEXT("EFactionId enum"), FactionEnum))
	{
		return false;
	}
	const int32 NumFactions = FactionEnum->NumEnums() - 1; // -1 for _Max
	TestEqual(TEXT("EFactionId has 3 factions (GDD)"), NumFactions, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
