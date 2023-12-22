// NeuQuant Neural-Net Quantization Algorithm
// ------------------------------------------
//
// Copyright (c) 1994 Anthony Dekker
//
// NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
// See "Kohonen neural networks for optimal colour quantization"
// in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
// for a discussion of the algorithm.
//
// Any party obtaining a copy of these files from the author, directly or
// indirectly, is granted, free of charge, a full and unrestricted irrevocable,
// world-wide, paid up, royalty-free, nonexclusive right and license to deal
// in this software and documentation files (the "Software"), including without
// limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons who receive
// copies from any such party to do so, with the only requirement being
// that this copyright notice remain intact.
///////////////////////////////////////////////////////////////////////
// History
// -------
// January 2001: Adaptation of the Neural-Net Quantization Algorithm
//               for the FreeImage 2 library
//               Author: Hervé Drolon (drolon@infonie.fr)
// March 2004:   Adaptation for the FreeImage 3 library (port to big endian processors)
//               Author: Hervé Drolon (drolon@infonie.fr)
// April 2004:   Algorithm rewritten as a C++ class. 
//               Fixed a bug in the algorithm with handling of 4-byte boundary alignment.
//               Author: Hervé Drolon (drolon@infonie.fr)
///////////////////////////////////////////////////////////////////////
// Four primes near 500 - assume no image has a length so large
// that it is divisible by all four primes
#include <array>
#include <vector>
#include "neuralnet_quantization.h"
// ==========================================================
#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503
// ----------------------------------------------------------------
NNQuantizer::NNQuantizer(int PaletteSize)
{
    img_data = nullptr;
	netsize = PaletteSize;
	maxnetpos = netsize - 1;
	initrad = netsize < 8 ? 1 : (netsize >> 3);
	initradius = (initrad * radiusbias);
	network = NULL;
	network = (pixel *)malloc(netsize * sizeof(pixel));
	bias = (int *)malloc(netsize * sizeof(int));
	freq = (int *)malloc(netsize * sizeof(int));
	radpower = (int *)malloc(initrad * sizeof(int));
	if( !network || !bias || !freq || !radpower ) {
		if(network) free(network);
		if(bias) free(bias);
		if(freq) free(freq);
		if(radpower) free(radpower);
		throw "Memory allocation failed";
	}
}
NNQuantizer::~NNQuantizer()
{
	if(network) free(network);
	if(bias) free(bias);
	if(freq) free(freq);
	if(radpower) free(radpower);
}
///////////////////////////////////////////////////////////////////////////
// Initialise network in range (0,0,0) to (255,255,255) and set parameters
// -----------------------------------------------------------------------
void NNQuantizer::initnet() {
	int i, *p;
	for (i = 0; i < netsize; i++) {
		p = network[i];
		p[0] = p[1] = p[2] = (i << (netbiasshift+8))/netsize;
		freq[i] = intbias/netsize;	/* 1/netsize */
		bias[i] = 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////////	
// Unbias network to give byte values 0..255 and record position i to prepare for sort
// ------------------------------------------------------------------------------------
void NNQuantizer::unbiasnet() {
	int i, j, temp;
	for (i = 0; i < netsize; i++) {
		for (j = 0; j < 3; j++) {
			// OLD CODE: network[i][j] >>= netbiasshift; 
			// Fix based on bug report by Juergen Weigert jw@suse.de
			temp = (network[i][j] + (1 << (netbiasshift - 1))) >> netbiasshift;
			if (temp > 255) temp = 255;
			network[i][j] = temp;
		}
		network[i][3] = i;			// record colour no 
	}
}
//////////////////////////////////////////////////////////////////////////////////
// Insertion sort of network and building of netindex[0..255] (to do after unbias)
// -------------------------------------------------------------------------------
void NNQuantizer::inxbuild() {
	int i,j,smallpos,smallval;
	int *p,*q;
	int previouscol,startpos;
	previouscol = 0;
	startpos = 0;
	for (i = 0; i < netsize; i++) {
		p = network[i];
		smallpos = i;
		smallval = p[1];			// index on g
		// find smallest in i..netsize-1
		for (j = i+1; j < netsize; j++) {
			q = network[j];
			if (q[1] < smallval) {	// index on g
				smallpos = j;
				smallval = q[1];	// index on g
			}
		}
		q = network[smallpos];
		// swap p (i) and q (smallpos) entries
        if (i != smallpos) {
            j = q[0];  q[0] = p[0];  p[0] = j;
            j = q[1];  q[1] = p[1];  p[1] = j;
            j = q[2];  q[2] = p[2];  p[2] = j;
            j = q[3];  q[3] = p[3];  p[3] = j;
        }
		// smallval entry is now in position i
		if (smallval != previouscol) {
			netindex[previouscol] = (startpos+i)>>1;
			for (j = previouscol+1; j < smallval; j++)
				netindex[j] = i;
			previouscol = smallval;
			startpos = i;
		}
	}
	netindex[previouscol] = (startpos+maxnetpos)>>1;
	for (j = previouscol+1; j < 256; j++)
		netindex[j] = maxnetpos; // really 256
}
///////////////////////////////////////////////////////////////////////////////
// Search for values 0..255 (after net is unbiased) and return colour index
// ----------------------------------------------------------------------------
int NNQuantizer::inxsearch(int v1, int v2, int v3) const {
	int i, j, dist, a, bestd;
	int *p;
	int best;
	bestd = 1000;		// biggest possible dist is 256*3
	best = -1;
	i = netindex[v2];	// index on g
	j = i-1;			// start at netindex[g] and work outwards
	while ((i < netsize) || (j >= 0)) {
		if (i < netsize) {
			p = network[i];
			dist = p[1] - v2;				// inx key
			if (dist >= bestd)
				i = netsize;	// stop iter
			else {
				i++;
				if (dist < 0)
					dist = -dist;
				a = p[0] - v1;
				if (a < 0)
					a = -a;
				dist += a;
				if (dist < bestd) {
					a = p[2] - v3;
					if (a<0)
						a = -a;
					dist += a;
					if (dist < bestd) {
						bestd = dist;
						best = p[3];
					}
				}
			}
		}
		if (j >= 0) {
			p = network[j];
			dist = v2 - p[1];			// inx key - reverse dif
			if (dist >= bestd)
				j = -1;	// stop iter
			else {
				j--;
				if (dist < 0)
					dist = -dist;
				a = p[0] - v1;
				if (a<0)
					a = -a;
				dist += a;
				if (dist < bestd) {
					a = p[2] - v3;
					if (a<0)
						a = -a;
					dist += a;
					if (dist < bestd) {
						bestd = dist;
						best = p[3];
					}
				}
			}
		}
	}
	return best;
}
///////////////////////////////
// Search for biased values
// ----------------------------
int NNQuantizer::contest(int v1, int v2, int v3) {
	// finds closest neuron (min dist) and updates freq
	// finds best neuron (min dist-bias) and returns position
	// for frequently chosen neurons, freq[i] is high and bias[i] is negative
	// bias[i] = gamma*((1/netsize)-freq[i])
	int i,dist,a,biasdist,betafreq;
	int bestpos,bestbiaspos,bestd,bestbiasd;
	int *p,*f, *n;
	bestd = ~(((int) 1)<<31);
	bestbiasd = bestd;
	bestpos = -1;
	bestbiaspos = bestpos;
	p = bias;
	f = freq;
	for (i = 0; i < netsize; i++) {
		n = network[i];
		dist = n[0] - v1;
		if (dist < 0)
			dist = -dist;
		a = n[1] - v2;
		if (a < 0)
			a = -a;
		dist += a;
		a = n[2] - v3;
		if (a < 0)
			a = -a;
		dist += a;
		if (dist < bestd) {
			bestd = dist;
			bestpos = i;
		}
		biasdist = dist - ((*p)>>(intbiasshift-netbiasshift));
		if (biasdist < bestbiasd) {
			bestbiasd = biasdist;
			bestbiaspos = i;
		}
		betafreq = (*f >> betashift);
		*f++ -= betafreq;
		*p++ += (betafreq << gammashift);
	}
	freq[bestpos] += beta_;
	bias[bestpos] -= betagamma;
	return bestbiaspos;
}
///////////////////////////////////////////////////////
// Move neuron i towards biased (b,g,r) by factor alpha
// ---------------------------------------------------- 
void NNQuantizer::altersingle(int alpha, int i, int v1, int v2, int v3) {
	int *n;
	n = network[i];				// alter hit neuron
    n[0] -= (alpha * (n[0] - v1)) / initalpha;
    n[1] -= (alpha * (n[1] - v2)) / initalpha;
    n[2] -= (alpha * (n[2] - v3)) / initalpha;
}
////////////////////////////////////////////////////////////////////////////////////
// Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
// ---------------------------------------------------------------------------------
void NNQuantizer::alterneigh(int rad, int i, int v1, int v2, int v3) {
	int j, k, lo, hi, a;
	int *p, *q;
	lo = i - rad;   if (lo < -1) lo = -1;
	hi = i + rad;   if (hi > netsize) hi = netsize;
	j = i+1;
	k = i-1;
	q = radpower;
	while ((j < hi) || (k > lo)) {
		a = (*(++q));
        if (j < hi) {
            p = network[j];
            p[0] -= (a * (p[0] - v1)) / alpharadbias;
            p[1] -= (a * (p[1] - v2)) / alpharadbias;
            p[2] -= (a * (p[2] - v3)) / alpharadbias;
            j++;
        }
        if (k > lo) {
            p = network[k];
            p[0] -= (a * (p[0] - v1)) / alpharadbias;
            p[1] -= (a * (p[1] - v2)) / alpharadbias;
            p[2] -= (a * (p[2] - v3)) / alpharadbias;
			k--;
		}
	}
}
/**
 Get a pixel sample at position pos. Handle 4-byte boundary alignment.
 @param pos pixel position in a WxHx3 pixel buffer
 @param b blue pixel component
 @param g green pixel component
 @param r red pixel component
*/
void NNQuantizer::getSample(long pos, int *v1, int *v2, int *v3) {
	// get equivalent pixel coordinates 
	// - assume it's a 24-bit image -
	*v1 = img_data[pos + 0] << netbiasshift;
	*v2 = img_data[pos + 1] << netbiasshift;
	*v3 = img_data[pos + 2] << netbiasshift;
}
/////////////////////
// Main Learning Loop
// ------------------
void NNQuantizer::learn(int sampling_factor) {
	int i, j, b, g, r;
	int radius, rad, alpha, step, delta, samplepixels;
	int alphadec; // biased by 10 bits
	long pos, lengthcount;
	// image size as viewed by the scan algorithm
	lengthcount = img_width * img_height * 3;
	// number of samples used for the learning phase
	samplepixels = lengthcount / (3 * sampling_factor);
	// decrease learning rate after delta pixel presentations
	delta = samplepixels / ncycles;
	if(delta == 0) {
		// avoid a 'divide by zero' error with very small images
		delta = 1;
	}
	// initialize learning parameters
	alphadec = 30 + ((sampling_factor - 1) / 3);
	alpha = initalpha;
	radius = initradius;
	
	rad = radius >> radiusbiasshift;
	if (rad <= 1) rad = 0;
	for (i = 0; i < rad; i++) 
		radpower[i] = alpha*(((rad*rad - i*i)*radbias)/(rad*rad));
	
	// initialize pseudo-random scan
	if ((lengthcount % prime1) != 0)
		step = 3*prime1;
	else {
		if ((lengthcount % prime2) != 0)
			step = 3*prime2;
		else {
			if ((lengthcount % prime3) != 0) 
				step = 3*prime3;
			else
				step = 3*prime4;
		}
	}
	
	i = 0;		// iteration
	pos = 0;	// pixel position
	while (i < samplepixels) {
		// get next learning sample
		getSample(pos, &b, &g, &r);
		// find winning neuron
		j = contest(b, g, r);
		// alter winner
		altersingle(alpha, j, b, g, r);
		// alter neighbours 
		if (rad) alterneigh(rad, j, b, g, r);
		// next sample
		pos += step;
		while (pos >= lengthcount) pos -= lengthcount;
	
		i++;
		if (i % delta == 0) {	
			// decrease learning rate and also the neighborhood
			alpha -= alpha / alphadec;
			radius -= radius / radiusdec;
			rad = radius >> radiusbiasshift;
			if (rad <= 1) rad = 0;
			for (j = 0; j < rad; j++) 
				radpower[j] = alpha * (((rad*rad - j*j) * radbias) / (rad*rad));
		}
	}
	
}
//////////////
// Quantizer
// -----------
bool NNQuantizer::Quantize(const RGBImageData& image, const std::vector<u8triple>& InitialPalette, int sampling) {
    // Check image size
    img_height = image.height;
    img_width = image.width;
    if (img_height == 0 || img_width == 0) {
        return false;
    }
    img_data = image.data.data();
    // For small images, adjust the sampling factor to avoid a 'divide by zero' error later 
    // (see delta in learn() routine)
    int adjust = (img_width * img_height) / ncycles;
    if (sampling >= adjust)
        sampling = 1;
    // Initialize the network and apply the learning algorithm
    size_t ReserveSize = InitialPalette.size();
    if (netsize > ReserveSize) {
        netsize -= ReserveSize;
        initnet();
        learn(sampling);
        unbiasnet();
        netsize += ReserveSize;
    }
    // Overwrite the last few palette entries with the reserved ones
    for (int i = 0; i < ReserveSize; i++) {
        network[netsize - ReserveSize + i][0] = InitialPalette[i].v[0];
        network[netsize - ReserveSize + i][1] = InitialPalette[i].v[1];
        network[netsize - ReserveSize + i][2] = InitialPalette[i].v[2];
        network[netsize - ReserveSize + i][3] = netsize - ReserveSize + i;
    }
    // Write the quantized palette
    new_palette.resize(netsize);
    for (int j = 0; j < netsize; j++) {
        new_palette[j].v[0] = network[j][0];
        new_palette[j].v[1] = network[j][1];
        new_palette[j].v[2] = network[j][2];
    }
    inxbuild();
    img_data = nullptr;
    return true;
}

std::vector<uint8_t> NNQuantizer::BuildImage(const RGBImageData & image) {
    std::vector<uint8_t> ret;
    ret.reserve(image.data.size() / 3);
    for (int i = 0; i < image.data.size(); i+=3) {
        ret.push_back(inxsearch(image.data[i], image.data[i + 1], image.data[i + 2]));
    }
    return ret;
}

float NNQuantizer::FittingRatio(const RGBImageData & image) const {
    int outlier = 0;
    for (int i = 0; i < image.data.size(); i += 3) {
        int idx = inxsearch(image.data[i], image.data[i + 1], image.data[i + 2]);
        int diff = 0;
        diff += std::abs(new_palette[idx].v[0] - image.data[i]);
        diff += std::abs(new_palette[idx].v[1] - image.data[i + 1]);
        diff += std::abs(new_palette[idx].v[2] - image.data[i + 2]);
        if (diff > 12) {
            outlier++;
        }
    }
    return (float)outlier / image.height / image.width;
}

