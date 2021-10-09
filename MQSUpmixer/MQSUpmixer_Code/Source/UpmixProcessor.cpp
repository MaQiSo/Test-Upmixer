/*
  ==============================================================================

    UpmixProcessor.cpp

    Author:  Qisong Ma

	UpmixProcessor function definition

  ==============================================================================
*/

#include "UpmixProcessor.h"

//==============================================================================
UpmixProcessor::UpmixProcessor() : OverlappingFFTProcessor(10, 2), 
				ambientGain(1.0f), mainGain(1.0f), decorrelation(1.0f), frontness(0.0f)
{
}

//==============================================================================
UpmixProcessor::~UpmixProcessor()
{
}

/// initialize heap size, use in PluginProcessor constructor
void UpmixProcessor::declareHeapSize()
{
	freqDomainBuffer_left.realloc(fftSize / 2 + 1);
	freqDomainBuffer_left.clear(fftSize / 2 + 1);
	freqDomainBuffer_right.realloc(fftSize / 2 + 1);
	freqDomainBuffer_right.clear(fftSize / 2 + 1);

	HAR.realloc(fftSize / 2 + 1);
	HAR.clear(fftSize / 2 + 1);
	HAL.realloc(fftSize / 2 + 1);
	HAL.clear(fftSize / 2 + 1);
	HFr.realloc(fftSize / 2 + 1);
	HFr.clear(fftSize / 2 + 1);
	HRe.realloc(fftSize / 2 + 1);
	HRe.clear(fftSize / 2 + 1);

	L.realloc(fftSize / 2 + 1);
	L.clear(fftSize / 2 + 1);
	C.realloc(fftSize / 2 + 1);
	C.clear(fftSize / 2 + 1);
	R.realloc(fftSize / 2 + 1);
	R.clear(fftSize / 2 + 1);
	Ls.realloc(fftSize / 2 + 1);
	Ls.clear(fftSize / 2 + 1);
	Rs.realloc(fftSize / 2 + 1);
	Rs.clear(fftSize / 2 + 1);
}

//==============================================================================
/// initialise decorrelation filter
/// use in PluginProcessor constructor. Use after declareHeapSize()!
void UpmixProcessor::setDecorrelationFilter()
{
	float decorFiltRate = 80.0f;
	float* rand1 = new float[(fftSize/2)+1];
	float* rand2 = new float[(fftSize/2)+1];

	UpmixProcessor::randLFOGen(rand1,decorFiltRate,true);
	UpmixProcessor::randLFOGen(rand2,decorFiltRate,false);

	for (int k = 0; k <= fftSize/2; k++)
	{
		// decorrelation for Left & Right
		HAR[k].imag(rand1[k]);
		HAR[k].real(0.0f);
		HAL[k].real(1.0f-rand1[k]);
		HAL[k].imag(0.0f);

		// decorrelation for Front & Rear
		HRe[k] = rand2[k];
	}

	delete[] rand1;
	rand1 = nullptr;
	delete[] rand2;
	rand2 = nullptr;
}

