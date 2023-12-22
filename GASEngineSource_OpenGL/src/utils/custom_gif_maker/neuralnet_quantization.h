#pragma once
#include <vector>
#include "utils/image_data.h"
/**
  NEUQUANT Neural-Net quantization algorithm by Anthony Dekker
*/

// ----------------------------------------------------------------
// Constant definitions
// ----------------------------------------------------------------

/** number of colours used: 
	for 256 colours, fixed arrays need 8kb, plus space for the image
*/
//static const int netsize = 256;

/**@name network definitions */
//@{
//static const int maxnetpos = (netsize - 1);
/// bias for colour values
static const int netbiasshift = 4;
/// no. of learning cycles
static const int ncycles = 100;
//@}

/**@name defs for freq and bias */
//@{
/// bias for fractions
static const int intbiasshift = 16;
static const int intbias = (((int)1) << intbiasshift);
/// gamma = 1024
static const int gammashift = 10;
// static const int gamma = (((int)1) << gammashift);
/// beta = 1 / 1024
static const int betashift = 10;
static const int beta_ = (intbias >> betashift);
static const int betagamma = (intbias << (gammashift-betashift));
//@}

/**@name defs for decreasing radius factor */
//@{
/// for 256 cols, radius starts
//static const int initrad = (netsize >> 3);
/// at 32.0 biased by 6 bits
static const int radiusbiasshift = 6;
static const int radiusbias = (((int)1) << radiusbiasshift);
/// and decreases by a 
//static const int initradius	= (initrad * radiusbias);
// factor of 1/30 each cycle
static const int radiusdec = 30;
//@}

/**@name defs for decreasing alpha factor */
//@{
/// alpha starts at 1.0
static const int alphabiasshift = 10;
static const int initalpha = (((int)1) << alphabiasshift);
//@}

/**@name radbias and alpharadbias used for radpower calculation */
//@{
static const int radbiasshift = 8;
static const int radbias = (((int)1) << radbiasshift);
static const int alpharadbshift = (alphabiasshift+radbiasshift);
static const int alpharadbias = (((int)1) << alpharadbshift);	
//@}

class NNQuantizer
{
protected:
	/**@name image parameters */
	//@{
	/// image width
	int img_width;
	/// image height
	int img_height;
	//@}

	/**@name network parameters */
	//@{

	int netsize, maxnetpos, initrad, initradius;

	/// BGRc
	typedef int pixel[4];
	/// the network itself
	pixel *network;

	/// for network lookup - really 256
	int netindex[256];

	/// bias array for learning
	int *bias;
	/// freq array for learning
	int *freq;
	/// radpower for precomputation
	int *radpower;
    /// data_ptr
    const uint8_t* img_data;
    /// result data
    std::vector<u8triple> new_palette;
	//@}

protected:
	/// Initialise network in range (0,0,0) to (255,255,255) and set parameters
	void initnet();	

	/// Unbias network to give byte values 0..255 and record position i to prepare for sort
	void unbiasnet();

	/// Insertion sort of network and building of netindex[0..255] (to do after unbias)
	void inxbuild();

	/// Search for values 0..255 (after net is unbiased) and return colour index
	int inxsearch(int v1, int v2, int v3) const;

	/// Search for biased values
	int contest(int v1, int v2, int v3);
	
	/// Move neuron i towards biased val by factor alpha
	void altersingle(int alpha, int i, int v1, int v2, int v3);

	/// Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
	void alterneigh(int rad, int i, int v1, int v2, int v3);

	/** Main Learning Loop
	@param sampling_factor sampling factor in [1..30]
	*/
	void learn(int sampling_factor);

	/// Get a pixel sample at position pos. Handle 4-byte boundary alignment.
	void getSample(long pos, int *v1, int *v2, int *v3);


public:
	/// Constructor
	NNQuantizer(int PaletteSize);

	/// Destructor
	~NNQuantizer();

	/** Quantizer
	@param dib input 24-bit dib to be quantized
	@param sampling a sampling factor in range 1..30. 
	1 => slower (but better), 30 => faster. Default value is 1
	@return returns whether successful
	*/
    bool Quantize(const RGBImageData& image, const std::vector<u8triple>& InitialPalette = {}, int sampling = 1);
    std::vector<u8triple> GetPalette() const { return new_palette; }
    std::vector<uint8_t> BuildImage(const RGBImageData& image);
    int LookupPixel(int v1, int v2, int v3) { return inxsearch(v1, v2, v3); }

    // 0-1 值越大，colortable越不适合输入图像
    float FittingRatio(const RGBImageData& image) const;
};
