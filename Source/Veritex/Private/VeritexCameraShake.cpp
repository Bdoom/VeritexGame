// Daniel Gleason (C) 2017

#include "VeritexCameraShake.h"
#include "Veritex.h"

UVeritexCameraShake::UVeritexCameraShake()
{
	OscillationDuration = .25;
	RotOscillation.Pitch.Amplitude = 5.f;
	RotOscillation.Pitch.Frequency = 50.f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Yaw.Amplitude = 5.f;
	RotOscillation.Yaw.Frequency = 50.f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	LocOscillation.X.Amplitude = 100.f;
	LocOscillation.X.Frequency = 1.f;
	LocOscillation.X.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

	LocOscillation.Z.Amplitude = 10.f;
	LocOscillation.Z.Frequency = 50.f;
	LocOscillation.Z.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
}
