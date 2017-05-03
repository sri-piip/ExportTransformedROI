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
#include "ExportTransformedROI.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>

#include <Windows.h>
#include <ShObjIdl.h>

// Poco header needed for the macros below 
#include <Poco/ClassLibrary.h>

// Declare that this object has AlgorithmBase subclasses
//  and declare each of those sub-classes
POCO_BEGIN_MANIFEST(sedeen::algorithm::AlgorithmBase)
POCO_EXPORT_CLASS(sedeen::algorithm::ExportTransformedROI)
POCO_END_MANIFEST

using namespace std;
namespace sedeen {
using namespace image; 
namespace algorithm {

std::string getImageLocation()
{
  IFileSaveDialog* pFileSave;

  std::wstring widePath = L"";

  auto hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			  IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

  if (SUCCEEDED(hr)) {
	COMDLG_FILTERSPEC fileTypes[] =
	{
	  { L"All Files", L"*.*"}
	};

	pFileSave->SetFileTypes(1, fileTypes);
	auto hr = pFileSave->Show(NULL);

	if (SUCCEEDED(hr)) {
	  IShellItem *pItem;
	  hr = pFileSave->GetResult(&pItem);
	  
	  if (SUCCEEDED(hr)) {
		PWSTR pszFilePath;

		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

		if (SUCCEEDED(hr)) {
		  widePath = pszFilePath;
		  CoTaskMemFree(pszFilePath);
		}
		pItem->Release();
	  }
	}
	pFileSave->Release();
  }

  CoUninitialize();

  if (widePath.empty())
	return "";

  auto wideStrLen = widePath.length();
  auto len = WideCharToMultiByte(CP_ACP, 0, widePath.c_str(), wideStrLen, 0, 0, 0, 0);

  std::string result(len, '\0');
  WideCharToMultiByte(CP_ACP, 0, widePath.c_str(), wideStrLen, &result[0], len, 0, 0);

  return result;
}

ExportTransformedROI::ExportTransformedROI()
		:image_list_(),
		region_list_(){}


void ExportTransformedROI::run()
{
	// Has the list of images changed
	auto image_list_changed = image_list_.isChanged();
	// Has display area changed
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
		true);
  // Bind system parameter for current view
	display_area_ = createDisplayAreaParameter(*this);

