// This file provides fast analysis functions written in c++. This file is needed to circumvent some python limitations where
// no sufficient pythonic solution is available.
#pragma once

#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <exception>
#include <algorithm>
#include <sstream>

#include "Basis.h"
#include "defines.h"

bool _debug = false;
bool _info = true;

// counts from the event number column of the cluster table how often a cluster occurs in every event
unsigned int getNclusterInEvents(int64_t*& rEventNumber, const unsigned int& rSize, int64_t*& rResultEventNumber, unsigned int*& rResultCount)
{
	unsigned int tResultIndex = 0;
	unsigned int tLastIndex = 0;
	int64_t tLastValue = 0;
	for (unsigned int i = 0; i < rSize; ++i) {  // loop over all events can count the consecutive equal event numbers
		if (i == 0)
			tLastValue = rEventNumber[i];
		else if (tLastValue != rEventNumber[i]) {
			rResultCount[tResultIndex] = i - tLastIndex;
			rResultEventNumber[tResultIndex] = tLastValue;
			tLastValue = rEventNumber[i];
			tLastIndex = i;
			tResultIndex++;
		}
	}
	// add last event
	rResultCount[tResultIndex] = rSize - tLastIndex;
	rResultEventNumber[tResultIndex] = tLastValue;
	return tResultIndex + 1;
}

//takes two event arrays and calculates an intersection array of event numbers occurring in both arrays
unsigned int getEventsInBothArrays(int64_t*& rEventArrayOne, const unsigned int& rSizeArrayOne, int64_t*& rEventArrayTwo, const unsigned int& rSizeArrayTwo, int64_t*& rEventArrayIntersection)
{
	int64_t tActualEventNumber = -1;
	unsigned int tActualIndex = 0;
	unsigned int tActualResultIndex = 0;
	for (unsigned int i = 0; i < rSizeArrayOne; ++i) {  // loop over all event numbers in first array
		if (rEventArrayOne[i] == tActualEventNumber)  // omit the same event number occuring again
			continue;
		tActualEventNumber = rEventArrayOne[i];
		for (unsigned int j = tActualIndex; j < rSizeArrayTwo; ++j) {
			if (rEventArrayTwo[j] >= tActualEventNumber) {
				tActualIndex = j;
				break;
			}
		}
		if (rEventArrayTwo[tActualIndex] == tActualEventNumber) {
			rEventArrayIntersection[tActualResultIndex] = tActualEventNumber;
			tActualResultIndex++;
		}
	}
	return tActualResultIndex++;
}

