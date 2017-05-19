
# ExportTransformedROI

This plugin is designed to copy regions of interest from one image to another as it is shown in the image below. It also takes into account any transformations used to align the source and the target images.

![alt text](https://github.com/sedeen-piip-plugins/ExportTransformedROI/blob/master/ExportTransformedROI_Image.png "Exported ROIs on the Aligned Images")

## Notes
Currently this plugin **works with images with the same resolution**. The ability of working with images with different resolutions will be incorporated in the next version of the plugin.

## User Manual
Please refer to the PDF file provided [here](https://github.com/sedeen-piip-plugins/ExportTransformedROI/blob/master/ExportTrandformedROI_UserManual.pdf)

## Issues
Before exporting the ROIs to the selected image, the user is mistakenly asked to confirm replacing the image. However, the plugin only exports and adds the ROIs to the selected image whitout replacing the image. Therefore, the user needs to choose "Yes" at this step.
This bug will be fixed in the next version of the plugin.

## Credits
ExportTransformedROI plugin has been developed by Martel lab at Sunnybrook Research Institute (SRI), University of Toronto.
[Funding provided by NIH.](https://itcr.nci.nih.gov/funded-project/pathology-image-informatics-platform-visualization-analysis-and-management)