//==============================================================================
/// used to initialise decorrelation filter,
/// generate an array of coefficient between 0 and 1, average at 0.5,
/// array length = halfFFTSize, take in osc rate, leftright to set generation 
/// mode, true for left-right decorrelation, false for front-rear
void UpmixProcessor::randLFOGen(float *y, float rate, bool leftright)
{
	auto maxFreqScale = fftSize / rate;
	auto minFreqScale = 0.2f * maxFreqScale;
	auto freqScale = 0.0f;
	auto countA = 0.0f;
	auto lastA = 0.0f;
	auto newA = 0.0f;
	auto currentA = 0.0f;
	auto stepA = 0.0f;
	auto phase = 0.0f;
	auto ampScale = 0.6f;
	auto newAmpScale = 0.6f;
	auto ampScaleRamp = exp2f(60.0f / fftSize) - 1.0f;
	auto intensivity = 0.6f;

	bool lastSign = false;

	// random seed
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (int n = 0; n <= (fftSize/2); ++n)
	{
		if (countA == 0.0f || countA >= freqScale)
		{
			// generate new random freq scale factor
			freqScale = floorf(maxFreqScale * std::rand() / RAND_MAX);
			// limit it to prevent rapid transitions
			freqScale = std::fmaxf(freqScale, minFreqScale);
			// generate new step coefficient
			auto newA = 0.1f + 0.9f * std::rand() / RAND_MAX;
			stepA = (newA - lastA) / freqScale;
			currentA = lastA;
			lastA = newA;
		}
		countA++;

		// clamping decorrelation in high-frequency
		auto clamp{ 1.0f };
		if (n < 45)
			clamp = log10f(0.2 * n + 1.0f);
		else if (leftright && n > 0.25f * fftSize)
			clamp = 1.0f - 0.398f * log10f(n - 0.25f * fftSize + 1.0f);
		else if(!leftright && n > 0.125f * fftSize)
			clamp = 1.0f - 0.368f * log10f(n - 0.125f * fftSize + 1.0f);

		// generate ouput
		y[n] = sinf(phase) * ampScale * 0.5f * clamp + 0.5f;
		// ramp amplitude
		ampScale += (newAmpScale - ampScale) * ampScaleRamp;
		currentA += stepA;
		// increment phase
		phase += 2.0f * MathConstants<float>::pi * rate * currentA / fftSize;
		if (phase >= 2 * MathConstants<float>::pi)
		{
			phase -= 2 * MathConstants<float>::pi;
		}

		// set new scale at zero crossing point
		if (y[n] == 0.5f || (y[n] > 0.5f) == lastSign)
		{
			lastSign = (y[n] > 0.5f);
			newAmpScale = 0.4 + 0.6 * std::rand() / RAND_MAX;
		}
	}

}