//takes two event number arrays and returns a event number array with the maximum occurrence of each event number in array one and two
unsigned int getMaxEventsInBothArrays(int64_t*& rEventArrayOne, const unsigned int& rSizeArrayOne, int64_t*& rEventArrayTwo, const unsigned int& rSizeArrayTwo, int64_t*& result, const unsigned int& rSizeArrayResult)
{
	int64_t tFirstActualEventNumber = rEventArrayOne[0];
	int64_t tSecondActualEventNumber = rEventArrayTwo[0];
	int64_t tFirstLastEventNumber = rEventArrayOne[rSizeArrayOne - 1];
	int64_t tSecondLastEventNumber = rEventArrayTwo[rSizeArrayTwo - 1];
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int tActualResultIndex = 0;
	unsigned int tFirstActualOccurrence = 0;
	unsigned int tSecondActualOccurrence = 0;

	bool first_finished = false;
	bool second_finished = false;

//	std::cout<<"tFirstActualEventNumber "<<tFirstActualEventNumber<<std::endl;
//	std::cout<<"tSecondActualEventNumber "<<tSecondActualEventNumber<<std::endl;
//	std::cout<<"tFirstLastEventNumber "<<tFirstLastEventNumber<<std::endl;
//	std::cout<<"tSecondLastEventNumber "<<tSecondLastEventNumber<<std::endl;
//	std::cout<<"rSizeArrayOne "<<rSizeArrayOne<<std::endl;
//	std::cout<<"rSizeArrayTwo "<<rSizeArrayTwo<<std::endl;
//	std::cout<<"rSizeArrayResult "<<rSizeArrayResult<<std::endl;

	while (!(first_finished && second_finished)) {
		if ((tFirstActualEventNumber <= tSecondActualEventNumber) || second_finished) {
			unsigned int ii;
			for (ii = i; ii < rSizeArrayOne; ++ii) {
				if (rEventArrayOne[ii] == tFirstActualEventNumber)
					tFirstActualOccurrence++;
				else
					break;
			}
			i = ii;
		}

		if ((tSecondActualEventNumber <= tFirstActualEventNumber) || first_finished) {
			unsigned int jj;
			for (jj = j; jj < rSizeArrayTwo; ++jj) {
				if (rEventArrayTwo[jj] == tSecondActualEventNumber)
					tSecondActualOccurrence++;
				else
					break;
			}
			j = jj;
		}

//		std::cout<<"tFirstActualEventNumber "<<tFirstActualEventNumber<<" "<<tFirstActualOccurrence<<" "<<first_finished<<std::endl;
//		std::cout<<"tSecondActualEventNumber "<<tSecondActualEventNumber<<" "<<tSecondActualOccurrence<<" "<<second_finished<<std::endl;

		if (tFirstActualEventNumber == tSecondActualEventNumber) {
//			std::cout<<"==, add "<<std::max(tFirstActualOccurrence, tSecondActualOccurrence)<<" x "<<tFirstActualEventNumber<<std::endl;
			if (tFirstActualEventNumber == tFirstLastEventNumber)
				first_finished = true;
			if (tSecondActualEventNumber == tSecondLastEventNumber)
				second_finished = true;
			for (unsigned int k = 0; k < std::max(tFirstActualOccurrence, tSecondActualOccurrence); ++k) {
				if (tActualResultIndex < rSizeArrayResult)
					result[tActualResultIndex++] = tFirstActualEventNumber;
				else
					throw std::out_of_range("The result histogram is too small. Increase size.");
			}
		}
		else if ((!first_finished && tFirstActualEventNumber < tSecondActualEventNumber) || second_finished) {
//			std::cout<<"==, add "<<tFirstActualOccurrence<<" x "<<tFirstActualEventNumber<<std::endl;
			if (tFirstActualEventNumber == tFirstLastEventNumber)
				first_finished = true;
			for (unsigned int k = 0; k < tFirstActualOccurrence; ++k) {
				if (tActualResultIndex < rSizeArrayResult)
					result[tActualResultIndex++] = tFirstActualEventNumber;
				else
					throw std::out_of_range("The result histogram is too small. Increase size.");
			}
		}
		else if ((!second_finished && tSecondActualEventNumber < tFirstActualEventNumber) || first_finished) {
//			std::cout<<"==, add "<<tSecondActualOccurrence<<" x "<<tSecondActualEventNumber<<std::endl;
			if (tSecondActualEventNumber == tSecondLastEventNumber)
				second_finished = true;
			for (unsigned int k = 0; k < tSecondActualOccurrence; ++k) {
				if (tActualResultIndex < rSizeArrayResult)
					result[tActualResultIndex++] = tSecondActualEventNumber;
				else
					throw std::out_of_range("The result histogram is too small. Increase size.");
			}
		}

		if (i < rSizeArrayOne)
			tFirstActualEventNumber = rEventArrayOne[i];
		if (j < rSizeArrayTwo)
			tSecondActualEventNumber = rEventArrayTwo[j];
		tFirstActualOccurrence = 0;
		tSecondActualOccurrence = 0;
	}

	return tActualResultIndex;
}

