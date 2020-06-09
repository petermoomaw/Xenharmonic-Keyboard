#include "pch.h"
#include <sstream>

#include "Common.h"
#include "Instrument.h"

#include <ppltasks.h>   // For create_task

using namespace Concurrency;
using namespace Windows::Foundation;

//#include "LatticeView.h"


inline unsigned int getMidiChannel(unsigned int i)
{
  if(i >=9)  // MIDI chanel 10 is reserved for percussion
	  return i+1;
  else
    return i;
}

Instrument::Instrument()
{
  programs.push_back(L"1. Acoustic Grand Piano");
  programs.push_back(L"2. Bright Acoustic Piano");
  programs.push_back(L"3. Electric Grand Piano");
  programs.push_back(L"4. Honky - tonk Piano");
  programs.push_back(L"5. Electric Piano 1");
  programs.push_back(L"6. Electric Piano 2");
  programs.push_back(L"7. Harpsichord");
  programs.push_back(L"8. Clavi");
  programs.push_back(L"9. Celesta");
  programs.push_back(L"10. Glockenspiel");
  programs.push_back(L"11. Music Box");
  programs.push_back(L"12. Vibraphone");
  programs.push_back(L"13. Marimba");
  programs.push_back(L"14. Xylophone");
  programs.push_back(L"15. Tubular Bells");
  programs.push_back(L"16. Dulcimer");
  programs.push_back(L"17. Drawbar Organ");
  programs.push_back(L"18. Percussive Organ");
  programs.push_back(L"19. Rock Organ");
  programs.push_back(L"20. Church Organ");
  programs.push_back(L"21. Reed Organ");
  programs.push_back(L"22. Accordion");
  programs.push_back(L"23. Harmonica");
  programs.push_back(L"24. Tango Accordion");
  programs.push_back(L"25. Acoustic Guitar(nylon)");
  programs.push_back(L"26. Acoustic Guitar(steel)");
  programs.push_back(L"27. Electric Guitar(jazz) ");
  programs.push_back(L"28. Electric Guitar(clean)");
  programs.push_back(L"29. Electric Guitar(muted)");
  programs.push_back(L"30. Overdriven Guitar");
  programs.push_back(L"31. Distortion Guitar");
  programs.push_back(L"32. Guitar harmonics");
  programs.push_back(L"33. Acoustic Bass");
  programs.push_back(L"34. Electric Bass(finger)");
  programs.push_back(L"35. Electric Bass(pick)");
  programs.push_back(L"36. Fretless Bass");
  programs.push_back(L"37. Slap Bass 1");
  programs.push_back(L"38. Slap Bass 2");
  programs.push_back(L"39. Synth Bass 1");
  programs.push_back(L"40. Synth Bass 2");
  programs.push_back(L"41. Violin");
  programs.push_back(L"42. Viola");
  programs.push_back(L"43. Cello");
  programs.push_back(L"44. Contrabass");
  programs.push_back(L"45. Tremolo Strings");
  programs.push_back(L"46. Pizzicato Strings");
  programs.push_back(L"47. Orchestral Harp");
  programs.push_back(L"48. Timpani");
  programs.push_back(L"49. String Ensemble 1");
  programs.push_back(L"50. String Ensemble 2");
  programs.push_back(L"51. SynthStrings 1");
  programs.push_back(L"52. SynthStrings 2");
  programs.push_back(L"53. Choir Aahs");
  programs.push_back(L"54. Voice Oohs");
  programs.push_back(L"55. Synth Voice");
  programs.push_back(L"56. Orchestra Hit");
  programs.push_back(L"57. Trumpet");
  programs.push_back(L"58. Trombone");
  programs.push_back(L"59. Tuba");
  programs.push_back(L"60. Muted Trumpet");
  programs.push_back(L"61. French Horn");
  programs.push_back(L"62. Brass Section");
  programs.push_back(L"63. SynthBrass 1");
  programs.push_back(L"64. SynthBrass 2");
  programs.push_back(L"65. Soprano Sax");
  programs.push_back(L"66. Alto Sax");
  programs.push_back(L"67. Tenor Sax");
  programs.push_back(L"68. Baritone Sax");
  programs.push_back(L"69. Oboe");
  programs.push_back(L"70. English Horn");
  programs.push_back(L"71. Bassoon");
  programs.push_back(L"72. Clarinet");
  programs.push_back(L"73. Piccolo");
  programs.push_back(L"74. Flute");
  programs.push_back(L"75. Recorder");
  programs.push_back(L"76. Pan Flute");
  programs.push_back(L"77. Blown Bottle");
  programs.push_back(L"78. Shakuhachi");
  programs.push_back(L"79. Whistle");
  programs.push_back(L"80. Ocarina");
  programs.push_back(L"81. Lead 1 (square)");
  programs.push_back(L"82. Lead 2 (sawtooth)");
  programs.push_back(L"83. Lead 3 (calliope)");
  programs.push_back(L"84. Lead 4 (chiff)");
  programs.push_back(L"85. Lead 5 (charang)");
  programs.push_back(L"86. Lead 6 (voice)");
  programs.push_back(L"87. Lead 7 (fifths)");
  programs.push_back(L"88. Lead 8 (bass + lead)");
  programs.push_back(L"89. Pad 1 (new age)");
  programs.push_back(L"90. Pad 2 (warm)");
  programs.push_back(L"91. Pad 3 (polysynth)");
  programs.push_back(L"92. Pad 4 (choir)");
  programs.push_back(L"93. Pad 5 (bowed)");
  programs.push_back(L"94. Pad 6 (metallic)");
  programs.push_back(L"95. Pad 7 (halo)");
  programs.push_back(L"96. Pad 8 (sweep)");
  programs.push_back(L"97. FX 1 (rain)");
  programs.push_back(L"98. FX 2 (soundtrack)");
  programs.push_back(L"99. FX 3 (crystal)");
  programs.push_back(L"100. FX 4 (atmosphere)");
  programs.push_back(L"101. FX 5 (brightness)");
  programs.push_back(L"102. FX 6 (goblins)");
  programs.push_back(L"103. FX 7 (echoes)");
  programs.push_back(L"104. FX 8 (sci - fi)");
  programs.push_back(L"105. Sitar");
  programs.push_back(L"106. Banjo");
  programs.push_back(L"107. Shamisen");
  programs.push_back(L"108. Koto");
  programs.push_back(L"109. Kalimba");
  programs.push_back(L"110. Bag pipe");
  programs.push_back(L"111. Fiddle");
  programs.push_back(L"112. Shanai");
  programs.push_back(L"113. Tinkle Bell");
  programs.push_back(L"114. Agogo");
  programs.push_back(L"115. Steel Drums");
  programs.push_back(L"116. Woodblock");
  programs.push_back(L"117. Taiko Drum");
  programs.push_back(L"118. Melodic Tom");
  programs.push_back(L"119. Synth Drum");
  programs.push_back(L"120. Reverse Cymbal");
  programs.push_back(L"121. Guitar Fret Noise");
  programs.push_back(L"122. Breath Noise");
  programs.push_back(L"123. Seashore");
  programs.push_back(L"124. Bird Tweet");
  programs.push_back(L"125. Telephone Ring");
  programs.push_back(L"126. Helicopter");
  programs.push_back(L"127. Applause");
  programs.push_back(L"128. Gunshot");


	// Get a list of all MIDI devices
	create_task(DeviceInformation::FindAllAsync(MidiOutPort::GetDeviceSelector())).then([this](DeviceInformationCollection^ devices)
	{
		DeviceInformationCollection^ deviceInformationCollection = devices;

		// If no devices are found, update the ListBox
		if ((deviceInformationCollection == nullptr) || (deviceInformationCollection->Size == 0))
		{

		}
		// If devices are found, enumerate them and add them to the list
		else
		{
			unsigned int i = 0;
			for (; i < devices->Size; i++)
			{
				DeviceInformation^ di = devices->GetAt(i);
				if (di != nullptr && di->Name == "Microsoft GS Wavetable Synth")
				{
					create_task(MidiOutPort::FromIdAsync(di->Id)).then([this](IMidiOutPort^ port)
					{
						this->device = port;
            setPichBendSize(lrint(maxPitchBendCents / 100));
					});
				}
			}

			for (i=0 ; i < devices->Size; i++)
			{
				DeviceInformation^ di = devices->GetAt(i);
				if (di != nullptr)
				{
					create_task(MidiOutPort::FromIdAsync(di->Id)).then([this](IMidiOutPort^ port)
					{
						this->device = port;
            setPichBendSize(lrint(maxPitchBendCents / 100));
					});
				}
			}
		}
	});
}


