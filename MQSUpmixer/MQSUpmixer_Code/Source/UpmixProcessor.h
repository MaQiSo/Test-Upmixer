/*
  ==============================================================================

    UpmixProcessor.h

    Author:  Qisong Ma

	UpmixProcessor header file, included in PluginProcessor.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "OverlappingFFTProcessor.h"

class UpmixProcessor : public OverlappingFFTProcessor
{
public:
	UpmixProcessor();
	~UpmixProcessor();

	void declareHeapSize();

	void setDecorrelationFilter();
	void randLFOGen(float *y, float rate, bool leftright);

	void setParameters(std::atomic<float>* _ambientGaindB, std::atomic<float>* _mainGaindB,
		std::atomic<float>* _decorrelation, std::atomic<float>* _frontness);

private:
	void processFrameInBuffer(const int maxNumChannels) override;

	HeapBlock<dsp::Complex<float>> freqDomainBuffer_left;
	HeapBlock<dsp::Complex<float>> freqDomainBuffer_right;

	HeapBlock<dsp::Complex<float>> HAR;
	HeapBlock<dsp::Complex<float>> HAL;
	HeapBlock<float> HFr;
	HeapBlock<float> HRe;

	HeapBlock<dsp::Complex<float>> L;
	HeapBlock<dsp::Complex<float>> C;
	HeapBlock<dsp::Complex<float>> R;
	HeapBlock<dsp::Complex<float>> Ls;
	HeapBlock<dsp::Complex<float>> Rs;

	float ambientGain;
	float mainGain;
	float decorrelation;
	float frontness;
};