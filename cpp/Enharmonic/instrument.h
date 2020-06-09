#pragma once
#include <iostream>

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Midi;

using namespace std;

#define NumChanels 10


//userData structure declaration
struct NoteInfo {
	int on;
	int midiNote;
	int velocity;
//	double* freqPtr;

};

class Instrument
{
public:
	Instrument();
	~Instrument();
  vector<wstring> programs;
	void stopNote(int noteID);
	int playNote(float freq);
	void updateNoteFreq(int noteID, float freqIn);
	void cleanup();

	wstring instrumentFile;

	//  MIDI Stuff
	unsigned char midi_program = 0;
	unsigned char midi_bank_MSB = 0;
	unsigned char midi_bank_LSB = 0;
	int velocity = 100; // MIDI note velocity parameter value 
  double maxPitchBendCents = 1200*4;
//  double maxPitchBendCents = 200;

	IMidiOutPort^  device;    // MIDI device interface for sending MIDI output

	void updateMidiProgram();
	void openMidiPort();
	void closeMidiPort();
	int getMidiNoteFromFreq(double freq);
  void setPichBendSize(int semiTones);
private:

	NoteInfo noteInfos[NumChanels];

	int getFingerIdAssignment();

	double centsFromFrequance(double freq);
};