Instrument::~Instrument()
{
//	ud->PERF_STATUS = 0;
//	csoundCleanup(ud.csound);
//    csoundDestroy(ud.csound);
}

void Instrument::setPichBendSize(int semiTones)
{
  maxPitchBendCents = semiTones *100;

  for (int channel = 0; channel < NumChanels; channel++)
  {
    device->SendMessage(ref new MidiControlChangeMessage(channel, 101, 0));
    device->SendMessage(ref new MidiControlChangeMessage(channel, 100, 0));
    device->SendMessage(ref new MidiControlChangeMessage(channel, 6, semiTones));
    device->SendMessage(ref new MidiControlChangeMessage(channel, 101, 127));
    device->SendMessage(ref new MidiControlChangeMessage(channel, 100, 127));
  }
}


void Instrument::cleanup()
{
	for (int i = 0; i < NumChanels; i++)
	{
//		noteInfos[i].freqPtr= 0;
		noteInfos[i].on = 0;
	}
}


int Instrument::getMidiNoteFromFreq(double freq)
{
	return 69 + lrint(centsFromFrequance(freq) / 100);    // MIDI note-on message: Key number (60 = middle C) 69 = A4
}

double Instrument::centsFromFrequance(double freq)
{
	return 1200 * log2(freq / 440);
}