//does the same as np.in1d but uses the fact that the arrays are sorted
void in1d_sorted(int64_t*& rEventArrayOne, const unsigned int& rSizeArrayOne, int64_t*& rEventArrayTwo, const unsigned int& rSizeArrayTwo, uint8_t*& rSelection)
{
	rSelection[0] = true;
	int64_t tActualEventNumber = -1;
	unsigned int tActualIndex = 0;
	for (unsigned int i = 0; i < rSizeArrayOne; ++i) {  // loop over all event numbers in first array
		tActualEventNumber = rEventArrayOne[i];
		for (unsigned int j = tActualIndex; j < rSizeArrayTwo; ++j) {
			if (rEventArrayTwo[j] >= tActualEventNumber) {
				tActualIndex = j;
				break;
			}
		}
		if (rEventArrayTwo[tActualIndex] == tActualEventNumber)
			rSelection[i] = 1;
		else
			rSelection[i] = 0;
	}
}

// fast 1d index histograming (bin size = 1, values starting from 0)
void histogram_1d(int*& x, const unsigned int& rSize, const unsigned int& rNbinsX, uint32_t*& rResult)
{
	for (unsigned int i = 0; i < rSize; ++i) {
		if (x[i] >= rNbinsX)
			throw std::out_of_range("The histogram indices are out of range");
		if (rResult[x[i]] < 4294967295)
			++rResult[x[i]];
		else
			throw std::out_of_range("The histogram has more than 4294967295 entries per bin. This is not supported.");
	}
}

// fast 2d index histograming (bin size = 1, values starting from 0)
void histogram_2d(int*& x, int*& y, const unsigned int& rSize, const unsigned int& rNbinsX, const unsigned int& rNbinsY, uint32_t*& rResult)
{
	for (unsigned int i = 0; i < rSize; ++i) {
		if (x[i] >= rNbinsX || y[i] >= rNbinsY)
			throw std::out_of_range("The histogram indices are out of range");
		if (rResult[x[i] * rNbinsY + y[i]] < 4294967295)
			++rResult[x[i] * rNbinsY + y[i]];
		else
			throw std::out_of_range("The histogram has more than 4294967295 entries per bin. This is not supported.");
	}
}

// fast 3d index histograming (bin size = 1, values starting from 0)
void histogram_3d(int*& x, int*& y, int*& z, const unsigned int& rSize, const unsigned int& rNbinsX, const unsigned int& rNbinsY, const unsigned int& rNbinsZ, uint16_t*& rResult)
{
	for (unsigned int i = 0; i < rSize; ++i) {
		if (x[i] >= rNbinsX || y[i] >= rNbinsY || z[i] >= rNbinsZ) {
			std::stringstream errorString;
			errorString << "The histogram indices (x/y/z)=(" << x[i] << "/" << y[i] << "/" << z[i] << ") are out of range.";
			throw std::out_of_range(errorString.str());
		}
		if (rResult[x[i] * rNbinsY * rNbinsZ + y[i] * rNbinsZ + z[i]] < 65535)
			++rResult[x[i] * rNbinsY * rNbinsZ + y[i] * rNbinsZ + z[i]];
		else
			throw std::out_of_range("The histogram has more than 65535 entries per bin. This is not supported.");
	}
}

// fast mapping of cluster hits to event numbers
void mapCluster(int64_t*& rEventArray, const unsigned int& rEventArraySize, ClusterInfo*& rClusterInfo, const unsigned int& rClusterInfoSize, ClusterInfo*& rMappedClusterInfo, const unsigned int& rMappedClusterInfoSize)
{
	unsigned int j = 0;
	for (unsigned int i = 0; i < rEventArraySize; ++i) {
		for (j; j < rClusterInfoSize; ++j) {
			if (rClusterInfo[j].eventNumber == rEventArray[i]) {
				if (i < rEventArraySize) {
					rMappedClusterInfo[i] = rClusterInfo[j];
					++i;
				}
				else
					return;
			}
			else
				break;
		}
	}
}

