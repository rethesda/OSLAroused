#include "ArousalSystemSLA.h"
#include "PersistedData.h"
#include "Papyrus/Papyrus.h"
#include "Settings.h"
#include "Utilities/Utils.h"

using namespace PersistedData;

float GetDaysSinceLastOrgasm(RE::Actor* actorRef)
{
	float lastOrgasmTime = PersistedData::LastOrgasmTimeData::GetSingleton()->GetData(actorRef->formID, 0.f);
	if (lastOrgasmTime < 0) {
		lastOrgasmTime = 0;
	}

	return RE::Calendar::GetSingleton()->GetCurrentGameTime() - lastOrgasmTime;
}

float ArousalSystemSLA::GetArousal(RE::Actor* actorRef, bool bUpdateState)
{
	//@TODO: If arousal blocked or is child, abort

	//@TODO: If arousal Locked, abort

	float newArousal = (GetDaysSinceLastOrgasm(actorRef) * GetLibido(actorRef)) + GetExposure(actorRef);
	if (newArousal < 0) {
		newArousal = 0;
	}
	else if (newArousal > 100) {
		newArousal = 100;
	}
	
	if (bUpdateState) {
		Papyrus::Events::SendActorArousalUpdatedEvent(actorRef, newArousal);
	}

	logger::debug("Got Arousal for {} val: {}", actorRef->GetDisplayFullName(), newArousal);
	logger::debug("Debug Values: Libido: {} Exposure: {} DaysSinceLast: {}", GetLibido(actorRef), GetExposure(actorRef), GetDaysSinceLastOrgasm(actorRef));

	//@TODO: Handle player updates

	//@TODO: Update Most aroused Actor IN Location

	return newArousal;
}


//SetArousal is actually "SetExposure" (since arousal is calculated)
float ArousalSystemSLA::SetArousal(RE::Actor* actorRef, float value, bool bSendEvent)
{
	value = std::clamp(value, 0.0f, 100.f);
	//@TODO: Update Faction Rank
	ArousalData::GetSingleton()->SetData(actorRef->formID, value);
	//Also update LastCheckTime (Which represents ExposureDate)
	LastCheckTimeData::GetSingleton()->SetData(actorRef->formID, RE::Calendar::GetSingleton()->GetCurrentGameTime());

	//Trigger arousal to update (Also handled sending the events)
	if (bSendEvent) {
		GetArousal(actorRef, bSendEvent);
	}

	return value;
}

//Arousal = Exposure
float ArousalSystemSLA::ModifyArousal(RE::Actor* actorRef, float value, bool bSendEvent)
{
	//@TODO: If arousal Locked, abort

	float modifiedValue = value * GetArousalMultiplier(actorRef);
	float exposure = GetExposure(actorRef);
	float timeRateHalfLife = Settings::GetSingleton()->GetTimeRateHalfLife();
	if (timeRateHalfLife > 0.1)
	{
		float timeSinceUpdate = RE::Calendar::GetSingleton()->GetCurrentGameTime() - LastCheckTimeData::GetSingleton()->GetData(actorRef->formID, 0.f);
		exposure = exposure * std::pow(1.5, -timeSinceUpdate / timeRateHalfLife) + modifiedValue;
	}

	logger::debug("ModifyArousal: {} + {} = {}", exposure - modifiedValue, modifiedValue, exposure);

	return SetArousal(actorRef, exposure, bSendEvent);
}

float ArousalSystemSLA::GetExposure(RE::Actor* actorRef)
{
	float exposure = ArousalData::GetSingleton()->GetData(actorRef->formID, -2.f);
	
	//Roll Initial Exposure
	if (exposure < -1.f)
	{
		exposure = Utilities::GenerateRandomFloat(0.f, 50.f);
		SetArousal(actorRef, exposure, false);
	}

	float timeRateHalfLife = Settings::GetSingleton()->GetTimeRateHalfLife();
	if (timeRateHalfLife > 0.1)
	{
		float timeSinceUpdate = RE::Calendar::GetSingleton()->GetCurrentGameTime() - LastCheckTimeData::GetSingleton()->GetData(actorRef->formID, 0.f);
		exposure = exposure * std::pow(1.5, -timeSinceUpdate / timeRateHalfLife);
	}

	//@TODO: UPdate Exposure Faction
	
	return exposure;
}