int Instrument::playNote(float freqIn)
{
	int noteID = getFingerIdAssignment();
//	PRINT(_T("playNote: noteID  = %d\n"), noteID);

	if (noteID != -1)
	{
		if (noteInfos[noteID].on != 0)
		{
			PRINT(_T("playNote: noteID[%d] != 0"), noteID);
		}
		else // if (noteInfos[noteID].freqPtr)
		{
			noteInfos[noteID].on = 1;
//			*noteInfos[noteID].freqPtr = freqIn;

			if (device)
			{	
				noteInfos[noteID].velocity = velocity;
				double cents = centsFromFrequance(freqIn);
				noteInfos[noteID].midiNote = 69 + lrint(cents/ 100);    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				if (noteInfos[noteID].midiNote < 0 || noteInfos[noteID].midiNote > 127)
				{
					noteInfos[noteID].on = 0;
					return -1;
				}
					

				unsigned int bend = lrint((cents - (noteInfos[noteID].midiNote- 69) * 100) * 8192.0 / maxPitchBendCents + 8192);

				//unsigned int bend = 8192; // This is zero pitch bend
                
				//union { unsigned long word; unsigned char data[4]; } message;
				//message.data[0] = 0b11100000 + noteID;  //1001nnnn is pitchbend for channel nnnn
				//message.data[1] = 0b01111111 & bend;   
				//message.data[2] = 0b01111111 & (bend >> 7);
				//message.data[3] = 0;     // Unused parameter
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}

				device->SendMessage(ref new MidiPitchBendChangeMessage(getMidiChannel(noteID), bend));		

				//message.data[0] = 0b10010000 + noteID;  //1001nnnn is note on for chanel nnnn
				//message.data[1] = noteInfos[noteID].midiNote;    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				//message.data[2] = noteInfos[noteID].velocity;   // MIDI note-on message: Key velocity (100 = loud)
				//message.data[3] = 0;     // Unused parameter

				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}

				device->SendMessage(ref new MidiNoteOnMessage(getMidiChannel(noteID), noteInfos[noteID].midiNote, noteInfos[noteID].velocity));
			}
		}
		//else
		//{
		//	PRINT(_T("playNote: freqPtr[%d] == 0"), noteID);
		//}
	}

	return noteID;
}

