### Version 6

Version 6 is the most recent version, as of reMarkable software version 3.0, released at the end of 2022.

Prior to this version, the file format was a straightforward representation of the drawn data (see ^1). With version 6,
the file format has been
redesigned from the ground up with a large variety of new features.

Version 6 was introduced by the reMarkable team for a few reasons:

First, the original reMarkable as well as the reMarkable 2 had the exact same 10.3" e-ink display with a resolution of 1872 x 1404 at 226 PPI,
for these two tablets, a static file format was perfectly acceptable.

However, reMarkable were working on more devices with different screen sizes and aspect ratios. Additionally, you could also start extending and changing the canvas size, which means a static file format wouldn't do anymore.
The solution to this problem was to introduce a scene tree. See [the scene tree section below](#scene-tree) for more information on how this works.

Second, updating the entire file format to a new version whenever a new feature is introduced or bug is fixed can be cumbersome.
For this, reMarkable introduced versioned blocks, see the [blocks section below](#blocks) for more information.

Third, because of the introduction of the new devices, the same file could theoretically be edited by multiple devices at the same time, which could lead to inconsistencies in the data integrity problems if not handled carefully.
Because of this, CRDTs were introduced.
