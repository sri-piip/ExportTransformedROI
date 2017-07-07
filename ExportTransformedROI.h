/*********************************************************************************
**********************************************************************************

	ExportTransformedROI is a plugin for PathCore Sedeen Viewer.

	This plugin has been developed by Rushin Shojaii
	in Dr. Anne Martel lab at Sunybrook Research Institute.

	This is a part of pathology image informatics platform (PIIP)
	funded by NIH.

	Contact info: rushin.shojaii@sri.utoronto.ca

	Date of the release of the code: April 26, 2017

**********************************************************************************
**********************************************************************************/


#ifndef DPTK_SRC_PLUGINS_EXPORTTRANSFORMEDROI_H
#define DPTK_SRC_PLUGINS_EXPORTTRANSFORMEDROI_H

//#include <numeric>

// DPTK headers - a minimal set 
#include "algorithm/AlgorithmBase.h"
#include "algorithm/Parameters.h"
#include "algorithm/Results.h"
#include "algorithm/ImageListParameter.h"	
#include "algorithm/RegionListParameter.h"
#include "geometry/graphic/GraphicItemBase.h"
#include "archive/Session.h"

// DPTK headers
#include "Algorithm.h"
#include "Archive.h"
#include "Geometry.h"
#include "Global.h"
#include "Image.h"
#include <global/geometry/SRTTransform.h>
#include <global/geometry/PointF.h>


namespace sedeen {
namespace algorithm {

	struct ImageAttr
	{
		sedeen::SizeF	spacing;
		sedeen::Size size;
		sedeen::PointF centre;

		double x_centre;
		double y_centre;
	};

class ExportTransformedROI : public AlgorithmBase 
{
 public:
	ExportTransformedROI();

 private:
	virtual void run();
	virtual void init(const image::ImageHandle& image);
	bool buildPipeline();
	SizeF getPixelSpacing(const image::ImageHandle& image);
	ImageAttr GetImageInfo(const image::ImageHandle& image); 
	ImageAttr GetImageInfo(std::basic_string<char, std::char_traits<char>, std::allocator<char>> destImage);
	

 private:
	TextResult output_text_;
	image::RawImage input_image_;
	image::RawImage output_image_;
	ImageListParameter image_list_;
	RegionListParameter region_list_;
    algorithm::DisplayAreaParameter display_area_;
	
};


} // namespace algorithm
} // namespace sedeen

#endif