// loop over the refHit, Hit arrays and compare the hits of same event number. If they are similar (within an error) correlation is assumed. If more than nBadEvents are not correlated, broken correlation is assumed.
// True/False is returned for correlated/not correlated data. The iRefHit index is the index of the first not correlated hit.
bool _checkForNoCorrelation(unsigned int& iRefHit, unsigned int& iHit, const int64_t*& rEventArray, const double*& rRefCol, double*& rCol, const double*& rRefRow, double*& rRow, uint8_t*& rCorrelated, const unsigned int& nHits, const double& rError, const unsigned int& nBadEvents)
{
	int64_t tRefEventNumber = rEventArray[iRefHit];  // last read reference hit event number
	int64_t tEventNumberOffset = rEventArray[iHit] - rEventArray[iRefHit];  // event number offset between reference hit and hit
	unsigned int tBadEvents = 0;  // consecutive not correlated events
	unsigned int tHitIndex = iRefHit;  // actual first not correlated hit index
	bool tIsCorrelated = false;  // flag for the actual event
	unsigned int tNVirtual = 0;  // number of pure virtual events (only have virtual hits)
	unsigned int tNrefHits = 0; // number of reference hits (including virtual) of actual event
	unsigned int tNHits = 0; // number of hits (including virtual) of actual event

	for (iRefHit; iRefHit < nHits; ++iRefHit) {
		if (tRefEventNumber != rEventArray[iRefHit]) {  // check if event is finished
			for (iHit; iHit < nHits && (rEventArray[iHit] - tEventNumberOffset) < rEventArray[iRefHit]; ++iHit){  // increase hit index until new event is reached
				tNHits++;
			}
			if (!tIsCorrelated) {
				if (tBadEvents == 0) {
					if (iRefHit > 0)
						tHitIndex = iRefHit - 1;
					for (tHitIndex; tHitIndex > 0; --tHitIndex) {  // the actual first not correlated hit is the first hit from the event before
						if (rEventArray[tHitIndex] < tRefEventNumber) {
							tHitIndex++;
							break;
						}
					}
				}
				if (tNVirtual != tNHits && tNVirtual != tNrefHits)  // if there are only virtual hits one cannot judge the correlation, do not increase bad event counter
					tBadEvents++;
			}
			else
				tBadEvents = 0;

			if (_debug) std::cout << "\ttNrefHits " << tNrefHits << "\ttNHits " << tNHits << "\ttNVirtual " << tNVirtual <<   "\n";

			if (tBadEvents >= nBadEvents) {  // a correlation is defined as broken if more than nBadEvents consecutive not correlated events exist
				iRefHit = tHitIndex;  // set reference hit to first not correlated hit
				return true;
			}

			for (iHit; iHit + 1 < nHits; ++iHit) {  // increase the hit index until the correct event is reached
				if (rEventArray[iHit] - tEventNumberOffset >= rEventArray[iRefHit])
					break;
			}
			tRefEventNumber = rEventArray[iRefHit];
			tIsCorrelated = false;
			tNVirtual = 0;
			tNHits = 0;
			tNrefHits = 0;
		}
		tNrefHits++;
		if (rRefCol[iRefHit] != 0 && rCol[iHit] != 0 && rRefRow[iRefHit] != 0 && rRow[iHit] != 0 && std::fabs(rRefCol[iRefHit] - rCol[iHit]) < rError && std::fabs(rRefRow[iRefHit] - rRow[iHit]) < rError)  // check for correlation of real hits
			tIsCorrelated = true;
		if ((rRefCol[iRefHit] == 0 && rCol[iHit] == 0 && rRefRow[iRefHit] == 0 && rRow[iHit] == 0) || rCorrelated[iHit] == 0)  // if virtual hits occur in both devices correlation is likely
			tNVirtual++;
		if (_debug) std::cout << "\n" << iRefHit << "\t" << iHit << "\t" << rEventArray[iRefHit] << "\t" << rEventArray[iHit] << "\t" << rRefCol[iRefHit] << " / " << rCol[iHit] << "\t" << rRefRow[iRefHit] << " / " << rRow[iHit] << "\t" << tIsCorrelated << "\t" << (int) rCorrelated[iHit]<< "\t" << tNVirtual << "\t" << tBadEvents << "\n";
		if (iHit + 1 >= nHits)
			break;
		if (rEventArray[iRefHit] + tEventNumberOffset == rEventArray[iHit + 1]){  // increase hit index if the event is still the same
			iHit++;
			tNHits++;
		}
	}

	return false;
}