void Instrument::updateNoteFreq(int noteID, float freqIn)
{
	if (noteInfos[noteID].on != 1)
	{
		PRINT(_T("updateNoteFreq: fingerIds[%d] != 1\n"), noteID);
	}
	else
	{
//		*(noteInfos[noteID].freqPtr) = freqIn;

		if(device)
		{
			double cents = centsFromFrequance(freqIn);

		//	double crap = (cents - (noteInfos[noteID].midiNote - 69) * 100) * 8192.0 / 200.0;
			int bend = lrint((cents - (noteInfos[noteID].midiNote - 69) * 100) * 8192.0 / maxPitchBendCents + 8192);

			if (bend < 0)
				bend = 0;
			else if (bend >= 8192 * 2 )
				bend = 8192 * 2 - 1;

			if (bend >= 8192 *2 || bend < 0)
			{
				int oldNote = noteInfos[noteID].midiNote;
				noteInfos[noteID].midiNote = 69 + lrint(cents / 100);    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				bend = lrint((cents - (noteInfos[noteID].midiNote - 69) * 100) * 8192.0 / maxPitchBendCents + 8192);

				//union { unsigned long word; unsigned char data[4]; } message;
				//message.data[3] = 0;     // Unused parameter


				//message.data[0] = 0b10110000 + getMidiChanel(noteID);  //1100nnnn  program chance for chanel nnnn
				//message.data[1] = 126;   // Mono
				//message.data[2] = 0;
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				////		throw "Warning: MIDI Output is not open.\n";
				//}


				//message.data[0] = 0b10110000 + getMidiChanel(noteID);  //1100nnnn  program chance for chanel nnnn
				//message.data[1] = 68;   // Legato On/off
				//message.data[2] = 100;
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//		throw "Warning: MIDI Output is not open.\n";
				//}


				//message.data[0] = 0b10110000 + getMidiChanel(noteID);  //1100nnnn  program chance for chanel nnnn
				//message.data[1] = 5;   // Portimento time
				//message.data[2] = 100;
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//		throw "Warning: MIDI Output is not open.\n";
				//}

				//message.data[0] = 0b10110000 + getMidiChanel(noteID);  //1100nnnn  program chance for chanel nnnn
				//message.data[1] = 65;   // Portimento On/off
				//message.data[2] = 100;
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//		throw "Warning: MIDI Output is not open.\n";
				//}

				//message.data[0] = 0b10110000 + getMidiChanel(noteID);  //1100nnnn  program chance for chanel nnnn
				//message.data[1] = 84;   // Amount of Portimento
				//message.data[2] = oldNote;
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//		throw "Warning: MIDI Output is not open.\n";
				//}

				
			


				//message.data[0] = 0b11100000 + noteID;  //1001nnnn is pitchbend for channel nnnn
				//message.data[1] = 0b01111111 & bend;
				//message.data[2] = 0b01111111 & (bend >> 7);
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}


				device->SendMessage(ref new MidiPitchBendChangeMessage(getMidiChannel(noteID), bend));

				//// message.data[0] = command byte of the MIDI message, for example: 0x90
				//// message.data[1] = first data byte of the MIDI message, for example: 60
				//// message.data[2] = second data byte of the MIDI message, for example 100
				//// message.data[3] = not used for any MIDI messages, so set to 0
				////message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
				//message.data[0] = 0b10010000 + noteID;  //1001nnnn is note on for chanel nnnn
				//message.data[1] = noteInfos[noteID].midiNote;    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				//message.data[2] = noteInfos[noteID].velocity;   // MIDI note-on message: Key velocity (100 = loud)
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}

				device->SendMessage(ref new MidiNoteOnMessage(getMidiChannel(noteID), noteInfos[noteID].midiNote, noteInfos[noteID].velocity));

				//message.data[0] = 0b10000000 + noteID;  //1000nnnn is note off for chanel nnnn
				//message.data[1] = oldNote;    // MIDI note-on message: Key number (60 = middle C) 69 = A4
				//message.data[2] = noteInfos[noteID].velocity;   // MIDI note-on message: Key velocity (100 = loud)
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}
			}
			else
			{
				//unsigned int bend = 8192;  // This is zero pitch bend

				//union { unsigned long word; unsigned char data[4]; } message;
				//message.data[0] = 0b11100000 + noteID;  //1001nnnn is pitchbend for channel nnnn
				//message.data[1] = 0b01111111 & bend;
				//message.data[2] = 0b01111111 & (bend >> 7);
				//message.data[3] = 0;     // Unused parameter
				//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
				//{
				//	//	throw "Warning: MIDI Output is not open.\n";
				//}

				device->SendMessage(ref new MidiPitchBendChangeMessage(getMidiChannel(noteID), bend));
			}
		}
	}
}