//In SLA, Libido is "TimeRate" 
float ArousalSystemSLA::GetLibido(RE::Actor* actorRef)
{
	float timeRateHalfLife = Settings::GetSingleton()->GetTimeRateHalfLife();
	if (timeRateHalfLife <= 0.1)
	{
		return 10.f;
	}

	float timeRate = BaseLibidoData::GetSingleton()->GetData(actorRef->formID, 10.f);
	float daysSinceLastOrgasm = GetDaysSinceLastOrgasm(actorRef);

	timeRate = timeRate * std::pow(1.5, -daysSinceLastOrgasm / timeRateHalfLife);

	//@TODO: Update TimeRate Faction
	return timeRate;
}

float ArousalSystemSLA::SetLibido(RE::Actor* actorRef, float value)
{
	value = std::clamp(value, 0.0f, 100.f);
	BaseLibidoData::GetSingleton()->SetData(actorRef->formID, value);
	return value;
}

float ArousalSystemSLA::ModifyLibido(RE::Actor* actorRef, float value)
{
	float val = GetLibido(actorRef) + value;
	return SetLibido(actorRef, val);
}


const RE::FormID kVoiceTypeFemaleSultryFormId = 0x13AE0;
const RE::FormID kVoiceTypeFemaleYoungEagerFormId = 0x13ADC;
const RE::FormID kVoiceTypeMaleYoungEagerFormId = 0x13AD1;
const RE::FormID kVoiceTypeMaleDrunkFormId = 0x13AD4;

const std::vector<RE::FormID> kArousedVoiceTypes = { kVoiceTypeFemaleSultryFormId, kVoiceTypeFemaleYoungEagerFormId, kVoiceTypeMaleYoungEagerFormId, kVoiceTypeMaleDrunkFormId };

const RE::FormID kVoiceTypeFemaleOldGrumpyFormId = 0x13AE2;
const RE::FormID kVoiceTypeFemaleOldKindlyFormId = 0x13AE1;
const RE::FormID kVoiceTypeMaleOldGrumpyFormId = 0x13AD7;
const RE::FormID kVoiceTypeMaleOldKindlyFormId = 0x13AD6;

const std::vector<RE::FormID> kUnarousedVoiceTypes = { kVoiceTypeFemaleOldGrumpyFormId, kVoiceTypeFemaleOldKindlyFormId, kVoiceTypeMaleOldGrumpyFormId, kVoiceTypeMaleOldKindlyFormId };
const float kDefaultExposureRate = 2.f;

//In SLA, ArousalMultiplier is "ExposureRate"
float ArousalSystemSLA::GetArousalMultiplier(RE::Actor* actorRef)
{
	float exposureRate = ArousalMultiplierData::GetSingleton()->GetData(actorRef->formID, -1.f);

	//If not set, roll initial value
	if (exposureRate < 0)
	{
		auto voiceType = actorRef->GetActorBase()->GetVoiceType();
		if (voiceType) {
			if (std::find(kArousedVoiceTypes.begin(), kArousedVoiceTypes.end(), voiceType->formID) != kArousedVoiceTypes.end()) {
				exposureRate = kDefaultExposureRate + 1.f;
			}
			else if (std::find(kUnarousedVoiceTypes.begin(), kUnarousedVoiceTypes.end(), voiceType->formID) != kUnarousedVoiceTypes.end()) {
				exposureRate = kDefaultExposureRate - 1.f;
			}
			else {
				exposureRate = kDefaultExposureRate;
			}
		}
		else {
			exposureRate = kDefaultExposureRate;
		}
	}

	return std::clamp(exposureRate, 0.f, 10.f);
}

//In SLA, ArousalMultiplier is "ExposureRate"
float ArousalSystemSLA::SetArousalMultiplier(RE::Actor* actorRef, float value)
{
	float res = value * 10;
	if (value < 0) {
		value = 0;
		res = 0;
	}
	else if (value > 10)
	{
		value = 10.f;
		res = 100.f;
	}

	//@TODO: SetFactionRank slaExposureRate
	ArousalMultiplierData::GetSingleton()->SetData(actorRef->formID, value);
	return value;
}

float ArousalSystemSLA::ModifyArousalMultiplier(RE::Actor* actorRef, float value)
{
	float res = GetArousalMultiplier(actorRef) + value;
	return SetArousalMultiplier(actorRef, res);
}

float ArousalSystemSLA::GetBaselineArousal(RE::Actor* actorRef)
{
	return GetExposure(actorRef);
}
