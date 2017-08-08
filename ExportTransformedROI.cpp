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

// Primary header
#include "./ExportTransformedROI.h"
#define _AFXDLL

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <afxdlgs.h>
#include "boost\range\algorithm.hpp"
#include "boost\filesystem.hpp"


// Poco header needed for the macros below 
#include <Poco/ClassLibrary.h>

// Declare that this object has AlgorithmBase subclasses
//  and declare each of those sub-classes
POCO_BEGIN_MANIFEST(sedeen::algorithm::AlgorithmBase)
POCO_EXPORT_CLASS(sedeen::algorithm::ExportTransformedROI)
POCO_END_MANIFEST

using namespace std;
using namespace boost;
namespace sedeen {
using namespace image; 
namespace algorithm {




ExportTransformedROI::ExportTransformedROI()
		:image_list_(),
		region_list_(){}


void ExportTransformedROI::run()
{
	// change in the list of opened images?
	auto image_list_changed = image_list_.isChanged();
	// change in the display area?
	auto display_changed = display_area_.isChanged();

	DisplayRegion display_region = display_area_;
	// Get the region currently on screen
	image::tile::Compositor compositor(image()->getFactory());
	input_image_ = compositor.getImage(display_region.source_region, display_region.output_size);

	// Build the exporting rois pipeline
	auto pipeline_changed = buildPipeline();
}


void ExportTransformedROI::init(const image::ImageHandle& image) 
{
	if (isNull(image)) return;
	image_list_ = createImageListParameter(*this);
	region_list_ = createRegionListParameter(*this,
		"Select ROI",
		"Select ROI to be saved",
		false);

	auto fDlgOptions = sedeen::file::FileDialogOptions();
	fDlgOptions.caption = "Select Destination";
	fDlgOptions.startDir = image->getMetaData()->get(image::StringTags::SOURCE_DESCRIPTION, 0);
	fDlgOptions.flags = sedeen::file::FileDialogFlags::DontConfirmOverwrite;
	target_location_ = createSaveFileDialogParameter(*this,
		"Destination",
		"Select target image",
		fDlgOptions,
		false);

  // Bind system parameter for current view
	display_area_ = createDisplayAreaParameter(*this);
	
  // Bind text results
	output_text_ = createTextResult(*this, "Text Result");
}


SizeF ExportTransformedROI::getPixelSpacing(const image::ImageHandle& imageh)
{
	SizeF spacing;
	if (image::hasPixelSpacing(imageh))
		spacing = image::getPixelSize(imageh);
	else
	{
		spacing.setWidth(1000);
		spacing.setHeight(1000);
	}
	return spacing;
}


ImageAttr ExportTransformedROI::GetImageInfo(const image::ImageHandle& image)
{
	ImageAttr imAttr;

	int dest_width = image->getMetaData()->get(image::IntegerTags::IMAGE_X_DIMENSION, 0);
	int dest_height = image->getMetaData()->get(image::IntegerTags::IMAGE_Y_DIMENSION, 0);
	imAttr.size = sedeen::Size(dest_width, dest_height);

	double x_spacing = 1000.0;
	double y_spacing = 1000.0;
	if (image->getMetaData()->has(image::DoubleTags::PIXEL_SIZE_X))
		x_spacing = image->getMetaData()->get(image::DoubleTags::PIXEL_SIZE_X, 0);
	if (image->getMetaData()->has(image::DoubleTags::PIXEL_SIZE_Y))
		y_spacing = image->getMetaData()->get(image::DoubleTags::PIXEL_SIZE_Y, 0);
	imAttr.spacing = sedeen::SizeF(x_spacing, y_spacing);

	double x_centre = image->getMetaData()->get(image::DoubleTags::IMAGE_CENTRE_X, 0);   //IMAGE_CENTRE_X is in mm
	double y_centre = image->getMetaData()->get(image::DoubleTags::IMAGE_CENTRE_Y, 0);	  //IMAGE_CENTRE_Y is in mm
	imAttr.centre = sedeen::PointF(x_centre, y_centre);

	return imAttr;
}


ImageAttr ExportTransformedROI::GetImageInfo(std::basic_string<char, std::char_traits<char>, std::allocator<char>> imagepath)
{
	ImageAttr imAttr;
	auto imgopener = image::createImageOpener();
	auto t_location = file::Location(imagepath);
	auto destImage = imgopener->open(t_location);
	if (!destImage)
		throw std::runtime_error("Could not open image for writing at: " + t_location.getFilename());

	imAttr = GetImageInfo(destImage);

	return imAttr;
}


bool ExportTransformedROI::buildPipeline() 
{
	bool pipeline_changed = false;

	auto s_spacing = getPixelSpacing(image());
	auto s_imageinfo = image_list_.info(image_list_.indexOf(image()));
	auto s_transform = s_imageinfo.transform;
	boost::filesystem::path s_imagefilename(s_imageinfo.location);


	sedeen::algorithm::parameter::SaveFileDialog::DataType t_imagefiledata = target_location_;
	auto t_imagefilename = t_imagefiledata.getFilename();
	if (!target_location_.isUserDefined() || t_imagefilename.empty())
		throw std::runtime_error("No target image has been selected.");

	std::vector<std::shared_ptr<GraphicItemBase>> selectedGraphicsList = region_list_;
	if (!region_list_.isUserDefined() || selectedGraphicsList.size() == 0)
		throw std::runtime_error("One or more annotations must be selected.");

	int exportConfirmed = MB_OK;
	std::string pszMessage = "Export Annotations to: " + t_imagefilename + "?";
	exportConfirmed = MessageBox(nullptr, LPCSTR(pszMessage.data()), LPCSTR("Confirm Export"), MB_OKCANCEL | MB_APPLMODAL | MB_ICONQUESTION);

	if (exportConfirmed == IDOK)
	{
		auto s_attr = GetImageInfo(image());
		auto t_attr = GetImageInfo(t_imagefilename); 
		auto spacingRatio = sedeen::SizeF(t_attr.spacing.width() / s_attr.spacing.width(),
			t_attr.spacing.height() / s_attr.spacing.height());

		Session s_session = Session(s_imageinfo.location);  // Source image session
		Session t_session = Session(t_imagefilename);		// Target image session
		if (s_session.loadFromFile() == FALSE)
			throw std::runtime_error("Source image session file cannot be opened.");

		SRTTransform identity_transform(0, 0, 1, 1, 0, 0, 0);  
		SRTTransform t_transform(identity_transform);
		if (t_session.loadFromFile() == TRUE)
			t_transform = t_session.getTransform();

		if ((t_transform.operator==(identity_transform) == FALSE) && (((int)round(spacingRatio.width()) != 1) || ((int)round(spacingRatio.height()) != 1)))
			throw std::runtime_error("No transformation for target image with different resolution is supported in this version.");

		t_session.setTransform(t_transform);
		t_session.setDimension(t_attr.size);
		t_session.setPixelSize(t_attr.spacing);

		auto xdiff = (s_attr.size.width() - t_attr.size.width()) / 2;
		auto ydiff = (s_attr.size.height() - t_attr.size.height()) / 2;
		auto sGraphics = s_session.getGraphics();

		std::vector< GraphicDescription > output_GraphicsDescript((int)selectedGraphicsList.size());
		std::vector<std::shared_ptr<GraphicItemBase>> sTgraphicsList(selectedGraphicsList);

		s_transform.setCenter(s_transform.center().getX() / s_spacing.width(), s_transform.center().getY() / s_spacing.height());
		s_transform.setTranslation((s_transform.translation().getX() / s_spacing.width()) - xdiff,
			(s_transform.translation().getY() / s_spacing.height()) - ydiff);

		t_transform.setCenter((t_transform.center().getX() / t_session.getPixelSize().width()),
			(t_transform.center().getY() / t_session.getPixelSize().height()));
		t_transform.setTranslation((t_transform.translation().getX() / t_session.getPixelSize().width()),
			(t_transform.translation().getY() / t_session.getPixelSize().height()));

		SRTTransform spacingTransform = t_transform;
		spacingTransform.setScale(1,1);
		spacingTransform.setTranslation(0, 0);
		spacingTransform.setRotation(0);
		if (((int)round(spacingRatio.width()) != 1) || ((int)round(spacingRatio.height()) != 1))
		{
			SRTTransform c_transform = t_transform;
			c_transform.setCenter(0, 0);
			sedeen::PointF cntr(t_attr.size.width() / 2, t_attr.size.height() / 2);
			transform(c_transform, cntr, TransformDirection::Inverse);
			spacingTransform.setCenter(cntr);
			spacingTransform.setScale(spacingRatio);
			t_transform.setCenter(cntr);
			spacingTransform.setTranslation(-t_session.getTransform().translation().getX() *spacingRatio.width(), -t_session.getTransform().translation().getY() *spacingRatio.height());
			t_transform.setTranslation(t_session.getTransform().translation().getX() / t_session.getPixelSize().width(), t_session.getTransform().translation().getY() / t_session.getPixelSize().height());
		}

		for (int i = 0; i < (int)selectedGraphicsList.size(); i++)
		{
			auto sgDesc = std::find_if(sGraphics.begin(), sGraphics.end(), [=](const sedeen::GraphicDescription & x)
			{
				if (strcmp(selectedGraphicsList[i]->name().c_str(), x.getName()) == 0)
					return true;
				else
					return false;
			});

			output_GraphicsDescript[i].setGeometry(sgDesc->getGeometry());
			output_GraphicsDescript[i].setDescription(sgDesc->getDescription());
			output_GraphicsDescript[i].setStyle(sgDesc->getStyle());
			auto graphicsNewName = boost::filesystem::path(sgDesc->getName() + std::string("_") + s_imagefilename.stem().string());
			output_GraphicsDescript[i].setName(graphicsNewName.string().c_str());

			auto gpoints = sgDesc->getPoints();
			for (int k = 0; k < gpoints.size(); k++)
			{
				transform(s_transform, gpoints[k], TransformDirection::Forward);
				if (((int)round(spacingRatio.width()) != 1) || ((int)round(spacingRatio.height()) != 1))
					transform(spacingTransform, gpoints[k], TransformDirection::Inverse);
				transform(t_transform, gpoints[k], TransformDirection::Inverse);
				output_GraphicsDescript[i].push_backPoints(gpoints[k]);
			}
		} //for (int i = 0; i < (int)selectedGraphicsList.size(); i++)

		if (t_session.loadFromFile() == TRUE)
		{
			auto sTgraphics = t_session.getGraphics();
			for (int i = 0; i < (int)selectedGraphicsList.size(); i++)
				sTgraphics.push_back(output_GraphicsDescript[i]);
			t_session.setGraphics(sTgraphics);
		}
		else
			t_session.setGraphics(output_GraphicsDescript);

		t_session.saveToFile();
		MessageBox(nullptr, LPCSTR("Export successfully completed."), LPCSTR("Notification"), MB_OK);

	}//if (exportConfirmed == IDOK)

	pipeline_changed = true;
	return pipeline_changed;
}

} // namespace algorithm
} // namespace sedeen