  // Bind text results
	output_text_ = createTextResult(*this, "Text Result");
}



bool ExportTransformedROI::buildPipeline() 
{
	using namespace image::tile;
	bool pipeline_changed = false;

	sedeen::SizeF spacing;
	if (image::hasPixelSpacing(image()))
		spacing = image::getPixelSize(image());
	else
	{
		spacing.setWidth(1000);
		spacing.setHeight(1000);
	}

	auto imageinfo = image_list_.info(image_list_.indexOf(image()));
	auto imagetransform = imageinfo.transform;
	auto activeimagefile = imageinfo.location;
	auto npos = activeimagefile.find_last_of('//', activeimagefile.size());
	auto extpos = activeimagefile.find('.', activeimagefile.size());
	auto activeImageName = activeimagefile.substr(npos + 1, extpos-1);

	auto filepath = getImageLocation();

	if (!filepath.empty())
	{
		Session sTImage = Session(filepath);
		Session sOImage = Session(imageinfo.location);  // Original active image session

		sedeen::Size Tsize;
		sedeen::Size Osize;
		 
		auto imgopener = image::createImageOpener();
		auto destLoc = file::Location(filepath);
		auto dest_image_ = imgopener->open(destLoc);

		if (!dest_image_) {
		  throw std::runtime_error("Could not open image for writing at: " + destLoc.getFilename());
		}

		int dest_width = dest_image_->getMetaData()->get(image::IntegerTags::IMAGE_X_DIMENSION, 0);
		int dest_height = dest_image_->getMetaData()->get(image::IntegerTags::IMAGE_Y_DIMENSION, 0);

		double x_spacing = 1.0;
		double y_spacing = 1.0;

		if (dest_image_->getMetaData()->has(image::DoubleTags::PIXEL_SIZE_X)) {
			  x_spacing = dest_image_->getMetaData()->get(image::DoubleTags::PIXEL_SIZE_X, 0);
		}
		if (dest_image_->getMetaData()->has(image::DoubleTags::PIXEL_SIZE_Y)) {
			  y_spacing = dest_image_->getMetaData()->get(image::DoubleTags::PIXEL_SIZE_Y, 0);
		}

		SRTTransform pointTargetTransform(0, 0, 1, 1, 0, 0, 0);
		if (sTImage.loadFromFile() == TRUE)
			pointTargetTransform = sTImage.getTransform();

		auto dim = sedeen::Size(dest_width, dest_height);
		sTImage.setDimension(dim);
		Tsize = sTImage.getDimension();

		auto Tspacing = sedeen::SizeF(x_spacing, y_spacing);
		if (image::hasPixelSpacing(dest_image_))
			Tspacing = image::getPixelSize(dest_image_);
		else
		{
			Tspacing.setWidth(1000);
			Tspacing.setHeight(1000);
		}


		sTImage.setPixelSize(Tspacing);
//		auto tScale = sedeen::SizeF(x_spacing / spacing.width(), y_spacing / spacing.height());

		if (sOImage.loadFromFile() == TRUE)
		{
			Osize = sOImage.getDimension();
			auto xdiff = (Osize.width() - Tsize.width())/2;
			auto ydiff = (Osize.height() - Tsize.height())/2;
			auto sOgraphics = sOImage.getGraphics();

			if (region_list_.isUserDefined())
			{
				std::vector<std::shared_ptr<GraphicItemBase>> graphicsList = region_list_;
				std::vector< GraphicDescription > graphicsDescript((int)graphicsList.size());
				std::vector<std::shared_ptr<GraphicItemBase>> sTgraphicsList(graphicsList);
		
				for (int i = 0; i < (int)graphicsList.size(); i++)
				{
					for (int j = 0; j < (int)sOgraphics.size(); j++)
					{
						if (strcmp(graphicsList[i]->name().c_str(), sOgraphics[j].getName()) == 0)
						{
							graphicsDescript[i].setGeometry(sOgraphics[j].getGeometry());
							graphicsDescript[i].setDescription(sOgraphics[j].getDescription());
							auto actImg = std::string(sOgraphics[j].getName()) + std::string("_") + activeImageName.c_str();
											
							graphicsDescript[i].setName( actImg.c_str());
							graphicsDescript[i].setStyle(sOgraphics[j].getStyle());
							auto gpoints = sOgraphics[j].getPoints();
							SRTTransform pointTransform = imagetransform;
							pointTransform.setCenter(pointTransform.center().getX() / spacing.width(), pointTransform.center().getY() / spacing.height());
							pointTransform.setTranslation((pointTransform.translation().getX() / spacing.width())-xdiff, (pointTransform.translation().getY() / spacing.height())-ydiff);
							pointTargetTransform = sTImage.getTransform();
							pointTargetTransform.setCenter(pointTargetTransform.center().getX() / sTImage.getPixelSize().width(), pointTargetTransform.center().getY() / sTImage.getPixelSize().height());
							pointTargetTransform.setTranslation((pointTargetTransform.translation().getX() / sTImage.getPixelSize().width()), (pointTargetTransform.translation().getY() / sTImage.getPixelSize().height()));
							//pointTargetTransform.setScale(tScale);
							for (int k = 0; k < gpoints.size(); k++)
							{
								transform(pointTransform, gpoints[k], TransformDirection::Forward);
								transform(pointTargetTransform, gpoints[k], TransformDirection::Inverse);
								graphicsDescript[i].push_backPoints(gpoints[k]);
							}
						}
					}
				}

				if (sTImage.loadFromFile() == TRUE)
				{
					auto sTgraphics = sTImage.getGraphics();
					for (int i = 0; i < (int)graphicsList.size(); i++)
					{
						sTgraphics.push_back(graphicsDescript[i]);
					}
					sTImage.setGraphics(sTgraphics);
				}
				else
				{
					sTImage.setGraphics(graphicsDescript);
				}
		
			} //if (region_list_.isUserDefined())
			
		}//if (sOImage.loadFromFile() == TRUE)
		sTImage.saveToFile();
	
	}//if (fileDlg.DoModal() == IDOK)
	
	pipeline_changed = true;
	return pipeline_changed;
}

} // namespace algorithm
} // namespace sedeen