//==============================================================================
/// override parent's function, processing upmix
void UpmixProcessor::processFrameInBuffer(const int maxNumChannels)
{
	auto channelData_left = fftInOutBuffer.getWritePointer(0);
	auto channelData_right = fftInOutBuffer.getWritePointer(1);

	float* const ChannelDataOut_1 = fftInOutBuffer.getWritePointer(0);
	float* const ChannelDataOut_2 = fftInOutBuffer.getWritePointer(1);
	float* const ChannelDataOut_3 = fftInOutBuffer.getWritePointer(2);
	float* const ChannelDataOut_4 = fftInOutBuffer.getWritePointer(3);
	float* const ChannelDataOut_5 = fftInOutBuffer.getWritePointer(4);

	fft->performRealOnlyForwardTransform(channelData_left, true);
	fft->performRealOnlyForwardTransform(channelData_right, true);

	for (int sample = 0; sample <= (fftSize / 2); ++sample)
	{
		// copy real part value for freqDomain buffer
		freqDomainBuffer_left[sample].real(channelData_left[2 * sample]);
		freqDomainBuffer_right[sample].real(channelData_right[2 * sample]);

		// copy imagine part value for freqDomain buffer
		freqDomainBuffer_left[sample].imag(channelData_left[2 * sample + 1]);
		freqDomainBuffer_right[sample].imag(channelData_right[2 * sample + 1]);
	}

	// define constants
	// sin(pi/6) = 0.5 | cos(pi/6) = 0.8660254 speaker angle
	// 3^0.5 = 1.7320508
	const auto sqrt3 = 1.7320508f;
	const auto halfsqrt2 = 0.70710678f;
	const auto sqrt2 = 1.4142136f;
	// frequency bins loop:
	//============================================================
	for (int sample = 0; sample <= (fftSize / 2); ++sample)
	{
		//================================================
		// apply decorrelation parameters
		auto hal = HAL[sample];
	    hal.real((HAL[sample].real() - 0.5f) * decorrelation + 0.5f);
		auto har = HAR[sample];
		har.imag((HAR[sample].imag() - 0.5f) * decorrelation + 0.5f);
		auto hre = (HRe[sample] - 0.5f) * decorrelation + 0.5f;

		// apply frontness parameters (high-cut filter for rear channel & high-boost for front)
		hre -= (sample > 0.125f * fftSize ? 
			frontness * 0.16f * log10f(sample - 0.125f * fftSize + 1.0f) : 0.0f);
		auto hfr = 1.0f - hre;

		//================================================
		// calculation of Panning Coefficients
		// LOW-COMPX...equation(10)(11)
		auto aXL = abs(freqDomainBuffer_left[sample]);
		auto aXR = abs(freqDomainBuffer_right[sample]);
		auto aLR = abs(freqDomainBuffer_left[sample] + freqDomainBuffer_right[sample]);
		auto sourceGain = pow((aLR / (aXL + aXR + FLT_MIN)), 0.5f);

		auto pXL = aXL * aXL;
		auto pXR = aXR * aXR;

		auto gL = sqrt(pXL / (pXL + pXR + FLT_MIN));
		auto gR = sqrt(pXR / (pXL + pXR + FLT_MIN));

		//================================================
		// calculation of Source Component Angle
		// LOW-COMPX...equation(12)
		auto psai = (gR - gL) / (gL + gR + FLT_MIN);

		//================================================
		// calculation of Source Component & Ambience Component
		// LOW-COMPX...equation(15)(16)
		// calculation of common denominator

		std::complex<float> denom = gL * har - gR * hal;

		std::complex<float> source = (freqDomainBuffer_left[sample] * har 
			- freqDomainBuffer_right[sample] * hal) / (denom + FLT_MIN);
		
		std::complex<float> ambience = (gL * freqDomainBuffer_right[sample] 
			- gR * freqDomainBuffer_left[sample]) / (denom + FLT_MIN);

		// apply gain
		source *= mainGain;
		ambience *= ambientGain;

		//================================================
		// 2-3 upmix Source Component using VBAP
		// LOW-COMPX...equation(19)(20)(23)

		auto px = 0.5f * psai;
		auto py = sqrt(1.0f - 0.25f * psai * psai); // sqrt(1 - px^2)

		auto g3L{ 0.0f };
		auto g3C{ 0.0f };
		auto g3R{ 0.0f };

		if (px < 0.0) // select loudspeaker pair: if Left
		{
			g3L = -2.0f * px;
			g3C = sqrt3 * px + py;

			// normalize
			auto gdenom = FLT_MIN + sqrt(g3L * g3L + g3C * g3C);
			g3L *= (1.0f / gdenom);
			g3C *= (halfsqrt2 / gdenom);
		}
		else // if Right
		{
			g3C = -sqrt3 * px + py;
			g3R = 2.0f * px;

			// normalize
			auto gdenom = FLT_MIN + sqrt(g3R * g3R + g3C * g3C);
			g3R *= (1.0f / gdenom);
			g3C *= (halfsqrt2 / gdenom);
		}

		// ambient decorrelation & speaker channel generation
		// LOW-COMPX...equation(24)(25)(26)(27)
		// compensate decorrelation differences
		L[sample] = hal * hfr * ambience + g3L * source;
		C[sample] = source * g3C;
		R[sample] = har * hfr * ambience + g3R * source;

		Ls[sample] = hal * hre * ambience;
		Rs[sample] = har * hre * ambience;

		// copy freqDomain real part output value for output buffer
		ChannelDataOut_1[2 * sample] = L[sample].real();
		ChannelDataOut_2[2 * sample] = R[sample].real();
		ChannelDataOut_3[2 * sample] = C[sample].real();
		ChannelDataOut_4[2 * sample] = Ls[sample].real();
		ChannelDataOut_5[2 * sample] = Rs[sample].real();

		// copy freqDomain imagine part output value for output buffer
		ChannelDataOut_1[2 * sample + 1] = L[sample].imag();
		ChannelDataOut_2[2 * sample + 1] = R[sample].imag();
		ChannelDataOut_3[2 * sample + 1] = C[sample].imag();
		ChannelDataOut_4[2 * sample + 1] = Ls[sample].imag();
		ChannelDataOut_5[2 * sample + 1] = Rs[sample].imag();
	}
	//============================================================

	fft->performRealOnlyInverseTransform(ChannelDataOut_1);
	fft->performRealOnlyInverseTransform(ChannelDataOut_2);
	fft->performRealOnlyInverseTransform(ChannelDataOut_3);
	fft->performRealOnlyInverseTransform(ChannelDataOut_4);
	fft->performRealOnlyInverseTransform(ChannelDataOut_5);
}

//==============================================================================
/// Setting all of the UpmixProcessor's 4 parameters, used in processBlock()
/// receive parameters from UI
void UpmixProcessor::setParameters(std::atomic<float>* _ambientGaindB, std::atomic<float>* _mainGaindB,
	std::atomic<float>* _decorrelation, std::atomic<float>* _frontness)
{
	float ambientGaindB = *_ambientGaindB;
	float mainGaindB = *_mainGaindB;

	ambientGain = Decibels::decibelsToGain(ambientGaindB);
	mainGain = Decibels::decibelsToGain(mainGaindB);

	decorrelation = *_decorrelation;
	frontness = *_frontness;
}