// loop over the refHit, Hit arrays and compare the hits of same event number. If they are similar (within an error) correlation is assumed. If more than nBadEvents are not correlated, broken correlation is assumed.
// True/False is returned for correlated/not correlated data. The iRefHit index is the index of the first not correlated hit.
bool _checkForCorrelation(unsigned int& iRefHit, unsigned int& iHit, const int64_t*& rEventArray, const double*& rRefCol, double*& rCol, const double*& rRefRow, double*& rRow, uint8_t*& rCorrelated, const unsigned int& nHits, const double& rError, const unsigned int nGoodEvents, bool print = false)
{
	int64_t tRefEventNumber = rEventArray[iRefHit] - 1;  // last read reference hit event number
	unsigned int tNgoodEvents = 0; // consecutive correlated events
	int64_t tEventNumberOffset = rEventArray[iHit] - rEventArray[iRefHit];  // event number offset between reference hit and hit
	unsigned int tBadEvents = 0;  // consecutive not correlated events
	unsigned int tHitIndex = iRefHit;  // actual first not correlated hit index
	bool tIsCorrelated = false;  // flag for the actual event
	bool tIsVirtual = false;  // pure virtual events only have virtual hits or already no correlation flag set, correlation cannot be judged here

	for (iRefHit; iRefHit < nHits; ++iRefHit) {
		if (tRefEventNumber != rEventArray[iRefHit]) {  // check if event is finished
			if (!tIsCorrelated && !tIsVirtual) {
				if (tBadEvents == 0) {
					if (iRefHit > 0)
						tHitIndex = iRefHit - 1;
					for (tHitIndex; tHitIndex > 0; --tHitIndex) {  // the actual first not correlated hit is the first hit from the event before
						if (rEventArray[tHitIndex] < tRefEventNumber) {
							tHitIndex++;
							break;
						}
					}
				}
				tNgoodEvents = 0;
			}

			for (iHit; iHit + 1 < nHits; ++iHit) {  // increase the hit index until the correct event is reached
				if (rEventArray[iHit] - tEventNumberOffset >= rEventArray[iRefHit])
					break;
			}
			if (!tIsVirtual && tIsCorrelated)
				tNgoodEvents++;
			tRefEventNumber = rEventArray[iRefHit];
			tIsCorrelated = false;
			tIsVirtual = false;
			if (nGoodEvents != 0 && tNgoodEvents >= nGoodEvents)
				break;
		}
		if (rRefCol[iRefHit] != 0 && rCol[iHit] != 0 && rRefRow[iRefHit] != 0 && rRow[iHit] != 0 && std::fabs(rRefCol[iRefHit] - rCol[iHit]) < rError && std::fabs(rRefRow[iRefHit] - rRow[iHit]) < rError)  // check for correlation of real hits
			tIsCorrelated = true;
		if ((rRefCol[iRefHit] == 0 && rCol[iHit] == 0 && rRefRow[iRefHit] == 0 && rRow[iHit] == 0) || rCorrelated[iHit] == 0)  // if virtual hits occur in both devices correlation is likely
			tIsVirtual = true;
		if (_debug) std::cout << "\n" << iRefHit << "\t" << iHit << "\t" << rEventArray[iRefHit] << "\t" << rEventArray[iHit] << "\t" << rRefCol[iRefHit] << " / " << rCol[iHit] << "\t" << rRefRow[iRefHit] << " / " << rRow[iHit] << "\t" << tIsCorrelated << "\t" << tIsVirtual << "\t" << tBadEvents << "\n";
		if (iHit + 1 >= nHits)
			break;
		if (rEventArray[iRefHit] + tEventNumberOffset == rEventArray[iHit + 1])  // increase hit index if the event is still the same
			iHit++;
	}
	if (nGoodEvents != 0 && tNgoodEvents < nGoodEvents)  // if good events are counted return false if too less good events are there
		return false;
	return true;
}

