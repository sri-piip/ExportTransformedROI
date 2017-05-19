<snippet>
  <content><![CDATA[
# ${1:ExportTransformedROI}

This plugin is designed to copy regions of interest from one image to another. It also takes into account any transformations used to align the source and the target images.

![alt text][]

## Notes
Currently this plugin **works with images with the same resolution**. This feature will be incorporated in the next version of the plugin.

## Bugs
Before exporting the ROIs to the selected image, the user is mistakenly asked to confirm replacing the image. However, the plugin only exports and adds the ROIs to the selected image whitout replacing the image. Therefore, the user needs to choose "Yes" at this step.
This bug will be fixed in the next version of the plugin.

## Credits
ExportTransformedROI plugin has been developed by Martel lab at Sunnybrook Research Institute (SRI), University of Toronto.
Funding provided by NIH.

## License