void Instrument::stopNote(int noteID)
{
//	PRINT(_T("stopNote: fingerId  = %d\n"), noteID);

	if (noteInfos[noteID].on != 1)
	{
		PRINT(_T("stopNote: fingerIds[%d] != 1\n"), noteID);
	}
	else
	{
		noteInfos[noteID].on = 0;

		if (device)
		{
			//union { unsigned long word; unsigned char data[4]; } message;
			//// message.data[0] = command byte of the MIDI message, for example: 0x90
			//// message.data[1] = first data byte of the MIDI message, for example: 60
			//// message.data[2] = second data byte of the MIDI message, for example 100
			//// message.data[3] = not used for any MIDI messages, so set to 0
			////message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
			//message.data[0] = 0b10000000 + noteID;  //1000nnnn is note off for chanel nnnn

			//message.data[1] = noteInfos[noteID].midiNote;    // MIDI note-on message: Key number (60 = middle C) 69 = A4
			//message.data[2] = noteInfos[noteID].velocity;   // MIDI note-on message: Key velocity (100 = loud)
			//message.data[3] = 0;     // Unused parameter


			//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
			//{
			//	//	throw "Warning: MIDI Output is not open.\n";
			//}

			device->SendMessage(ref new MidiNoteOffMessage(getMidiChannel(noteID), noteInfos[noteID].midiNote, noteInfos[noteID].velocity));
		}
	}
}


int Instrument::getFingerIdAssignment()
{
	for (int i = 0; i < NumChanels; i++)
	{
		if (noteInfos[i].on == 0)
		{
			return i;
		}
	}
	return -1;
}


void Instrument::updateMidiProgram()
{
	for (unsigned int i = 0; i < NumChanels;i++)
	{
		//union { unsigned long word; unsigned char data[4]; } message;
		////// message.data[0] = command byte of the MIDI message, for example: 0x90
		////// message.data[1] = first data byte of the MIDI message, for example: 60
		////// message.data[2] = second data byte of the MIDI message, for example 100
		////// message.data[3] = not used for any MIDI messages, so set to 0

		//message.data[0] = 0b10110000 + getMidiChannel(i);  //1100nnnn  control change for chanel nnnn
		//message.data[1] = 0;     //Bank Select MSB
		//message.data[2] = midi_bank_MSB;
		//message.data[3] = 0;     // Unused parameter
		//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
		//{
		////	throw "Warning: MIDI Output is not open.\n";
		//}

		if(midi_bank_MSB >=0 && midi_bank_MSB <= 127)
		  device->SendMessage(ref new MidiControlChangeMessage(getMidiChannel(i), 0, midi_bank_MSB));

		//message.data[0] = 0b10110000 + getMidiChannel(i);  //1100nnnn  control change for chanel nnnn
		//message.data[1] = 32;   //Bank Select LSB
		//message.data[2] = midi_bank_LSB;
		//if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
		//{
		//	//		throw "Warning: MIDI Output is not open.\n";
		//}

		if (midi_bank_LSB >= 0 && midi_bank_LSB <= 127)
		  device->SendMessage(ref new MidiControlChangeMessage(getMidiChannel(i), 32, midi_bank_LSB));

//		message.data[0] = 0b11000000 + getMidiChannel(i);  //1100nnnn  program chance for chanel nnnn
//		message.data[1] = midi_program;
//		message.data[2] = 0;   // Unsused
//		if (midiOutShortMsg(device, message.word) != MMSYSERR_NOERROR)
//		{
////			throw "Warning: MIDI Output is not open.\n";
//		}

		if (midi_program >= 0 && midi_program <= 127)
		  device->SendMessage(ref new MidiProgramChangeMessage(getMidiChannel(i), midi_program));
	}

  setPichBendSize(lrint(maxPitchBendCents/100));
}

void Instrument::openMidiPort()
{
	//if (device)
	//	return;

	////// Open the MIDI output port
	////if (midiOutOpen(&device, midiport, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	////{
	////	midiport = -1;
	////}

	//create_task(MidiOutPort::FromIdAsync(midiDeviceId)).then([this](IMidiOutPort^ port)
	//{
	//	device = port;
	//});
  
}

void Instrument::closeMidiPort()
{
	if (device == nullptr)
		return;

	
	//// Close the MIDI output port
	//if (midiOutClose(device) != MMSYSERR_NOERROR)
	//{
	////	midiport = -1;
	//}

//	device->Close();
	device = nullptr;
}