bool _findCorrelation(unsigned int& iRefHit, unsigned int& iHit, const int64_t*& rEventArray, const double*& rRefCol, double*& rCol, const double*& rRefRow, double*& rRow, uint8_t*& rCorrelated, const unsigned int& nHits, const double& rError, const unsigned int& correltationSearchRange, const unsigned int& nGoodEvents, const unsigned int& goodEventsSearchRange)
{
	unsigned int tSearchDistance = correltationSearchRange; // search range (index) for correlation

	unsigned int tLastNotZeroHit = 0; // hit index of last non virtual hit, for loop speed up

	// Determine the search distance in the reference hit array
	unsigned int tStopRefHitIndex = nHits;
	if (iRefHit + correltationSearchRange < nHits)
		tStopRefHitIndex = iRefHit + correltationSearchRange;

	for (iRefHit; iRefHit < tStopRefHitIndex; ++iRefHit) {
		if (rRefCol[iRefHit] == 0 && rRefRow[iRefHit] == 0)  // hit has to be non virtual (column/row != 0)
			continue;

		if (_debug) std::cout << "Try to find hit for " << iRefHit << ": " << rEventArray[iRefHit] << " " << rRefCol[iRefHit] << " " << rRefRow[iRefHit] << "\n";

		// Determine the search distance for the correlated hit
		unsigned int tStartHitIndex = 0;
		unsigned int tStopHitIndex = nHits;
		if (int(iRefHit - tSearchDistance) > 0)
			tStartHitIndex = iRefHit - tSearchDistance;
		if (iRefHit + tSearchDistance < nHits)
			tStopHitIndex = iRefHit + tSearchDistance;
		if (_debug) std::cout << "Search between " << tStartHitIndex << " and " << tStopHitIndex << "\n";

//		std::cout<<"tLastNotZeroHit "<<tLastNotZeroHit<<" "<<tStartHitIndex<<"\n";

		// Loop over the hits within the search distance and try to find a fitting hit. All fitting hits are checked to have subsequent correlated hits. Otherwise it is only correlation by chance.
		for (iHit = tStartHitIndex; iHit < tStopHitIndex; ++iHit) {
			if (rCol[iHit] == 0 && rCol[iRefHit] == 0)  //skip virtual hits
				continue;
			else
				tLastNotZeroHit = iHit;
			// Search for correlated hit candidate
			if (std::fabs(rRefCol[iRefHit] - rCol[iHit]) < rError && std::fabs(rRefRow[iRefHit] - rRow[iHit]) < rError) {  // check for correlation
				if (_debug) std::cout << "Try correlated hit canditate " << iHit << ": " << rEventArray[iHit] << " " << rCol[iHit] << " " << rRow[iHit] << "... ";
				unsigned int tiHit = iHit;  // temporaly store actual candidate index to be able to return to it after correlation check
				unsigned int ttRefHit = iRefHit;  // temporaly store actual candidate index to be able to return to it after correlation check
				if (_checkForCorrelation(ttRefHit, tiHit, rEventArray, rRefCol, rCol, rRefRow, rRow, rCorrelated, tStopRefHitIndex, rError, nGoodEvents, true)) {  // correlated hit candidate is correct if 5 / 10 events are also correlated (including the candidate)
					if (_debug) std::cout << " SUCCESS! Is correlated hit!\n";
					return true;
				}
				else if (_debug) std::cout << "\n";
			}
		}
		if (_debug) std::cout << "No correlated hit for " << iRefHit << ": " << rEventArray[iRefHit] << " " << rRefCol[iRefHit] << " " << rRefRow[iRefHit] << "\n";
		rCorrelated[iRefHit] |= 2;  // second bit shows uncorrelated hit (no match found)
	}

	return false;
}

