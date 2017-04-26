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


class ExportTransformedROI : public AlgorithmBase 
{
 public:
	ExportTransformedROI();

 private:
	virtual void run();
	virtual void init(const image::ImageHandle& image);
	bool buildPipeline();

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