bool _fixAlignment(unsigned int iRefHit, unsigned int iHit, const int64_t*& rEventArray, const double*& rRefCol, double*& rCol, const double*& rRefRow, double*& rRow, uint8_t*& rCorrelated, const unsigned int& nHits)
{
	if (_debug) std::cout << "Fix alignment " << iRefHit << ": " << rEventArray[iRefHit] << "/" << rRefCol[iRefHit] << "/" << rRefRow[iRefHit] << " = " << iHit << ": " << rEventArray[iHit] << "/" << rCol[iHit] << "/" << rRow[iHit] << "\n";
	unsigned int tNrefhits = 0;  // number of reference hits of actual event
	unsigned int tNhits = 0;  // number of hits of actual event
	int64_t tRefEventNumber = 0;  // last read reference hit event number
	int64_t tEventNumberOffset = rEventArray[iHit] - rEventArray[iRefHit];  // event number offset between reference hit and hit

	// negative offsets need temporary array for shifting
	unsigned int tHitIndex = iHit; // store start reference hit index for copying later
	double* tColCopy = 0;
	double* tRowCopy = 0;
	uint8_t* tCorrelated = 0;
	if (tEventNumberOffset < 0) {
		tColCopy = new double[nHits];
		tRowCopy = new double[nHits];
		tCorrelated = new uint8_t[nHits];
		for (unsigned int i = 0; i < nHits; ++i) {
			tColCopy[i] = 0;  // initialize as virtual hits only
			tRowCopy[i] = 0;  // initialize as virtual hits only
			tCorrelated[i] = rCorrelated[i];  // copy original
		}
	}

	for (iRefHit; iRefHit < nHits; ++iRefHit) {
		if (tRefEventNumber != rEventArray[iRefHit]) {  // check if event is finished
			for (iHit; iHit < nHits; ++iHit) {  // increase the hit index until the correct event is reached
				if (rEventArray[iHit] - tEventNumberOffset >= rEventArray[iRefHit])  // if all event numbers exist == should be sufficient
					break;
				if (rCol[iHit] != 0 && rRow[iHit] != 0)  // only count real hits
					tNhits++;
			}

			if (tNhits > tNrefhits) {  // if more hits than ref hits exist in one event -> mark as unsure if correlated, because ref hit array size is fixed --> hits get lost while copying
				for (unsigned int tiRefHit = iRefHit - 1; tiRefHit > 0; --tiRefHit) {  // mark all hits of last, incomplete event as not correlated
					if (tRefEventNumber == rEventArray[tiRefHit])
						rCorrelated[tiRefHit] = 0;
					else
						break;
				}
			}

			tNrefhits = 1;
			tNhits = 0;
			tRefEventNumber = rEventArray[iRefHit];
		}
		else
			tNrefhits++;

		if (iHit >= nHits) {  // nothing to copy anymore
			if (tEventNumberOffset < 0)
				std::cout << "EEEERRRROOOOORRR" << std::endl;
			return true;
		}

		// copy hits to correct position
		if (iHit < nHits) {
			if (tEventNumberOffset > 0) {
				rCol[iRefHit] = rCol[iHit];
				rRow[iRefHit] = rRow[iHit];
				rCorrelated[iRefHit] = ((rCorrelated[iRefHit] & rCorrelated[iHit]) & 1);  // leave unsure correlation flag intact, no correlation flag is expected and reset
				rCol[iHit] = 0;
				rRow[iHit] = 0;
			}
			else if (tEventNumberOffset < 0) {
				tColCopy[iRefHit] = rCol[iHit];
				tRowCopy[iRefHit] = rRow[iHit];
				tCorrelated[iRefHit] = ((rCorrelated[iRefHit] & rCorrelated[iHit]) & 1);  // leave unsure correlation flag intact, no correlation flag is expected and reset
			}
		}
		if (iHit < nHits - 1 && rEventArray[iRefHit] + tEventNumberOffset == rEventArray[iHit + 1]) {  // increase hit index if the event is still the same
			iHit++;
			tNhits++;
		}
	}

	if (tEventNumberOffset < 0) {
		for (unsigned int i = tHitIndex; i < nHits; ++i) {  // copy results
			rCol[i] = tColCopy[i];
			rRow[i] = tRowCopy[i];
			rCorrelated[i] = tCorrelated[i];
		}
		delete[] tColCopy;
		delete[] tRowCopy;
	}

	return true;
}

// Fix the event alignment with hit position information, crazy...
unsigned int fixEventAlignment(const int64_t*& rEventArray, const double*& rRefCol, double*& rCol, const double*& rRefRow, double*& rRow, uint8_t*& rCorrelated, const unsigned int& nHits, const double& rError, const unsigned int& nBadEvents, const unsigned int& correltationSearchRange, const unsigned int& nGoodEvents, const unsigned int& goodEventsSearchRange)
{
	// traverse both hit arrays starting from 0
	unsigned int iRefHit = 0;
	unsigned int iHit = 0;

	unsigned int tNfixes = 0;  // number of fixes done

	// correlation array is used to signal: 0 = unsure about correlation due to multiple event hits to less event hits merge
	//								     1 = event is correlated, start assumption
	//									 2 = event hit has no corresponding hit
	// mapped to simple correlated / uncorrelated array for the final result

	for (iRefHit; iRefHit < nHits ; ++iRefHit){
		iHit = iRefHit;
		if (_checkForNoCorrelation(iRefHit, iHit, rEventArray, rRefCol, rCol, rRefRow, rRow, rCorrelated, nHits, rError, nBadEvents)){ // true if all hits are correlated, nothing to do, thus return
			if (_info) std::cout << "No correlation starting at index (event) " << iRefHit << " (" << rEventArray[iRefHit] << ")\n";
			if (!_findCorrelation(iRefHit, iHit, rEventArray, rRefCol, rCol, rRefRow, rRow, rCorrelated, nHits, rError, correltationSearchRange, nGoodEvents, goodEventsSearchRange)) {
				if (_info) std::cout << "Found no correlation up to reference hit " << iRefHit - 1 << "\n";
				for (unsigned int i = 0; i < nHits; ++i){
					if ((rCorrelated[i] & 2) == 2)
						rCorrelated[i] = 0;
				}
				return tNfixes;
			}
			else {
				if (iRefHit != iHit){
					if (_info) std::cout << "Start fixing correlation for " <<iRefHit<< ": "<< rRefCol[iRefHit] << "/" << rRefRow[iRefHit] << " = " << iHit<< ": "<< rCol[iHit] << "/" << rRow[iHit] << "\n";
					_fixAlignment(iRefHit, iHit, rEventArray, rRefCol, rCol, rRefRow, rRow, rCorrelated, nHits);
					tNfixes++;
					std::cout<<"FIXED IT\n";
					for (unsigned int i = 0; i < nHits; ++i){
						std::cout << i  << "\t" << rEventArray[i] <<"\t" << rRefCol[i] << " / " << rCol[i] << "\t" << rRefRow[i] << " / " << rRow[i] << "\t" << (int) rCorrelated[i] << "\n";
					}
				}
				else
					if (_info) std::cout << "Correlation is back at " <<iRefHit<< ": "<< rRefCol[iRefHit] << "/" << rRefRow[iRefHit] << " = " << iHit<< ": "<< rCol[iHit] << "/" << rRow[iHit] << "\n";
			}
		}
		else{  // everything is correlated, nothing to do
			for (unsigned int i = 0; i < nHits; ++i){
				if ((rCorrelated[i] & 2) == 2)
					rCorrelated[i] = 0;
			}
			return tNfixes;
		}

	}

	for (unsigned int i = 0; i < nHits; ++i){
		if ((rCorrelated[i] & 2) == 2)
			rCorrelated[i] = 0;
	}

	return tNfixes;
}

